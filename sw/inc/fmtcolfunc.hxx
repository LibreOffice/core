/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
