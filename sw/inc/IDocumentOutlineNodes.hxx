/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentOutlineNodes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:28:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef IDOCUMENTOUTLINENODES_HXX_INCLUDED
#define IDOCUMENTOUTLINENODES_HXX_INCLUDED

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
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
