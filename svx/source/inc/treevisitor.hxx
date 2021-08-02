/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
