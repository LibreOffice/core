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
#include <boost/utility.hpp>

class SwDoc;

namespace sw
{

class DocumentOutlineNodesManager : public IDocumentOutlineNodes,
                                    public ::boost::noncopyable
{
public:

    DocumentOutlineNodesManager( SwDoc& i_rSwdoc );

    //typedef ::std::vector< const SwTextNode* > tSortedOutlineNodeList;

    sal_Int32 getOutlineNodesCount() const SAL_OVERRIDE;

    int getOutlineLevel( const sal_Int32 nIdx ) const SAL_OVERRIDE;
    OUString getOutlineText( const sal_Int32 nIdx,
                                  const bool bWithNumber = true,
                                  const bool bWithSpacesForLevel = false,
                                  const bool bWithFootnote = true ) const SAL_OVERRIDE;
    SwTextNode* getOutlineNode( const sal_Int32 nIdx ) const SAL_OVERRIDE;
    void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const SAL_OVERRIDE;

    virtual ~DocumentOutlineNodesManager();

private:

    SwDoc& m_rDoc;
};

}

#endif // INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTOUTLINENODESMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
