/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IDocumentOutlineNodes.hxx,v $
 * $Revision: 1.3 $
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
