//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// classes curve3 and curve4
//
//----------------------------------------------------------------------------

#ifndef AGG_CURVES_INCLUDED
#define AGG_CURVES_INCLUDED

#include "agg_basics.h"
#include "agg_vertex_iterator.h"

namespace agg
{

    // See Implemantation agg_curves.cpp


    //------------------------------------------------------------------curve3
    class curve3
    {
    public:
        curve3() :
          m_num_steps(0), m_step(0), m_scale(1.0) { }

        curve3(double x1, double y1,
               double x2, double y2,
               double x3, double y3) :
          m_num_steps(0), m_step(0), m_scale(1.0)
        {
            init(x1, y1, x2, y2, x3, y3);
        }

        void reset() { m_num_steps = 0; m_step = -1; }
        void init(double x1, double y1,
                  double x2, double y2,
                  double x3, double y3);
        void approximation_scale(double s) { m_scale = s; }
        double approximation_scale() const { return m_scale;  }

        void     rewind(unsigned id);
        unsigned vertex(double* x, double* y);

        typedef curve3 source_type;
        typedef vertex_iterator<source_type> iterator;
        iterator begin(unsigned id) { return iterator(*this, id); }
        iterator end() { return iterator(path_cmd_stop); }

    private:
        int      m_num_steps;
        int      m_step;
        double   m_scale;
        double   m_start_x;
        double   m_start_y;
        double   m_end_x;
        double   m_end_y;
        double   m_fx;
        double   m_fy;
        double   m_dfx;
        double   m_dfy;
        double   m_ddfx;
        double   m_ddfy;
        double   m_saved_fx;
        double   m_saved_fy;
        double   m_saved_dfx;
        double   m_saved_dfy;
    };







    //-----------------------------------------------------------------curve4
    class curve4
    {
    public:
        curve4() :
          m_num_steps(0), m_step(0), m_scale(1.0) { }

        curve4(double x1, double y1,
               double x2, double y2,
               double x3, double y3,
               double x4, double y4) :
          m_num_steps(0), m_step(0), m_scale(1.0)
        {
            init(x1, y1, x2, y2, x3, y3, x4, y4);
        }

        void reset() { m_num_steps = 0; m_step = -1; }
        void init(double x1, double y1,
                  double x2, double y2,
                  double x3, double y3,
                  double x4, double y4);

        void approximation_scale(double s) { m_scale = s; }
        double approximation_scale() const { return m_scale;  }

        void     rewind(unsigned id);
        unsigned vertex(double* x, double* y);

        typedef curve4 source_type;
        typedef vertex_iterator<source_type> iterator;
        iterator begin(unsigned id) { return iterator(*this, id); }
        iterator end() { return iterator(path_cmd_stop); }

    private:
        int      m_num_steps;
        int      m_step;
        double   m_scale;
        double   m_start_x;
        double   m_start_y;
        double   m_end_x;
        double   m_end_y;
        double   m_fx;
        double   m_fy;
        double   m_dfx;
        double   m_dfy;
        double   m_ddfx;
        double   m_ddfy;
        double   m_dddfx;
        double   m_dddfy;
        double   m_saved_fx;
        double   m_saved_fy;
        double   m_saved_dfx;
        double   m_saved_dfy;
        double   m_saved_ddfx;
        double   m_saved_ddfy;
    };




}

#endif
