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

#include <bf_svx/dialogs.hrc>              // fuer SID_REHEARSE_TIMINGS
#include <bf_svx/unoprov.hxx>
#include <vcl/svapp.hxx>

#include "unopres.hxx"
#include "unocpres.hxx"
#include "bf_sd/docshell.hxx"
#include "sdattr.hxx"
#include "cusshow.hxx"
#include "unoprnms.hxx"
#include "unohelp.hxx"
#include "unopage.hxx"

#include <vos/mutex.hxx>

namespace binfilter {

using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star;

const SfxItemPropertyMap* ImplGetPresentationPropertyMap()
{
    static const SfxItemPropertyMap aPresentationPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("IsShowAll"),				ATTR_PRESENT_ALL,				&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_ALLOWANIM),	ATTR_PRESENT_ANIMATION_ALLOWED,	&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_CUSTOMSHOW),	ATTR_PRESENT_CUSTOMSHOW,		&::getCppuType((const OUString*)0),		0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_FIRSTPAGE),	ATTR_PRESENT_DIANAME,			&::getCppuType((const OUString*)0),		0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_ONTOP),		ATTR_PRESENT_ALWAYS_ON_TOP,		&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_AUTOMATIC),	ATTR_PRESENT_MANUEL,			&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_ENDLESS),		ATTR_PRESENT_ENDLESS,			&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_FULLSCREEN),	ATTR_PRESENT_FULLSCREEN,		&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_MOUSEVISIBLE),ATTR_PRESENT_MOUSE,				&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_PAUSE),		ATTR_PRESENT_PAUSE_TIMEOUT,		&::getCppuType((const sal_Int32*)0),	0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_STARTWITHNAV),ATTR_PRESENT_NAVIGATOR,			&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_SHOW_USEPEN),		ATTR_PRESENT_PEN,				&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN("IsTransitionOnClick"),		ATTR_PRESENT_CHANGE_PAGE,		&::getBooleanCppuType(),				0, 0 },
        { MAP_CHAR_LEN("IsShowLogo"),				ATTR_PRESENT_SHOW_PAUSELOGO,	&::getBooleanCppuType(),				0, 0 },
        { 0,0,0,0,0}

    };

    return aPresentationPropertyMap_Impl;
}

SfxItemPropertyMap map_impl[] = { 0,0,0,0 };

///////////////////////////////////////////////////////////////////////////////

SdXPresentation::SdXPresentation(SdXImpressDocument& rMyModel) throw()
:	maPropSet(ImplGetPresentationPropertyMap()), mrModel(rMyModel)
{
}

SdXPresentation::~SdXPresentation() throw()
{
}

// XServiceInfo
OUString SAL_CALL SdXPresentation::getImplementationName(  )
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdXPresentation") );
}

sal_Bool SAL_CALL SdXPresentation::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames(  ) );
}

uno::Sequence< OUString > SAL_CALL SdXPresentation::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.Presentation") );
    uno::Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdXPresentation::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );
    return maPropSet.getPropertySetInfo();
 }

void SAL_CALL SdXPresentation::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    SdDrawDocument* pDoc = mrModel.GetDoc();
    if(pDoc == NULL)
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    sal_Bool bValuesChanged = sal_False;

    switch( pMap ? pMap->nWID : -1 )
    {
    case ATTR_PRESENT_ALL:
    {
        sal_Bool bVal;
        
        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if( pDoc->GetPresAll() != bVal )
        {
            pDoc->SetPresAll( bVal );
            bValuesChanged = sal_True;
            if( bVal )
                pDoc->SetCustomShow( false );
        }
        break;
    }
    case ATTR_PRESENT_CHANGE_PAGE:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if ( bVal == pDoc->GetPresLockedPages() )
        {
            bValuesChanged = sal_True;
            pDoc->SetPresLockedPages( !bVal );
        }
        break;
    }

    case ATTR_PRESENT_ANIMATION_ALLOWED:
    {
        sal_Bool bVal;
        
        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->IsAnimationAllowed() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetAnimationAllowed(bVal);
        }
        break;
    }
    case ATTR_PRESENT_CUSTOMSHOW:
    {
        OUString aShow;
        if(!(aValue >>= aShow))
            throw lang::IllegalArgumentException();

        const String aShowName( aShow );

        List* pCustomShowList = pDoc->GetCustomShowList(sal_False);
        if(pCustomShowList)
        {
            SdCustomShow* pCustomShow;
            for( pCustomShow = (SdCustomShow*) pCustomShowList->First(); pCustomShow != NULL; pCustomShow = (SdCustomShow*) pCustomShowList->Next() )
            {
                if( pCustomShow->GetName() == aShowName )
                    break;
            }

            pDoc->SetCustomShow( true );
            bValuesChanged = sal_True;
        }
        break;
    }
    case ATTR_PRESENT_ENDLESS:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetPresEndless() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetPresEndless(bVal);
        }
        break;
    }
    case ATTR_PRESENT_FULLSCREEN:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetPresFullScreen() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetPresFullScreen(bVal);
        }
        break;
    }
    case ATTR_PRESENT_DIANAME:
    {
        OUString aPresPage;
        
        aValue >>= aPresPage;
        pDoc->SetPresPage(SdDrawPage::getUiNameFromPageApiName(aPresPage));
        pDoc->SetCustomShow(false);
        pDoc->SetPresAll(false);

        bValuesChanged = sal_True;
        break;
    }
    case ATTR_PRESENT_MANUEL:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetPresManual() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetPresManual(bVal);
        }
        break;
    }
    case ATTR_PRESENT_MOUSE:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetPresMouseVisible() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetPresMouseVisible(bVal);
        }
        break;
    }
    case ATTR_PRESENT_ALWAYS_ON_TOP:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetPresAlwaysOnTop() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetPresAlwaysOnTop(bVal);
        }
        break;
    }
    case ATTR_PRESENT_NAVIGATOR:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetStartPresWithNavigator() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetStartPresWithNavigator(bVal);
        }
        break;
    }
    case ATTR_PRESENT_PEN:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        if(pDoc->GetPresMouseAsPen() != bVal)
        {
            bValuesChanged = sal_True;
            pDoc->SetPresMouseAsPen(bVal);
        }
        break;
    }
    case ATTR_PRESENT_PAUSE_TIMEOUT:
    {
        sal_Int32 nValue;
        if(!(aValue >>= nValue) || (nValue < 0))
            throw lang::IllegalArgumentException();

        pDoc->SetPresPause( (sal_uInt32)nValue );
        break;
    }
    case ATTR_PRESENT_SHOW_PAUSELOGO:
    {
        sal_Bool bVal;

        if(! sd::any2bool( aValue, bVal ) )
            throw lang::IllegalArgumentException();

        pDoc->SetPresShowLogo( bVal );
    }
    }

    mrModel.SetModified();
}

uno::Any SAL_CALL SdXPresentation::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any any;

    SdDrawDocument* pDoc = mrModel.GetDoc();
    if(pDoc == NULL)
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    switch( pMap ? pMap->nWID : -1 )
    {
    case ATTR_PRESENT_ALL:
        any <<= (sal_Bool) ( !pDoc->IsCustomShow() && pDoc->GetPresAll() );
        break;
    case ATTR_PRESENT_CHANGE_PAGE:
        sd::bool2any( !pDoc->GetPresLockedPages(), any );
        break;
    case ATTR_PRESENT_ANIMATION_ALLOWED:
        sd::bool2any( pDoc->IsAnimationAllowed(), any );
        break;
    case ATTR_PRESENT_CUSTOMSHOW:
    {
        List* pList = pDoc->GetCustomShowList(sal_False);
        SdCustomShow* pShow = (pList && pDoc->IsCustomShow())?(SdCustomShow*)pList->GetCurObject():NULL;
        OUString aShowName;

        if(pShow)
            aShowName = pShow->GetName();

        any <<= aShowName;
        break;
    }
    case ATTR_PRESENT_ENDLESS:
        sd::bool2any( pDoc->GetPresEndless(), any );
        break;
    case ATTR_PRESENT_FULLSCREEN:
        sd::bool2any( pDoc->GetPresFullScreen(), any );
        break;
    case ATTR_PRESENT_DIANAME:
        {
            OUString aSlideName;

            if( !pDoc->IsCustomShow() && !pDoc->GetPresAll() )
                aSlideName = SdDrawPage::getPageApiNameFromUiName( pDoc->GetPresPage() );

            any <<= aSlideName;
        }
        break;
    case ATTR_PRESENT_MANUEL:
        sd::bool2any( pDoc->GetPresManual(), any );
        break;
    case ATTR_PRESENT_MOUSE:
        sd::bool2any( pDoc->GetPresMouseVisible(), any );
        break;
    case ATTR_PRESENT_ALWAYS_ON_TOP:
        sd::bool2any( pDoc->GetPresAlwaysOnTop(), any );
        break;
    case ATTR_PRESENT_NAVIGATOR:
        sd::bool2any( pDoc->GetStartPresWithNavigator(), any );
        break;
    case ATTR_PRESENT_PEN:
        sd::bool2any( pDoc->GetPresMouseAsPen(), any );
        break;
    case ATTR_PRESENT_PAUSE_TIMEOUT:
        any <<= (sal_Int32)pDoc->GetPresPause();
        break;
    case ATTR_PRESENT_SHOW_PAUSELOGO:
        sd::bool2any( pDoc->IsPresShowLogo(), any );
        break;
    }

    return any;
}

void SAL_CALL SdXPresentation::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener )	throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void SAL_CALL SdXPresentation::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener )	throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void SAL_CALL SdXPresentation::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )	throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void SAL_CALL SdXPresentation::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener )	throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

// XPresentation
void SAL_CALL SdXPresentation::start(  ) throw(uno::RuntimeException)
{
}

void SAL_CALL SdXPresentation::end(  ) throw(uno::RuntimeException)
{
}

void SAL_CALL SdXPresentation::rehearseTimings(  ) throw(uno::RuntimeException)
{
}


}
