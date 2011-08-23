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


#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <bf_svtools/style.hxx>
#endif

#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <bf_svx/unoprov.hxx>

#include "unohelp.hxx"
#include "unokywds.hxx"
#include "unowcntr.hxx"
#include "unogsfm.hxx"
#include "unogstyl.hxx"
#include "drawdoc.hxx"
#include "glob.hrc"
#include "helpids.h"
#include "sdresid.hxx"
namespace binfilter {


using namespace ::rtl;
using namespace ::vos;
using namespace ::com::sun::star;

UNO3_GETIMPLEMENTATION_IMPL( SdUnoGraphicStyleFamily );

SdUnoGraphicStyleFamily::SdUnoGraphicStyleFamily( SdXImpressDocument* pModel ) throw()
: mxModel( pModel ), mpModel( pModel )
{
    mpStyles = new SvUnoWeakContainer;
}

SdUnoGraphicStyleFamily::~SdUnoGraphicStyleFamily() throw()
{
    delete mpStyles;
}

struct SdUnoGStyleNameMapper_s
{
    char*	mpName;
    sal_uInt16	mnResId;
    sal_uInt16	mnHelpId;
}
SdUnoGStyleNameMapper[] =
{
    { sUNO_StyleName_standard,		STR_STANDARD_STYLESHEET_NAME,	HID_STANDARD_STYLESHEET_NAME },
    { sUNO_StyleName_objwitharrow,	STR_POOLSHEET_OBJWITHARROW,		HID_POOLSHEET_OBJWITHARROW },
    { sUNO_StyleName_objwithshadow,	STR_POOLSHEET_OBJWITHSHADOW,	HID_POOLSHEET_OBJWITHSHADOW },
    { sUNO_StyleName_objwithoutfill,STR_POOLSHEET_OBJWITHOUTFILL,	HID_POOLSHEET_OBJWITHOUTFILL },
    { sUNO_StyleName_text,			STR_POOLSHEET_TEXT,				HID_POOLSHEET_TEXT },
    { sUNO_StyleName_textbody,		STR_POOLSHEET_TEXTBODY,			HID_POOLSHEET_TEXTBODY },
    { sUNO_StyleName_textbodyjust,	STR_POOLSHEET_TEXTBODY_JUSTIFY,	HID_POOLSHEET_TEXTBODY_JUSTIFY },
    { sUNO_StyleName_textbodyindent,STR_POOLSHEET_TEXTBODY_INDENT,	HID_POOLSHEET_TEXTBODY_INDENT },
    { sUNO_StyleName_title,			STR_POOLSHEET_TITLE,			HID_POOLSHEET_TITLE },
    { sUNO_StyleName_title1,		STR_POOLSHEET_TITLE1,			HID_POOLSHEET_TITLE1 },
    { sUNO_StyleName_title2,		STR_POOLSHEET_TITLE2,			HID_POOLSHEET_TITLE2 },
    { sUNO_StyleName_headline,		STR_POOLSHEET_HEADLINE,			HID_POOLSHEET_HEADLINE },
    { sUNO_StyleName_headline1,		STR_POOLSHEET_HEADLINE1,		HID_POOLSHEET_HEADLINE1 },
    { sUNO_StyleName_headline2,		STR_POOLSHEET_HEADLINE2,		HID_POOLSHEET_HEADLINE2 },
    { sUNO_StyleName_measure,		STR_POOLSHEET_MEASURE,			HID_POOLSHEET_MEASURE },
    { NULL, 0, 0 }
};

String SdUnoGraphicStyleFamily::getInternalStyleName( const OUString& rName ) throw()
{
    struct SdUnoGStyleNameMapper_s* pMap = SdUnoGStyleNameMapper;
    while( pMap->mpName )
    {
        if( rName.compareToAscii( pMap->mpName ) == 0 )
        {
            String aStr( SdResId(pMap->mnResId) );
            return aStr;
        }

        pMap++;
    }

    // check for user suffix and delete if found
    const sal_Unicode *pChar = rName.getStr();
    sal_Int32 nLen = rName.getLength();
    if (nLen > 8 &&
        pChar[nLen-7] == ' ' && 
        pChar[nLen-6] == '(' &&
        pChar[nLen-5] == 'u' &&
        pChar[nLen-4] == 's' &&
        pChar[nLen-3] == 'e' &&
        pChar[nLen-2] == 'r' &&
        pChar[nLen-1] == ')')
    {
        return rName.copy( 0, nLen-7 );
    }

    return rName;
}

OUString SdUnoGraphicStyleFamily::getExternalStyleName( const String& rName ) throw()
{
    sal_Bool bNeedsUser = sal_False;

    // check if we have an internal name and return the coresponding programatic name
    struct SdUnoGStyleNameMapper_s* pMap = SdUnoGStyleNameMapper;
    while( pMap->mpName )
    {
        String aCompare( SdResId(pMap->mnResId) );
        if( rName == aCompare )
            return OUString::createFromAscii( pMap->mpName );

        // check if the internal name equals a programatic name and add user
        if( rName.CompareToAscii( pMap->mpName ) == 0 )
        {
            bNeedsUser = sal_True;
            break;
        }

        pMap++;
    }

    if( !bNeedsUser )
    {
        // check if this string ends with a " (user)", if so, add another one
        // check for user suffix and delete if found
        const sal_Unicode *pChar = rName.GetBuffer();
        sal_Int32 nLen = rName.Len();
        if (nLen > 8 &&
            pChar[nLen-7] == ' ' && 
            pChar[nLen-6] == '(' &&
            pChar[nLen-5] == 'u' &&
            pChar[nLen-4] == 's' &&
            pChar[nLen-3] == 'e' &&
            pChar[nLen-2] == 'r' &&
            pChar[nLen-1] == ')')
        {
            bNeedsUser = sal_True;
        }
    }

    if( bNeedsUser )
    {
        OUString aName( rName );
        aName += OUString( RTL_CONSTASCII_USTRINGPARAM ( " (user)" ) );
        return aName;
    }

    return rName;
}

// XServiceInfo
OUString SAL_CALL SdUnoGraphicStyleFamily::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdUnoGraphicStyleFamily") );
}

sal_Bool SAL_CALL SdUnoGraphicStyleFamily::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdUnoGraphicStyleFamily::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    OUString aServiceName( OUString::createFromAscii(sUNO_Service_StyleFamily) );
    uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

// XNameAccess
uno::Any SAL_CALL SdUnoGraphicStyleFamily::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( NULL == pDoc || 0 == aName.getLength() )
        throw container::NoSuchElementException();

    SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pSSPool->Find( getInternalStyleName( aName ), SFX_STYLE_FAMILY_PARA );
    if( pStyleSheet == NULL )
        throw container::NoSuchElementException();

    uno::Any aAny;
    createStyle( pStyleSheet, aAny );
    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdUnoGraphicStyleFamily::getElementNames()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    SfxStyleSheetIterator* pIter = NULL;

    sal_Int32 nCount = 0;
    if( pDoc )
    {
        SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
        pIter = pSSPool->CreateIterator(SFX_STYLE_FAMILY_PARA, 0xFFFF );
        nCount  = pIter->Count();
    }
            
    uno::Sequence< OUString > aNameSequence( nCount );

    if( nCount )
    {
        OUString* pNamesSeq = aNameSequence.getArray();

        SfxStyleSheetBase* pStyleSheet = pIter->First();
        while( nCount  && pStyleSheet )
        {
            *pNamesSeq++ = getExternalStyleName( pStyleSheet->GetName() );
            pStyleSheet = pIter->Next();
            nCount--;
        }

        DBG_ASSERT( pStyleSheet == NULL && nCount == 0, "memory overwrite catched! [CL]" );
    }

    return aNameSequence;
}

sal_Bool SAL_CALL SdUnoGraphicStyleFamily::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc == NULL )
        return sal_False;

    SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pSSPool->Find( getInternalStyleName( aName ), SFX_STYLE_FAMILY_PARA );
    return pStyleSheet != NULL;
}

// XElementAccess
uno::Type SAL_CALL SdUnoGraphicStyleFamily::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( style::XStyle );
}

sal_Bool SAL_CALL SdUnoGraphicStyleFamily::hasElements()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return mpModel->GetDoc() != NULL;
}

// XIndexAccess
/** the number of styles inside a pseudo style family is fix */
sal_Int32 SAL_CALL SdUnoGraphicStyleFamily::getCount()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nCount = 0;

    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc != NULL )
    {
        SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
        SfxStyleSheetIterator* pIter = pSSPool->CreateIterator(SFX_STYLE_FAMILY_PARA, 0xFFFF );
        nCount  = pIter->Count();
        delete pIter;
    }
    
    return nCount;
}

uno::Any SAL_CALL SdUnoGraphicStyleFamily::getByIndex( sal_Int32 Index ) throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;

    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc == NULL )
        return aAny;

    SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
    SfxStyleSheetIterator* pIter = pSSPool->CreateIterator(SFX_STYLE_FAMILY_PARA, 0xFFFF );
    const sal_Int32 nCount  = pIter->Count();

    if( Index < 0 || Index >= nCount )
        throw lang::IndexOutOfBoundsException();

    createStyle( (*pIter)[(sal_uInt16)Index], aAny );

    delete pIter;

    return aAny;
}

// XNameContainer
void SAL_CALL SdUnoGraphicStyleFamily::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdUnoGraphicStyle* pStyle = getStyleFromAny( aElement );

    if( pStyle == NULL || pStyle->isValid() || 0 == aName.getLength() )
        throw lang::IllegalArgumentException();

    String aStyleName( getInternalStyleName(aName) );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pSSPool->Find( aStyleName, SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
    if( pStyleSheet )
        throw container::ElementExistException();

    pStyleSheet = &pSSPool->Make( aStyleName, SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_USERDEF );

    pStyle->create( mpModel, pStyleSheet );

    mpModel->SetModified();
}

void SAL_CALL SdUnoGraphicStyleFamily::removeByName( const OUString& Name )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    String aStyleName( Name );

    SdDrawDocument* pDoc = mpModel->GetDoc();
    SfxStyleSheetBase* pStyleSheet = NULL;
    SfxStyleSheetBasePool* pSSPool = NULL;

    if( pDoc && 0 != Name.getLength() )
    {
        pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
        if( pSSPool )
            SfxStyleSheetBase* pStyleSheet = pSSPool->Find( getInternalStyleName(Name), SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );
    }

    if( NULL == pStyleSheet || pStyleSheet->IsUserDefined() )
        throw container::NoSuchElementException();

    pSSPool->Erase( pStyleSheet );

    mpModel->SetModified();
}

// XNameReplace
void SAL_CALL SdUnoGraphicStyleFamily::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdUnoGraphicStyle* pStyle = getStyleFromAny( aElement );

    SdDrawDocument* pDoc = mpModel->GetDoc();

    if( NULL == pDoc || pStyle->isValid() || 0 == aName.getLength() )
        throw lang::IllegalArgumentException();

    SfxStyleSheetBase* pStyleSheet = NULL;
    const String aStyleName( aName );

    SfxStyleSheetBasePool* pSSPool = (SfxStyleSheetBasePool*)pDoc->GetStyleSheetPool();
    if( pSSPool )
        pStyleSheet = pSSPool->Find( getInternalStyleName(aName), SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL );

    if( NULL == pStyleSheet )
        throw container::NoSuchElementException();

    pSSPool->Erase( pStyleSheet );

    pStyleSheet = &pSSPool->Make( aStyleName, SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_USERDEF );
    pStyle->create( mpModel, pStyleSheet );

    mpModel->SetModified();
}

// XSingleServiceFactory
uno::Reference< uno::XInterface > SAL_CALL SdUnoGraphicStyleFamily::createInstance(  )
    throw(uno::Exception, uno::RuntimeException)
{
    return (::cppu::OWeakObject*)new SdUnoGraphicStyle();
}

uno::Reference< uno::XInterface > SAL_CALL SdUnoGraphicStyleFamily::createInstanceWithArguments( const uno::Sequence< uno::Any >& aArguments )
    throw(uno::Exception, uno::RuntimeException)
{
    return (::cppu::OWeakObject*)new SdUnoGraphicStyle();
}

sal_Bool SdUnoGraphicStyleFamily_searchfunc( uno::WeakReference< uno::XInterface > xRef, void* pSearchData )
{
    uno::Reference< uno::XInterface > xStyle( xRef );
    if( xStyle.is() )
    {
        SdUnoGraphicStyle* pStyle = SdUnoGraphicStyle::getImplementation( xStyle );
        if( pStyle )
        {
            if( pStyle->getStyleSheet() == (SfxStyleSheetBase*)pSearchData )
                return sal_True;
        }
    }
    return sal_False;
}

void SdUnoGraphicStyleFamily::createStyle( SfxStyleSheetBase* pStyleSheet, uno::Any& rAny ) const throw()
{
    SdDrawDocument* pDoc = mpModel->GetDoc();

    DBG_ASSERT( pStyleSheet, "need a style for insert! [CL]" );
    DBG_ASSERT( pDoc, "need a document here! [CL]" );

    uno::WeakReference< uno::XInterface > xRef;
    uno::Reference< style::XStyle >  xStyle;

    if( mpStyles->findRef( xRef, (void*)pStyleSheet, SdUnoGraphicStyleFamily_searchfunc ) )
    {
        uno::Reference< uno::XInterface > xInt( xRef );
        if( xInt.is() )
        {
            rAny = xInt->queryInterface( ITYPE( style::XStyle ) );
            if( rAny.hasValue() )
                return;
        }
    }

    if( !xStyle.is() )
    {
        xStyle = new SdUnoGraphicStyle( mpModel, pStyleSheet );
        uno::WeakReference<uno::XInterface> wRef(xStyle);
        mpStyles->insert(wRef);
    }

    rAny <<= xStyle;
}

SdUnoGraphicStyle* SdUnoGraphicStyleFamily::getStyleFromAny( const uno::Any& Element )
    throw( lang::IllegalArgumentException  )
{
    SdUnoGraphicStyle* pStyle = NULL;

    if( Element.hasValue() && Element.getValueTypeClass() == uno::TypeClass_INTERFACE )
    {
        uno::Reference< uno::XInterface > xStyle( *(uno::Reference< uno::XInterface > *)Element.getValue() );

        pStyle = SdUnoGraphicStyle::getImplementation( xStyle );
    }

    if( NULL == pStyle )
        throw lang::IllegalArgumentException();

    return pStyle;
}

}
