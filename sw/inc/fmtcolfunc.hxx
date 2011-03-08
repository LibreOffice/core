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
#ifndef _FMTCOLFUNC_HXX
#define _FMTCOLFUNC_HXX

class SwFmt;
class SwNumRuleItem;

// namespace <TxtFmtCollFunc> for functions and procedures working on
// paragraph styles (instances of <SwTxtFmtColl>
namespace TxtFmtCollFunc
{
    /** Checks, if assignment of paragraph style to list level of outline style
        has to be deleted, and deletes the assignment, if needed.

        OD 2006-11-22 #i71574#
        The assignment of a paragraph style to a list level of the outline style
        has to be deleted, if the numbering rule, which is set at the paragraph
        style isn't the outline style.

        @author OD
    */
    void CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle(
                                    SwFmt* pFmt,
                                    const SwNumRuleItem* pNewNumRuleItem = 0L );

    /** determines the list style, which directly set at the given paragraph style

        OD 2008-03-04 #refactorlists#

        @author OD

        @param rTxtFmtColl
        input parameter - paragraph style for which the list style should be retrieved

        @return pointer to <SwNumRule> instance, if the given paragraph style
        has directly set a list style, 0 otherwise
    */
    SwNumRule* GetNumRule( SwTxtFmtColl& rTxtFmtColl );

    /** adds the given paragraph style at the directly set list style

        OD 2008-03-04 #refactorlists#
        Note: If the given paragraph style has no directly set list style, nothing happens

        @param rTxtFmtColl
        input parameter - paragraph style which is added to its directly set list style

        @author OD
    */
    void AddToNumRule( SwTxtFmtColl& rTxtFmtColl );

    /** removes te given paragraph style from the directly set list style

        OD 2008-03-04 #refactorlists#
        Note: If the given paragraph style has no directly set list style, nothing happens

        @param rTxtFmtColl
        input parameter - paragraph style which is removed from its directly set list style

        @author OD
    */
    void RemoveFromNumRule( SwTxtFmtColl& rTxtFmtColl );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
