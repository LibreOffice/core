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

#ifndef IDOCUMENTOUTLINENODES_HXX_INCLUDED
#define IDOCUMENTOUTLINENODES_HXX_INCLUDED

#include <sal/types.h>
#include <tools/string.hxx>
#include <vector>
class SwTxtNode;

/** Provides outline nodes of a document.
*/
class IDocumentOutlineNodes
{
public:
    typedef ::std::vector< const SwTxtNode* > tSortedOutlineNodeList;

    virtual sal_Int32 getOutlineNodesCount() const = 0;

    virtual int getOutlineLevel( const sal_Int32 nIdx ) const = 0;
    virtual String getOutlineText( const sal_Int32 nIdx,
                                   const bool bWithNumber = true,
                                   const bool bWithSpacesForLevel = false ) const = 0;
    virtual SwTxtNode* getOutlineNode( const sal_Int32 nIdx ) const = 0;

    virtual void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const = 0;

protected:
    virtual ~IDocumentOutlineNodes() {};
};

 #endif // IDOCUMENTOUTLINENODES_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
