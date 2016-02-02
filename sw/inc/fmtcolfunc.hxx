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
#ifndef INCLUDED_SW_INC_FMTCOLFUNC_HXX
#define INCLUDED_SW_INC_FMTCOLFUNC_HXX

class SwFormat;
class SwNumRuleItem;

// namespace <TextFormatCollFunc> for functions and procedures working on
// paragraph styles (instances of <SwTextFormatColl>
namespace TextFormatCollFunc
{
    /** Checks, if assignment of paragraph style to list level of outline style
        has to be deleted, and deletes the assignment, if needed.

        #i71574#
        The assignment of a paragraph style to a list level of the outline style
        has to be deleted, if the numbering rule, which is set at the paragraph
        style isn't the outline style.

        @author OD
    */
    void CheckTextFormatCollForDeletionOfAssignmentToOutlineStyle(
                                    SwFormat* pFormat,
                                    const SwNumRuleItem* pNewNumRuleItem = nullptr );

    /** determines the list style, which directly set at the given paragraph style

        @author OD

        @param rTextFormatColl
        input parameter - paragraph style for which the list style should be retrieved

        @return pointer to <SwNumRule> instance, if the given paragraph style
        has directly set a list style, 0 otherwise
    */
    SwNumRule* GetNumRule( SwTextFormatColl& rTextFormatColl );

    /** adds the given paragraph style at the directly set list style

        Note: If the given paragraph style has no directly set list style, nothing happens

        @param rTextFormatColl
        input parameter - paragraph style which is added to its directly set list style

        @author OD
    */
    void AddToNumRule( SwTextFormatColl& rTextFormatColl );

    /** removes the given paragraph style from the directly set list style

        Note: If the given paragraph style has no directly set list style, nothing happens

        @param rTextFormatColl
        input parameter - paragraph style which is removed from its directly set list style

        @author OD
    */
    void RemoveFromNumRule( SwTextFormatColl& rTextFormatColl );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
