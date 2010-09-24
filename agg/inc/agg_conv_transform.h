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
// class conv_transform
//
//----------------------------------------------------------------------------
#ifndef AGG_CONV_TRANSFORM_INCLUDED
#define AGG_CONV_TRANSFORM_INCLUDED

#include "agg_basics.h"
#include "agg_trans_affine.h"
#include "agg_vertex_iterator.h"

namespace agg
{

    //----------------------------------------------------------conv_transform
    template<class VertexSource, class Transformer=trans_affine> class conv_transform
    {
    public:
        conv_transform(VertexSource& source, const Transformer& tr) :
            m_source(&source), m_trans(&tr) {}

        void set_source(VertexSource& source) { m_source = &source; }

        void rewind(unsigned id)
        {
            m_source->rewind(id);
        }

        unsigned vertex(double* x, double* y)
        {
            unsigned cmd = m_source->vertex(x, y);
            if(is_vertex(cmd))
            {
                m_trans->transform(x, y);
            }
            return cmd;
        }

        void transformer(const Transformer& tr)
        {
            m_trans = &tr;
        }

        typedef conv_transform<VertexSource, Transformer> source_type;
        typedef vertex_iterator<source_type> iterator;
        iterator begin(unsigned id) { return iterator(*this, id); }
        iterator end() { return iterator(path_cmd_stop); }

    private:
        conv_transform(const conv_transform<VertexSource>&);
        const conv_transform<VertexSource>&
            operator = (const conv_transform<VertexSource>&);

        VertexSource*      m_source;
        const Transformer* m_trans;
    };


}

#endif
