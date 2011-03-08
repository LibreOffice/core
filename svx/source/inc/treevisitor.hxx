/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#ifndef SVX_TREE_VISITOR_HXX
#define SVX_TREE_VISITOR_HXX

#include <stack>

template< class ELEMENT, class NODEINFO, class PROCESSOR >
class TreeVisitor
{
public:
    TreeVisitor( NODEINFO _nodeInfo )
        :m_visitedRoot( false )
        ,m_root()
        ,m_current()
        ,m_nodeInfo( _nodeInfo )
    {
    }

    void    process( const ELEMENT& _root, PROCESSOR& _processor )
    {
        m_root = _root;
        m_visitedRoot = false;

        while ( do_step() )
            _processor.process( m_current );
    }

private:
    bool    do_step();

private:
    bool            m_visitedRoot;
    ELEMENT         m_root;
    ELEMENT         m_current;
    const NODEINFO  m_nodeInfo;

    ::std::stack< size_t >  m_pathToCurrent;
    ::std::stack< ELEMENT > m_currentAncestors;
};

template< class ELEMENT, class NODEINFO, class PROCESSOR >
bool TreeVisitor< ELEMENT, NODEINFO, PROCESSOR >::do_step()
{
    if ( !m_visitedRoot )
    {
        m_current = m_root;
        m_visitedRoot = true;
        return true;
    }

    // can we step down from the current node?
    size_t childCount = m_nodeInfo.childCount( m_current );
    if ( childCount )
    {
        m_currentAncestors.push( m_current );
        m_current = m_nodeInfo.getChild( m_current, 0 );
        m_pathToCurrent.push( 0 );
        return true;
    }

    // is there a right sibling of the current node?
    while ( !m_pathToCurrent.empty() )
    {
        const ELEMENT& currentParent = m_currentAncestors.top();
        childCount = m_nodeInfo.childCount( currentParent );

        size_t currentChildPos = m_pathToCurrent.top();
        if ( ++currentChildPos < childCount )
        {
            // yes there is
            m_pathToCurrent.top() = currentChildPos;
            m_current = m_nodeInfo.getChild( currentParent, currentChildPos );
            return true;
        }

        // no there isn't => step up
        m_currentAncestors.pop();
        m_pathToCurrent.pop();
    }

    return false;
}

#endif // SVX_TREE_VISITOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
