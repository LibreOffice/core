/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IDocumentListItems.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:28:02 $
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

#ifndef IDOCUMENTLISTITEMS_HXX_INCLUDED
#define IDOCUMENTLISTITEMS_HXX_INCLUDED

#include <vector>
#include <tools/string.hxx>
class SwNodeNum;

/** Provides numbered items of a document.
*/
class IDocumentListItems
{
public:
    typedef ::std::vector< const SwNodeNum* > tSortedNodeNumList;

    virtual void addListItem( const SwNodeNum& rNodeNum ) = 0;
    virtual void removeListItem( const SwNodeNum& rNodeNum ) = 0;

    virtual String getListItemText( const SwNodeNum& rNodeNum,
                                    const bool bWithNumber = true,
                                    const bool bWithSpacesForLevel = false ) const = 0;

    /** get vector of all list items
    */
    virtual void getListItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const = 0;

    /** get vector of all list items, which are numbered
    */
    virtual void getNumItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const = 0;

protected:
    virtual ~IDocumentListItems() {};
};

 #endif // IDOCUMENTLISTITEMS_HXX_INCLUDED
