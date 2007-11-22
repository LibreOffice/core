/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swselectionlist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:36:26 $
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
#ifndef _SWSELECTIONLIST_HXX
#define _SWSELECTIONLIST_HXX

#include <list>

class SwPaM;
class SwFrm;

/** This class is used as parameter for creation of a block cursor selection

    This class will be created by a block cursor. Its responsibility is
    to collect a group of selected text portions which are part of a common
    context.
    Definition of context:
    A page header is a context.
    A page footer is a context.
    A footnote is a context.
    Every fly frame builds a context together with its linked colleagues.
    The content of the page bodies builds a context.
*/

class SwSelectionList
{
    std::list< SwPaM* > aList;  // container for the selected text portions
    const SwFrm* pContext;      // the context of these text portions
public:
    /** Ctor to create an empty list for a given context

        @param pInitCxt
        The frame (normally a SwTxtFrm) where the block cursor selection starts,
        it will be used to get the allowed context for the text selections.
    */
    explicit SwSelectionList( const SwFrm* pInitCxt );

    /** Start of the container for the selected text portions
    */
    std::list<SwPaM*>::iterator getStart() { return aList.begin(); }

    /** End of the container for the selected text portions
    */
    std::list<SwPaM*>::iterator getEnd() { return aList.end(); }

    /** Adds a text portion to the selection list

        @param pPam
        represents a text portion to select
    */
    void insertPaM( SwPaM* pPam ) { aList.push_front( pPam ); }

    /** Reports if the list does not contain any text portion

        @return true, if list is empty
    */
    bool isEmpty() const { return aList.empty(); }

    /** Checks if the context of the list is equal to the context of the frame

        If the list does not have already a context, the context of the frame
        will define the list's context.
        If the list has already a context, it will be compared to the context of
        the given frame.

        @param pCheck
        The frame to check

        @return true, if the context of the frame is equal to the one of the list
    */
    bool checkContext( const SwFrm* pCheck );
};

#endif  //_SWSELECTIONLIST_HXX
