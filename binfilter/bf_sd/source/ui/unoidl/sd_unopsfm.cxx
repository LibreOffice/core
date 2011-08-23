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


#include <vcl/svapp.hxx>

#include <bf_svtools/style.hxx>

#include <unokywds.hxx>

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <bf_svx/unoprov.hxx>

#include "unohelp.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "bf_sd/docshell.hxx"
#include "unowcntr.hxx"
#include "unopsfm.hxx"
#include "unopstyl.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "helpids.h"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

static struct StyleNameMapping_t
{
    char * mpName;
    PresentationObjects meObject;
    sal_uInt16 mnRID;
}
    StyleNameMapping[] =
{
    { sUNO_PseudoSheet_Title,				PO_TITLE,				STR_LAYOUT_TITLE },
    { sUNO_PseudoSheet_SubTitle,			PO_SUBTITLE,			STR_LAYOUT_SUBTITLE },
    { sUNO_PseudoSheet_Background,			PO_BACKGROUND,			STR_LAYOUT_BACKGROUND },
    { sUNO_PseudoSheet_Background_Objects,	PO_BACKGROUNDOBJECTS,	STR_LAYOUT_BACKGROUNDOBJECTS },
    { sUNO_PseudoSheet_Notes,				PO_NOTES,				STR_LAYOUT_NOTES },
    { sUNO_PseudoSheet_Outline1,			PO_OUTLINE_1,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline2,			PO_OUTLINE_2,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline3,			PO_OUTLINE_3,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline4,			PO_OUTLINE_4,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline5,			PO_OUTLINE_5,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline6,			PO_OUTLINE_6,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline7,			PO_OUTLINE_7,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline8,			PO_OUTLINE_8,			STR_LAYOUT_OUTLINE },
    { sUNO_PseudoSheet_Outline9,			PO_OUTLINE_9,			STR_LAYOUT_OUTLINE }
};

static const sal_uInt16 nPseudoStyleCount = 14;

UNO3_GETIMPLEMENTATION_IMPL( SdUnoPseudoStyleFamily );

SdUnoPseudoStyleFamily::SdUnoPseudoStyleFamily( SdXImpressDocument* pModel, SdPage* pPage ) throw()
: mxModel( pModel ), mpModel( pModel ), mpPage( pPage )
{
    DBG_ASSERT( pPage && mxModel.is(), "no page or model? [CL] " );

    mpStyles = new SvUnoWeakContainer;
    if( mpModel && mpModel->GetDoc() )
        StartListening( *mpModel->GetDoc() );
}

SdUnoPseudoStyleFamily::~SdUnoPseudoStyleFamily() throw()
{
    delete mpStyles;
}

OUString SdUnoPseudoStyleFamily::getExternalStyleName( const String& rStyleName ) throw()
{
    String aExtName;

    String aStyleName( rStyleName );
    aStyleName.Erase(0, aStyleName.Search(String( RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR)))+4);

    sal_uInt16 nIndex;
    for( nIndex = 0; nIndex < nPseudoStyleCount; nIndex++ )
    {
        SdResId aRID(StyleNameMapping[nIndex].mnRID);
        String aCompareString(aRID);
        
        if( StyleNameMapping[nIndex].mnRID == STR_LAYOUT_OUTLINE )
        {
            aCompareString += sal_Unicode(' ');
            aCompareString += sal_Unicode('1' + StyleNameMapping[nIndex].meObject - PO_OUTLINE_1 );
        }

        if( aStyleName == aCompareString )
        {
            aExtName = OUString::createFromAscii(StyleNameMapping[nIndex].mpName);
            break;
        }
    }

    return aExtName;
}

// XServiceInfo
OUString SAL_CALL SdUnoPseudoStyleFamily::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdUnoPseudoStyleFamily") );
}

sal_Bool SAL_CALL SdUnoPseudoStyleFamily::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdUnoPseudoStyleFamily::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    OUString aServiceName( OUString::createFromAscii(sUNO_Service_StyleFamily) );
    uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XNameAccess
uno::Any SAL_CALL SdUnoPseudoStyleFamily::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( 0 == aName.getLength() )
        throw container::NoSuchElementException();

    sal_uInt16 nIndex;
    for( nIndex = 0; nIndex < nPseudoStyleCount; nIndex++ )
    {
        if( aName.compareToAscii( StyleNameMapping[nIndex].mpName ) == 0 )
            break;
    }

    if( !isValid() || nIndex >= nPseudoStyleCount )
        throw container::NoSuchElementException();

    return getByIndex( nIndex );
}

uno::Sequence< OUString > SAL_CALL SdUnoPseudoStyleFamily::getElementNames()
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Sequence< OUString > aNameSequence( nPseudoStyleCount );
    OUString* pNamesSeq = aNameSequence.getArray();
    StyleNameMapping_t* pNames = StyleNameMapping;
    sal_uInt16 nCount = nPseudoStyleCount;
    while( nCount-- )
    {
        *pNamesSeq++ = OUString::createFromAscii(pNames->mpName );
        pNames++;
    }

    return aNameSequence;
}

sal_Bool SAL_CALL SdUnoPseudoStyleFamily::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( aName.getLength() )
    {
        StyleNameMapping_t* pNames = StyleNameMapping;
        sal_uInt16 nCount = nPseudoStyleCount;
        while( nCount-- )
        {
            if( aName.compareToAscii( pNames->mpName ) == 0 )
                return sal_True;

            pNames++;
        }
    }

    return sal_False;
}

// XElementAccess
uno::Type SAL_CALL SdUnoPseudoStyleFamily::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( style::XStyle );
}

sal_Bool SAL_CALL SdUnoPseudoStyleFamily::hasElements()
    throw(uno::RuntimeException)
{
    return sal_True;
}

// XIndexAccess

/** the number of styles inside a pseudo style family is fix */
sal_Int32 SAL_CALL SdUnoPseudoStyleFamily::getCount()
    throw(uno::RuntimeException)
{
    return nPseudoStyleCount;
}

uno::Any SAL_CALL SdUnoPseudoStyleFamily::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( !isValid() || Index < 0 || Index >= nPseudoStyleCount )
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;
    createStyle( (sal_uInt16)Index, aAny );

    if(!aAny.hasValue())
        throw lang::IndexOutOfBoundsException();

    return aAny;
}

struct StyleSearchData_t
{
    SfxStyleSheetBase* mpStyleSheet;
    PresentationObjects meObject;
};

sal_Bool SdUnoPseudoStyleFamily_searchfunc( uno::WeakReference< uno::XInterface > xRef, void* pSearchData )
{
    uno::Reference< uno::XInterface > xStyle( xRef );
    if(xStyle.is())
    {
        SdUnoPseudoStyle* pStyle = SdUnoPseudoStyle::getImplementation( xStyle );
        if( pStyle )
        {
            if( pStyle->getStyleSheet() == ((StyleSearchData_t*)pSearchData)->mpStyleSheet &&
                pStyle->getPresObject() == ((StyleSearchData_t*)pSearchData)->meObject )
                return sal_True;
        }
    }
    return sal_False;
}

void SdUnoPseudoStyleFamily::createStyle( sal_uInt16 nIndex, uno::Any& rAny ) throw()
{
    SdDrawDocument* pDoc = mpModel->GetDoc();

    DBG_ASSERT( nIndex < nPseudoStyleCount, "index overflow! [CL]" );
    DBG_ASSERT( pDoc, "need a document here! [CL]" );

    String aLayoutName( mpPage->GetLayoutName() );
    aLayoutName.Erase(aLayoutName.Search(String( RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR)))+4);

    sal_uInt16 nRID = StyleNameMapping[nIndex].mnRID;
    SdResId aRID(nRID);
    aLayoutName += String(aRID);

    if( STR_LAYOUT_OUTLINE == nRID )
    {
        aLayoutName += sal_Unicode(' ');
        aLayoutName += sal_Unicode('1' + StyleNameMapping[nIndex].meObject - PO_OUTLINE_1 );
    }

    SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pSSPool->Find( aLayoutName, (SfxStyleFamily) SD_LT_FAMILY,
                                                    SFXSTYLEBIT_ALL );
    DBG_ASSERT( pStyleSheet, "no style sheet found! [CL]" );

    if( pStyleSheet )
    {
        uno::Reference< style::XStyle >  xStyle;
        createStyle( pStyleSheet, StyleNameMapping[nIndex].meObject, xStyle );
        rAny <<= xStyle;
    }
    else
    {
        rAny.clear();
    }
}

void SdUnoPseudoStyleFamily::createStyle( SfxStyleSheetBase* pStyleSheet, uno::Reference< style::XStyle > & rxRef ) throw()
{
    uno::Any aAny( getByName( getExternalStyleName( pStyleSheet->GetName() ) ) );
    if( aAny.hasValue() && aAny.getValueType() == ITYPE( style::XStyle ) )
        rxRef = *(uno::Reference< style::XStyle > *)aAny.getValue();
    else
        rxRef = NULL;
}

void SdUnoPseudoStyleFamily::createStyle( SfxStyleSheetBase* pStyleSheet, PresentationObjects eObject, uno::Reference< style::XStyle > & rxRef ) throw()
{
    rxRef = NULL;

    StyleSearchData_t aSearchData;
    aSearchData.mpStyleSheet = pStyleSheet;
    aSearchData.meObject = eObject;

    uno::WeakReference< uno::XInterface > xRef;
    if( mpStyles->findRef( xRef, (void*)&aSearchData, SdUnoPseudoStyleFamily_searchfunc ) )
    {
        uno::Reference< uno::XInterface > xInt( xRef );
        if( xInt.is() )
        {
            uno::Any aAny( xInt->queryInterface( ITYPE( style::XStyle ) ) );
            if( aAny >>= rxRef )
                return;
        }
    }

    rxRef = new SdUnoPseudoStyle( mpModel, mpPage, aSearchData.mpStyleSheet, aSearchData.meObject );

    uno::WeakReference<uno::XInterface> wRef(rxRef);
    mpStyles->insert(wRef);
}

void SdUnoPseudoStyleFamily::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    sal_Bool bGoneDead = sal_False;

    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint )
    {
        switch( pSdrHint->GetKind() )
        {
        case HINT_PAGEORDERCHG:
            if( pSdrHint->GetPage() == mpPage && !mpPage->IsInserted() )
                bGoneDead = sal_True;
            break;
        case HINT_MODELCLEARED:
            bGoneDead = sal_True;
            break;
        }
    }

    if( bGoneDead )
    {
        if( IsListening( rBC ) )
            EndListening( rBC );
        mpPage = NULL;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
