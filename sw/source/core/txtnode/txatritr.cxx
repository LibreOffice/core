/*************************************************************************
 *
 *  $RCSfile: txatritr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-21 12:50:03 $
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


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#ifndef _TXATRITR_HXX
#include <txatritr.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif

using namespace ::com::sun::star::i18n;


SwScriptIterator::SwScriptIterator( const String& rStr, xub_StrLen nStt )
    : rText( rStr ), nChgPos( rStr.Len() ), nCurScript( ScriptType::WEAK )
{
    if( pBreakIt->xBreak.is() )
    {
        xub_StrLen nPos = nStt;
        nCurScript = pBreakIt->xBreak->getScriptType( rText, nPos );
        if( ScriptType::WEAK == nCurScript )
        {
            if( nPos )
            {
                nPos = (xub_StrLen)pBreakIt->xBreak->beginOfScript(
                                                rText, nPos, nCurScript );
                if( nPos && nPos < rText.Len() )
                {
                    nStt = --nPos;
                    nCurScript = pBreakIt->xBreak->getScriptType( rText,nPos);
                }
            }
        }

        nChgPos = (xub_StrLen)pBreakIt->xBreak->endOfScript(
                                                rText, nStt, nCurScript );
    }
}

sal_Bool SwScriptIterator::Next()
{
    sal_Bool bRet = sal_False;
    if( pBreakIt->xBreak.is() && nChgPos < rText.Len() )
    {
        nCurScript = pBreakIt->xBreak->getScriptType( rText, nChgPos );
        nChgPos = (xub_StrLen)pBreakIt->xBreak->endOfScript(
                                            rText, nChgPos, nCurScript );
        bRet = sal_True;
    }
    else
        nChgPos = rText.Len();
    return bRet;
}

// --------------------------------------------------------------------

SwTxtAttrIterator::SwTxtAttrIterator( const SwTxtNode& rTNd, USHORT nWhchId,
                                        xub_StrLen nStt )
    : aSIter( rTNd.GetTxt(), nStt ), rTxtNd( rTNd ),
    nChgPos( nStt ), nAttrPos( 0 ), pParaItem( 0 ), nWhichId( nWhchId )
{
    SearchNextChg();
}

sal_Bool SwTxtAttrIterator::Next()
{
    sal_Bool bRet = sal_False;
    if( nChgPos < aSIter.GetText().Len() )
    {
        bRet = sal_True;
        if( aStack.Count() )
        {
            do {
                const SwTxtAttr* pHt = (SwTxtAttr*)aStack[ 0 ];
                USHORT nEndPos = *pHt->GetEnd();
                if( nChgPos >= nEndPos )
                    aStack.Remove( 0 );
                else
                    break;
            } while( aStack.Count() );
        }

        if( aStack.Count() )
        {
            sal_uInt16 nSavePos = nAttrPos;
            SearchNextChg();
            if( aStack.Count() )
            {
                const SwTxtAttr* pHt = (SwTxtAttr*)aStack[ 0 ];
                USHORT nEndPos = *pHt->GetEnd();
                if( nChgPos >= nEndPos )
                {
                    nChgPos = nEndPos;
                    nAttrPos = nSavePos;

                    if( RES_TXTATR_CHARFMT == pHt->Which() )
                        pCurItem = &pHt->GetCharFmt().GetCharFmt()->GetAttr(
                                        GetWhichOfScript( nWhichId,
                                                aSIter.GetCurrScript() ) );
                    else
                        pCurItem = &pHt->GetAttr();

                    aStack.Remove( 0 );
                }
            }
        }
        else
            SearchNextChg();
    }
    return bRet;
}

void SwTxtAttrIterator::AddToStack( const SwTxtAttr& rAttr )
{
    void* pAdd = (void*)&rAttr;
    USHORT nIns = 0, nEndPos = *rAttr.GetEnd();
    for( ; nIns < aStack.Count(); ++nIns )
        if( *((SwTxtAttr*)aStack[ nIns ] )->GetEnd() > nEndPos )
            break;

    aStack.Insert( pAdd, nIns );
}

void SwTxtAttrIterator::SearchNextChg()
{
    USHORT nWh = 0;
    if( nChgPos == aSIter.GetScriptChgPos() )
    {
        aSIter.Next();
        pParaItem = 0;
        nAttrPos = 0;       // must be restart at the beginning, because
                            // some attributes can start before or inside
                            // the current scripttype!
        aStack.Remove( 0, aStack.Count() );
    }
    if( !pParaItem )
        pParaItem = &rTxtNd.GetSwAttrSet().Get( nWh =
                    GetWhichOfScript( nWhichId, aSIter.GetCurrScript() ) );

    xub_StrLen nStt = nChgPos;
    nChgPos = aSIter.GetScriptChgPos();
    pCurItem = pParaItem;

    const SwpHints* pHts = rTxtNd.GetpSwpHints();
    if( pHts )
    {
        if( !nWh )
            nWh = GetWhichOfScript( nWhichId, aSIter.GetCurrScript() );

        const SfxPoolItem* pItem;
        SwFmt* pFmt;
        for( ; nAttrPos < pHts->Count(); ++nAttrPos )
        {
            const SwTxtAttr* pHt = (*pHts)[ nAttrPos ];
            const USHORT* pEnd = pHt->GetEnd();
            const USHORT nHtStt = *pHt->GetStart();
            if( nHtStt < nStt && ( !pEnd || *pEnd <= nStt ))
                continue;

            if( nHtStt >= nChgPos )
                break;

            if( ( nWh == pHt->Which() && 0 != (pItem = &pHt->GetAttr()) ) ||
                ( RES_TXTATR_CHARFMT == pHt->Which() &&
                  0 != (pFmt = pHt->GetCharFmt().GetCharFmt()) &&
                  SFX_ITEM_SET == pFmt->GetItemState( nWh, TRUE, &pItem )) )
            {
                if( nHtStt > nStt )
                {
                    if( nChgPos > nHtStt )
                        nChgPos = nHtStt;
                    break;
                }
                AddToStack( *pHt );
                pCurItem = pItem;
                if( *pEnd < nChgPos )
                    nChgPos = *pEnd;
            }
        }
    }
}


