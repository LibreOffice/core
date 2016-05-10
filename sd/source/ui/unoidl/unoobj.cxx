/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <utility>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <svl/itemprop.hxx>
#include <svl/style.hxx>
#include <svx/svdpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <svtools/unoimap.hxx>
#include <svtools/unoevent.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxsids.hrc>
#include <comphelper/extract.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/unoprov.hxx>
#include <svx/unoshape.hxx>
#include <svx/svditer.hxx>
#include <svx/svdotext.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <svl/instrm.hxx>
#include <editeng/outlobj.hxx>
#include "CustomAnimationPreset.hxx"
#include "Outliner.hxx"
#include "sdresid.hxx"
#include <comphelper/serviceinfohelper.hxx>
#include <svx/svdogrp.hxx>

#include "anminfo.hxx"
#include "unohelp.hxx"
#include "unoobj.hxx"
#include "unoprnms.hxx"
#include "unomodel.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "unokywds.hxx"
#include "unopage.hxx"
#include "DrawDocShell.hxx"
#include "helpids.h"
#include "glob.hxx"
#include "glob.hrc"
#include "unolayer.hxx"
#include "imapinfo.hxx"
#include "EffectMigration.hxx"

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::animations;

using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::drawing::XShape;

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
#define WID_ISANIMATION     18
#define WID_THAT_NEED_ANIMINFO 19

#define WID_ISEMPTYPRESOBJ  20
#define WID_ISPRESOBJ       21
#define WID_MASTERDEPEND    22

#define WID_NAVORDER        23
#define WID_PLACEHOLDERTEXT 24
#define WID_LEGACYFRAGMENT  25

        #define IMPRESS_MAP_ENTRIES \
        { OUString(UNO_NAME_OBJ_LEGACYFRAGMENT),WID_LEGACYFRAGMENT,  cppu::UnoType<drawing::XShape>::get(),                 0, 0},\
        { OUString(UNO_NAME_OBJ_ANIMATIONPATH), WID_ANIMPATH,        cppu::UnoType<drawing::XShape>::get(),                 0, 0},\
        { OUString(UNO_NAME_OBJ_BOOKMARK),      WID_BOOKMARK,        cppu::UnoType<OUString>::get(),                        0, 0},\
        { OUString(UNO_NAME_OBJ_DIMCOLOR),      WID_DIMCOLOR,        cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { OUString(UNO_NAME_OBJ_DIMHIDE),       WID_DIMHIDE,         cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString(UNO_NAME_OBJ_DIMPREV),       WID_DIMPREV,         cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString(UNO_NAME_OBJ_EFFECT),        WID_EFFECT,          cppu::UnoType<presentation::AnimationEffect>::get(),   0, 0},\
        { OUString(UNO_NAME_OBJ_ISEMPTYPRESOBJ),WID_ISEMPTYPRESOBJ,  cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString(UNO_NAME_OBJ_ISPRESOBJ),     WID_ISPRESOBJ,       cppu::UnoType<bool>::get(),                            css::beans::PropertyAttribute::READONLY, 0},\
        { OUString(UNO_NAME_OBJ_MASTERDEPENDENT),WID_MASTERDEPEND,   cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString(UNO_NAME_OBJ_CLICKACTION),   WID_CLICKACTION,     cppu::UnoType<presentation::ClickAction>::get(),       0, 0},\
        { OUString(UNO_NAME_OBJ_PLAYFULL),      WID_PLAYFULL,        cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString(UNO_NAME_OBJ_PRESORDER),     WID_PRESORDER,       cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { OUString(UNO_NAME_OBJ_STYLE),         WID_STYLE,           cppu::UnoType<style::XStyle>::get(),                   css::beans::PropertyAttribute::MAYBEVOID, 0},\
        { OUString(UNO_NAME_OBJ_SOUNDFILE),     WID_SOUNDFILE,       cppu::UnoType<OUString>::get(),                        0, 0},\
        { OUString(UNO_NAME_OBJ_SOUNDON),       WID_SOUNDON,         cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString(UNO_NAME_OBJ_SPEED),         WID_SPEED,           cppu::UnoType<presentation::AnimationSpeed>::get(),    0, 0},\
        { OUString(UNO_NAME_OBJ_TEXTEFFECT),    WID_TEXTEFFECT,      cppu::UnoType<presentation::AnimationEffect>::get(),   0, 0},\
        { OUString(UNO_NAME_OBJ_BLUESCREEN),    WID_BLUESCREEN,      cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { OUString(UNO_NAME_OBJ_VERB),          WID_VERB,            cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { OUString("IsAnimation"),              WID_ISANIMATION,     cppu::UnoType<bool>::get(),                            0, 0},\
        { OUString("NavigationOrder"),          WID_NAVORDER,        cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { OUString("PlaceholderText"),          WID_PLACEHOLDERTEXT, cppu::UnoType<OUString>::get(),                        0, 0},\
        { OUString(), 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry* lcl_GetImpress_SdXShapePropertyGraphicMap_Impl()
    {

        static const SfxItemPropertyMapEntry aImpress_SdXShapePropertyGraphicMap_Impl[] =
        {
            { OUString("ImageMap"),             WID_IMAGEMAP,        cppu::UnoType<container::XIndexContainer>::get(),    0, 0 },
            IMPRESS_MAP_ENTRIES
        };
        return aImpress_SdXShapePropertyGraphicMap_Impl;
    }

    static const SfxItemPropertyMapEntry* lcl_GetImpress_SdXShapePropertySimpleMap_Impl()
    {

        static const SfxItemPropertyMapEntry aImpress_SdXShapePropertySimpleMap_Impl[] =
        {
            IMPRESS_MAP_ENTRIES
        };
        return aImpress_SdXShapePropertySimpleMap_Impl;
    }

    #define DRAW_MAP_ENTRIES\
        { OUString(UNO_NAME_OBJ_BOOKMARK),      WID_BOOKMARK,       cppu::UnoType<OUString>::get(),                 0, 0},\
        { OUString(UNO_NAME_OBJ_CLICKACTION),   WID_CLICKACTION,    cppu::UnoType<presentation::ClickAction>::get(),0, 0},\
        { OUString(UNO_NAME_OBJ_STYLE),         WID_STYLE,          cppu::UnoType<style::XStyle>::get(),            css::beans::PropertyAttribute::MAYBEVOID, 0},\
        { OUString("NavigationOrder"),          WID_NAVORDER,       cppu::UnoType<sal_Int32>::get(),                0, 0},\
        { OUString(), 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry* lcl_GetDraw_SdXShapePropertySimpleMap_Impl()
    {
        static const SfxItemPropertyMapEntry aDraw_SdXShapePropertyMap_Impl[] =
        {
            DRAW_MAP_ENTRIES
        };
        return aDraw_SdXShapePropertyMap_Impl;
    }
    static const SfxItemPropertyMapEntry* lcl_GetDraw_SdXShapePropertyGraphicMap_Impl()
    {
        static const SfxItemPropertyMapEntry aDraw_SdXShapePropertyGraphicMap_Impl[] =
        {
            { OUString("ImageMap"),             WID_IMAGEMAP,        cppu::UnoType<container::XIndexContainer>::get(),    0, 0 },
            DRAW_MAP_ENTRIES
        };
        return aDraw_SdXShapePropertyGraphicMap_Impl;
    }
    static const SfxItemPropertyMapEntry* lcl_ImplGetShapePropertyMap( bool bImpress, bool bGraphicObj )
    {
        const SfxItemPropertyMapEntry* pRet = nullptr;
        if( bImpress )
        {
            if( bGraphicObj )
                pRet = lcl_GetImpress_SdXShapePropertyGraphicMap_Impl();
            else
                pRet = lcl_GetImpress_SdXShapePropertySimpleMap_Impl();
        }
        else
        {
            if( bGraphicObj )
                pRet = lcl_GetDraw_SdXShapePropertyGraphicMap_Impl();
            else
                pRet = lcl_GetDraw_SdXShapePropertySimpleMap_Impl();
        }
        return pRet;

    }
    static const SvxItemPropertySet* lcl_ImplGetShapePropertySet( bool bImpress, bool bGraphicObj )
    {
        const SvxItemPropertySet* pRet = nullptr;
        if( bImpress )
        {
            if( bGraphicObj )
            {
                static SvxItemPropertySet aImpress_SdXShapePropertyGraphicSet_Impl( lcl_GetImpress_SdXShapePropertyGraphicMap_Impl(), SdrObject::GetGlobalDrawObjectItemPool());
                pRet = &aImpress_SdXShapePropertyGraphicSet_Impl;
            }
            else
            {
                static SvxItemPropertySet aImpress_SdXShapePropertySet_Impl(lcl_GetImpress_SdXShapePropertySimpleMap_Impl(), SdrObject::GetGlobalDrawObjectItemPool());
                pRet = &aImpress_SdXShapePropertySet_Impl;
            }
        }
        else
        {
            if( bGraphicObj )
            {
                static SvxItemPropertySet aDraw_SdXShapePropertyGraphicSet_Impl(lcl_GetDraw_SdXShapePropertyGraphicMap_Impl(), SdrObject::GetGlobalDrawObjectItemPool());
                pRet = &aDraw_SdXShapePropertyGraphicSet_Impl;
            }
            else
            {
                static SvxItemPropertySet aDraw_SdXShapePropertySet_Impl( lcl_GetDraw_SdXShapePropertySimpleMap_Impl(), SdrObject::GetGlobalDrawObjectItemPool());
                pRet = &aDraw_SdXShapePropertySet_Impl;
            }
        }
        return pRet;
    }
    static const SfxItemPropertyMapEntry* lcl_GetEmpty_SdXShapePropertyMap_Impl()
    {
        static const SfxItemPropertyMapEntry aEmpty_SdXShapePropertyMap_Impl[] =
        {
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };
        return aEmpty_SdXShapePropertyMap_Impl;
    }

    static const SvxItemPropertySet* lcl_GetEmpty_SdXShapePropertySet_Impl()
    {
        static SvxItemPropertySet aEmptyPropSet( lcl_GetEmpty_SdXShapePropertyMap_Impl(), SdrObject::GetGlobalDrawObjectItemPool() );
        return &aEmptyPropSet;
    }
const SvEventDescription* ImplGetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SFX_EVENT_MOUSEOVER_OBJECT, "OnMouseOver" },
        { SFX_EVENT_MOUSEOUT_OBJECT, "OnMouseOut" },
        { 0, nullptr }
    };

    return aMacroDescriptionsImpl;
}

SdXShape::SdXShape( SvxShape* pShape, SdXImpressDocument* pModel) throw()
:   mpShape( pShape ),
    mpPropSet( pModel?
                    lcl_ImplGetShapePropertySet(pModel->IsImpressDocument(), pShape->getShapeKind() == OBJ_GRAF )
                :   lcl_GetEmpty_SdXShapePropertySet_Impl() ),
    mpMap( pModel?
                    lcl_ImplGetShapePropertyMap(pModel->IsImpressDocument(), pShape->getShapeKind() == OBJ_GRAF )
                :   lcl_GetEmpty_SdXShapePropertyMap_Impl() ),
    mpModel(pModel)
{

    pShape->setMaster( this );
}

SdXShape::~SdXShape() throw()
{
}

void SdXShape::dispose()
{
    mpShape->setMaster( nullptr );
    delete this;
}

uno::Any SAL_CALL SdXShape::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException, std::exception)
{
    return mpShape->queryInterface( rType );
}

void SAL_CALL SdXShape::acquire() throw()
{
    mpShape->acquire();
}

void SAL_CALL SdXShape::release() throw()
{
    mpShape->release();
}

bool SdXShape::queryAggregation( const css::uno::Type & rType, css::uno::Any& aAny )
{
    if( mpModel && mpModel ->IsImpressDocument() )
    {
        if( rType == cppu::UnoType<document::XEventsSupplier>::get())
        {
            aAny <<= uno::Reference< document::XEventsSupplier >(this);
            return true;
        }
    }

    return false;
}

uno::Sequence< uno::Type > SAL_CALL SdXShape::getTypes()
    throw (uno::RuntimeException)
{
    if( mpModel && !mpModel->IsImpressDocument() )
    {
        return mpShape->_getTypes();
    }
    else
    {
        sal_uInt32 nObjId = mpShape->getShapeKind();
        uno::Sequence< uno::Type > aTypes;
        SdTypesCache& gImplTypesCache = SD_MOD()->gImplTypesCache;
        SdTypesCache::iterator aIter( gImplTypesCache.find( nObjId ) );
        if( aIter == gImplTypesCache.end() )
        {
            aTypes = mpShape->_getTypes();
            sal_uInt32 nCount = aTypes.getLength();
            aTypes.realloc( nCount+1 );
            aTypes[nCount] = cppu::UnoType<lang::XTypeProvider>::get();

            gImplTypesCache.insert(std::make_pair(nObjId, aTypes));
        }
        else
        {
            // use the already computed implementation id
            aTypes = (*aIter).second;
        }
        return aTypes;
    }
}

// XPropertyState
beans::PropertyState SAL_CALL SdXShape::getPropertyState( const OUString& PropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpPropSet->getPropertyMapEntry(PropertyName) )
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        SdrObject* pObj = mpShape->GetSdrObject();
        if( pObj == nullptr || ( pObj->GetPage()->IsMasterPage() && pObj->IsEmptyPresObj() ) )
            return beans::PropertyState_DEFAULT_VALUE;

        return mpShape->_getPropertyState( PropertyName );
    }
}

void SAL_CALL SdXShape::setPropertyToDefault( const OUString& PropertyName ) throw( beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpPropSet->getPropertyMapEntry(PropertyName) )
    {
        return;
    }
    else
    {
        mpShape->_setPropertyToDefault(PropertyName);
    }
}

uno::Any SAL_CALL SdXShape::getPropertyDefault( const OUString& aPropertyName ) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if( mpPropSet->getPropertyMapEntry(aPropertyName) )
    {
        return getPropertyValue( aPropertyName );
    }
    else
    {
        uno::Any aRet( mpShape->_getPropertyDefault(aPropertyName) );

        if ( aPropertyName == sUNO_shape_layername )
        {
            OUString aName;
            if( aRet >>= aName )
            {
                aName = SdLayer::convertToExternalName( aName );
                aRet <<= aName;
            }
        }
        return aRet;
    }
}

//XPropertySet
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL SdXShape::getPropertySetInfo()
    throw(css::uno::RuntimeException)
{
    sal_uIntPtr nObjId = reinterpret_cast<sal_uIntPtr>(mpShape->getPropertyMapEntries());
    css::uno::Reference<css::beans::XPropertySetInfo> pInfo;

    SdExtPropertySetInfoCache& rCache = (mpModel && mpModel->IsImpressDocument()) ?
        SD_MOD()->gImplImpressPropertySetInfoCache : SD_MOD()->gImplDrawPropertySetInfoCache;

    SdExtPropertySetInfoCache::iterator aIter( rCache.find( nObjId ) );
    if( aIter == rCache.end() )
    {
        uno::Reference< beans::XPropertySetInfo > xInfo( mpShape->_getPropertySetInfo() );
        pInfo = new SfxExtItemPropertySetInfo( mpMap, xInfo->getProperties() );

        rCache.insert(std::make_pair(nObjId, pInfo));
    }
    else
    {
        // use the already computed implementation id
        pInfo = (*aIter).second;
    }

    return pInfo;
}

void SAL_CALL SdXShape::setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue )
    throw (css::beans::UnknownPropertyException,
           css::beans::PropertyVetoException,
           css::lang::IllegalArgumentException,
           css::lang::WrappedTargetException,
           css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

    if( pEntry )
    {
        SdrObject* pObj = mpShape->GetSdrObject();
        if( pObj )
        {
            SdAnimationInfo* pInfo = GetAnimationInfo((pEntry->nWID <= WID_THAT_NEED_ANIMINFO));

            switch(pEntry->nWID)
            {
                case WID_NAVORDER:
                {
                    sal_Int32 nNavOrder = 0;
                    if(!(aValue >>= nNavOrder))
                        throw lang::IllegalArgumentException();

                    SdrObjList* pObjList = pObj->GetObjList();
                    if( pObjList )
                        pObjList->SetObjectNavigationPosition( *pObj, (nNavOrder < 0) ? SAL_MAX_UINT32 : static_cast< sal_uInt32 >( nNavOrder ) );
                    break;
                }

                case WID_EFFECT:
                {
                    AnimationEffect eEffect;
                    if(!(aValue >>= eEffect))
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetAnimationEffect( mpShape, eEffect );
                    break;
                }
                case WID_TEXTEFFECT:
                {
                    AnimationEffect eEffect;
                    if(!(aValue >>= eEffect))
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetTextAnimationEffect( mpShape, eEffect );
                    break;
                }
                case WID_SPEED:
                {
                    AnimationSpeed eSpeed;
                    if(!(aValue>>=eSpeed))
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetAnimationSpeed( mpShape, eSpeed );
                    break;
                }
                case WID_ISANIMATION:
                {
                    bool bIsAnimation(false);

                    if(!(aValue >>= bIsAnimation))
                    {
                        throw lang::IllegalArgumentException();
                    }

                    if(bIsAnimation)
                    {
                        SdrObjGroup* pGroup = dynamic_cast< SdrObjGroup* >(pObj);
                        SdPage* pPage = pGroup ? dynamic_cast< SdPage* >(pGroup->GetPage()) : nullptr;

                        if (pPage)
                        {
                            // #i42894# Animated Group object, migrate that effect
                            EffectMigration::CreateAnimatedGroup(*pGroup, *pPage);

                            // #i42894# unfortunately when doing this all group members have to
                            // be moved to the page as direct members, else the currently
                            // available forms of animation do not work. If it succeeds,
                            // the group is empty and can be removed and deleted
                            if(!pGroup->GetSubList()->GetObjCount())
                            {
                                pPage->NbcRemoveObject(pGroup->GetOrdNum());
                                delete pGroup;
                            }
                        }
                    }
                    //pInfo->mbIsMovie = bIsAnimation;
                    break;
                }
                case WID_BOOKMARK:
                {
                    OUString aString;
                    if(!(aValue >>= aString))
                        throw lang::IllegalArgumentException();

                    pInfo->SetBookmark( SdDrawPage::getUiNameFromPageApiName( aString ) );
                    break;
                }
                case WID_CLICKACTION:
                    ::cppu::any2enum< presentation::ClickAction >( pInfo->meClickAction, aValue);
                    break;

// TODO: WID_PLAYFULL:
                case WID_SOUNDFILE:
                {
                    OUString aString;
                    if(!(aValue >>= aString))
                        throw lang::IllegalArgumentException();
                    pInfo->maSoundFile = aString;
                    EffectMigration::UpdateSoundEffect( mpShape, pInfo );
                    break;
                }

                case WID_SOUNDON:
                {
                    if( !(aValue >>= pInfo->mbSoundOn) )
                        throw lang::IllegalArgumentException();
                    EffectMigration::UpdateSoundEffect( mpShape, pInfo );
                    break;
                }
                case WID_VERB:
                {
                    sal_Int32 nVerb = 0;
                    if(!(aValue >>= nVerb))
                        throw lang::IllegalArgumentException();

                    pInfo->mnVerb = (sal_uInt16)nVerb;
                    break;
                }
                case WID_DIMCOLOR:
                {
                    sal_Int32 nColor = 0;

                    if( !(aValue >>= nColor) )
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetDimColor( mpShape, nColor );
                    break;
                }
                case WID_DIMHIDE:
                {
                    bool bDimHide = false;
                    if( !(aValue >>= bDimHide) )
                        lang::IllegalArgumentException();

                    EffectMigration::SetDimHide( mpShape, bDimHide );
                    break;
                }
                case WID_DIMPREV:
                {
                    bool bDimPrevious = false;
                    if( !(aValue >>= bDimPrevious) )
                        lang::IllegalArgumentException();

                    EffectMigration::SetDimPrevious( mpShape, bDimPrevious );
                    break;
                }
                case WID_PRESORDER:
                {
                    sal_Int32 nNewPos = 0;
                    if( !(aValue >>= nNewPos) )
                        lang::IllegalArgumentException();

                    EffectMigration::SetPresentationOrder( mpShape, nNewPos );
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

                case WID_LEGACYFRAGMENT:
                    {
                        uno::Reference< io::XInputStream > xInputStream;
                        aValue >>= xInputStream;
                        if( xInputStream.is() )
                        {
                            SvInputStream aStream( xInputStream );
                            SdrObject* pObject = mpShape->GetSdrObject();
                            SvxMSDffManager::ReadObjText( aStream, pObject );
                        }
                    }
                    break;

                case WID_ANIMPATH:
                {
                    uno::Reference< drawing::XShape > xShape( aValue, uno::UNO_QUERY );
                    SdrPathObj* pObj2 = xShape.is() ? dynamic_cast< SdrPathObj* >( GetSdrObjectFromXShape( xShape ) ) : nullptr;

                    if( pObj2 == nullptr )
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetAnimationPath( mpShape, pObj2 );
                    break;
                }
                case WID_IMAGEMAP:
                {
                    SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():nullptr;
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
                            pObj->AppendUserData(new SdIMapInfo(aImageMap) );
                        }
                    }
                }
                break;
            }
        }
    }
    else
    {
        uno::Any aAny( aValue );

        if ( aPropertyName == sUNO_shape_layername )
        {
            OUString aName;
            if( aAny >>= aName )
            {
                aName = SdLayer::convertToInternalName( aName );
                aAny <<= aName;
            }
        }

        mpShape->_setPropertyValue(aPropertyName, aAny);
    }

    if( mpModel )
        mpModel->SetModified();
}

css::uno::Any SAL_CALL SdXShape::getPropertyValue( const OUString& PropertyName )
    throw (css::beans::UnknownPropertyException,
           css::lang::WrappedTargetException,
           css::uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

    if( pEntry && mpShape->GetSdrObject() )
    {
        SdAnimationInfo* pInfo = GetAnimationInfo();

        switch(pEntry->nWID)
        {
        case WID_NAVORDER:
            {
                const sal_uInt32 nNavOrder = mpShape->GetSdrObject()->GetNavigationPosition();
                aRet <<= nNavOrder == SAL_MAX_UINT32 ? static_cast<sal_Int32>(-1) : static_cast< sal_Int32 >(nNavOrder);
            }
            break;
        case WID_EFFECT:
            aRet <<= EffectMigration::GetAnimationEffect( mpShape );
            break;
        case WID_TEXTEFFECT:
            aRet <<= EffectMigration::GetTextAnimationEffect( mpShape );
            break;
        case WID_ISPRESOBJ:
            aRet <<= IsPresObj();
            break;
        case WID_ISEMPTYPRESOBJ:
            aRet <<= IsEmptyPresObj();
            break;
        case WID_MASTERDEPEND:
            aRet <<= IsMasterDepend();
            break;
        case WID_SPEED:
            aRet <<= EffectMigration::GetAnimationSpeed( mpShape );
            break;
        case WID_ISANIMATION:
            aRet <<= (pInfo && pInfo->mbIsMovie);
            break;
        case WID_PLACEHOLDERTEXT:
            aRet <<= GetPlaceholderText();
            break;
        case WID_BOOKMARK:
        {
            OUString aString;
            SdDrawDocument* pDoc = mpModel ? mpModel->GetDoc() : nullptr;
            if (pInfo && pDoc)
            {
                // is the bookmark a page?
                bool bIsMasterPage;
                if(pDoc->GetPageByName( pInfo->GetBookmark(), bIsMasterPage ) != SDRPAGE_NOTFOUND)
                {
                    aString = SdDrawPage::getPageApiNameFromUiName( pInfo->GetBookmark() );
                }
                else
                {
                    aString = pInfo->GetBookmark() ;
                    sal_Int32 nPos = aString.lastIndexOf( '#' );
                    if( nPos >= 0 )
                    {
                        OUString aURL( aString.copy( 0, nPos+1 ) );
                        OUString aName( aString.copy( nPos+1 ) );
                        if(pDoc->GetPageByName( aName, bIsMasterPage ) != SDRPAGE_NOTFOUND)
                        {
                            aURL += SdDrawPage::getPageApiNameFromUiName( aName );
                            aString = aURL;
                        }
                    }
                }
            }

            aRet <<= aString;
            break;
        }
        case WID_CLICKACTION:
            aRet = ::cppu::enum2any< presentation::ClickAction >( pInfo?pInfo->meClickAction:presentation::ClickAction_NONE );
            break;
        case WID_PLAYFULL:
            aRet <<= ( pInfo && pInfo->mbPlayFull );
            break;
        case WID_SOUNDFILE:
            aRet <<= EffectMigration::GetSoundFile( mpShape );
            break;
        case WID_SOUNDON:
            aRet <<= EffectMigration::GetSoundOn( mpShape );
            break;
        case WID_BLUESCREEN:
            aRet <<= (sal_Int32)( pInfo?pInfo->maBlueScreen.GetColor():0x00ffffff );
            break;
        case WID_VERB:
            aRet <<= (sal_Int32)( pInfo?pInfo->mnVerb:0 );
            break;
        case WID_DIMCOLOR:
            aRet <<= EffectMigration::GetDimColor( mpShape );
            break;
        case WID_DIMHIDE:
            aRet <<= EffectMigration::GetDimHide( mpShape );
            break;
        case WID_DIMPREV:
            aRet <<= EffectMigration::GetDimPrevious( mpShape );
            break;
        case WID_PRESORDER:
            aRet <<= EffectMigration::GetPresentationOrder( mpShape );
            break;
        case WID_STYLE:
            aRet = GetStyleSheet();
            break;
        case WID_ANIMPATH:
            if( pInfo && pInfo->mpPathObj )
                aRet <<= pInfo->mpPathObj->getUnoShape();
            break;
        case WID_IMAGEMAP:
            {
                uno::Reference< uno::XInterface > xImageMap;

                SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():nullptr;
                if( pDoc )
                {

                    SdIMapInfo* pIMapInfo = pDoc->GetIMapInfo(mpShape->GetSdrObject());
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
        aRet = mpShape->_getPropertyValue(PropertyName);

        if ( PropertyName == sUNO_shape_layername )
        {
            OUString aName;
            if( aRet >>= aName )
            {
                aName = SdLayer::convertToExternalName( aName );
                aRet <<= aName;
            }
        }
    }

    return aRet;
}

/** */
SdAnimationInfo* SdXShape::GetAnimationInfo( bool bCreate ) const
    throw (std::exception)
{
    SdAnimationInfo* pInfo = nullptr;

    SdrObject* pObj = mpShape->GetSdrObject();
    if(pObj)
        pInfo = SdDrawDocument::GetShapeUserData(*pObj, bCreate);

    return pInfo;
}

uno::Sequence< OUString > SAL_CALL SdXShape::getSupportedServiceNames() throw(css::uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( mpShape->_getSupportedServiceNames() );

    comphelper::ServiceInfoHelper::addToSequence( aSeq, 2, "com.sun.star.presentation.Shape",
                                                  "com.sun.star.document.LinkTarget" );

    SdrObject* pObj = mpShape->GetSdrObject();
    if(pObj && pObj->GetObjInventor() == SdrInventor )
    {
        sal_uInt32 nInventor = pObj->GetObjIdentifier();
        switch( nInventor )
        {
        case OBJ_TITLETEXT:
            comphelper::ServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.TitleTextShape" );
            break;
        case OBJ_OUTLINETEXT:
            comphelper::ServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.OutlinerShape" );
            break;
        }
    }
    return aSeq;
}

/** checks if this is a presentation object
 */
bool SdXShape::IsPresObj() const
    throw (std::exception)
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if(pObj)
    {
        SdPage* pPage = dynamic_cast<SdPage* >(pObj->GetPage());
        if(pPage)
            return pPage->GetPresObjKind(pObj) != PRESOBJ_NONE;
    }
    return false;
}

/** checks if this presentation object is empty
 */
bool SdXShape::IsEmptyPresObj() const throw()
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if( (pObj != nullptr) && pObj->IsEmptyPresObj() )
    {
        // check if the object is in edit, than its temporarily not empty
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
        if( pTextObj == nullptr )
            return true;

        OutlinerParaObject* pParaObj = pTextObj->GetEditOutlinerParaObject();
        if( pParaObj )
        {
            delete pParaObj;
        }
        else
        {
            return true;
        }
    }

    return false;
}

OUString SdXShape::GetPlaceholderText() const
{
    // only possible if this actually *is* a presentation object
    if( !IsPresObj() )
        return OUString();

    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        return OUString();

    SdPage* pPage = dynamic_cast< SdPage* >(pObj->GetPage());
    DBG_ASSERT( pPage, "no page?" );
    if( pPage == nullptr )
        return OUString();

    return pPage->GetPresObjText( pPage->GetPresObjKind(pObj) );
 }

/** sets/reset the empty status of a presentation object
*/
void SdXShape::SetEmptyPresObj(bool bEmpty)
    throw (css::uno::RuntimeException, std::exception)
{
    // only possible if this actually *is* a presentation object
    if( !IsPresObj() )
        return;

    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        return;

    if( pObj->IsEmptyPresObj() != bEmpty )
    {
        if(!bEmpty)
        {
            OutlinerParaObject* pOutlinerParaObject = pObj->GetOutlinerParaObject();
            const bool bVertical = pOutlinerParaObject && pOutlinerParaObject->IsVertical();

            // really delete SdrOutlinerObj at pObj
            pObj->NbcSetOutlinerParaObject(nullptr);
            if( bVertical && dynamic_cast<SdrTextObj*>( pObj )  )
                static_cast<SdrTextObj*>(pObj)->SetVerticalWriting( true );

            SdrGrafObj* pGraphicObj = dynamic_cast<SdrGrafObj*>( pObj  );
            if( pGraphicObj )
            {
                Graphic aEmpty;
                pGraphicObj->SetGraphic(aEmpty);
            }
            else
            {
                SdrOle2Obj* pOleObj = dynamic_cast< SdrOle2Obj* >( pObj );
                if( pOleObj )
                {
                    pOleObj->SetGraphic( nullptr );
                }
            }
        }
        else
        {
            // now set an empty OutlinerParaObject at pObj without
            // any content but with the style of the old OutlinerParaObjects
            // first paragraph
            do
            {
                SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():nullptr;
                DBG_ASSERT( pDoc, "no document?" );
                if( pDoc == nullptr)
                    break;

                ::sd::Outliner* pOutliner = pDoc->GetInternalOutliner();
                DBG_ASSERT( pOutliner, "no outliner?" );
                if( pOutliner == nullptr )
                    break;

                SdPage* pPage = dynamic_cast< SdPage* >(pObj->GetPage());
                DBG_ASSERT( pPage, "no page?" );
                if( pPage == nullptr )
                    break;

                OutlinerParaObject* pOutlinerParaObject = pObj->GetOutlinerParaObject();
                pOutliner->SetText( *pOutlinerParaObject );
                const bool bVertical = pOutliner->IsVertical();

                pOutliner->Clear();
                pOutliner->SetVertical( bVertical );
                pOutliner->SetStyleSheetPool( static_cast<SfxStyleSheetPool*>(pDoc->GetStyleSheetPool()) );
                pOutliner->SetStyleSheet( 0, pPage->GetTextStyleSheetForObject( pObj ) );
                pOutliner->Insert( pPage->GetPresObjText( pPage->GetPresObjKind(pObj) ) );
                pObj->SetOutlinerParaObject( pOutliner->CreateParaObject() );
                pOutliner->Clear();
            }
            while(false);
        }

        pObj->SetEmptyPresObj(bEmpty);
    }
}

bool SdXShape::IsMasterDepend() const throw()
{
    SdrObject* pObj = mpShape->GetSdrObject();
    return pObj && pObj->GetUserCall() != nullptr;
}

void SdXShape::SetMasterDepend( bool bDepend ) throw()
{
    if( IsMasterDepend() != bDepend )
    {
        SdrObject* pObj = mpShape->GetSdrObject();
        if( pObj )
        {
            if( bDepend )
            {
                SdPage* pPage = dynamic_cast< SdPage* >(pObj->GetPage());
                pObj->SetUserCall( pPage );
            }
            else
            {
                pObj->SetUserCall( nullptr );
            }
        }
    }
}

void SdXShape::SetStyleSheet( const uno::Any& rAny ) throw( lang::IllegalArgumentException, beans::UnknownPropertyException, uno::RuntimeException )
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        throw beans::UnknownPropertyException();

    uno::Reference< style::XStyle > xStyle( rAny, uno::UNO_QUERY );
    SfxStyleSheet* pStyleSheet = SfxUnoStyleSheet::getUnoStyleSheet( xStyle );

    const SfxStyleSheet* pOldStyleSheet = pObj->GetStyleSheet();
    if( pOldStyleSheet != pStyleSheet )
    {
        if( pStyleSheet == nullptr || (pStyleSheet->GetFamily() != SD_STYLE_FAMILY_GRAPHICS && pStyleSheet->GetFamily() != SD_STYLE_FAMILY_MASTERPAGE) )
            throw lang::IllegalArgumentException();

        pObj->SetStyleSheet( pStyleSheet, false );

        SdDrawDocument* pDoc = mpModel? mpModel->GetDoc() : nullptr;
        if( pDoc )
        {
            ::sd::DrawDocShell* pDocSh = pDoc->GetDocSh();
            ::sd::ViewShell* pViewSh = pDocSh ? pDocSh->GetViewShell() : nullptr;

            if( pViewSh )
                pViewSh->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
        }
    }
}

uno::Any SdXShape::GetStyleSheet() const throw( beans::UnknownPropertyException  )
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        throw beans::UnknownPropertyException();

    SfxStyleSheet* pStyleSheet = pObj->GetStyleSheet();
    // it is possible for shapes inside a draw to have a presentation style
    // but we don't want this for the api
    if( (pStyleSheet == nullptr) || ((pStyleSheet->GetFamily() != SD_STYLE_FAMILY_GRAPHICS) && !mpModel->IsImpressDocument()) )
        return Any();

    return Any( uno::Reference< style::XStyle >( dynamic_cast< SfxUnoStyleSheet* >( pStyleSheet ) ) );
}

class SdUnoEventsAccess : public cppu::WeakImplHelper< css::container::XNameReplace, css::lang::XServiceInfo >
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
    const OUString      maStrScript;

    SdXShape*   mpShape;
    uno::Reference< document::XEventsSupplier > mxShape;

public:
    SdUnoEventsAccess( SdXShape* pShape ) throw();

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
};

// XEventsSupplier
uno::Reference< container::XNameReplace > SAL_CALL SdXShape::getEvents(  ) throw(css::uno::RuntimeException, std::exception)
{
    return new SdUnoEventsAccess( this );
}

SdUnoEventsAccess::SdUnoEventsAccess( SdXShape* pShape ) throw()
: maStrOnClick( "OnClick" ),
  maStrServiceName( "com.sun.star.documents.Events" ),
  maStrEventType( "EventType" ),
  maStrPresentation( "Presentation" ),
  maStrLibrary("Library"),
  maStrMacroName("MacroName"),
  maStrClickAction( "ClickAction" ),
  maStrBookmark( "Bookmark" ),
  maStrEffect( "Effect" ),
  maStrPlayFull( "PlayFull" ),
  maStrVerb( "Verb" ),
  maStrSoundURL( "SoundURL" ),
  maStrSpeed( "Speed" ),
  maStrStarBasic( "StarBasic" ),
  maStrScript( "Script" ),
  mpShape( pShape ), mxShape( pShape )
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
    pInfo->SetBookmark( "" );
    pInfo->mbSecondSoundOn = false;
    pInfo->mbSecondPlayFull = false;
    pInfo->meClickAction = presentation::ClickAction_NONE;
    pInfo->meSecondEffect = presentation::AnimationEffect_NONE;
    pInfo->meSecondSpeed = presentation::AnimationSpeed_MEDIUM;
    pInfo->mnVerb = 0;
}

// XNameReplace
void SAL_CALL SdUnoEventsAccess::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpShape == nullptr || aName != maStrOnClick )
        throw container::NoSuchElementException();

    uno::Sequence< beans::PropertyValue > aProperties;
    if( !aElement.hasValue() || aElement.getValueType() != getElementType() || !(aElement >>= aProperties) )
        throw lang::IllegalArgumentException();

    sal_Int32 nFound = 0;
    const beans::PropertyValue* pProperties = aProperties.getConstArray();

    OUString aStrEventType;
    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    presentation::AnimationEffect eEffect = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed eSpeed = presentation::AnimationSpeed_MEDIUM;
    OUString aStrSoundURL;
    bool bPlayFull = false;
    sal_Int32 nVerb = 0;
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
        else if( ( ( nFound & FOUND_MACRO ) == 0 ) && ( pProperties->Name == maStrMacroName || pProperties->Name == maStrScript ) )
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

    bool bOk = false;
    do
    {
        if( ( nFound & FOUND_EVENTTYPE ) == 0 )
            break;

        if( aStrEventType == maStrPresentation )
        {
            if( ( nFound & FOUND_CLICKACTION ) == 0 )
                break;

            SdAnimationInfo* pInfo = mpShape->GetAnimationInfo();
            if( presentation::ClickAction_NONE == eClickAction && nullptr == pInfo )
            {
                bOk = true;
                break;
            }

            if( nullptr == pInfo )
                pInfo = mpShape->GetAnimationInfo( true );

            DBG_ASSERT( pInfo, "shape animation info could not be created!" );
            if( nullptr == pInfo )
                break;

            clearEventsInAnimationInfo( pInfo );
            pInfo->meClickAction = eClickAction;

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
                    bOk = true;
                }
                break;

            case presentation::ClickAction_PROGRAM:
            case presentation::ClickAction_BOOKMARK:
            case presentation::ClickAction_DOCUMENT:
                if( nFound & FOUND_BOOKMARK )
                {
                    if( eClickAction == presentation::ClickAction_BOOKMARK )
                    {
                        aStrBookmark = getUiNameFromPageApiNameImpl( aStrBookmark );
                    }
                    else if( eClickAction == presentation::ClickAction_DOCUMENT )
                    {
                        sal_Int32 nPos = aStrBookmark.lastIndexOf( '#' );
                        if( nPos >= 0 )
                        {
                            OUString aURL( aStrBookmark.copy( 0, nPos+1 ) );
                            aURL += getUiNameFromPageApiNameImpl( aStrBookmark.copy( nPos+1 ) );
                            aStrBookmark = aURL;
                        }
                    }

                    pInfo->SetBookmark( aStrBookmark );
                    bOk = true;
                }
                break;

            case presentation::ClickAction_MACRO:
                if( nFound & FOUND_MACRO )
                {
                    pInfo->SetBookmark( aStrMacro );
                    bOk = true;
                }
                break;

            case presentation::ClickAction_VERB:
                if( nFound & FOUND_VERB )
                {
                    pInfo->mnVerb = (sal_uInt16)nVerb;
                    bOk = true;
                }
                break;

            case presentation::ClickAction_VANISH:
                if( ( nFound & FOUND_EFFECT ) == 0 )
                    break;

                pInfo->meSecondEffect = eEffect;
                pInfo->meSecondSpeed = nFound & FOUND_SPEED ? eSpeed : presentation::AnimationSpeed_MEDIUM;

                bOk = true;

                SAL_FALLTHROUGH;

            case presentation::ClickAction_SOUND:
                if( nFound & FOUND_SOUNDURL )
                {
                    pInfo->SetBookmark( aStrSoundURL );
                    if( eClickAction != presentation::ClickAction_SOUND )
                        pInfo->mbSecondSoundOn = !aStrSoundURL.isEmpty();
                    pInfo->mbSecondPlayFull = (nFound & FOUND_PLAYFULL) && bPlayFull;

                    bOk = true;
                }
                break;
            case presentation::ClickAction_MAKE_FIXED_SIZE:
                break;
            }
        }
        else
        {
            SdAnimationInfo* pInfo = mpShape->GetAnimationInfo( true );

            DBG_ASSERT( pInfo, "shape animation info could not be created!" );
            if( nullptr == pInfo )
                break;

            clearEventsInAnimationInfo( pInfo );
            pInfo->meClickAction = presentation::ClickAction_MACRO;

            if ( SfxApplication::IsXScriptURL( aStrMacro ) )
            {
                pInfo->SetBookmark( aStrMacro );
            }
            else
            {
                OUString aMacro = aStrMacro;

                OUString aLibName   = aMacro.getToken(0, '.');
                OUString aModulName = aMacro.getToken(1, '.');
                OUString aMacroName = aMacro.getToken(2, '.');

                OUStringBuffer sBuffer;
                sBuffer.append( aMacroName );
                sBuffer.append( '.' );
                sBuffer.append( aModulName );
                sBuffer.append( '.' );
                sBuffer.append( aLibName );
                sBuffer.append( '.' );

                if ( aStrLibrary == "StarOffice" )
                {
                    sBuffer.append( "BASIC" );
                }
                else
                {
                    sBuffer.append( aStrLibrary );
                }

                pInfo->SetBookmark( sBuffer.makeStringAndClear() );
            }
            bOk = true;
        }
    }
    while(false);

    if( !bOk )
        throw lang::IllegalArgumentException();
}

// XNameAccess
uno::Any SAL_CALL SdUnoEventsAccess::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( mpShape == nullptr || aName != maStrOnClick )
        throw container::NoSuchElementException();

    SdAnimationInfo* pInfo = mpShape->GetAnimationInfo();

    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    if( pInfo )
        eClickAction = pInfo->meClickAction;

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
        if ( !SfxApplication::IsXScriptURL( pInfo->GetBookmark() ) )
            nPropertyCount += 1;
        break;

    case presentation::ClickAction_SOUND:
        nPropertyCount += 2;
        break;

    case presentation::ClickAction_VANISH:
        nPropertyCount += 4;
        break;
    default:
        break;
    }

    uno::Sequence< beans::PropertyValue > aProperties( nPropertyCount );
    beans::PropertyValue* pProperties = aProperties.getArray();

    uno::Any aAny;

    if( eClickAction == presentation::ClickAction_MACRO )
    {
        if ( SfxApplication::IsXScriptURL( pInfo->GetBookmark() ) )
        {
            // Scripting Framework URL
            aAny <<= maStrScript;
            pProperties->Name = maStrEventType;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= OUString( pInfo->GetBookmark() );
            pProperties->Name = maStrScript;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;
        }
        else
        {
            // Old Basic macro URL
            aAny <<= maStrStarBasic;
            pProperties->Name = maStrEventType;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            OUString aMacro = pInfo->GetBookmark();

            // aMacro has got following format:
            // "Macroname.Modulname.Libname.Documentname" or
            // "Macroname.Modulname.Libname.Applicationname"
            OUString aMacroName = aMacro.getToken(0, '.');
            OUString aModulName = aMacro.getToken(1, '.');
            OUString aLibName   = aMacro.getToken(2, '.');

            OUStringBuffer sBuffer;
            sBuffer.append( aLibName );
            sBuffer.append( '.' );
            sBuffer.append( aModulName );
            sBuffer.append( '.' );
            sBuffer.append( aMacroName );

            aAny <<= OUString( sBuffer.makeStringAndClear() );
            pProperties->Name = maStrMacroName;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= OUString( "StarOffice" );
            pProperties->Name = maStrLibrary;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
        }
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
                const OUString aStrBookmark( getPageApiNameFromUiName( pInfo->GetBookmark()) );
                pProperties->Name = maStrBookmark;
                pProperties->Handle = -1;
                pProperties->Value <<= aStrBookmark;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_DOCUMENT:
        case presentation::ClickAction_PROGRAM:
            {
                OUString aString( pInfo->GetBookmark());
                sal_Int32 nPos = aString.lastIndexOf( '#' );
                if( nPos >= 0 )
                {
                    OUString aURL( aString.copy( 0, nPos+1 ) );
                    aURL += getPageApiNameFromUiName( aString.copy( nPos+1 ) );
                    aString = aURL;
                }
                pProperties->Name = maStrBookmark;
                pProperties->Handle = -1;
                pProperties->Value <<= aString;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_VANISH:
            aAny <<= pInfo->meSecondEffect;
            pProperties->Name = maStrEffect;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= pInfo->meSecondSpeed;
            pProperties->Name = maStrSpeed;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            SAL_FALLTHROUGH;

        case presentation::ClickAction_SOUND:
            if( eClickAction == presentation::ClickAction_SOUND || pInfo->mbSecondSoundOn )
            {
                aAny <<= OUString( pInfo->GetBookmark());
                pProperties->Name = maStrSoundURL;
                pProperties->Handle = -1;
                pProperties->Value = aAny;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = maStrPlayFull;
                pProperties->Handle = -1;
                pProperties->Value = css::uno::makeAny(pInfo->mbSecondPlayFull);
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_VERB:
            aAny <<= (sal_Int32)pInfo->mnVerb;
            pProperties->Name = maStrVerb;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            break;
        default:
            break;
        }
    }

    aAny <<= aProperties;
    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdUnoEventsAccess::getElementNames(  )
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aStr( &maStrOnClick, 1 );
    return aStr;
}

sal_Bool SAL_CALL SdUnoEventsAccess::hasByName( const OUString& aName )
    throw(uno::RuntimeException, std::exception)
{
    return aName == maStrOnClick;
}

// XElementAccess
uno::Type SAL_CALL SdUnoEventsAccess::getElementType(  )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get();
}

sal_Bool SAL_CALL SdUnoEventsAccess::hasElements(  ) throw(uno::RuntimeException, std::exception)
{
    return sal_True;
}

// XServiceInfo
OUString SAL_CALL SdUnoEventsAccess::getImplementationName(  )
    throw(uno::RuntimeException, std::exception)
{
    return OUString( "SdUnoEventsAccess" );
}

sal_Bool SAL_CALL SdUnoEventsAccess::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdUnoEventsAccess::getSupportedServiceNames(  )
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aStr( &maStrServiceName, 1 );
    return aStr;
}

void SdXShape::modelChanged( SdrModel* pNewModel )
{
    if( pNewModel )
    {
        uno::Reference< uno::XInterface > xModel( pNewModel->getUnoModel() );
        mpModel = SdXImpressDocument::getImplementation( xModel );
    }
    else
    {
        mpModel = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
