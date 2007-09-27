/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtcolfunc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:02:56 $
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

}
#endif
