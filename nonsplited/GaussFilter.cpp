#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "GaussFilter.h"

GaussFilter::GaussFilter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	// HLS_FLATTEN_ARRAY(val); (jieyun comment: since r_val, g_val, b_val are not array?)
#endif
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);
        
#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
  	o_result_r.clk_rst(i_clk, i_rst);
  	o_result_g.clk_rst(i_clk, i_rst);
  	o_result_b.clk_rst(i_clk, i_rst);
#endif
}

GaussFilter::~GaussFilter() {}

// gauss mask
const int mask[MASK_X][MASK_Y] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

void GaussFilter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result_r.reset();
		o_result_g.reset();
		o_result_b.reset();
#endif
		wait();
	}
	while (true) {
		{
			HLS_CONSTRAIN_LATENCY(0, 1, "lat00");
			r_val = 0;
			g_val = 0;
			b_val = 0;
			cnt = 0;
		}
		for (unsigned int v = 0; v < MASK_Y; ++v) {
			for (unsigned int u = 0; u < MASK_X; ++u) {
// std::cout << u << v << endl;
				sc_dt::sc_uint<24> rgb;
#ifndef NATIVE_SYSTEMC
				{
					HLS_DEFINE_PROTOCOL("input");
					rgb = i_rgb.get();
					wait();
				}
#else
				rgb = i_rgb.read();
#endif
				{
					HLS_CONSTRAIN_LATENCY(0, 2, "lat01");
        			if ((rgb(7, 0) != 0) && (rgb(15, 8) != 0) && (rgb(23, 16) != 0)) cnt += mask[u][v];
					r_val += rgb(7, 0) * mask[u][v];
					g_val += rgb(15, 8) * mask[u][v];
					b_val += rgb(23, 16) * mask[u][v];
				}
			}
		}
		{
			HLS_CONSTRAIN_LATENCY(0, 8, "lat02");
			r_val = r_val / cnt;
			g_val = g_val / cnt;
			b_val = b_val / cnt;
		}
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result_r.put(r_val);
			o_result_g.put(g_val);
			o_result_b.put(b_val);
			wait();
		}
#else
		o_result_r.write(r_val);
		o_result_g.write(g_val);
		o_result_b.write(b_val);
#endif
	}
}
