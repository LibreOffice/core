/*************************************************************************
 *
 *  $RCSfile: unoobj.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-16 16:52:29 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#include <cppuhelper/extract.hxx>
#include <svx/unoprov.hxx>
#include <svx/unoshape.hxx>
#include <svx/svditer.hxx>
#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif

#include "anminfo.hxx"
#include "unohelp.hxx"
#include "unoobj.hxx"
#include "unoprnms.hxx"
#include "unomodel.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "viewshel.hxx"
#include "unokywds.hxx"
#include "unostyls.hxx"
#include "unopsfm.hxx"
#include "unogsfm.hxx"
#include "unopstyl.hxx"
#include "viewshel.hxx"
#include "docshell.hxx"
#include "helpids.h"
#include "glob.hxx"

using namespace ::vos;
using namespace ::rtl;
using namespace ::com::sun::star;

#define WID_EFFECT          1
#define WID_SPEED           2
#define WID_TEXTEFFECT      3
#define WID_BOOKMARK        4
#define WID_CLICKACTION     5
#define WID_PLAYFULL        6
#define WID_SOUNDFILE       7
#define WID_SOUNDON         8
#define WID_BLUESCREEN      9
#define WID_VERB            10
#define WID_DIMCOLOR        11
#define WID_DIMHIDE         12
#define WID_DIMPREV         13
#define WID_PRESORDER       14
#define WID_STYLE           15

#define WID_ISEMPTYPRESOBJ  20
#define WID_ISPRESOBJ       21
#define WID_MASTERDEPEND    22

#define WID_THAT_NEED_ANIMINFO 19

const SfxItemPropertyMap* ImplGetShapePropertyMap( sal_Bool bImpress )
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static const SfxItemPropertyMap aImpress_SdXShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_OBJ_BOOKMARK),      WID_BOOKMARK,        &::getCppuType((const OUString*)0),                        0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_DIMCOLOR),      WID_DIMCOLOR,        &::getCppuType((const sal_Int32*)0),                       0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_DIMHIDE),       WID_DIMHIDE,         &::getBooleanCppuType(),                                   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_DIMPREV),       WID_DIMPREV,         &::getBooleanCppuType(),                                   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_EFFECT),        WID_EFFECT,          &::getCppuType((const presentation::AnimationEffect*)0),   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_ISEMPTYPRESOBJ),WID_ISEMPTYPRESOBJ,  &::getBooleanCppuType(),                                   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_ISPRESOBJ),     WID_ISPRESOBJ,       &::getBooleanCppuType(),                                   ::com::sun::star::beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_MASTERDEPENDENT),WID_MASTERDEPEND,   &::getBooleanCppuType(),                                   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_CLICKACTION),   WID_CLICKACTION,     &::getCppuType((const presentation::ClickAction*)0),       0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_PLAYFULL),      WID_PLAYFULL,        &::getBooleanCppuType(),                                   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_PRESORDER),     WID_PRESORDER,       &::getCppuType((const sal_Int32*)0),                       0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_STYLE),         WID_STYLE,           &ITYPE( style::XStyle),                                    ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_SOUNDFILE),     WID_SOUNDFILE,       &::getCppuType((const OUString*)0),                        0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_SOUNDON),       WID_SOUNDON,         &::getBooleanCppuType(),                                   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_SPEED),         WID_SPEED,           &::getCppuType((const presentation::AnimationSpeed*)0),    0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_TEXTEFFECT),    WID_TEXTEFFECT,      &::getCppuType((const presentation::AnimationEffect*)0),   0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_BLUESCREEN),    WID_BLUESCREEN,      &::getCppuType((const sal_Int32*)0),                       0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_VERB),          WID_VERB,            &::getCppuType((const sal_Int32*)0),                       0, 0},

        { 0,0,0,0,0}
    };

    static const SfxItemPropertyMap aDraw_SdXShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(UNO_NAME_OBJ_BOOKMARK),      WID_BOOKMARK,       &::getCppuType((const OUString*)0),                 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_CLICKACTION),   WID_CLICKACTION,    &::getCppuType((const presentation::ClickAction*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_STYLE),         WID_STYLE,          &ITYPE(style::XStyle),                              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { 0,0,0,0,0}
    };

    if( bImpress )
        return aImpress_SdXShapePropertyMap_Impl;
    else
        return aDraw_SdXShapePropertyMap_Impl;
}

SfxItemPropertyMap aEmpty_SdXShapePropertyMap_Impl[] =
{
    { 0,0,0,0,0}
};

/*************************************************************************
|*
|*    Vergleichsfkt. fuer QSort
|*
\************************************************************************/
struct SortStruct
{
    SdrObject*  pObj;
    sal_uInt32      nOrder;
};

typedef SortStruct  SORT;
typedef SORT*       PSORT;

extern "C" int __LOADONCALLAPI SortFunc( const void* p1, const void* p2 );

SdXShape::SdXShape() throw()
:   maPropSet(aEmpty_SdXShapePropertyMap_Impl),
    mpMap(aEmpty_SdXShapePropertyMap_Impl),
    mpModel(NULL)
{
}

SdXShape::SdXShape(uno::Reference< drawing::XShape > & xShape, SdXImpressDocument* pModel) throw()
:   maPropSet( pModel?ImplGetShapePropertyMap(pModel->IsImpressDocument()):aEmpty_SdXShapePropertyMap_Impl),
    mpMap( pModel?ImplGetShapePropertyMap(pModel->IsImpressDocument()):aEmpty_SdXShapePropertyMap_Impl),
    mpModel(pModel)
{
    m_refCount++;
    {
        uno::Any aAny( xShape->queryInterface( ITYPE( uno::XAggregation ) ) );
        xShape = NULL;

        aAny >>= mxShapeAgg;
    }

    if( mxShapeAgg.is() )
        mxShapeAgg->setDelegator( (::cppu::OWeakObject*)this );

    {
        uno::Any aAny = mxShapeAgg->queryInterface( ITYPE( drawing::XShape ) );
        aAny >>= xShape;
    }

    m_refCount--;
}

SdXShape::~SdXShape() throw()
{
}

// XInterface
uno::Any SAL_CALL SdXShape::queryInterface( const ::com::sun::star::uno::Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;

    if( rType == ::getCppuType((const uno::Reference< beans::XPropertySet >*)0) )
    {
        aAny <<= uno::Reference< beans::XPropertySet >(this);
    }
    else if( rType == ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0) )
    {
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    }
    else
    {
        aAny = OWeakObject::queryInterface(rType);

        if(!aAny.hasValue() && mxShapeAgg.is())
            return mxShapeAgg->queryAggregation( rType );
    }

    return aAny;
}

void SAL_CALL SdXShape::acquire()
    throw(::com::sun::star::uno::RuntimeException)
{
    OWeakObject::acquire();
}

void SAL_CALL SdXShape::release()
    throw(::com::sun::star::uno::RuntimeException)
{
    OWeakObject::release();
}

//XPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL SdXShape::getPropertySetInfo()
    throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference< beans::XPropertySetInfo > aRet;
    if(mxShapeAgg.is())
    {
        uno::Reference< beans::XPropertySet > xPrSet;
        uno::Any aAny( mxShapeAgg->queryAggregation( ITYPE( beans::XPropertySet )));

        if( aAny >>= xPrSet )
        {
            uno::Reference< beans::XPropertySetInfo > xInfo( xPrSet->getPropertySetInfo() );
            // PropertySetInfo verlaengern!
            const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
            aRet = new SfxExtItemPropertySetInfo( mpMap, aPropSeq );
        }
    }
    if(!aRet.is())
        aRet = new SfxItemPropertySetInfo( mpMap );
    return aRet;
}

void SAL_CALL SdXShape::setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxShapeAgg.is())
        throw uno::RuntimeException();

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(aPropertyName);

    if( pMap && GetSdrObject() )
    {
        SdAnimationInfo* pInfo = GetAnimationInfo((pMap->nWID <= WID_THAT_NEED_ANIMINFO)?sal_True:sal_False);

        switch(pMap->nWID)
        {
        case WID_EFFECT:
            ::cppu::any2enum< presentation::AnimationEffect >( pInfo->eEffect, aValue );
            break;
        case WID_TEXTEFFECT:
            ::cppu::any2enum< presentation::AnimationEffect >( pInfo->eTextEffect, aValue);
            break;
        case WID_SPEED:
            ::cppu::any2enum< presentation::AnimationSpeed >( pInfo->eSpeed, aValue);
            break;
        case WID_BOOKMARK:
        {
            OUString aString;
            if(!(aValue >>= aString))
                throw lang::IllegalArgumentException();

            pInfo->aBookmark = aString;
            break;
        }
        case WID_CLICKACTION:
            ::cppu::any2enum< presentation::ClickAction >( pInfo->eClickAction, aValue);
            break;
        case WID_PLAYFULL:
            pInfo->bPlayFull = ::cppu::any2bool(aValue);
            break;
        case WID_SOUNDFILE:
        {
            OUString aString;
            if(!(aValue >>= aString))
                throw lang::IllegalArgumentException();
            pInfo->aSoundFile = aString;
            break;
        }
        case WID_SOUNDON:
            pInfo->bSoundOn = ::cppu::any2bool(aValue);
            break;
        case WID_BLUESCREEN:
        {
            sal_Int32 nColor;
            if(!(aValue >>= nColor))
                throw lang::IllegalArgumentException();

            pInfo->aBlueScreen.SetColor( nColor );
            break;
        }
        case WID_VERB:
        {
            sal_Int32 nVerb;
            if(!(aValue >>= nVerb))
                throw lang::IllegalArgumentException();

            pInfo->nVerb = nVerb;
            break;
        }
        case WID_DIMCOLOR:
        {
            sal_Int32 nColor;
            if(!(aValue >>= nColor))
                throw lang::IllegalArgumentException();

            pInfo->aDimColor.SetColor( (ColorData) nColor );
            break;
        }
        case WID_DIMHIDE:
            pInfo->bDimHide = ::cppu::any2bool(aValue);
            break;
        case WID_DIMPREV:
            pInfo->bDimPrevious = ::cppu::any2bool(aValue);
            break;
        case WID_PRESORDER:
        {
            sal_Int32 nPos;
            if(!(aValue >>= nPos))
                throw lang::IllegalArgumentException();

            SetPresentationOrderPos( nPos );
            break;
        }
        case WID_STYLE:
            SetStyleSheet( aValue );
            break;
        case WID_ISEMPTYPRESOBJ:
            SetEmptyPresObj( ::cppu::any2bool(aValue) );
            break;
        case WID_MASTERDEPEND:
            SetMasterDepend( ::cppu::any2bool(aValue) );
            break;
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet >  xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation( ITYPE( beans::XPropertySet ) ) );
        if( aAny >>= xPrSet)
            xPrSet->setPropertyValue(aPropertyName, aValue);
    }

    if( mpModel )
        mpModel->SetModified();
}

::com::sun::star::uno::Any SAL_CALL SdXShape::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxShapeAgg.is())
        throw uno::RuntimeException();

    uno::Any aRet;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    if( pMap && GetSdrObject() )
    {
        SdAnimationInfo* pInfo = GetAnimationInfo(sal_False);

        switch(pMap->nWID)
        {
        case WID_EFFECT:
            aRet = ::cppu::enum2any< presentation::AnimationEffect >( pInfo?pInfo->eEffect:presentation::AnimationEffect_NONE );
            break;
        case WID_TEXTEFFECT:
            aRet = ::cppu::enum2any< presentation::AnimationEffect >( pInfo?pInfo->eTextEffect:presentation::AnimationEffect_NONE );
            break;
        case WID_ISPRESOBJ:
            aRet = ::cppu::bool2any(IsPresObj());
            break;
        case WID_ISEMPTYPRESOBJ:
            aRet = ::cppu::bool2any(IsEmptyPresObj());
            break;
        case WID_MASTERDEPEND:
            aRet = ::cppu::bool2any(IsMasterDepend());
            break;
        case WID_SPEED:
            aRet = ::cppu::enum2any< presentation::AnimationSpeed >( pInfo?pInfo->eSpeed:presentation::AnimationSpeed_MEDIUM );
            break;
        case WID_BOOKMARK:
        {
            OUString aString;
            if( pInfo )
                aString = pInfo->aBookmark ;
            aRet <<= aString;
            break;
        }
        case WID_CLICKACTION:
            aRet = ::cppu::enum2any< presentation::ClickAction >( pInfo?pInfo->eClickAction:presentation::ClickAction_NONE );
            break;
        case WID_PLAYFULL:
            aRet = ::cppu::bool2any( pInfo && pInfo->bPlayFull );
            break;
        case WID_SOUNDFILE:
        {
            OUString aString;
            if( pInfo )
                aString = pInfo->aSoundFile ;
            aRet <<= aString;
            break;
        }
        case WID_SOUNDON:
            aRet = ::cppu::bool2any( pInfo && pInfo->bSoundOn );
            break;
        case WID_BLUESCREEN:
            aRet <<= (sal_Int32)( pInfo?pInfo->aBlueScreen.GetColor():0x00ffffff );
            break;
        case WID_VERB:
            aRet <<= (sal_Int32)( pInfo?pInfo->nVerb:0 );
            break;
        case WID_DIMCOLOR:
            aRet <<= (sal_Int32)( pInfo?pInfo->aDimColor.GetColor():0x00ffffff );
            break;
        case WID_DIMHIDE:
            aRet = ::cppu::bool2any( pInfo && pInfo->bDimHide );
            break;
        case WID_DIMPREV:
            aRet = ::cppu::bool2any( pInfo && pInfo->bDimPrevious );
            break;
        case WID_PRESORDER:
            aRet <<= (sal_Int32)( GetPresentationOrderPos() );
            break;
        case WID_STYLE:
            aRet = GetStyleSheet();
            break;
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet >  xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertySet >*)0)));

        if( aAny >>= xPrSet)
            aRet = xPrSet->getPropertyValue(PropertyName);
    }

    return aRet;
}

void SAL_CALL SdXShape::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if(mxShapeAgg.is())
    {
        uno::Reference< beans::XPropertySet >  xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertySet >*)0)));
        if( aAny >>= xPrSet)
            xPrSet->addPropertyChangeListener(aPropertyName, xListener);
    }

}

void SAL_CALL SdXShape::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if(mxShapeAgg.is())
    {
        uno::Reference< beans::XPropertySet >  xPrSet;
        uno::Any aAny( mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertySet >*)0)));
        if( aAny >>= xPrSet )
            xPrSet->removePropertyChangeListener(aPropertyName, aListener);
    }
}

void SAL_CALL SdXShape::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if(mxShapeAgg.is())
    {
        uno::Reference< beans::XPropertySet >  xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertySet >*)0)));
        if( aAny >>= xPrSet )
            xPrSet->addVetoableChangeListener(PropertyName, aListener);
    }
}

void SAL_CALL SdXShape::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
    throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    if(mxShapeAgg.is())
    {
        uno::Reference< beans::XPropertySet > xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertySet >*)0)));
        if( aAny >>= xPrSet )
            xPrSet->removeVetoableChangeListener(PropertyName, aListener);
    }
}

/** */
SdAnimationInfo* SdXShape::GetAnimationInfo( sal_Bool bCreate ) const throw()
{
    SdAnimationInfo* pInfo = NULL;

    SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
    if(pDoc)
    {
        SdrObject* pObj = GetSdrObject();
        if(pObj)
        {
            pInfo = pDoc->GetAnimationInfo(pObj);
            if( pInfo == NULL && bCreate )
            {
                pInfo = new SdAnimationInfo(pDoc);
                pObj->InsertUserData( pInfo );
            }
        }
    }

    return pInfo;
}

// XServiceInfo
OUString SAL_CALL SdXShape::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdXShape") );
}

sal_Bool SAL_CALL SdXShape::supportsService( const ::rtl::OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< ::rtl::OUString > SAL_CALL SdXShape::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq;

    uno::Reference< lang::XServiceInfo > xParentInfo;
    if(mxShapeAgg.is())
    {
        uno::Any aAny( mxShapeAgg->queryAggregation( ITYPE( lang::XServiceInfo )));
        if( aAny >>= xParentInfo )
            aSeq = xParentInfo->getSupportedServiceNames();
    }

    SvxServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.presentation.shape",
                                                  "com.sun.star.document.LinkTarget" );

    SdrObject* pObj = GetSdrObject();
    if(pObj && pObj->GetObjInventor() == SdrInventor )
    {
        sal_uInt32 nInventor = pObj->GetObjIdentifier();
        switch( nInventor )
        {
        case OBJ_TITLETEXT:
            SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.TitleTextShape" );
            break;
        case OBJ_OUTLINETEXT:
            SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.OutlinerShape" );
            break;
        }
    }
    return aSeq;
}


/** returns the SdrObject of the aggregatet SvxShape
 */
SdrObject* SdXShape::GetSdrObject() const throw()
{
    if(mxShapeAgg.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( mxShapeAgg );
        if(pShape)
            return pShape->GetSdrObject();
    }

    return NULL;
}

/** checks if this is a presentation object
 */
sal_Bool SdXShape::IsPresObj() const throw()
{
    SdrObject* pObj = GetSdrObject();
    if(pObj)
    {
        SdPage* pPage = PTR_CAST(SdPage,pObj->GetPage());
        if(pPage)
            return pPage->GetPresObjKind(pObj) != PRESOBJ_NONE;
    }
    return sal_False;
}

/** converts this shape to a presentation object or
    a ordinary shape to a presentation object.
void SdXShape::SetPresObj( sal_Bool bPresObj ) throw()
{
    if( IsPresObj() != bPresObj )
    {
        SdrObject* pObj = GetSdrObject();
        if( pObj )
        {
            SdPage* pPage = PTR_CAST(SdPage,pObj->GetPage());
            if(pPage)
            {
                List* pPresObjList = pPage->GetPresObjList();
                if( bPresObj )
                {
                    // convert a shape to a pres obj
                    List* pPresObjList = pPage->GetPresObjList();
                    pPresObjList->Insert( pObj );

                    // check if this is now a valid pres obj
                    if( pPage->GetPresObjKind( pObj ) == PRESOBJ_NONE )
                    {
                        // this is an invalid pres obj, so remove
                        // it from the pres obj list
                        pPresObjList->Remove( pObj );
                    }
                }
                else
                {
                    // convert a pres obj to a shape
                    pObj->SetUserCall( pPage );
                    pPresObjList->Remove(pObj);
                }
            }
        }
    }
}
 */

/** checks if this presentation object is empty
 */
sal_Bool SdXShape::IsEmptyPresObj() const throw()
{
    SdrObject* pObj = GetSdrObject();
    return pObj && pObj->IsEmptyPresObj();
}

/** sets/reset the empty status of a presentation object
*/
void SdXShape::SetEmptyPresObj( sal_Bool bEmpty ) throw()
{
    // only possible if this actually *is* a presentation object
    if( !IsPresObj() )
        return;

    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        if( pObj->IsEmptyPresObj() != bEmpty )
        {
            SdPage* pPage = PTR_CAST(SdPage,pObj->GetPage());
            if(pPage)
            {
                uno::Reference< text::XTextRange > xTextRange( (drawing::XShape*)this, uno::UNO_QUERY );
                if( xTextRange.is() )
                {
                    OUString aEmptyStr;
                    if( bEmpty)
                        aEmptyStr = pPage->GetPresObjText( pPage->GetPresObjKind(pObj) );

                    xTextRange->setString( aEmptyStr );
                }
            }
            pObj->SetEmptyPresObj( bEmpty );
        }
    }
}

sal_Bool SdXShape::IsMasterDepend() const throw()
{
    SdrObject* pObj = GetSdrObject();
    return pObj && pObj->GetUserCall() != NULL;
}

void SdXShape::SetMasterDepend( sal_Bool bDepend ) throw()
{
    if( IsMasterDepend() != bDepend )
    {
        SdrObject* pObj = GetSdrObject();
        if( pObj )
        {
            if( bDepend )
            {
                SdPage* pPage = PTR_CAST(SdPage,pObj->GetPage());
                pObj->SetUserCall( pPage );
            }
            else
            {
                pObj->SetUserCall( NULL );
            }
        }
    }
}

/**
 */
inline sal_Bool IsPathObj( SdrObject* pObj, SdAnimationInfo* pInfo )
{
    // Wenn es sich um das Pfad-Objekt bei dem Effekt "An Kurve entlang"
    // handelt, soll es nicht in der Tabelle aufgenommen werden
    // "bInvisibleInPresentation" ist der einzige Hinweis darauf, ob
    // es sich um das Pfad-Objekt handelt

    const SdrObjKind eKind = (SdrObjKind)pObj->GetObjIdentifier();
    return pInfo->bInvisibleInPresentation &&
           pObj->GetObjInventor() == SdrInventor &&
           (eKind == OBJ_LINE || eKind == OBJ_PLIN || eKind == OBJ_PATHLINE );
}

/** Returns the position of the given SdrObject in the Presentation order.
 *  This function returns -1 if the SdrObject is not in the Presentation order
 *  or if its the path-object.
 */
sal_Int32 SdXShape::GetPresentationOrderPos() const throw()
{
    SdrObject* pObj = GetSdrObject();
    SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
    if(pDoc == NULL || pObj == NULL)
        return -1;

    SdrObjListIter aIter( *pObj->GetPage(), IM_FLAT );


    SdAnimationInfo* pInfo = pDoc->GetAnimationInfo( pObj );
    if(pInfo == NULL || pInfo->bActive == sal_False || IsPathObj( pObj, pInfo ) )
       return -1;

    sal_Int32 nPos = 0;

    while( aIter.IsMore() )
    {
        SdrObject* pIterObj = aIter.Next();
        if(pIterObj == pObj)
            continue;

        SdAnimationInfo* pIterInfo = pDoc->GetAnimationInfo( pIterObj );
        if( pIterInfo )
        {
            if( !IsPathObj(pIterObj, pIterInfo ) )
            {
                if( pIterInfo->nPresOrder < pInfo->nPresOrder )
                    nPos++;
            }
        }
    }

    return nPos;
}


/** Sets the position of the given SdrObject in the Presentation order.
 */
void SdXShape::SetPresentationOrderPos( sal_Int32 nPos ) throw()
{
    SdrObject* pObj = GetSdrObject();
    SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
    if(pDoc == 0 || pObj == NULL)
        return;

    List aAnmList;

    // Erstmal alle animierten Objekte in eine List packen,
    // ausgenommen unser eigenes
    SdrObjListIter aIter( *pObj->GetPage(), IM_FLAT );
    while( aIter.IsMore())
    {
        SdrObject* pIterObj = aIter.Next();
        if( pIterObj != pObj && pDoc->GetAnimationInfo( pIterObj ) )
            aAnmList.Insert(pIterObj, LIST_APPEND);
    }

    const sal_Int32 nCount = aAnmList.Count();

    if ( nCount )
    {
        PSORT   pSort = new SORT[ nCount ];
        sal_Int32   nOrderIndex = 0;
        sal_Int32   nAppendIndex = LIST_APPEND - nCount;

        for( SdrObject* pIterObj = (SdrObject*) aAnmList.First(); pIterObj; pIterObj = (SdrObject*) aAnmList.Next() )
        {
            const SdAnimationInfo*  pInfo = pDoc->GetAnimationInfo( pIterObj );
            PSORT                   pSortTmp = &pSort[ nOrderIndex++ ];

            pSortTmp->pObj = pIterObj;
            pSortTmp->nOrder = ( pInfo->nPresOrder != LIST_APPEND ) ? pInfo->nPresOrder : nAppendIndex++;
        }

        // Liste loeschen, die Information steckt jetzt im Array
        aAnmList.Clear();

        // Array nach Ordnungsnummern sortieren
        qsort( pSort, nCount, sizeof( SORT ), SortFunc );

        // Animationliste neu aufbauen
        for( sal_Int32 i = 0; i < nCount; i++ )
            aAnmList.Insert( pSort[ i ].pObj, LIST_APPEND );

        delete[] pSort;
    }

    aAnmList.Insert(pObj, nPos);

    sal_Int32 nIdx = 0;
    for( SdrObject* pIterObj = (SdrObject*) aAnmList.First(); pIterObj; pIterObj = (SdrObject*) aAnmList.Next() )
    {
        SdAnimationInfo* pInfo = pDoc->GetAnimationInfo( pIterObj );
        pInfo->nPresOrder = nIdx++;
    }
}

void SdXShape::SetStyleSheet( const uno::Any& rAny ) throw( lang::IllegalArgumentException )
{
    SdUnoPseudoStyle* pStyleSheet = NULL;

    if( rAny.hasValue() && rAny.getValueTypeClass() == uno::TypeClass_INTERFACE )
        pStyleSheet = SdUnoPseudoStyle::getImplementation(*(uno::Reference< uno::XInterface > *)rAny.getValue() );

    SdrObject* pObj = GetSdrObject();

    // check if it is a style and if its not a presentation style
    if( NULL == pObj || NULL == pStyleSheet || pStyleSheet->getStyleSheet()->GetFamily() == SFX_STYLE_FAMILY_PSEUDO )
        throw lang::IllegalArgumentException();

    // check if this is a praesentation object by checking the stylesheet
    SfxStyleSheet* pOldStyleSheet = pObj->GetStyleSheet();

    if( pOldStyleSheet &&
        pOldStyleSheet->GetFamily() != SFX_STYLE_FAMILY_PARA &&
        pOldStyleSheet->GetHelpId( String() ) == HID_PSEUDOSHEET_BACKGROUNDOBJECTS )
        throw lang::IllegalArgumentException();

    pObj->SetStyleSheet( (SfxStyleSheet*)pStyleSheet->getStyleSheet(), sal_False );

    SdDrawDocument* pDoc = mpModel? mpModel->GetDoc() : NULL;

    if( pDoc )
    {
        SdDrawDocShell* pDocSh = pDoc->GetDocSh();
        SdViewShell*    pViewSh = pDocSh ? pDocSh->GetViewShell() : NULL;

        if( pViewSh )
            pViewSh->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
    }
}

uno::Any SdXShape::GetStyleSheet() const throw( beans::UnknownPropertyException  )
{
    SdrObject* pObj = GetSdrObject();
    if( pObj == NULL )
        throw beans::UnknownPropertyException();

    uno::Any aAny;

    SfxStyleSheet* pStyleSheet = pObj->GetStyleSheet();
    if(!pStyleSheet)
        return aAny;

    // it is possible for shapes inside a draw to have a presentation style
    // but we don't want this for the api
    if( pStyleSheet->GetFamily() != SFX_STYLE_FAMILY_PARA && !mpModel->IsImpressDocument() )
        return aAny;

    // style::XStyleFamiliesSupplier
    uno::Reference< container::XNameAccess >  xFamilies( mpModel->getStyleFamilies() );

    uno::Reference< style::XStyle >  xStyle;

    if( pStyleSheet->GetFamily() != SFX_STYLE_FAMILY_PARA )
    {
        SdrPage* pPage = pObj->GetPage();
        if( !pPage->IsMasterPage() )
            pPage = pPage->GetMasterPage(0);

        String aLayoutName( pPage->GetLayoutName() );
        aLayoutName = aLayoutName.Erase(aLayoutName.Search( String( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ) )));

        aAny = xFamilies->getByName( aLayoutName );
        uno::Reference< container::XNameAccess >  xStyleFam( *(uno::Reference< container::XNameAccess > *)aAny.getValue() );

        SdUnoPseudoStyleFamily *pStyleFamily = SdUnoPseudoStyleFamily::getImplementation( xStyleFam );
        if( pStyleFamily )
            pStyleFamily->createStyle( pStyleSheet, xStyle );
    }
    else
    {
        const OUString aSFN( OUString::createFromAscii( sUNO_Graphic_Style_Family_Name ) );
        aAny = xFamilies->getByName( aSFN );
        uno::Reference< container::XNameAccess > xStyleFam( *(uno::Reference< container::XNameAccess > *)aAny.getValue() );

        SdUnoGraphicStyleFamily *pStyleFamily = SdUnoGraphicStyleFamily::getImplementation(xStyleFam);

        if( pStyleFamily )
        {
            pStyleFamily->createStyle( pStyleSheet, aAny );
            return aAny;
        }
    }

    aAny.setValue( &xStyle, ITYPE( style::XStyle ) );
    return aAny;
}


