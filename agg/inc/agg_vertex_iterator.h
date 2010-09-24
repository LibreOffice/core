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
// classes: vertex_iterator
//          vertex_source_adaptor
//          vertex_source_adaptor_with_id
//
//----------------------------------------------------------------------------
#ifndef AGG_VERTEX_ITERATOR_INCLUDED
#define AGG_VERTEX_ITERATOR_INCLUDED

#include "agg_basics.h"


namespace agg
{

    //---------------------------------------------------------vertex_iterator
    template<class VertexSource> class vertex_iterator
    {
    public:
        vertex_iterator() {}
        vertex_iterator(unsigned cmd) { m_vertex.cmd = cmd; }
        vertex_iterator(const vertex_iterator& i) : m_vs(i.m_vs), m_vertex(i.m_vertex) {}
        vertex_iterator(VertexSource& vs, unsigned id) : m_vs(&vs)
        {
            m_vs->rewind(id);
            m_vertex.cmd = m_vs->vertex(&m_vertex.x, &m_vertex.y);
        }
        vertex_iterator& operator++()
        {
            m_vertex.cmd = m_vs->vertex(&m_vertex.x, &m_vertex.y);
            return *this;
        }

        const vertex_type& operator*() const { return m_vertex; }
        const vertex_type* operator->() const { return &m_vertex; }

        bool operator != (const vertex_iterator& i)
        {
            return m_vertex.cmd != i.m_vertex.cmd;
        }

    private:
        VertexSource* m_vs;
        vertex_type   m_vertex;
    };


    //---------------------------------------------------vertex_source_adaptor
    template<class VertexContainer> class vertex_source_adaptor
    {
    public:
        vertex_source_adaptor(const VertexContainer& container) :
            m_container(&container) {}

        void rewind(unsigned)
        {
            m_iterator = m_container->begin();
            m_end = m_container->end();
        }

        unsigned vertex(double* x, double* y)
        {
            unsigned cmd = path_cmd_stop;
            if(m_iterator != m_end)
            {
                *x  = m_iterator->x;
                *y  = m_iterator->y;
                cmd = m_iterator->cmd;
                ++m_iterator;
            }
            return cmd;
        }

    private:
        const VertexContainer* m_container;
        typename VertexContainer::const_iterator m_iterator;
        typename VertexContainer::const_iterator m_end;
    };



    //-------------------------------------------vertex_source_adaptor_with_id
    template<class VertexContainer> class vertex_source_adaptor_with_id
    {
    public:
        vertex_source_adaptor_with_id(const VertexContainer& container) :
            m_container(&container) {}

        void rewind(unsigned id)
        {
            m_iterator = m_container->begin(id);
            m_end = m_container->end();
        }

        unsigned vertex(double* x, double* y)
        {
            unsigned cmd = path_cmd_stop;
            if(m_iterator != m_end)
            {
                *x  = m_iterator->x;
                *y  = m_iterator->y;
                cmd = m_iterator->cmd;
                ++m_iterator;
            }
            return cmd;
        }

    private:
        const VertexContainer* m_container;
        typename VertexContainer::const_iterator m_iterator;
        typename VertexContainer::const_iterator m_end;
    };



}


#endif
