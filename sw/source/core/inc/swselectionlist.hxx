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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWSELECTIONLIST_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWSELECTIONLIST_HXX

#include <list>

class SwPaM;
class SwFrame;

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
    const SwFrame* pContext;      // the context of these text portions
public:
    /** Ctor to create an empty list for a given context

        @param pInitCxt
        The frame (normally a SwTextFrame) where the block cursor selection starts,
        it will be used to get the allowed context for the text selections.
    */
    explicit SwSelectionList( const SwFrame* pInitCxt );

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
    bool checkContext( const SwFrame* pCheck );
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_SWSELECTIONLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
