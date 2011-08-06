/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Kohei Yoshida <kyoshida@novell.com> (Novell, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef __SC_SUBTOTALPARAM_HXX__
#define __SC_SUBTOTALPARAM_HXX__

#include "global.hxx"
#include "address.hxx"

struct SC_DLLPUBLIC ScSubTotalParam
{
    SCCOL           nCol1;          // selected area
    SCROW           nRow1;
    SCCOL           nCol2;
    SCROW           nRow2;
    sal_uInt16      nUserIndex;                 // index into list
    bool            bRemoveOnly:1;
    bool            bReplace:1;                 // replace existing results
    bool            bPagebreak:1;               // page break at change of group
    bool            bCaseSens:1;                //
    bool            bDoSort:1;                  // presort
    bool            bAscending:1;               // sort ascending
    bool            bUserDef:1;                 // sort user defined
    bool            bIncludePattern:1;          // sort formats
    bool            bGroupActive[MAXSUBTOTAL];  // active groups
    SCCOL           nField[MAXSUBTOTAL];        // associated field
    SCCOL           nSubTotals[MAXSUBTOTAL];    // number of SubTotals
    SCCOL*          pSubTotals[MAXSUBTOTAL];    // array of columns to be calculated
    ScSubTotalFunc* pFunctions[MAXSUBTOTAL];    // array of associated functions

    ScSubTotalParam();
    ScSubTotalParam( const ScSubTotalParam& r );

    ScSubTotalParam& operator= ( const ScSubTotalParam& r );
    bool operator== ( const ScSubTotalParam& r ) const;
    void Clear();
    void SetSubTotals( sal_uInt16 nGroup,
                       const SCCOL* ptrSubTotals,
                       const ScSubTotalFunc* ptrFuncions,
                       sal_uInt16 nCount );
};

#endif
