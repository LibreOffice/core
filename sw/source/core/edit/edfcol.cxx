/*************************************************************************
 *
 *  $RCSfile: edfcol.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>

#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>      // fuer SwTxtFmtColls
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>    // fuer MACROS
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>   // fuer SwXXXFmtColl
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif

/*************************************
 * FormatColl
 *************************************/
// TXT


SwTxtFmtColl& SwEditShell::GetDfltTxtFmtColl() const
{
    return *((SwTxtFmtColl*) (GetDoc()->GetDfltTxtFmtColl()));
}


USHORT SwEditShell::GetTxtFmtCollCount() const
{
    return GetDoc()->GetTxtFmtColls()->Count();
}


SwTxtFmtColl& SwEditShell::GetTxtFmtColl( USHORT nFmtColl) const
{
    return *((*(GetDoc()->GetTxtFmtColls()))[nFmtColl]);
}


void SwEditShell::SetTxtFmtColl(SwTxtFmtColl *pFmt)
{
    SwTxtFmtColl *pLocal = pFmt? pFmt: (*GetDoc()->GetTxtFmtColls())[0];
    StartAllAction();
    GetDoc()->StartUndo();
    FOREACHPAM_START(this)

        if( !PCURCRSR->HasReadonlySel() )
            GetDoc()->SetTxtFmtColl(*PCURCRSR, pLocal);

    FOREACHPAM_END()
    GetDoc()->EndUndo();
    EndAllAction();

}


SwTxtFmtColl* SwEditShell::MakeTxtFmtColl(const String& rFmtCollName,
        SwTxtFmtColl* pParent)
{
    SwTxtFmtColl *pColl;
    if ( pParent == 0 )
        pParent = &GetTxtFmtColl(0);
    if (  (pColl=GetDoc()->MakeTxtFmtColl(rFmtCollName, pParent)) == 0 )
        ASSERT( FALSE, "MakeTxtFmtColl failed" );
    return pColl;

}


void SwEditShell::FillByEx(SwTxtFmtColl* pColl, BOOL bReset)
{
    if( bReset )
        pColl->ResetAllAttr();

    const SfxItemSet* pSet = GetCrsr()->GetCntntNode()->GetpSwAttrSet();
    if( pSet )
    {
        // JP 05.10.98: falls eines der Attribute Break/PageDesc/NumRule(auto)
        //      im ItemSet steht, so entferne die VORM setzen an der Collection.
        //      Ansonst wird zu viel gemacht oder falsche gemacht (NumRules!)
        //      Bug 57568

        // AutoNumRules NICHT in die Vorlagen uebernehmen
        const SfxPoolItem* pItem;
        const SwNumRule* pRule = 0;
        if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE ) ||
            SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC,FALSE ) ||
            ( SFX_ITEM_SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                FALSE, &pItem ) && 0 != (pRule = GetDoc()->FindNumRulePtr(
                ((SwNumRuleItem*)pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() )
            )
        {
            SfxItemSet aSet( *pSet );
            aSet.ClearItem( RES_BREAK );
            aSet.ClearItem( RES_PAGEDESC );

            if( pRule || (SFX_ITEM_SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                FALSE, &pItem ) && 0 != (pRule = GetDoc()->FindNumRulePtr(
                ((SwNumRuleItem*)pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() ))
                aSet.ClearItem( RES_PARATR_NUMRULE );

            if( aSet.Count() )
                pColl->SetAttr( aSet );
        }
        else
            pColl->SetAttr( *pSet );
    }
}




