/*************************************************************************
 *
 *  $RCSfile: unofored.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-21 17:34:23 $
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

#pragma hdrstop

#ifndef _EEITEM_HXX //autogen
#include "eeitem.hxx"
#endif

#include <svtools/itemset.hxx>
#include <editeng.hxx>
#include <editobj.hxx>      // nur fuer die GetText-Kruecke

#include "unofored.hxx"

//------------------------------------------------------------------------

SvxEditEngineForwarder::SvxEditEngineForwarder( EditEngine& rEngine ) :
    rEditEngine( rEngine )
{
}

SvxEditEngineForwarder::~SvxEditEngineForwarder()
{
    //  die EditEngine muss ggf. von aussen geloescht werden
}

USHORT SvxEditEngineForwarder::GetParagraphCount() const
{
    return rEditEngine.GetParagraphCount();
}

USHORT SvxEditEngineForwarder::GetTextLen( USHORT nParagraph ) const
{
    return rEditEngine.GetTextLen( nParagraph );
}

String SvxEditEngineForwarder::GetText( const ESelection& rSel ) const
{
    String aRet = rEditEngine.GetText( rSel, LINEEND_LF );
    aRet.ConvertLineEnd();
    return aRet;
}

SfxItemSet SvxEditEngineForwarder::GetAttribs( const ESelection& rSel ) const
{
    return rEditEngine.GetAttribs( rSel );
}

SfxItemSet SvxEditEngineForwarder::GetParaAttribs( USHORT nPara ) const
{
    SfxItemSet aSet( rEditEngine.GetParaAttribs( nPara ) );

    USHORT nWhich = EE_PARA_START;
    while( nWhich <= EE_PARA_END )
    {
        if( aSet.GetItemState( nWhich, TRUE ) != SFX_ITEM_ON )
        {
            if( rEditEngine.HasParaAttrib( nPara, nWhich ) )
                aSet.Put( rEditEngine.GetParaAttrib( nPara, nWhich ) );
        }
        nWhich++;
    }

    return aSet;
}

void SvxEditEngineForwarder::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
{
    rEditEngine.SetParaAttribs( nPara, rSet );
}

SfxItemPool* SvxEditEngineForwarder::GetPool() const
{
    return rEditEngine.GetEmptyItemSet().GetPool();
}

void SvxEditEngineForwarder::GetPortions( USHORT nPara, SvUShorts& rList ) const
{
    rEditEngine.GetPortions( nPara, rList );
}

void SvxEditEngineForwarder::QuickInsertText( const String& rText, const ESelection& rSel )
{
    rEditEngine.QuickInsertText( rText, rSel );
}

void SvxEditEngineForwarder::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    rEditEngine.QuickInsertField( rFld, rSel );
}

void SvxEditEngineForwarder::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    rEditEngine.QuickSetAttribs( rSet, rSel );
}

XubString SvxEditEngineForwarder::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return rEditEngine.CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

USHORT GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, USHORT nWhich )
{
    EECharAttribArray aAttribs;

    const SfxPoolItem*  pLastItem = NULL;

    SfxItemState eState = SFX_ITEM_DEFAULT;

    // check all paragraphs inside the selection
    for( USHORT nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        SfxItemState eParaState = SFX_ITEM_DEFAULT;

        // calculate start and endpos for this paragraph
        USHORT nPos = 0;
        if( rSel.nStartPara == nPara )
            nPos = rSel.nStartPos;

        USHORT nEndPos = rSel.nEndPos;
        if( rSel.nEndPara != nPara )
            nEndPos = rEditEngine.GetTextLen( nPara );


        // get list of char attribs
        rEditEngine.GetCharAttribs( nPara, aAttribs );

        BOOL bEmpty = TRUE;     // we found no item inside the selektion of this paragraph
        BOOL bGaps  = FALSE;    // we found items but theire gaps between them
        USHORT nLastEnd = nPos;

        const SfxPoolItem* pParaItem = NULL;

        for( USHORT nAttrib = 0; nAttrib < aAttribs.Count(); nAttrib++ )
        {
            struct EECharAttrib aAttrib = aAttribs.GetObject( nAttrib );
            DBG_ASSERT( aAttrib.pAttr, "GetCharAttribs gives corrupt data" );

            if( aAttrib.nStart >= nEndPos )
                break;  // break if we are already behind our selektion

            if( aAttrib.nEnd <= nPos )
                continue;   // or if the attribute ends before our selektion

            if( aAttrib.pAttr->Which() != nWhich )
                continue; // skip if is not the searched item

            // if we already found an item
            if( pParaItem )
            {
                // ... and its different to this one than the state is dont care
                if( *pParaItem != *aAttrib.pAttr )
                    return SFX_ITEM_DONTCARE;
            }
            else
            {
                pParaItem = aAttrib.pAttr;
            }

            if( bEmpty )
                bEmpty = FALSE;

            if( !bGaps && aAttrib.nStart > nLastEnd )
                bGaps = TRUE;

            nLastEnd = aAttrib.nEnd;
        }

        if( !bEmpty && !bGaps && nLastEnd < ( nEndPos - 1 ) )
            bGaps = TRUE;
/*
        // since we have no portion with our item or if there were gaps
        if( bEmpty || bGaps )
        {
            // we need to check the paragraph item
            const SfxItemSet& rParaSet = rEditEngine.GetParaAttribs( nPara );
            if( rParaSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                eState = SFX_ITEM_SET;
                // get item from the paragraph
                const SfxPoolItem* pTempItem = rParaSet.GetItem( nWhich );
                if( pParaItem )
                {
                    if( *pParaItem != *pTempItem )
                        return SFX_ITEM_DONTCARE;
                }
                else
                {
                    pParaItem = pTempItem;
                }

                // set if theres no last item or if its the same
                eParaState = SFX_ITEM_SET;
            }
            else if( bEmpty )
            {
                eParaState = SFX_ITEM_DEFAULT;
            }
            else if( bGaps )
            {
                // gaps and item not set in paragraph, thats a dont care
                return SFX_ITEM_DONTCARE;
            }
        }
        else
        {
            eParaState = SFX_ITEM_SET;
        }
*/
        if( bEmpty )
            eParaState = SFX_ITEM_DEFAULT;
        else if( bGaps )
            eParaState = SFX_ITEM_DONTCARE;
        else
            eParaState = SFX_ITEM_SET;

        // if we already found an item check if we found the same
        if( pLastItem )
        {
            if( (pParaItem == NULL) || (*pLastItem != *pParaItem) )
                return SFX_ITEM_DONTCARE;
        }
        else
        {
            pLastItem = pParaItem;
            eState = eParaState;
        }
    }

    return eState;
}

USHORT SvxEditEngineForwarder::GetItemState( const ESelection& rSel, USHORT nWhich ) const
{
    return GetSvxEditEngineItemState( rEditEngine, rSel, nWhich );
}

USHORT SvxEditEngineForwarder::GetItemState( USHORT nPara, USHORT nWhich ) const
{
    const SfxItemSet& rSet = rEditEngine.GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}

//------------------------------------------------------------------------



