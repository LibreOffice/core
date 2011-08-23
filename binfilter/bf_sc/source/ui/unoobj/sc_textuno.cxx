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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "scitems.hxx"
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <bf_svx/editobj.hxx>



#ifndef _SVX_ITEMDATA_HXX
#include <bf_svx/itemdata.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#include <bf_svx/flditem.hxx>
#include <bf_svx/unofored.hxx>
#include <rtl/uuid.h>
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>


#include "textuno.hxx"
#include "fielduno.hxx"
#include "servuno.hxx"
#include "editsrc.hxx"
#include "docsh.hxx"
#include "editutil.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "hints.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "docfunc.hxx"
#include "scmod.hxx"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

const SfxItemPropertyMap* lcl_GetHdFtPropertyMap()
{
    static SfxItemPropertyMap aHdFtPropertyMap_Impl[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        SVX_UNOEDIT_NUMBERING_PROPERTIE,	// for completeness of service ParagraphProperties
        {0,0,0,0}
    };
    static BOOL bTwipsSet = FALSE;

    if (!bTwipsSet)
    {
        //	modify PropertyMap to include CONVERT_TWIPS flag for font height
        //	(headers/footers are in twips)

        SfxItemPropertyMap* pEntry = aHdFtPropertyMap_Impl;
        while (pEntry->pName)
        {
            if ( ( pEntry->nWID == EE_CHAR_FONTHEIGHT ||
                   pEntry->nWID == EE_CHAR_FONTHEIGHT_CJK ||
                   pEntry->nWID == EE_CHAR_FONTHEIGHT_CTL ) &&
                 pEntry->nMemberId == MID_FONTHEIGHT )
            {
                pEntry->nMemberId |= CONVERT_TWIPS;
            }

            ++pEntry;
        }
        bTwipsSet = TRUE;
    }

    return aHdFtPropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScHeaderFooterContentObj, "ScHeaderFooterContentObj", "com.sun.star.sheet.HeaderFooterContent" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFooterTextObj, "ScHeaderFooterTextObj", "stardiv.one.Text.Text" )

//------------------------------------------------------------------------

ScHeaderFooterContentObj::ScHeaderFooterContentObj( const EditTextObject* pLeft,
                                                    const EditTextObject* pCenter,
                                                    const EditTextObject* pRight ) :
    pLeftText	( NULL ),
    pCenterText	( NULL ),
    pRightText	( NULL )
{
    if ( pLeft )
        pLeftText	= pLeft->Clone();
    if ( pCenter )
        pCenterText	= pCenter->Clone();
    if ( pRight )
        pRightText	= pRight->Clone();
}

ScHeaderFooterContentObj::~ScHeaderFooterContentObj()
{
    delete pLeftText;
    delete pCenterText;
    delete pRightText;
}

void ScHeaderFooterContentObj::AddListener( SfxListener& rListener )
{
    rListener.StartListening( aBC );
}

void ScHeaderFooterContentObj::RemoveListener( SfxListener& rListener )
{
    rListener.EndListening( aBC );
}

void ScHeaderFooterContentObj::UpdateText( USHORT nPart, EditEngine& rSource )
{
    EditTextObject* pNew = rSource.CreateTextObject();
    switch (nPart)
    {
        case SC_HDFT_LEFT:
            delete pLeftText;
            pLeftText = pNew;
            break;
        case SC_HDFT_CENTER:
            delete pCenterText;
            pCenterText = pNew;
            break;
        default:				// SC_HDFT_RIGHT
            delete pRightText;
            pRightText = pNew;
            break;
    }

    aBC.Broadcast( ScHeaderFooterChangedHint( nPart ) );
}

// XHeaderFooterContent

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getLeftText()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextObj( *this, SC_HDFT_LEFT );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getCenterText()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextObj( *this, SC_HDFT_CENTER );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterContentObj::getRightText()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextObj( *this, SC_HDFT_RIGHT );
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterContentObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScHeaderFooterContentObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScHeaderFooterContentObj* ScHeaderFooterContentObj::getImplementation(
                                const uno::Reference<sheet::XHeaderFooterContent> xObj )
{
    ScHeaderFooterContentObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScHeaderFooterContentObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}


//------------------------------------------------------------------------

ScHeaderFooterTextData::ScHeaderFooterTextData( ScHeaderFooterContentObj& rContent,
                                                    USHORT nP ) :
    rContentObj( rContent ),
    nPart( nP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    bDataValid( FALSE ),
    bInUpdate( FALSE )
{
    rContentObj.acquire();				// must not go away
    rContentObj.AddListener( *this );
}

ScHeaderFooterTextData::~ScHeaderFooterTextData()
{
    ScUnoGuard aGuard;		//	needed for EditEngine dtor

    rContentObj.RemoveListener( *this );

    delete pForwarder;
    delete pEditEngine;

    rContentObj.release();
}

void ScHeaderFooterTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScHeaderFooterChangedHint ) )
    {
        if ( ((const ScHeaderFooterChangedHint&)rHint).GetPart() == nPart )
        {
            if (!bInUpdate)				// not for own updates
                bDataValid = FALSE;		// text has to be fetched again
        }
    }
}

SvxTextForwarder* ScHeaderFooterTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        SfxItemPool* pEnginePool = EditEngine::CreatePool();
        pEnginePool->FreezeIdRanges();
        ScHeaderEditEngine* pHdrEngine = new ScHeaderEditEngine( pEnginePool, TRUE );

        pHdrEngine->EnableUndo( FALSE );
        pHdrEngine->SetRefMapMode( MAP_TWIP );

        //	default font must be set, independently of document
        //	-> use global pool from module

        SfxItemSet aDefaults( pHdrEngine->GetEmptyItemSet() );
        const ScPatternAttr& rPattern = (const ScPatternAttr&)SC_MOD()->GetPool().GetDefaultItem(ATTR_PATTERN);
        rPattern.FillEditItemSet( &aDefaults );
        //	FillEditItemSet adjusts font height to 1/100th mm,
        //	but for header/footer twips is needed, as in the PatternAttr:
        aDefaults.Put( rPattern.GetItem(ATTR_FONT_HEIGHT), EE_CHAR_FONTHEIGHT );
        aDefaults.Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CJK );
        aDefaults.Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CTL );
        pHdrEngine->SetDefaults( aDefaults );

        ScHeaderFieldData aData;
        ScHeaderFooterTextObj::FillDummyFieldData( aData );
        pHdrEngine->SetData( aData );

        pEditEngine = pHdrEngine;
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    const EditTextObject* pData;
    if (nPart == SC_HDFT_LEFT)
        pData = rContentObj.GetLeftEditObject();
    else if (nPart == SC_HDFT_CENTER)
        pData = rContentObj.GetCenterEditObject();
    else
        pData = rContentObj.GetRightEditObject();

    if (pData)
    {
        pEditEngine->SetText(*pData);

        // #i75599# If there's an invalid field ID in a binary file, the result is
        // a SvxFieldItem with NULL as SvxFieldData. These have to be removed here.

        USHORT nParCount = pEditEngine->GetParagraphCount();
        for (USHORT nPar=0; nPar<nParCount; nPar++)
        {
            SvUShorts aPortions;
            pEditEngine->GetPortions( nPar, aPortions );
            
            for ( USHORT nPos = aPortions.Count(); nPos; )
            {
                --nPos;
                USHORT nEnd = aPortions.GetObject( nPos );
                USHORT nStart = nPos ? aPortions.GetObject( nPos - 1 ) : 0;
                // fields are single characters
                if ( nEnd == nStart+1 )
                {
                    ESelection aFieldSel( nPar, nStart, nPar, nEnd );
                    SfxItemSet aSet = pEditEngine->GetAttribs( aFieldSel );
                    const SfxPoolItem* pItem = NULL;
                    if ( aSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_ON )
                    {
                        const SvxFieldItem* pFieldItem = static_cast<const SvxFieldItem*>(pItem);
                        const SvxFieldData* pData = pFieldItem->GetField();
                        if ( !pData )
                        {
                            // remove the invalid field
                            pEditEngine->QuickDelete( aFieldSel );
                        }
                    }
                }
            }
        }
    }

    bDataValid = TRUE;
    return pForwarder;
}

void ScHeaderFooterTextData::UpdateData()
{
    if ( pEditEngine )
    {
        bInUpdate = TRUE;	// don't reset bDataValid during UpdateText

        rContentObj.UpdateText( nPart, *pEditEngine );

        bInUpdate = FALSE;
    }
}

//------------------------------------------------------------------------

ScHeaderFooterTextObj::ScHeaderFooterTextObj( ScHeaderFooterContentObj& rContent,
                                                USHORT nP ) :
    aTextData( rContent, nP ),
    pUnoText( NULL )
{
    //	ScHeaderFooterTextData acquires rContent
    //	pUnoText is created on demand (getString/setString work without it)
}

void ScHeaderFooterTextObj::CreateUnoText_Impl()
{
    if ( !pUnoText )
    {
        //	can't be aggregated because getString/setString is handled here
        ScSharedHeaderFooterEditSource aEditSource( &aTextData );
        pUnoText = new SvxUnoText( &aEditSource, lcl_GetHdFtPropertyMap(), uno::Reference<text::XText>() );
        pUnoText->acquire();
    }
}

ScHeaderFooterTextObj::~ScHeaderFooterTextObj()
{
    if (pUnoText)
        pUnoText->release();
}

const SvxUnoText& ScHeaderFooterTextObj::GetUnoText()
{
    if (!pUnoText)
        CreateUnoText_Impl();
    return *pUnoText;
}

// XText

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursor()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScHeaderFooterTextCursor( *this );
}

uno::Reference<text::XTextCursor> SAL_CALL ScHeaderFooterTextObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->createTextCursorByRange(aTextPosition);
    //!	wie ScCellObj::createTextCursorByRange, wenn SvxUnoTextRange_getReflection verfuegbar
}

void ScHeaderFooterTextObj::FillDummyFieldData( ScHeaderFieldData& rData )	// static
{
    String aDummy = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( "???" ));
    rData.aTitle		= aDummy;
    rData.aLongDocName	= aDummy;
    rData.aShortDocName	= aDummy;
    rData.aTabName		= aDummy;
    rData.nPageNo		= 1;
    rData.nTotalPages	= 99;
}

::rtl::OUString SAL_CALL ScHeaderFooterTextObj::getString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ::rtl::OUString aRet;
    const EditTextObject* pData;

    USHORT nPart = aTextData.GetPart();
    ScHeaderFooterContentObj& rContentObj = aTextData.GetContentObj();

    if (nPart == SC_HDFT_LEFT)
        pData = rContentObj.GetLeftEditObject();
    else if (nPart == SC_HDFT_CENTER)
        pData = rContentObj.GetCenterEditObject();
    else
        pData = rContentObj.GetRightEditObject();
    if (pData)
    {
        // for pure text, no font info is needed in pool defaults
        ScHeaderEditEngine aEditEngine( EditEngine::CreatePool(), TRUE );

        ScHeaderFieldData aData;
        FillDummyFieldData( aData );
        aEditEngine.SetData( aData );

        aEditEngine.SetText(*pData);
        aRet = ScEditUtil::GetSpaceDelimitedString( aEditEngine );
    }
    return aRet;
}

void SAL_CALL ScHeaderFooterTextObj::setString( const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aText;

    // for pure text, no font info is needed in pool defaults
    ScHeaderEditEngine aEditEngine( EditEngine::CreatePool(), TRUE );
    aEditEngine.SetText( aString );

    aTextData.GetContentObj().UpdateText( aTextData.GetPart(), aEditEngine );
}

void SAL_CALL ScHeaderFooterTextObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                            const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    pUnoText->insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertControlCharacter(
                                            const uno::Reference<text::XTextRange>& xRange,
                                            sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    pUnoText->insertControlCharacter( xRange, nControlCharacter, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::insertTextContent(
                                            const uno::Reference<text::XTextRange >& xRange,
                                            const uno::Reference<text::XTextContent >& xContent,
                                            sal_Bool bAbsorb )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( xContent.is() && xRange.is() )
    {
        ScHeaderFieldObj* pHeaderField = ScHeaderFieldObj::getImplementation( xContent );

        SvxUnoTextRangeBase* pTextRange =
            ScHeaderFooterTextCursor::getImplementation( xRange );

#if 0
        if (!pTextRange)
            pTextRange = (SvxUnoTextRange*)xRange->getImplementation(
                                            SvxUnoTextRange_getReflection() );
        //!	bei SvxUnoTextRange testen, ob in passendem Objekt !!!
#endif

        if ( pHeaderField && !pHeaderField->IsInserted() && pTextRange )
        {
            SvxEditSource* pEditSource = pTextRange->GetEditSource();
            ESelection aSelection = pTextRange->GetSelection();

            if (!bAbsorb)
            {
                //	don't replace -> append at end
                aSelection.Adjust();
                aSelection.nStartPara = aSelection.nEndPara;
                aSelection.nStartPos  = aSelection.nEndPos;
            }

            SvxFieldItem aItem = pHeaderField->CreateFieldItem();

            SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
            pForwarder->QuickInsertField( aItem, aSelection );
            pEditSource->UpdateData();

            //	neue Selektion: ein Zeichen
            aSelection.Adjust();
            aSelection.nEndPara = aSelection.nStartPara;
            aSelection.nEndPos = aSelection.nStartPos + 1;
            pHeaderField->InitDoc( &aTextData.GetContentObj(), aTextData.GetPart(), aSelection );

            //	#91431# for bAbsorb=FALSE, the new selection must be behind the inserted content
            //	(the xml filter relies on this)
            if (!bAbsorb)
                aSelection.nStartPos = aSelection.nEndPos;

            pTextRange->SetSelection( aSelection );

            return;
        }
    }

    if (!pUnoText)
        CreateUnoText_Impl();
    pUnoText->insertTextContent( xRange, xContent, bAbsorb );
}

void SAL_CALL ScHeaderFooterTextObj::removeTextContent(
                                            const uno::Reference<text::XTextContent>& xContent )
                                throw(container::NoSuchElementException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( xContent.is() )
    {
        ScHeaderFieldObj* pHeaderField = ScHeaderFieldObj::getImplementation( xContent );
        if ( pHeaderField && pHeaderField->IsInserted() )
        {
            //!	Testen, ob das Feld in dieser Zelle ist
            pHeaderField->DeleteField();
            return;
        }
    }
    if (!pUnoText)
        CreateUnoText_Impl();
    pUnoText->removeTextContent( xContent );
}

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextObj::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->getText();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextObj::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->getEnd();
}

// XTextFieldsSupplier

uno::Reference<container::XEnumerationAccess> SAL_CALL ScHeaderFooterTextObj::getTextFields()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    // all fields
    return new ScHeaderFieldsObj( &aTextData.GetContentObj(), aTextData.GetPart(), SC_SERVICE_INVALID );
}

uno::Reference<container::XNameAccess> SAL_CALL ScHeaderFooterTextObj::getTextFieldMasters()
                                                throw(uno::RuntimeException)
{
    //	sowas gibts nicht im Calc (?)
    return NULL;
}

// XTextRangeMover

void SAL_CALL ScHeaderFooterTextObj::moveTextRange(
                                        const uno::Reference<text::XTextRange>& xRange,
                                        sal_Int16 nParagraphs )
                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    pUnoText->moveTextRange( xRange, nParagraphs );
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFooterTextObj::createEnumeration()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->createEnumeration();
}

// XElementAccess

uno::Type SAL_CALL ScHeaderFooterTextObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->getElementType();
}

sal_Bool SAL_CALL ScHeaderFooterTextObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (!pUnoText)
        CreateUnoText_Impl();
    return pUnoText->hasElements();
}

//------------------------------------------------------------------------

ScCellTextCursor::ScCellTextCursor(const ScCellTextCursor& rOther) :
    SvxUnoTextCursor( rOther ),
    rTextObj( rOther.rTextObj )
{
    rTextObj.acquire();
}

ScCellTextCursor::ScCellTextCursor(ScCellObj& rText) :
    SvxUnoTextCursor( rText.GetUnoText() ),
    rTextObj( rText )
{
    rTextObj.acquire();
}

ScCellTextCursor::~ScCellTextCursor() throw()
{
    rTextObj.release();
}

// SvxUnoTextCursor methods reimplemented here to return the right objects:

uno::Reference<text::XText> SAL_CALL ScCellTextCursor::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return &rTextObj;
}

uno::Reference<text::XTextRange> SAL_CALL ScCellTextCursor::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	use other object for range than cursor?

    ScCellTextCursor* pNew = new ScCellTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel = GetSelection();
    aNewSel.nEndPara = aNewSel.nStartPara;
    aNewSel.nEndPos  = aNewSel.nStartPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

uno::Reference<text::XTextRange> SAL_CALL ScCellTextCursor::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	use other object for range than cursor?

    ScCellTextCursor* pNew = new ScCellTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel = GetSelection();
    aNewSel.nStartPara = aNewSel.nEndPara;
    aNewSel.nStartPos  = aNewSel.nEndPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return SvxUnoTextCursor::getSomething( rId );
}

// static
const uno::Sequence<sal_Int8>& ScCellTextCursor::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScCellTextCursor* ScCellTextCursor::getImplementation( const uno::Reference<uno::XInterface> xObj )
{
    ScCellTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScCellTextCursor*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor(const ScHeaderFooterTextCursor& rOther) :
    SvxUnoTextCursor( rOther ),
    rTextObj( rOther.rTextObj )
{
    rTextObj.acquire();
}

ScHeaderFooterTextCursor::ScHeaderFooterTextCursor(ScHeaderFooterTextObj& rText) :
    SvxUnoTextCursor( rText.GetUnoText() ),
    rTextObj( rText )
{
    rTextObj.acquire();
}

ScHeaderFooterTextCursor::~ScHeaderFooterTextCursor() throw()
{
    rTextObj.release();
}

// SvxUnoTextCursor methods reimplemented here to return the right objects:

uno::Reference<text::XText> SAL_CALL ScHeaderFooterTextCursor::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return &rTextObj;
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextCursor::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	use other object for range than cursor?

    ScHeaderFooterTextCursor* pNew = new ScHeaderFooterTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel = GetSelection();
    aNewSel.nEndPara = aNewSel.nStartPara;
    aNewSel.nEndPos  = aNewSel.nStartPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFooterTextCursor::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	use other object for range than cursor?

    ScHeaderFooterTextCursor* pNew = new ScHeaderFooterTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel = GetSelection();
    aNewSel.nStartPara = aNewSel.nEndPara;
    aNewSel.nStartPos  = aNewSel.nEndPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFooterTextCursor::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return SvxUnoTextCursor::getSomething( rId );
}

// static
const uno::Sequence<sal_Int8>& ScHeaderFooterTextCursor::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScHeaderFooterTextCursor* ScHeaderFooterTextCursor::getImplementation(
                                const uno::Reference<uno::XInterface> xObj )
{
    ScHeaderFooterTextCursor* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScHeaderFooterTextCursor*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScDrawTextCursor::ScDrawTextCursor(const ScDrawTextCursor& rOther) :
    SvxUnoTextCursor( rOther ),
    xParentText( rOther.xParentText )
{
}

ScDrawTextCursor::ScDrawTextCursor( const uno::Reference<text::XText>& xParent,
                                    const SvxUnoTextBase& rText ) :
    SvxUnoTextCursor( rText ),
    xParentText( xParent )

{
}

ScDrawTextCursor::~ScDrawTextCursor() throw()
{
}

// SvxUnoTextCursor methods reimplemented here to return the right objects:

uno::Reference<text::XText> SAL_CALL ScDrawTextCursor::getText() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return xParentText;
}

uno::Reference<text::XTextRange> SAL_CALL ScDrawTextCursor::getStart() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	use other object for range than cursor?

    ScDrawTextCursor* pNew = new ScDrawTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel = GetSelection();
    aNewSel.nEndPara = aNewSel.nStartPara;
    aNewSel.nEndPos  = aNewSel.nStartPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

uno::Reference<text::XTextRange> SAL_CALL ScDrawTextCursor::getEnd() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //!	use other object for range than cursor?

    ScDrawTextCursor* pNew = new ScDrawTextCursor( *this );
    uno::Reference<text::XTextRange> xRange( static_cast<SvxUnoTextRangeBase*>(pNew) );

    ESelection aNewSel = GetSelection();
    aNewSel.nStartPara = aNewSel.nEndPara;
    aNewSel.nStartPos  = aNewSel.nEndPos;
    pNew->SetSelection( aNewSel );

    return xRange;
}

//------------------------------------------------------------------------

ScSimpleEditSourceHelper::ScSimpleEditSourceHelper()
{
    SfxItemPool* pEnginePool = EditEngine::CreatePool();
    pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
    pEnginePool->FreezeIdRanges();

    pEditEngine = new ScFieldEditEngine( pEnginePool, NULL, TRUE );		// TRUE: become owner of pool
    pForwarder = new SvxEditEngineForwarder( *pEditEngine );
    pOriginalSource = new ScSimpleEditSource( pForwarder );
}

ScSimpleEditSourceHelper::~ScSimpleEditSourceHelper()
{
    ScUnoGuard aGuard;		//	needed for EditEngine dtor

    delete pOriginalSource;
    delete pForwarder;
    delete pEditEngine;
}

ScEditEngineTextObj::ScEditEngineTextObj() :
    SvxUnoText( GetOriginalSource(), ScCellObj::GetEditPropertyMap(), uno::Reference<text::XText>() )
{
}

ScEditEngineTextObj::~ScEditEngineTextObj() throw()
{
}

void ScEditEngineTextObj::SetText( const EditTextObject& rTextObject )
{
    GetEditEngine()->SetText( rTextObject );

    ESelection aSel;
    ::binfilter::GetSelection( aSel, GetEditSource()->GetTextForwarder() );
    SetSelection( aSel );
}

EditTextObject* ScEditEngineTextObj::CreateTextObject()
{
    return GetEditEngine()->CreateTextObject();
}

//------------------------------------------------------------------------

ScCellTextData::ScCellTextData(ScDocShell* pDocSh, const ScAddress& rP) :
    pDocShell( pDocSh ),
    aCellPos( rP ),
    pEditEngine( NULL ),
    pForwarder( NULL ),
    bDataValid( FALSE ),
    bInUpdate( FALSE ),
    pOriginalSource( NULL ),
    bDirty( FALSE ),
    bDoUpdate( TRUE )
{
    if (pDocShell)
        pDocShell->GetDocument()->AddUnoObject(*this);
}

ScCellTextData::~ScCellTextData()
{
    ScUnoGuard aGuard;		//	needed for EditEngine dtor

    if (pDocShell)
    {
        pDocShell->GetDocument()->RemoveUnoObject(*this);
        pDocShell->GetDocument()->DisposeFieldEditEngine(pEditEngine);
    }
    else
        delete pEditEngine;

    delete pForwarder;

    delete pOriginalSource;
}

ScSharedCellEditSource* ScCellTextData::GetOriginalSource()
{
    if (!pOriginalSource)
        pOriginalSource = new ScSharedCellEditSource( this );
    return pOriginalSource;
}

void ScCellTextData::GetCellText(const ScAddress& rCellPos, String& rText)
{
    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        pDoc->GetInputString( rCellPos.Col(), rCellPos.Row(), rCellPos.Tab(), rText );
    }
}

SvxTextForwarder* ScCellTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        if ( pDocShell )
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            pEditEngine = pDoc->CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScFieldEditEngine( pEnginePool, NULL, TRUE );
        }
        //	currently, GetPortions doesn't work if UpdateMode is FALSE,
        //	this will be fixed (in EditEngine) by src600
//		pEditEngine->SetUpdateMode( FALSE );
        pEditEngine->EnableUndo( FALSE );
        if (pDocShell)
            pEditEngine->SetRefDevice(pDocShell->GetVirtualDevice_100th_mm());
        else
            pEditEngine->SetRefMapMode( MAP_100TH_MM );
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    String aText;

    if (pDocShell)
    {
        ScDocument* pDoc = pDocShell->GetDocument();

        SfxItemSet aDefaults( pEditEngine->GetEmptyItemSet() );
        const ScPatternAttr* pPattern =
                pDoc->GetPattern( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab() );
        pPattern->FillEditItemSet( &aDefaults );
        pPattern->FillEditParaItems( &aDefaults );	// including alignment etc. (for reading)

        const ScBaseCell* pCell = pDoc->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_EDIT )
            pEditEngine->SetTextNewDefaults( *((const ScEditCell*)pCell)->GetData(), aDefaults );
        else
        {
            GetCellText( aCellPos, aText );
            if (aText.Len())
                pEditEngine->SetTextNewDefaults( aText, aDefaults );
            else
                pEditEngine->SetDefaults(aDefaults);
        }
    }

    bDataValid = TRUE;
    return pForwarder;
}

void ScCellTextData::UpdateData()
{
    if ( bDoUpdate )
    {
        DBG_ASSERT(pEditEngine != NULL, "no EditEngine for UpdateData()");
        if ( pDocShell && pEditEngine )
        {
            //	during the own UpdateData call, bDataValid must not be reset,
            //	or things like attributes after the text would be lost
            //	(are not stored in the cell)

            bInUpdate = TRUE;	// prevents bDataValid from being reset

            ScDocFunc aFunc(*pDocShell);
            aFunc.PutData( aCellPos, *pEditEngine, FALSE, TRUE );	// always as text

            bInUpdate = FALSE;
            bDirty = FALSE;
        }
    }
    else
        bDirty = TRUE;
}

void ScCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //!	Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            pDocShell = NULL;						// invalid now

            DELETEZ( pForwarder );
            DELETEZ( pEditEngine );		// EditEngine uses document's pool
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            if (!bInUpdate)							// not for own UpdateData calls
                bDataValid = FALSE;					// text has to be read from the cell again
        }
    }
}

ScCellTextObj::ScCellTextObj(ScDocShell* pDocSh, const ScAddress& rP) :
    ScCellTextData( pDocSh, rP ),
    SvxUnoText( GetOriginalSource(), ScCellObj::GetEditPropertyMap(), uno::Reference<text::XText>() )
{
}

ScCellTextObj::~ScCellTextObj() throw()
{
}

}
