/*************************************************************************
 *
 *  $RCSfile: editutil.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-25 18:23:30 $
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

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/algitem.hxx>
#include <svx/editview.hxx>
#include <svx/editstat.hxx>
#include <svx/escpitem.hxx>
#include <svx/flditem.hxx>
#include <vcl/system.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include "editutil.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"

// STATIC DATA -----------------------------------------------------------

//  Delimiters zusaetzlich zu EditEngine-Default:

const sal_Char __FAR_DATA ScEditUtil::pCalcDelimiters[] = "=();+-*/^&<>";


//------------------------------------------------------------------------

String ScEditUtil::ModifyDelimiters( const String& rOld )
{
    String aRet = rOld;
    aRet.EraseAllChars( '_' );  // underscore is used in function argument names
    aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( pCalcDelimiters ) );
    return aRet;
}

String ScEditUtil::GetSpaceDelimitedString( const EditEngine& rEngine )
{
    String aRet;
    USHORT nParCount = rEngine.GetParagraphCount();
    for (USHORT nPar=0; nPar<nParCount; nPar++)
    {
        if (nPar > 0)
            aRet += ' ';
        aRet += rEngine.GetText( nPar );
    }
    return aRet;
}

//------------------------------------------------------------------------

Rectangle ScEditUtil::GetEditArea( const ScPatternAttr* pPattern, BOOL bForceToTop )
{
    // bForceToTop = always align to top, for editing
    // (FALSE for querying URLs etc.)

    USHORT i;

    if (!pPattern)
        pPattern = pDoc->GetPattern( nCol, nRow, nTab );

    Point aStartPos = aScrPos;

    const ScMergeAttr* pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
    long nCellX = (long) ( pDoc->GetColWidth(nCol,nTab) * nPPTX );
    if ( pMerge->GetColMerge() > 1 )
    {
        USHORT nCountX = pMerge->GetColMerge();
        for (i=1; i<nCountX; i++)
            nCellX += (long) ( pDoc->GetColWidth(nCol+i,nTab) * nPPTX );
    }
    long nCellY = (long) ( pDoc->GetRowHeight(nRow,nTab) * nPPTY );
    if ( pMerge->GetRowMerge() > 1 )
    {
        USHORT nCountY = pMerge->GetRowMerge();
        for (i=1; i<nCountY; i++)
            nCellY += (long) ( pDoc->GetRowHeight(nRow+i,nTab) * nPPTY );
    }

    const SvxMarginItem* pMargin = (const SvxMarginItem*)&pPattern->GetItem(ATTR_MARGIN);
    USHORT nIndent = 0;
    if ( ((const SvxHorJustifyItem&)pPattern->GetItem(ATTR_HOR_JUSTIFY)).GetValue() ==
                SVX_HOR_JUSTIFY_LEFT )
        nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
    long nPixDifX   = (long) ( ( pMargin->GetLeftMargin() + nIndent ) * nPPTX );
    aStartPos.X()   += nPixDifX;
    nCellX          -= nPixDifX + (long) ( pMargin->GetRightMargin() * nPPTX );     // wegen Umbruch etc.

    //  vertikale Position auf die in der Tabelle anpassen

    long nPixDifY;
    long nTopMargin = (long) ( pMargin->GetTopMargin() * nPPTY );
    SvxCellVerJustify eJust = (SvxCellVerJustify) ((const SvxVerJustifyItem&)pPattern->
                                                GetItem(ATTR_VER_JUSTIFY)).GetValue();
    if ( eJust == SVX_VER_JUSTIFY_TOP )
        nPixDifY = nTopMargin;
    else
    {
        MapMode aMode = pDev->GetMapMode();
        pDev->SetMapMode( MAP_PIXEL );

        long nTextHeight = pDoc->GetNeededSize( nCol, nRow, nTab,
                                                pDev, nPPTX, nPPTY, aZoomX, aZoomY, FALSE );
        if (!nTextHeight)
        {                                   // leere Zelle
            Font aFont;
            pPattern->GetFont( aFont, pDev, &aZoomY );
            pDev->SetFont(aFont);
            nTextHeight = pDev->GetTextHeight() + nTopMargin +
                            (long) ( pMargin->GetBottomMargin() * nPPTY );
        }

        pDev->SetMapMode(aMode);

        if ( nTextHeight > nCellY + nTopMargin || bForceToTop )
            nPixDifY = 0;                           // zu gross -> oben anfangen
        else
        {
            if ( eJust == SVX_VER_JUSTIFY_CENTER )
                nPixDifY = nTopMargin + ( nCellY - nTextHeight ) / 2;
            else
                nPixDifY = nCellY - nTextHeight + nTopMargin;       // JUSTIFY_BOTTOM
        }
    }

    aStartPos.Y() += nPixDifY;
    nCellY      -= nPixDifY;

                                                        //  -1 -> Gitter nicht ueberschreiben
    return Rectangle( aStartPos, Size(nCellX-1,nCellY-1) );
}

//------------------------------------------------------------------------

ScEditAttrTester::ScEditAttrTester( EditEngine* pEng ) :
    pEngine( pEng ),
    pEditAttrs( NULL ),
    bNeedsObject( FALSE ),
    bNeedsCellAttr( FALSE )
{
    if ( pEngine->GetParagraphCount() > 1 )
    {
        bNeedsObject = TRUE;            //! Zellatribute finden ?
    }
    else
    {
        const SfxPoolItem* pItem = NULL;
        pEditAttrs = new SfxItemSet( pEngine->GetAttribs(
                                        ESelection(0,0,0,pEngine->GetTextLen(0)) ) );
        const SfxItemPool* pEditPool = pEditAttrs->GetPool();

        for (USHORT nId = EE_CHAR_START; nId <= EE_CHAR_END && !bNeedsObject; nId++)
        {
            SfxItemState eState = pEditAttrs->GetItemState( nId, FALSE, &pItem );
            if (eState == SFX_ITEM_DONTCARE)
                bNeedsObject = TRUE;
            else if (eState == SFX_ITEM_SET)
            {
                if ( nId == EE_CHAR_ESCAPEMENT )        // Hoch-/Tiefstellen immer ueber EE
                {
                    if ( (SvxEscapement)((const SvxEscapementItem*)pItem)->GetEnumValue()
                            != SVX_ESCAPEMENT_OFF )
                        bNeedsObject = TRUE;
                }
                else
                    if (!bNeedsCellAttr)
                        if ( *pItem != pEditPool->GetDefaultItem(nId) )
                            bNeedsCellAttr = TRUE;
                //  SetDefaults an der EditEngine setzt Pool-Defaults
            }
        }

        //  Feldbefehle enthalten?

        SfxItemState eFieldState = pEditAttrs->GetItemState( EE_FEATURE_FIELD, FALSE );
        if ( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
            bNeedsObject = TRUE;

        //  not converted characters?

        SfxItemState eConvState = pEditAttrs->GetItemState( EE_FEATURE_NOTCONV, FALSE );
        if ( eConvState == SFX_ITEM_DONTCARE || eConvState == SFX_ITEM_SET )
            bNeedsObject = TRUE;
    }
}

ScEditAttrTester::~ScEditAttrTester()
{
    delete pEditAttrs;
}


//------------------------------------------------------------------------

ScEnginePoolHelper::ScEnginePoolHelper( SfxItemPool* pEnginePoolP,
                BOOL bDeleteEnginePoolP )
            :
            pEnginePool( pEnginePoolP ),
            bDeleteEnginePool( bDeleteEnginePoolP ),
            pDefaults( NULL ),
            bDeleteDefaults( FALSE )
{
}


ScEnginePoolHelper::ScEnginePoolHelper( const ScEnginePoolHelper& rOrg )
            :
            pEnginePool( rOrg.bDeleteEnginePool ? rOrg.pEnginePool->Clone() : rOrg.pEnginePool ),
            bDeleteEnginePool( rOrg.bDeleteEnginePool ),
            pDefaults( NULL ),
            bDeleteDefaults( FALSE )
{
}


ScEnginePoolHelper::~ScEnginePoolHelper()
{
    if ( bDeleteDefaults )
        delete pDefaults;
    if ( bDeleteEnginePool )
        delete pEnginePool;
}


//------------------------------------------------------------------------

ScEditEngineDefaulter::ScEditEngineDefaulter( SfxItemPool* pEnginePoolP,
                BOOL bDeleteEnginePoolP )
            :
            ScEnginePoolHelper( pEnginePoolP, bDeleteEnginePoolP ),
            EditEngine( pEnginePoolP )
{
}


ScEditEngineDefaulter::ScEditEngineDefaulter( const ScEditEngineDefaulter& rOrg )
            :
            ScEnginePoolHelper( rOrg ),
            EditEngine( pEnginePool )
{
}


ScEditEngineDefaulter::~ScEditEngineDefaulter()
{
}


void ScEditEngineDefaulter::SetDefaults( const SfxItemSet& rSet, BOOL bRememberCopy )
{
    if ( bRememberCopy )
    {
        if ( bDeleteDefaults )
            delete pDefaults;
        pDefaults = new SfxItemSet( rSet );
        bDeleteDefaults = TRUE;
    }
    const SfxItemSet& rNewSet = bRememberCopy ? *pDefaults : rSet;
    BOOL bUndo = IsUndoEnabled();
    EnableUndo( FALSE );
    USHORT nPara = GetParagraphCount();
    for ( USHORT j=0; j<nPara; j++ )
    {
        SetParaAttribs( j, rNewSet );
    }
    if ( bUndo )
        EnableUndo( TRUE );
}


void ScEditEngineDefaulter::SetDefaults( SfxItemSet* pSet, BOOL bTakeOwnership )
{
    if ( bDeleteDefaults )
        delete pDefaults;
    pDefaults = pSet;
    bDeleteDefaults = bTakeOwnership;
    if ( pDefaults )
        SetDefaults( *pDefaults, FALSE );
}


void ScEditEngineDefaulter::SetDefaultItem( const SfxPoolItem& rItem )
{
    if ( !pDefaults )
    {
        pDefaults = new SfxItemSet( GetEmptyItemSet() );
        bDeleteDefaults = TRUE;
    }
    pDefaults->Put( rItem );
    SetDefaults( *pDefaults, FALSE );
}


void ScEditEngineDefaulter::SetText( const EditTextObject& rTextObject )
{
    EditEngine::SetText( rTextObject );
    if ( pDefaults )
        SetDefaults( *pDefaults, FALSE );
}


void ScEditEngineDefaulter::SetTextNewDefaults( const EditTextObject& rTextObject,
            const SfxItemSet& rSet, BOOL bRememberCopy )
{
    EditEngine::SetText( rTextObject );
    SetDefaults( rSet, bRememberCopy );
}


void ScEditEngineDefaulter::SetText( const String& rText )
{
    EditEngine::SetText( rText );
    if ( pDefaults )
        SetDefaults( *pDefaults, FALSE );
}


void ScEditEngineDefaulter::SetTextNewDefaults( const String& rText,
            const SfxItemSet& rSet, BOOL bRememberCopy )
{
    EditEngine::SetText( rText );
    SetDefaults( rSet, bRememberCopy );
}


//------------------------------------------------------------------------

ScTabEditEngine::ScTabEditEngine( ScDocument* pDoc )
        : ScEditEngineDefaulter( pDoc->GetEnginePool() )
{
    SetEditTextObjectPool( pDoc->GetEditPool() );
    Init((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN));
}

ScTabEditEngine::ScTabEditEngine( const ScPatternAttr& rPattern,
            SfxItemPool* pEnginePool, SfxItemPool* pTextObjectPool )
        : ScEditEngineDefaulter( pEnginePool )
{
    if ( pTextObjectPool )
        SetEditTextObjectPool( pTextObjectPool );
    Init( rPattern );
}

void ScTabEditEngine::Init( const ScPatternAttr& rPattern )
{
    SetRefMapMode(MAP_100TH_MM);
    SfxItemSet* pEditDefaults = new SfxItemSet( GetEmptyItemSet() );
    rPattern.FillEditItemSet( pEditDefaults );
    SetDefaults( pEditDefaults );
    // wir haben keine StyleSheets fuer Text
    SetControlWord( GetControlWord() & ~EE_CNTRL_RTFSTYLESHEETS );
}

//------------------------------------------------------------------------
//      Feldbefehle fuer Kopf- und Fusszeilen
//------------------------------------------------------------------------

//
//      Zahlen aus \sw\source\core\doc\numbers.cxx
//

String lcl_GetRomanStr( USHORT nNo )
{
    String aStr;
    if( nNo < 4000 )        // mehr kann nicht dargestellt werden
    {
//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
        sal_Char *cRomanArr = "mdclxvi--";  // +2 Dummy-Eintraege !!
        USHORT nMask = 1000;
        while( nMask )
        {
            BYTE nZahl = BYTE(nNo / nMask);
            BYTE nDiff = 1;
            nNo %= nMask;

            if( 5 < nZahl )
            {
                if( nZahl < 9 )
                    aStr += *(cRomanArr-1);
                ++nDiff;
                nZahl -= 5;
            }
            switch( nZahl )
            {
            case 3:     { aStr += *cRomanArr; }
            case 2:     { aStr += *cRomanArr; }
            case 1:     { aStr += *cRomanArr; }
                        break;

            case 4:     {
                          aStr += *cRomanArr;
                          aStr += *(cRomanArr-nDiff);
                        }
                        break;
            case 5:     { aStr += *(cRomanArr-nDiff); }
                        break;
            }

            nMask /= 10;            // zur naechsten Dekade
            cRomanArr += 2;
        }
    }
    return aStr;
}

String lcl_GetCharStr( USHORT nNo )
{
    DBG_ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );
    String aStr;

    const USHORT coDiff = 'Z' - 'A' +1;
    USHORT nCalc;

    do {
        nCalc = nNo % coDiff;
        if( !nCalc )
            nCalc = coDiff;
        aStr.Insert( (sal_Unicode)('a' - 1 + nCalc ), 0 );
        nNo -= nCalc;
        if( nNo )
            nNo /= coDiff;
    } while( nNo );
    return aStr;
}

String lcl_GetNumStr( USHORT nNo, SvxNumType eType )
{
    String aTmpStr( '0' );
    if( nNo )
    {
        switch( eType )
        {
        case SVX_CHARS_UPPER_LETTER:
        case SVX_CHARS_LOWER_LETTER:
            aTmpStr = lcl_GetCharStr( nNo );
            break;

        case SVX_ROMAN_UPPER:
        case SVX_ROMAN_LOWER:
            aTmpStr = lcl_GetRomanStr( nNo );
            break;

        case SVX_NUMBER_NONE:
            aTmpStr.Erase();
            break;

//      CHAR_SPECIAL:
//          ????

//      case ARABIC:    ist jetzt default
        default:
            aTmpStr = String::CreateFromInt32( nNo );
            break;
        }

        if( SVX_CHARS_UPPER_LETTER == eType || SVX_ROMAN_UPPER == eType )
            aTmpStr.ToUpperAscii();
    }
    return aTmpStr;
}

ScHeaderFieldData::ScHeaderFieldData()
{
    nPageNo = nTotalPages = 0;
    eNumType = SVX_ARABIC;
}

ScHeaderEditEngine::ScHeaderEditEngine( SfxItemPool* pEnginePool, BOOL bDeleteEnginePool )
        : ScEditEngineDefaulter( pEnginePool, bDeleteEnginePool )
{
}

String __EXPORT ScHeaderEditEngine::CalcFieldValue( const SvxFieldItem& rField,
                                    USHORT nPara, USHORT nPos,
                                    Color*& rTxtColor, Color*& rFldColor )
{
    String aRet;
    const SvxFieldData* pFieldData = rField.GetField();
    if ( pFieldData )
    {
        TypeId aType = pFieldData->Type();
        if (aType == TYPE(SvxPageField))
            aRet = lcl_GetNumStr( (USHORT)aData.nPageNo,aData.eNumType );
        else if (aType == TYPE(SvxPagesField))
            aRet = lcl_GetNumStr( (USHORT)aData.nTotalPages,aData.eNumType );
        else if (aType == TYPE(SvxTimeField))
            aRet = Application::GetAppInternational().GetTime(aData.aTime);
        else if (aType == TYPE(SvxFileField))
            aRet = aData.aTitle;
        else if (aType == TYPE(SvxExtFileField))
        {
            switch ( ((const SvxExtFileField*)pFieldData)->GetFormat() )
            {
                case SVXFILEFORMAT_FULLPATH :
                    aRet = aData.aLongDocName;
                break;
                default:
                    aRet = aData.aShortDocName;
            }
        }
        else if (aType == TYPE(SvxTableField))
            aRet = aData.aTabName;
        else if (aType == TYPE(SvxDateField))
            aRet = Application::GetAppInternational().GetDate(aData.aDate);
        else
        {
            DBG_ERROR("unbekannter Feldbefehl");
            aRet = '?';
        }
    }
    else
    {
        DBG_ERROR("FieldData ist 0");
        aRet = '?';
    }

    return aRet;
}

//------------------------------------------------------------------------
//
//                          Feld-Daten
//
//------------------------------------------------------------------------

ScFieldEditEngine::ScFieldEditEngine( SfxItemPool* pEnginePool,
            SfxItemPool* pTextObjectPool, BOOL bDeleteEnginePool )
        :
        ScEditEngineDefaulter( pEnginePool, bDeleteEnginePool ),
        bExecuteURL( TRUE )
{
    if ( pTextObjectPool )
        SetEditTextObjectPool( pTextObjectPool );
    //  EE_CNTRL_URLSFXEXECUTE nicht, weil die Edit-Engine den ViewFrame nicht kennt
    // wir haben keine StyleSheets fuer Text
    SetControlWord( (GetControlWord() | EE_CNTRL_MARKFIELDS) & ~EE_CNTRL_RTFSTYLESHEETS );
}

String __EXPORT ScFieldEditEngine::CalcFieldValue( const SvxFieldItem& rField,
                                    USHORT nPara, USHORT nPos,
                                    Color*& rTxtColor, Color*& rFldColor )
{
    String aRet;
    const SvxFieldData* pFieldData = rField.GetField();

    if ( pFieldData )
    {
        TypeId aType = pFieldData->Type();

        if (aType == TYPE(SvxURLField))
        {
            switch ( ((const SvxURLField*)pFieldData)->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
                case SVXURLFORMAT_REPR:
                    aRet = ((const SvxURLField*)pFieldData)->GetRepresentation();
                    break;

                case SVXURLFORMAT_URL:
                    aRet = ((const SvxURLField*)pFieldData)->GetURL();
                    break;
            }
            rTxtColor = new Color( COL_LIGHTBLUE ); //!!! woher nehmen???
        }
        else
        {
            DBG_ERROR("unbekannter Feldbefehl");
            aRet = '?';
        }
    }

    if (!aRet.Len())        // leer ist baeh
        aRet = ' ';         // Space ist Default der Editengine

    return aRet;
}

void __EXPORT ScFieldEditEngine::FieldClicked( const SvxFieldItem& rField, USHORT, USHORT )
{
    const SvxFieldData* pFld = rField.GetField();

    if ( pFld && pFld->ISA( SvxURLField ) && bExecuteURL )
    {
        const SvxURLField* pURLField = (const SvxURLField*) pFld;
        ScGlobal::OpenURL( pURLField->GetURL(), pURLField->GetTargetFrame() );
    }
}

