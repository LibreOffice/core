/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
