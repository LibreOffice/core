/*************************************************************************
 *
 *  $RCSfile: unoobj.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-15 12:16:27 $
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
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
#ifndef _SVTOOLS_UNOIMAP_HXX
#include <svtools/unoimap.hxx>
#endif
#ifndef _SVTOOLS_UNOEVENT_HXX_
#include <svtools/unoevent.hxx>
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
#include <svx/unoapi.hxx>

#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif

#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif

#ifndef _SD_SPOUTLINER_HXX
#include "sdoutl.hxx"
#endif

#ifndef _SDRESID_HXX
#include "sdresid.hxx"
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
#include "glob.hrc"
#include "unolayer.hxx"
#include "imapinfo.hxx"

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

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
#define WID_ANIMPATH        16
#define WID_IMAGEMAP        17

#define WID_ISEMPTYPRESOBJ  20
#define WID_ISPRESOBJ       21
#define WID_MASTERDEPEND    22

#define WID_THAT_NEED_ANIMINFO 19

const SfxItemPropertyMap* ImplGetShapePropertyMap( sal_Bool bImpress, sal_Bool bGraphicObj )
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    static const SfxItemPropertyMap aImpress_SdXShapePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("ImageMap"),             WID_IMAGEMAP,        &::getCppuType((const uno::Reference< container::XIndexContainer >*)0),    0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_OBJ_ANIMATIONPATH), WID_ANIMPATH,        &ITYPE(drawing::XShape),                                   0, 0},
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
        { MAP_CHAR_LEN("ImageMap"),             WID_IMAGEMAP,        &ITYPE(container::XIndexContainer),    0, 0 },
        { MAP_CHAR_LEN(UNO_NAME_OBJ_BOOKMARK),      WID_BOOKMARK,       &::getCppuType((const OUString*)0),                 0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_CLICKACTION),   WID_CLICKACTION,    &::getCppuType((const presentation::ClickAction*)0),0, 0},
        { MAP_CHAR_LEN(UNO_NAME_OBJ_STYLE),         WID_STYLE,          &ITYPE(style::XStyle),                              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { 0,0,0,0,0}
    };

    if( bImpress )
        return &aImpress_SdXShapePropertyMap_Impl[ bGraphicObj ? 0 : 1 ];
    else
        return &aDraw_SdXShapePropertyMap_Impl[ bGraphicObj ? 0 : 1 ];
}

SfxItemPropertyMap aEmpty_SdXShapePropertyMap_Impl[] =
{
    { 0,0,0,0,0}
};


const SvEventDescription* ImplGetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SFX_EVENT_MOUSEOVER_OBJECT, "OnMouseOver" },
        { SFX_EVENT_MOUSEOUT_OBJECT, "OnMouseOut" },
        { 0, NULL }
    };

    return aMacroDescriptionsImpl;
}

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
:   maPropSet( pModel?
                    ImplGetShapePropertyMap(pModel->IsImpressDocument(), xShape->getShapeType().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_Service_GraphicObjectShape)))
                :   aEmpty_SdXShapePropertyMap_Impl ),
    mpMap( pModel?
                    ImplGetShapePropertyMap(pModel->IsImpressDocument(), xShape->getShapeType().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_Service_GraphicObjectShape)))
                :   aEmpty_SdXShapePropertyMap_Impl ),
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
    else if( rType == ::getCppuType((const uno::Reference< beans::XPropertyState >*)0) )
    {
        aAny <<= uno::Reference< beans::XPropertyState >(this);
    }
    else if( rType == ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0) )
    {
        aAny <<= uno::Reference< lang::XTypeProvider >(this);
    }
    else if( rType == ::getCppuType((const uno::Reference< lang::XServiceInfo >*)0) )
    {
        aAny <<= uno::Reference< lang::XServiceInfo >(this);
    }
    else if( rType == ::getCppuType(( const uno::Reference< document::XEventsSupplier >*)0) )
    {
        aAny <<= uno::Reference< document::XEventsSupplier >(this);
    }
    else
    {
        aAny = OWeakObject::queryInterface(rType);

        if(!aAny.hasValue() && mxShapeAgg.is())
            return mxShapeAgg->queryAggregation( rType );
    }

    return aAny;
}

uno::Sequence< uno::Type > SAL_CALL SdXShape::getTypes()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypeSequence;

    uno::Reference< lang::XTypeProvider > xBaseProvider;
    mxShapeAgg->queryAggregation( ::getCppuType((const uno::Reference< lang::XTypeProvider >*)0) ) >>= xBaseProvider;
    DBG_ASSERT( xBaseProvider.is(), "SdXShape: No XTypeProvider from aggregatet shape!" );

    if( xBaseProvider.is() )
    {
        const uno::Sequence< uno::Type > aBaseTypes( xBaseProvider->getTypes() );
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const sal_Int32 nOwnTypes = 1;      // !DANGER! Keep this updated!

        aTypeSequence.realloc( nBaseTypes  + nOwnTypes );
        uno::Type* pTypes = aTypeSequence.getArray();

        *pTypes++ = ::getCppuType((const uno::Reference< lang::XTypeProvider>*)0);

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }
    return aTypeSequence;
}

// XPropertyState
beans::PropertyState SAL_CALL SdXShape::getPropertyState( const OUString& PropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxShapeAgg.is())
        throw uno::RuntimeException();

    uno::Any aRet;

    if( maPropSet.getPropertyMapEntry(PropertyName) )
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        uno::Reference< beans::XPropertyState >  xPrSet;
        mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertyState >*)0)) >>= xPrSet;

        if(xPrSet.is())
        {
            return xPrSet->getPropertyState(PropertyName);
        }
        else
        {
            return beans::PropertyState_DIRECT_VALUE;
        }
    }
}

uno::Sequence< beans::PropertyState > SAL_CALL SdXShape::getPropertyStates( const uno::Sequence< OUString >& aPropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException)
{
    const sal_Int32 nCount = aPropertyName.getLength();
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aRet( nCount );
    beans::PropertyState* pState = aRet.getArray();;

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
        pState[nIdx] = getPropertyState( pNames[nIdx] );

    return aRet;
}

void SAL_CALL SdXShape::setPropertyToDefault( const OUString& PropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxShapeAgg.is())
        throw uno::RuntimeException();

    uno::Any aRet;

    const SfxItemPropertyMap* pMap = maPropSet.getPropertyMapEntry(PropertyName);

    if( maPropSet.getPropertyMapEntry(PropertyName) )
    {
        return;
    }
    else
    {
        uno::Reference< beans::XPropertyState > xPrSet;
        mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertyState >*)0)) >>= xPrSet;

        if( xPrSet.is() )
        {
            xPrSet->setPropertyToDefault(PropertyName);
        }

    }
}

uno::Any SAL_CALL SdXShape::getPropertyDefault( const OUString& aPropertyName ) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if(!mxShapeAgg.is())
        throw uno::RuntimeException();

    uno::Any aRet;

    if( maPropSet.getPropertyMapEntry(aPropertyName) )
    {
        return getPropertyValue( aPropertyName );
    }
    else
    {
        uno::Reference< beans::XPropertyState >  xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertyState >*)0)));

        if( aAny >>= xPrSet)
        {
            aRet = xPrSet->getPropertyDefault(aPropertyName);

            if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_shape_layername ) ) )
            {
                OUString aName;
                if( aRet >>= aName )
                {
                    aName = SdLayer::convertToExternalName( aName );
                    aRet <<= aName;
                }
            }
        }
    }

    return aRet;
}

uno::Sequence< sal_Int8 > SAL_CALL SdXShape::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
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

    if( pMap )
    {
        SdrObject* pObj = GetSdrObject();
        if( pObj )
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

                    pInfo->nVerb = (USHORT)nVerb;
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
                case WID_ANIMPATH:
                {
                    uno::Reference< drawing::XShape > xShape;
                    aValue >>= xShape;

                    SdrObject* pObj = NULL;
                    if(xShape.is())
                        pObj = GetSdrObjectFromXShape( xShape );

                    if( pObj == NULL || !pObj->ISA( SdrPathObj ) )
                        throw lang::IllegalArgumentException();

                    pInfo->pPathObj = (SdrPathObj*)pObj;

                    SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
                    if( pDoc )
                    {
                        pInfo = pDoc->GetAnimationInfo(pObj);
                        if( pInfo == NULL )
                        {
                            pInfo = new SdAnimationInfo(pDoc);
                            pObj->InsertUserData( pInfo );
                        }
                        pInfo->bInvisibleInPresentation = sal_True;
                    }

                    break;
                }
                case WID_IMAGEMAP:
                {
                    SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
                    if( pDoc )
                    {
                        ImageMap aImageMap;
                        uno::Reference< uno::XInterface > xImageMap;
                        aValue >>= xImageMap;

                        if( !xImageMap.is() || !SvUnoImageMap_fillImageMap( xImageMap, aImageMap ) )
                            throw lang::IllegalArgumentException();

                        SdIMapInfo* pIMapInfo = pDoc->GetIMapInfo(pObj);
                        if( pIMapInfo )
                        {
                            // replace existing image map
                            pIMapInfo->SetImageMap( aImageMap );
                        }
                        else
                        {
                            // insert new user data with image map
                            pObj->InsertUserData(new SdIMapInfo(aImageMap) );
                        }
                    }
                }
            }
        }
    }
    else
    {
        uno::Any aAny( aValue );

        if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_shape_layername ) ) )
        {
            OUString aName;
            if( aAny >>= aName )
            {
                aName = SdLayer::convertToInternalName( aName );
                aAny <<= aName;
            }
        }
        else if( aPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_shape_zorder ) ) )
        {
            SdrObject* pObj = GetSdrObject();
            SdPage* pPage = pObj ? (SdPage*)pObj->GetPage() : NULL;
            if( pPage && pPage->IsMasterPage() && pPage->GetPageKind() == PK_STANDARD )
            {
                sal_Int32 nOrdNum;
                if( aAny >>= nOrdNum )
                {
                    // if this is a masterpage, there is always a background shape with the ord num 0
                    // so we add one to the api ordnum to hide the background shape over the api
                    if( nOrdNum > 0 )
                    {
                        nOrdNum++;
                        aAny <<= nOrdNum;
                    }
                    else
                    {
                        DBG_ERROR( "Masterpage without a background shape, ZOrder property will be corrupt!" );
                    }
                }
            }
        }

        uno::Reference< beans::XPropertySet >  xPrSet;
        mxShapeAgg->queryAggregation( ITYPE( beans::XPropertySet ) ) >>= xPrSet;
        if( xPrSet.is() )
            xPrSet->setPropertyValue(aPropertyName, aAny);
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
        case WID_ANIMPATH:
            if( pInfo && pInfo->pPathObj )
                aRet <<= pInfo->pPathObj->getUnoShape();
            break;
        case WID_IMAGEMAP:
            {
                uno::Reference< uno::XInterface > xImageMap;

                SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
                if( pDoc )
                {

                    SdIMapInfo* pIMapInfo = pDoc->GetIMapInfo(GetSdrObject());
                    if( pIMapInfo )
                    {
                        const ImageMap& rIMap = pIMapInfo->GetImageMap();
                        xImageMap = SvUnoImageMap_createInstance( rIMap, ImplGetSupportedMacroItems() );
                    }
                    else
                    {
                        xImageMap = SvUnoImageMap_createInstance(ImplGetSupportedMacroItems() );
                    }
                }

                aRet <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
                break;
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet >  xPrSet;
        uno::Any aAny(mxShapeAgg->queryAggregation(::getCppuType((const uno::Reference< beans::XPropertySet >*)0)));

        if( aAny >>= xPrSet)
        {
            aRet = xPrSet->getPropertyValue(PropertyName);

            if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_shape_layername ) ) )
            {
                OUString aName;
                if( aRet >>= aName )
                {
                    aName = SdLayer::convertToExternalName( aName );
                    aRet <<= aName;
                }
            }
            else if( PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sUNO_shape_zorder ) ) )
            {
                SdrObject* pObj = GetSdrObject();
                SdPage* pPage = pObj ? (SdPage*)pObj->GetPage() : NULL;
                if( pPage && pPage->IsMasterPage() && pPage->GetPageKind() == PK_STANDARD )
                {
                    sal_Int32 nOrdNum;
                    if( aRet >>= nOrdNum )
                    {
                        // if this is a masterpage, there is always a background shape with the ord num 0
                        // so we add one to the api ordnum to hide the background shape over the api
                        if( nOrdNum > 0 )
                        {
                            nOrdNum--;
                            aRet <<= nOrdNum;
                        }
                        else
                        {
                            DBG_ERROR( "Masterpage without a background shape, ZOrder property will be corrupt!" );
                        }
                    }
                }
            }
        }

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
    if( pObj == NULL )
        return;

    if( pObj->IsEmptyPresObj() != bEmpty )
    {
        if(!bEmpty)
        {
            // really delete SdrOutlinerObj at pObj
            pObj->NbcSetOutlinerParaObject(0L);
        }
        else
        {
            // now set an empty OutlinerParaObject at pObj without
            // any content but with the style of the old OutlinerParaObjects
            // first paragraph
            do
            {
                SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():NULL;
                DBG_ASSERT( pDoc, "no document?" );
                if( pDoc == NULL)
                    break;

                SdOutliner* pOutliner = pDoc->GetInternalOutliner();
                DBG_ASSERT( pOutliner, "no outliner?" );
                if( pOutliner == NULL )
                    break;

                SdPage* pPage = PTR_CAST(SdPage,pObj->GetPage());
                DBG_ASSERT( pPage, "no page?" );
                if( pPage == NULL )
                    break;

                OutlinerParaObject* pOutlinerParaObject = pObj->GetOutlinerParaObject();
                pOutliner->SetText( *pOutlinerParaObject );
                SfxStyleSheetPool* pStyle = pOutliner->GetStyleSheetPool();
                pOutliner->Clear();
                pOutliner->SetStyleSheetPool( (SfxStyleSheetPool*)pDoc->GetStyleSheetPool() );
                pOutliner->SetStyleSheet( 0, pPage->GetTextStyleSheetForObject( pObj ) );
                pOutliner->Insert( pPage->GetPresObjText( pPage->GetPresObjKind(pObj) ) );
                pObj->SetOutlinerParaObject( pOutliner->CreateParaObject() );
                pOutliner->Clear();
            }
            while(0);
        }

        pObj->SetEmptyPresObj(bEmpty);
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

class SdUnoEventsAccess : public cppu::WeakImplHelper2< com::sun::star::container::XNameReplace, com::sun::star::lang::XServiceInfo >
{
private:
    const OUString      maStrOnClick;
    const OUString      maStrServiceName;
    const OUString      maStrEventType;
    const OUString      maStrPresentation;
    const OUString      maStrLibrary;
    const OUString      maStrMacroName;
    const OUString      maStrClickAction;
    const OUString      maStrBookmark;
    const OUString      maStrEffect;
    const OUString      maStrPlayFull;
    const OUString      maStrVerb;
    const OUString      maStrSoundURL;
    const OUString      maStrSpeed;
    const OUString      maStrStarBasic;

    SdXShape*   mpShape;
    uno::Reference< document::XEventsSupplier > mxShape;

public:
    SdUnoEventsAccess( SdXShape* pShape ) throw();

    // XNameReplace
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

// XEventsSupplier
uno::Reference< container::XNameReplace > SAL_CALL SdXShape::getEvents(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return new SdUnoEventsAccess( this );
}

SdUnoEventsAccess::SdUnoEventsAccess( SdXShape* pShape ) throw()
: mpShape( pShape ), mxShape( pShape ),
  maStrOnClick( RTL_CONSTASCII_USTRINGPARAM("OnClick") ),
  maStrServiceName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.documents.Events") ),
  maStrEventType( RTL_CONSTASCII_USTRINGPARAM("EventType") ),
  maStrPresentation( RTL_CONSTASCII_USTRINGPARAM("Presentation") ),
  maStrClickAction( RTL_CONSTASCII_USTRINGPARAM("ClickAction") ),
  maStrBookmark( RTL_CONSTASCII_USTRINGPARAM("Bookmark") ),
  maStrEffect( RTL_CONSTASCII_USTRINGPARAM("Effect") ),
  maStrPlayFull( RTL_CONSTASCII_USTRINGPARAM("PlayFull") ),
  maStrVerb( RTL_CONSTASCII_USTRINGPARAM("Verb") ),
  maStrSoundURL( RTL_CONSTASCII_USTRINGPARAM("SoundURL") ),
  maStrSpeed( RTL_CONSTASCII_USTRINGPARAM("Speed") ),
  maStrStarBasic( RTL_CONSTASCII_USTRINGPARAM("StarBasic") ),
  maStrLibrary(RTL_CONSTASCII_USTRINGPARAM("Library")),
  maStrMacroName(RTL_CONSTASCII_USTRINGPARAM("MacroName"))
{
}

#define FOUND_CLICKACTION   0x0001
#define FOUND_BOOKMARK      0x0002
#define FOUND_EFFECT        0x0004
#define FOUND_PLAYFULL      0x0008
#define FOUND_VERB          0x0010
#define FOUND_SOUNDURL      0x0020
#define FOUND_SPEED         0x0040
#define FOUND_EVENTTYPE     0x0080
#define FOUND_MACRO         0x0100
#define FOUND_LIBRARY       0x0200

static void clearEventsInAnimationInfo( SdAnimationInfo* pInfo )
{
    const String aEmpty;
    pInfo->aBookmark = aEmpty;
    pInfo->bSecondSoundOn = sal_False;
    pInfo->bSecondPlayFull = sal_False;
    pInfo->eClickAction = presentation::ClickAction_NONE;
    pInfo->eSecondEffect = presentation::AnimationEffect_NONE;
    pInfo->eSecondSpeed = presentation::AnimationSpeed_MEDIUM;
    pInfo->nVerb = 0;
}

// XNameReplace
void SAL_CALL SdUnoEventsAccess::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpShape == NULL || aName != maStrOnClick )
        throw container::NoSuchElementException();

    uno::Sequence< beans::PropertyValue > aProperties;
    if( !aElement.hasValue() || aElement.getValueType() != getElementType() || !(aElement >>= aProperties) )
        throw lang::IllegalArgumentException();

    sal_Int32 nFound = 0;
    const beans::PropertyValue* pProperties = aProperties.getConstArray();

    OUString aStrEventType;
    presentation::ClickAction eClickAction;
    presentation::AnimationEffect eEffect;
    presentation::AnimationSpeed eSpeed;
    OUString aStrSoundURL;
    sal_Bool bPlayFull;
    sal_Int32 nVerb;
    OUString aStrMacro;
    OUString aStrLibrary;
    OUString aStrBookmark;

    const sal_Int32 nCount = aProperties.getLength();
    sal_Int32 nIndex;
    for( nIndex = 0; nIndex < nCount; nIndex++, pProperties++ )
    {
        if( ( ( nFound & FOUND_EVENTTYPE ) == 0 ) && pProperties->Name == maStrEventType )
        {
            if( pProperties->Value >>= aStrEventType )
            {
                nFound |= FOUND_EVENTTYPE;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_CLICKACTION ) == 0 ) && pProperties->Name == maStrClickAction )
        {
            if( pProperties->Value >>= eClickAction )
            {
                nFound |= FOUND_CLICKACTION;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_MACRO ) == 0 ) && pProperties->Name == maStrMacroName )
        {
            if( pProperties->Value >>= aStrMacro )
            {
                nFound |= FOUND_MACRO;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_LIBRARY ) == 0 ) && pProperties->Name == maStrLibrary )
        {
            if( pProperties->Value >>= aStrLibrary )
            {
                nFound |= FOUND_LIBRARY;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_EFFECT ) == 0 ) && pProperties->Name == maStrEffect )
        {
            if( pProperties->Value >>= eEffect )
            {
                nFound |= FOUND_EFFECT;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_BOOKMARK ) == 0 ) && pProperties->Name == maStrBookmark )
        {
            if( pProperties->Value >>= aStrBookmark )
            {
                nFound |= FOUND_BOOKMARK;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_SPEED ) == 0 ) && pProperties->Name == maStrSpeed )
        {
            if( pProperties->Value >>= eSpeed )
            {
                nFound |= FOUND_SPEED;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_SOUNDURL ) == 0 ) && pProperties->Name == maStrSoundURL )
        {
            if( pProperties->Value >>= aStrSoundURL )
            {
                nFound |= FOUND_SOUNDURL;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_PLAYFULL ) == 0 ) && pProperties->Name == maStrPlayFull )
        {
            if( pProperties->Value >>= bPlayFull )
            {
                nFound |= FOUND_PLAYFULL;
                continue;
            }
        }
        else if( ( ( nFound & FOUND_VERB ) == 0 ) && pProperties->Name == maStrVerb )
        {
            if( pProperties->Value >>= nVerb )
            {
                nFound |= FOUND_VERB;
                continue;
            }
        }

        throw lang::IllegalArgumentException();
    }

    sal_Bool bOk = sal_False;
    do
    {
        if( ( nFound & FOUND_EVENTTYPE ) == 0 )
            break;

        if( aStrEventType == maStrPresentation )
        {
            if( ( nFound & FOUND_CLICKACTION ) == 0 )
                break;

            SdAnimationInfo* pInfo = mpShape->GetAnimationInfo( sal_False );
            if( presentation::ClickAction_NONE == eClickAction && NULL == pInfo )
            {
                bOk = sal_True;
                break;
            }

            if( NULL == pInfo )
                pInfo = mpShape->GetAnimationInfo( sal_True );

            DBG_ASSERT( pInfo, "shape animation info could not be created!" );
            if( NULL == pInfo )
                break;

            clearEventsInAnimationInfo( pInfo );
            pInfo->eClickAction = eClickAction;

            switch( eClickAction )
            {
            case presentation::ClickAction_NONE:
            case presentation::ClickAction_PREVPAGE:
            case presentation::ClickAction_NEXTPAGE:
            case presentation::ClickAction_FIRSTPAGE:
            case presentation::ClickAction_LASTPAGE:
            case presentation::ClickAction_INVISIBLE:
            case presentation::ClickAction_STOPPRESENTATION:
                {
                    bOk = sal_True;
                }
                break;

            case presentation::ClickAction_PROGRAM:
            case presentation::ClickAction_BOOKMARK:
            case presentation::ClickAction_DOCUMENT:
                if( nFound & FOUND_BOOKMARK )
                {
                    if( eClickAction == presentation::ClickAction_BOOKMARK )
                    {
                        const OUString aApiPageName( RTL_CONSTASCII_USTRINGPARAM("page") );

                        if( aStrBookmark.indexOf( aApiPageName ) == 0 )
                        {
                            if( aStrBookmark.indexOf( aApiPageName ) == 0 )
                            {
                                sal_Int32 nPageNumber = aStrBookmark.copy( aApiPageName.getLength() ).toInt32();
                                OUStringBuffer sBuffer;
                                String aPageName( SdResId(STR_PAGE) );
                                sBuffer.append( aPageName );
                                sBuffer.append( sal_Unicode( ' ' ) );
                                sBuffer.append( nPageNumber );
                                aStrBookmark = sBuffer.makeStringAndClear();
                            }
                        }
                    }

                    pInfo->aBookmark = aStrBookmark;
                    bOk = sal_True;
                }
                break;

            case presentation::ClickAction_MACRO:
                if( nFound & FOUND_MACRO )
                {
                    pInfo->aBookmark = aStrMacro;
                    bOk = sal_True;
                }
                break;

            case presentation::ClickAction_VERB:
                if( nFound & FOUND_VERB )
                {
                    pInfo->nVerb = (USHORT)nVerb;
                    bOk = sal_True;
                }
                break;

            case presentation::ClickAction_VANISH:
                if( ( nFound & FOUND_EFFECT ) == 0 )
                    break;

                pInfo->eSecondEffect = eEffect;
                pInfo->eSecondSpeed = nFound & FOUND_SPEED ? eSpeed : presentation::AnimationSpeed_MEDIUM;

                bOk = sal_True;

                // NOTE: No break here!!!

            case presentation::ClickAction_SOUND:
                if( nFound & FOUND_SOUNDURL )
                {
                    pInfo->aBookmark = aStrSoundURL;
                    if( eClickAction != presentation::ClickAction_SOUND )
                        pInfo->bSecondSoundOn = aStrSoundURL.getLength() != 0;
                    pInfo->bSecondPlayFull = nFound & FOUND_PLAYFULL ? bPlayFull : sal_False;

                    bOk = sal_True;
                }
                break;
            }
        }
        else
        {
            SdAnimationInfo* pInfo = mpShape->GetAnimationInfo( sal_True );

            DBG_ASSERT( pInfo, "shape animation info could not be created!" );
            if( NULL == pInfo )
                break;

            clearEventsInAnimationInfo( pInfo );
            pInfo->eClickAction = presentation::ClickAction_MACRO;

            String aMacro = aStrMacro;

            String aLibName   = aMacro.GetToken(0, sal_Unicode('.'));
            String aModulName = aMacro.GetToken(1, sal_Unicode('.'));
            String aMacroName = aMacro.GetToken(2, sal_Unicode('.'));

            OUStringBuffer sBuffer;
            sBuffer.append( aMacroName );
            sBuffer.append( sal_Unicode('.') );
            sBuffer.append( aModulName );
            sBuffer.append( sal_Unicode('.') );
            sBuffer.append( aLibName );
            sBuffer.append( sal_Unicode('.') );

            if( aStrLibrary.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StarOffice" ) ) )
            {
                sBuffer.append( OUString( RTL_CONSTASCII_USTRINGPARAM( "BASIC" ) ) );
            }
            else
            {
                sBuffer.append( aStrLibrary );
            }

            pInfo->aBookmark = sBuffer.makeStringAndClear();
            bOk = sal_True;
        }
    }
    while(0);

    if( !bOk )
        throw lang::IllegalArgumentException();
}

// XNameAccess
uno::Any SAL_CALL SdUnoEventsAccess::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( mpShape == NULL || aName != maStrOnClick )
        throw container::NoSuchElementException();

    SdAnimationInfo* pInfo = mpShape->GetAnimationInfo( sal_False );

    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    if( pInfo )
        eClickAction = pInfo->eClickAction;

    sal_Int32 nPropertyCount = 2;
    switch( eClickAction )
    {
    case presentation::ClickAction_NONE:
    case presentation::ClickAction_PREVPAGE:
    case presentation::ClickAction_NEXTPAGE:
    case presentation::ClickAction_FIRSTPAGE:
    case presentation::ClickAction_LASTPAGE:
    case presentation::ClickAction_INVISIBLE:
    case presentation::ClickAction_STOPPRESENTATION:
        break;
    case presentation::ClickAction_PROGRAM:
    case presentation::ClickAction_VERB:
    case presentation::ClickAction_BOOKMARK:
    case presentation::ClickAction_DOCUMENT:
    case presentation::ClickAction_MACRO:
        nPropertyCount += 1;
        break;

    case presentation::ClickAction_SOUND:
        nPropertyCount += 2;
        break;

    case presentation::ClickAction_VANISH:
        nPropertyCount += 4;
        break;
    }

    uno::Sequence< beans::PropertyValue > aProperties( nPropertyCount );
    beans::PropertyValue* pProperties = aProperties.getArray();

    uno::Any aAny;

    if( eClickAction == presentation::ClickAction_MACRO )
    {
        aAny <<= maStrStarBasic;;
        pProperties->Name = maStrEventType;
        pProperties->Handle = -1;
        pProperties->Value = aAny;
        pProperties->State = beans::PropertyState_DIRECT_VALUE;
        pProperties++;

        String aMacro = pInfo->aBookmark;

        // aMacro has got following format:
        // "Macroname.Modulname.Libname.Documentname" or
        // "Macroname.Modulname.Libname.Applicationsname"
        String aMacroName = aMacro.GetToken(0, sal_Unicode('.'));
        String aModulName = aMacro.GetToken(1, sal_Unicode('.'));
        String aLibName   = aMacro.GetToken(2, sal_Unicode('.'));
        String aDocName   = aMacro.GetToken(3, sal_Unicode('.'));

        OUStringBuffer sBuffer;
        sBuffer.append( aLibName );
        sBuffer.append( sal_Unicode('.') );
        sBuffer.append( aModulName );
        sBuffer.append( sal_Unicode('.') );
        sBuffer.append( aMacroName );

        aAny <<= OUString( sBuffer.makeStringAndClear() );
        pProperties->Name = maStrMacroName;
        pProperties->Handle = -1;
        pProperties->Value = aAny;
        pProperties->State = beans::PropertyState_DIRECT_VALUE;
        pProperties++;

        aAny <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice" ) );
        pProperties->Name = maStrLibrary;
        pProperties->Handle = -1;
        pProperties->Value = aAny;
        pProperties->State = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        aAny <<= maStrPresentation;
        pProperties->Name = maStrEventType;
        pProperties->Handle = -1;
        pProperties->Value = aAny;
        pProperties->State = beans::PropertyState_DIRECT_VALUE;
        pProperties++;

        aAny <<= eClickAction;
        pProperties->Name = maStrClickAction;
        pProperties->Handle = -1;
        pProperties->Value = aAny;
        pProperties->State = beans::PropertyState_DIRECT_VALUE;
        pProperties++;

        switch( eClickAction )
        {
        case presentation::ClickAction_NONE:
        case presentation::ClickAction_PREVPAGE:
        case presentation::ClickAction_NEXTPAGE:
        case presentation::ClickAction_FIRSTPAGE:
        case presentation::ClickAction_LASTPAGE:
        case presentation::ClickAction_INVISIBLE:
        case presentation::ClickAction_STOPPRESENTATION:
            break;
        case presentation::ClickAction_BOOKMARK:
            {
                String aPageName( SdResId(STR_PAGE) );
                aPageName += sal_Unicode( ' ' );

                const OUString aStrBookmark( pInfo->aBookmark );
                if( aStrBookmark.indexOf( aPageName ) == 0 )
                {
                    sal_Int32 nPageNumber = aStrBookmark.copy( aPageName.Len() ).toInt32();
                    OUStringBuffer sBuffer;
                    sBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "page" ) );
                    sBuffer.append( nPageNumber );
                    aAny <<= sBuffer.makeStringAndClear();
                }
                else
                {
                    aAny <<= aStrBookmark;
                }

                pProperties->Name = maStrBookmark;
                pProperties->Handle = -1;
                pProperties->Value = aAny;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_DOCUMENT:
        case presentation::ClickAction_PROGRAM:
            aAny <<= OUString( pInfo->aBookmark );
            pProperties->Name = maStrBookmark;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            break;

        case presentation::ClickAction_VANISH:
            aAny <<= pInfo->eSecondEffect;
            pProperties->Name = maStrEffect;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= pInfo->eSecondSpeed;
            pProperties->Name = maStrSpeed;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            // NOTE: no break here!!!

        case presentation::ClickAction_SOUND:
            if( eClickAction == presentation::ClickAction_SOUND || pInfo->bSecondSoundOn )
            {
                aAny <<= OUString( pInfo->aBookmark );
                pProperties->Name = maStrSoundURL;
                pProperties->Handle = -1;
                pProperties->Value = aAny;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = maStrPlayFull;
                pProperties->Handle = -1;
                pProperties->Value = ::cppu::bool2any(pInfo->bSecondPlayFull);
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_VERB:
            aAny <<= (sal_Int32)pInfo->nVerb;
            pProperties->Name = maStrVerb;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            break;
        }
    }

    aAny <<= aProperties;
    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdUnoEventsAccess::getElementNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aStr( &maStrOnClick, 1 );
    return aStr;
}

sal_Bool SAL_CALL SdUnoEventsAccess::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    return aName == maStrOnClick;
}

// XElementAccess
uno::Type SAL_CALL SdUnoEventsAccess::getElementType(  )
    throw(uno::RuntimeException)
{
    return *SEQTYPE(::getCppuType((const uno::Sequence< beans::PropertyValue >*)0));
}

sal_Bool SAL_CALL SdUnoEventsAccess::hasElements(  ) throw(uno::RuntimeException)
{
    return sal_True;
}

// XServiceInfo
OUString SAL_CALL SdUnoEventsAccess::getImplementationName(  )
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdUnoEventsAccess" ) );
}

sal_Bool SAL_CALL SdUnoEventsAccess::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return ServiceName == maStrServiceName;
}

uno::Sequence< OUString > SAL_CALL SdUnoEventsAccess::getSupportedServiceNames(  )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aStr( &maStrServiceName, 1 );
    return aStr;
}
