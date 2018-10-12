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

#ifndef INCLUDED_SW_INC_IDOCUMENTOUTLINENODES_HXX
#define INCLUDED_SW_INC_IDOCUMENTOUTLINENODES_HXX

#include <rtl/ustring.hxx>
#include <vector>

class SwTextNode;
class SwRootFrame;

/** Provides outline nodes of a document.
*/
class IDocumentOutlineNodes
{
public:
    typedef std::vector< const SwTextNode* > tSortedOutlineNodeList;

    virtual tSortedOutlineNodeList::size_type getOutlineNodesCount() const = 0;

    virtual int getOutlineLevel( const tSortedOutlineNodeList::size_type nIdx ) const = 0;
    virtual OUString getOutlineText( const tSortedOutlineNodeList::size_type nIdx,
                                   SwRootFrame const* pLayout,
                                   const bool bWithNumber = true,
                                   const bool bWithSpacesForLevel = false,
                                   const bool bWithFootnote = true ) const = 0;
    virtual SwTextNode* getOutlineNode( const tSortedOutlineNodeList::size_type nIdx ) const = 0;

    virtual bool isOutlineInLayout(tSortedOutlineNodeList::size_type nIdx,
            SwRootFrame const& rLayout) const = 0;

    virtual void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const = 0;

protected:
    virtual ~IDocumentOutlineNodes() {};
};

 #endif // INCLUDED_SW_INC_IDOCUMENTOUTLINENODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
