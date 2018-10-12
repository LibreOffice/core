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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTOUTLINENODESMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTOUTLINENODESMANAGER_HXX

#include <IDocumentOutlineNodes.hxx>

class SwDoc;

namespace sw
{

class DocumentOutlineNodesManager : public IDocumentOutlineNodes
{
public:

    DocumentOutlineNodesManager( SwDoc& i_rSwdoc );

    //typedef std::vector< const SwTextNode* > tSortedOutlineNodeList;

    tSortedOutlineNodeList::size_type getOutlineNodesCount() const override;

    int getOutlineLevel( const tSortedOutlineNodeList::size_type nIdx ) const override;
    OUString getOutlineText( const tSortedOutlineNodeList::size_type nIdx,
                                  SwRootFrame const* pLayout,
                                  const bool bWithNumber = true,
                                  const bool bWithSpacesForLevel = false,
                                  const bool bWithFootnote = true ) const override;
    SwTextNode* getOutlineNode( const tSortedOutlineNodeList::size_type nIdx ) const override;
    bool isOutlineInLayout(tSortedOutlineNodeList::size_type nIdx,
            SwRootFrame const& rLayout) const override;
    void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const override;

    virtual ~DocumentOutlineNodesManager() override;

private:

    DocumentOutlineNodesManager(DocumentOutlineNodesManager const&) = delete;
    DocumentOutlineNodesManager& operator=(DocumentOutlineNodesManager const&) = delete;

    SwDoc& m_rDoc;
};

}

#endif // INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTOUTLINENODESMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
