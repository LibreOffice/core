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

#include <memory>
#include <string_view>
#include <utility>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <rtl/ustrbuf.hxx>
#include <svl/itemprop.hxx>
#include <svl/style.hxx>
#include <svx/svdpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <svtools/unoimap.hxx>
#include <svtools/unoevent.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/event.hxx>
#include <sfx2/sfxsids.hrc>
#include <comphelper/extract.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdotext.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/ImageMapInfo.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <svl/instrm.hxx>
#include <editeng/outlobj.hxx>
#include <Outliner.hxx>
#include <comphelper/sequence.hxx>
#include <svx/svdogrp.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

#include <anminfo.hxx>
#include "unoobj.hxx"
#include <unoprnms.hxx>
#include <unomodel.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>
#include <unopage.hxx>
#include <DrawDocShell.hxx>
#include <EffectMigration.hxx>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::animations;

using ::com::sun::star::uno::Any;
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
        { UNO_NAME_OBJ_LEGACYFRAGMENT,WID_LEGACYFRAGMENT,  cppu::UnoType<drawing::XShape>::get(),                 0, 0},\
        { UNO_NAME_OBJ_ANIMATIONPATH, WID_ANIMPATH,        cppu::UnoType<drawing::XShape>::get(),                 0, 0},\
        { UNO_NAME_OBJ_BOOKMARK,      WID_BOOKMARK,        cppu::UnoType<OUString>::get(),                        0, 0},\
        { UNO_NAME_OBJ_DIMCOLOR,      WID_DIMCOLOR,        cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { UNO_NAME_OBJ_DIMHIDE,       WID_DIMHIDE,         cppu::UnoType<bool>::get(),                            0, 0},\
        { UNO_NAME_OBJ_DIMPREV,       WID_DIMPREV,         cppu::UnoType<bool>::get(),                            0, 0},\
        { UNO_NAME_OBJ_EFFECT,        WID_EFFECT,          cppu::UnoType<presentation::AnimationEffect>::get(),   0, 0},\
        { UNO_NAME_OBJ_ISEMPTYPRESOBJ,WID_ISEMPTYPRESOBJ,  cppu::UnoType<bool>::get(),                            0, 0},\
        { UNO_NAME_OBJ_ISPRESOBJ,     WID_ISPRESOBJ,       cppu::UnoType<bool>::get(),                            css::beans::PropertyAttribute::READONLY, 0},\
        { UNO_NAME_OBJ_MASTERDEPENDENT,WID_MASTERDEPEND,   cppu::UnoType<bool>::get(),                            0, 0},\
        { UNO_NAME_OBJ_CLICKACTION,   WID_CLICKACTION,     cppu::UnoType<presentation::ClickAction>::get(),       0, 0},\
        { UNO_NAME_OBJ_PLAYFULL,      WID_PLAYFULL,        cppu::UnoType<bool>::get(),                            0, 0},\
        { UNO_NAME_OBJ_PRESORDER,     WID_PRESORDER,       cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { UNO_NAME_OBJ_STYLE,         WID_STYLE,           cppu::UnoType<style::XStyle>::get(),                   css::beans::PropertyAttribute::MAYBEVOID, 0},\
        { UNO_NAME_OBJ_SOUNDFILE,     WID_SOUNDFILE,       cppu::UnoType<OUString>::get(),                        0, 0},\
        { UNO_NAME_OBJ_SOUNDON,       WID_SOUNDON,         cppu::UnoType<bool>::get(),                            0, 0},\
        { UNO_NAME_OBJ_SPEED,         WID_SPEED,           cppu::UnoType<presentation::AnimationSpeed>::get(),    0, 0},\
        { UNO_NAME_OBJ_TEXTEFFECT,    WID_TEXTEFFECT,      cppu::UnoType<presentation::AnimationEffect>::get(),   0, 0},\
        { UNO_NAME_OBJ_BLUESCREEN,    WID_BLUESCREEN,      cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { UNO_NAME_OBJ_VERB,          WID_VERB,            cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { u"IsAnimation",              WID_ISANIMATION,     cppu::UnoType<bool>::get(),                            0, 0},\
        { u"NavigationOrder",          WID_NAVORDER,        cppu::UnoType<sal_Int32>::get(),                       0, 0},\
        { u"PlaceholderText",          WID_PLACEHOLDERTEXT, cppu::UnoType<OUString>::get(),                        0, 0},\
        { u"", 0, css::uno::Type(), 0, 0 }

    static const SfxItemPropertyMapEntry* lcl_GetImpress_SdXShapePropertyGraphicMap_Impl()
    {

        static const SfxItemPropertyMapEntry aImpress_SdXShapePropertyGraphicMap_Impl[] =
        {
            { u"ImageMap",             WID_IMAGEMAP,        cppu::UnoType<container::XIndexContainer>::get(),    0, 0 },
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
        { UNO_NAME_OBJ_BOOKMARK,      WID_BOOKMARK,       cppu::UnoType<OUString>::get(),                 0, 0},\
        { UNO_NAME_OBJ_CLICKACTION,   WID_CLICKACTION,    cppu::UnoType<presentation::ClickAction>::get(),0, 0},\
        { UNO_NAME_OBJ_STYLE,         WID_STYLE,          cppu::UnoType<style::XStyle>::get(),            css::beans::PropertyAttribute::MAYBEVOID, 0},\
        { u"NavigationOrder",          WID_NAVORDER,       cppu::UnoType<sal_Int32>::get(),                0, 0},\
        { u"", 0, css::uno::Type(), 0, 0 }

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
            { u"ImageMap",             WID_IMAGEMAP,        cppu::UnoType<container::XIndexContainer>::get(),    0, 0 },
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
            { u"", 0, css::uno::Type(), 0, 0 }
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
        { SvMacroItemId::OnMouseOver, "OnMouseOver" },
        { SvMacroItemId::OnMouseOut,  "OnMouseOut" },
        { SvMacroItemId::NONE, nullptr }
    };

    return aMacroDescriptionsImpl;
}

SdXShape::SdXShape(SvxShape* pShape, SdXImpressDocument* pModel)
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

SdXShape::~SdXShape() noexcept
{
}

void SdXShape::dispose()
{
    mpShape->setMaster( nullptr );
    delete this;
}

uno::Any SAL_CALL SdXShape::queryInterface( const uno::Type & rType )
{
    return mpShape->queryInterface( rType );
}

void SAL_CALL SdXShape::acquire() noexcept
{
    mpShape->acquire();
}

void SAL_CALL SdXShape::release() noexcept
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
            aTypes.getArray()[nCount] = cppu::UnoType<lang::XTypeProvider>::get();

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
beans::PropertyState SAL_CALL SdXShape::getPropertyState( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    if( mpPropSet->getPropertyMapEntry(PropertyName) )
    {
        return beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        SdrObject* pObj = mpShape->GetSdrObject();
        if( pObj == nullptr || ( pObj->getSdrPageFromSdrObject()->IsMasterPage() && pObj->IsEmptyPresObj() ) )
            return beans::PropertyState_DEFAULT_VALUE;

        return mpShape->_getPropertyState( PropertyName );
    }
}

void SAL_CALL SdXShape::setPropertyToDefault( const OUString& PropertyName )
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

uno::Any SAL_CALL SdXShape::getPropertyDefault( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    if( mpPropSet->getPropertyMapEntry(aPropertyName) )
    {
        return getPropertyValue( aPropertyName );
    }
    else
    {
        uno::Any aRet( mpShape->_getPropertyDefault(aPropertyName) );
        return aRet;
    }
}

//XPropertySet
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL SdXShape::getPropertySetInfo()
{
    SfxItemPropertyMapEntry const * nObjId = mpShape->getPropertyMapEntries();
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
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

    if( pEntry )
    {
        SdrObject* pObj = mpShape->GetSdrObject();
        if( pObj )
        {
            SdAnimationInfo* pInfo = GetAnimationInfo(pEntry->nWID <= WID_THAT_NEED_ANIMINFO);

            switch(pEntry->nWID)
            {
                case WID_NAVORDER:
                {
                    sal_Int32 nNavOrder = 0;
                    if(!(aValue >>= nNavOrder))
                        throw lang::IllegalArgumentException();

                    SdrObjList* pObjList = pObj->getParentSdrObjListFromSdrObject();
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
                        SdPage* pPage = pGroup ? dynamic_cast< SdPage* >(pGroup->getSdrPageFromSdrObject()) : nullptr;

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

                                // always use SdrObject::Free(...) for SdrObjects (!)
                                SdrObject* pTemp(pGroup);
                                SdrObject::Free(pTemp);
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

                    pInfo->mnVerb = static_cast<sal_uInt16>(nVerb);
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
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetDimHide( mpShape, bDimHide );
                    break;
                }
                case WID_DIMPREV:
                {
                    bool bDimPrevious = false;
                    if( !(aValue >>= bDimPrevious) )
                        throw lang::IllegalArgumentException();

                    EffectMigration::SetDimPrevious( mpShape, bDimPrevious );
                    break;
                }
                case WID_PRESORDER:
                {
                    sal_Int32 nNewPos = 0;
                    if( !(aValue >>= nNewPos) )
                        throw lang::IllegalArgumentException();

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
                    SdrPathObj* pObj2 = xShape.is() ? dynamic_cast<SdrPathObj*>(SdrObject::getSdrObjectFromXShape(xShape)) : nullptr;

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

                        SvxIMapInfo* pIMapInfo = SvxIMapInfo::GetIMapInfo(pObj);
                        if( pIMapInfo )
                        {
                            // replace existing image map
                            pIMapInfo->SetImageMap( aImageMap );
                        }
                        else
                        {
                            // insert new user data with image map
                            pObj->AppendUserData(std::unique_ptr<SdrObjUserData>(new SvxIMapInfo(aImageMap) ));
                        }
                    }
                }
                break;
            }
        }
    }
    else
    {
        mpShape->_setPropertyValue(aPropertyName, aValue);
    }

    if( mpModel )
        mpModel->SetModified();
}

css::uno::Any SAL_CALL SdXShape::getPropertyValue( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    uno::Any aRet;

    const SfxItemPropertyMapEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

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
            aRet <<= ( pInfo?pInfo->meClickAction:presentation::ClickAction_NONE );
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
            aRet <<= pInfo ? pInfo->maBlueScreen : Color(0x00ffffff);
            break;
        case WID_VERB:
            aRet <<= static_cast<sal_Int32>( pInfo?pInfo->mnVerb:0 );
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
        case WID_IMAGEMAP:
            {
                uno::Reference< uno::XInterface > xImageMap;

                SdDrawDocument* pDoc = mpModel?mpModel->GetDoc():nullptr;
                if( pDoc )
                {

                    SvxIMapInfo* pIMapInfo = SvxIMapInfo::GetIMapInfo(mpShape->GetSdrObject());
                    if( pIMapInfo )
                    {
                        const ImageMap& rIMap = pIMapInfo->GetImageMap();
                        xImageMap = SvUnoImageMap_createInstance( rIMap, ImplGetSupportedMacroItems() );
                    }
                    else
                    {
                        xImageMap = SvUnoImageMap_createInstance();
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
    }

    return aRet;
}

/** */
SdAnimationInfo* SdXShape::GetAnimationInfo( bool bCreate ) const
{
    SdAnimationInfo* pInfo = nullptr;

    SdrObject* pObj = mpShape->GetSdrObject();
    if(pObj)
        pInfo = SdDrawDocument::GetShapeUserData(*pObj, bCreate);

    return pInfo;
}

uno::Sequence< OUString > SAL_CALL SdXShape::getSupportedServiceNames()
{
    std::vector<std::u16string_view> aAdd{ u"com.sun.star.presentation.Shape",
                                       u"com.sun.star.document.LinkTarget" };

    SdrObject* pObj = mpShape->GetSdrObject();
    if(pObj && pObj->GetObjInventor() == SdrInventor::Default )
    {
        sal_uInt32 nInventor = pObj->GetObjIdentifier();
        switch( nInventor )
        {
        case OBJ_TITLETEXT:
            aAdd.emplace_back(u"com.sun.star.presentation.TitleTextShape");
            break;
        case OBJ_OUTLINETEXT:
            aAdd.emplace_back(u"com.sun.star.presentation.OutlinerShape");
            break;
        }
    }
    return comphelper::concatSequences(mpShape->_getSupportedServiceNames(), aAdd);
}

/** checks if this is a presentation object
 */
bool SdXShape::IsPresObj() const
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if(pObj)
    {
        SdPage* pPage = dynamic_cast<SdPage* >(pObj->getSdrPageFromSdrObject());
        if(pPage)
            return pPage->GetPresObjKind(pObj) != PresObjKind::NONE;
    }
    return false;
}

/** checks if this presentation object is empty
 */
bool SdXShape::IsEmptyPresObj() const
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if( (pObj != nullptr) && pObj->IsEmptyPresObj() )
    {
        // check if the object is in edit, then if it's temporarily not empty
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
        if( pTextObj == nullptr )
            return true;

        return !pTextObj->CanCreateEditOutlinerParaObject();
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

    SdPage* pPage = dynamic_cast< SdPage* >(pObj->getSdrPageFromSdrObject());
    DBG_ASSERT( pPage, "no page?" );
    if( pPage == nullptr )
        return OUString();

    return pPage->GetPresObjText( pPage->GetPresObjKind(pObj) );
}

/** sets/reset the empty status of a presentation object
*/
void SdXShape::SetEmptyPresObj(bool bEmpty)
{
    // only possible if this actually *is* a presentation object
    if( !IsPresObj() )
        return;

    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        return;

    if( pObj->IsEmptyPresObj() == bEmpty )
        return;

    if(!bEmpty)
    {
        OutlinerParaObject* pOutlinerParaObject = pObj->GetOutlinerParaObject();
        const bool bVertical = pOutlinerParaObject && pOutlinerParaObject->IsEffectivelyVertical();

        // really delete SdrOutlinerObj at pObj
        pObj->NbcSetOutlinerParaObject(std::nullopt);
        if( bVertical )
            if (auto pTextObj = dynamic_cast<SdrTextObj*>( pObj ) )
                pTextObj->SetVerticalWriting( true );

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
                pOleObj->ClearGraphic();
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

            SdOutliner* pOutliner = pDoc->GetInternalOutliner();
            DBG_ASSERT( pOutliner, "no outliner?" );
            if( pOutliner == nullptr )
                break;

            SdPage* pPage = dynamic_cast< SdPage* >(pObj->getSdrPageFromSdrObject());
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

bool SdXShape::IsMasterDepend() const noexcept
{
    SdrObject* pObj = mpShape->GetSdrObject();
    return pObj && pObj->GetUserCall() != nullptr;
}

void SdXShape::SetMasterDepend( bool bDepend ) noexcept
{
    if( IsMasterDepend() == bDepend )
        return;

    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj )
    {
        if( bDepend )
        {
            SdPage* pPage = dynamic_cast< SdPage* >(pObj->getSdrPageFromSdrObject());
            pObj->SetUserCall( pPage );
        }
        else
        {
            pObj->SetUserCall( nullptr );
        }
    }
}

void SdXShape::SetStyleSheet( const uno::Any& rAny )
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        throw beans::UnknownPropertyException();

    uno::Reference< style::XStyle > xStyle( rAny, uno::UNO_QUERY );
    SfxStyleSheet* pStyleSheet = SfxUnoStyleSheet::getUnoStyleSheet( xStyle );

    const SfxStyleSheet* pOldStyleSheet = pObj->GetStyleSheet();
    if( pOldStyleSheet == pStyleSheet )
        return;

    if( pStyleSheet == nullptr || (pStyleSheet->GetFamily() != SfxStyleFamily::Para && pStyleSheet->GetFamily() != SfxStyleFamily::Page) )
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

uno::Any SdXShape::GetStyleSheet() const
{
    SdrObject* pObj = mpShape->GetSdrObject();
    if( pObj == nullptr )
        throw beans::UnknownPropertyException();

    SfxStyleSheet* pStyleSheet = pObj->GetStyleSheet();
    // it is possible for shapes inside a draw to have a presentation style
    // but we don't want this for the api
    if( (pStyleSheet == nullptr) || ((pStyleSheet->GetFamily() != SfxStyleFamily::Para) && !mpModel->IsImpressDocument()) )
        return Any();

    return Any( uno::Reference< style::XStyle >( dynamic_cast< SfxUnoStyleSheet* >( pStyleSheet ) ) );
}

class SdUnoEventsAccess : public cppu::WeakImplHelper< css::container::XNameReplace, css::lang::XServiceInfo >
{
private:
    SdXShape*   mpShape;

public:
    explicit SdUnoEventsAccess(SdXShape* pShape) noexcept;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

// XEventsSupplier
uno::Reference< container::XNameReplace > SAL_CALL SdXShape::getEvents(  )
{
    return new SdUnoEventsAccess( this );
}

constexpr OUStringLiteral gaStrOnClick( u"OnClick" );
constexpr OUStringLiteral gaStrServiceName( u"com.sun.star.documents.Events" );
constexpr OUStringLiteral gaStrEventType( u"EventType" );
constexpr OUStringLiteral gaStrPresentation( u"Presentation" );
constexpr OUStringLiteral gaStrLibrary(u"Library");
constexpr OUStringLiteral gaStrMacroName(u"MacroName");
constexpr OUStringLiteral gaStrClickAction( u"ClickAction" );
constexpr OUStringLiteral gaStrBookmark( u"Bookmark" );
constexpr OUStringLiteral gaStrEffect( u"Effect" );
constexpr OUStringLiteral gaStrPlayFull( u"PlayFull" );
constexpr OUStringLiteral gaStrVerb( u"Verb" );
constexpr OUStringLiteral gaStrSoundURL( u"SoundURL" );
constexpr OUStringLiteral gaStrSpeed( u"Speed" );
constexpr OUStringLiteral gaStrStarBasic( u"StarBasic" );
constexpr OUStringLiteral gaStrScript( u"Script" );

SdUnoEventsAccess::SdUnoEventsAccess( SdXShape* pShape ) noexcept
  : mpShape( pShape )
{
}

namespace {

enum class FoundFlags {
    NONE          = 0x0000,
    ClickAction   = 0x0001,
    Bookmark      = 0x0002,
    Effect        = 0x0004,
    PlayFull      = 0x0008,
    Verb          = 0x0010,
    SoundUrl      = 0x0020,
    Speed         = 0x0040,
    EventType     = 0x0080,
    Macro         = 0x0100,
    Library       = 0x0200,
};

}

namespace o3tl {
    template<> struct typed_flags<FoundFlags> : is_typed_flags<FoundFlags, 0x03ff> {};
}

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
{
    if( mpShape == nullptr || aName != gaStrOnClick )
        throw container::NoSuchElementException();

    uno::Sequence< beans::PropertyValue > aProperties;
    if( !aElement.hasValue() || aElement.getValueType() != getElementType() || !(aElement >>= aProperties) )
        throw lang::IllegalArgumentException();

    FoundFlags nFound = FoundFlags::NONE;

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

    for( const beans::PropertyValue& rProperty : std::as_const(aProperties) )
    {
        if( !( nFound & FoundFlags::EventType ) && rProperty.Name == gaStrEventType )
        {
            if( rProperty.Value >>= aStrEventType )
            {
                nFound |= FoundFlags::EventType;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::ClickAction ) && rProperty.Name == gaStrClickAction )
        {
            if( rProperty.Value >>= eClickAction )
            {
                nFound |= FoundFlags::ClickAction;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::Macro ) && ( rProperty.Name == gaStrMacroName || rProperty.Name == gaStrScript ) )
        {
            if( rProperty.Value >>= aStrMacro )
            {
                nFound |= FoundFlags::Macro;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::Library ) && rProperty.Name == gaStrLibrary )
        {
            if( rProperty.Value >>= aStrLibrary )
            {
                nFound |= FoundFlags::Library;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::Effect ) && rProperty.Name == gaStrEffect )
        {
            if( rProperty.Value >>= eEffect )
            {
                nFound |= FoundFlags::Effect;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::Bookmark ) && rProperty.Name == gaStrBookmark )
        {
            if( rProperty.Value >>= aStrBookmark )
            {
                nFound |= FoundFlags::Bookmark;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::Speed ) && rProperty.Name == gaStrSpeed )
        {
            if( rProperty.Value >>= eSpeed )
            {
                nFound |= FoundFlags::Speed;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::SoundUrl ) && rProperty.Name == gaStrSoundURL )
        {
            if( rProperty.Value >>= aStrSoundURL )
            {
                nFound |= FoundFlags::SoundUrl;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::PlayFull ) && rProperty.Name == gaStrPlayFull )
        {
            if( rProperty.Value >>= bPlayFull )
            {
                nFound |= FoundFlags::PlayFull;
                continue;
            }
        }
        else if( !( nFound & FoundFlags::Verb ) && rProperty.Name == gaStrVerb )
        {
            if( rProperty.Value >>= nVerb )
            {
                nFound |= FoundFlags::Verb;
                continue;
            }
        }

        throw lang::IllegalArgumentException();
    }

    bool bOk = false;
    do
    {
        if( !( nFound & FoundFlags::EventType ) )
            break;

        if( aStrEventType == gaStrPresentation )
        {
            if( !( nFound & FoundFlags::ClickAction ) )
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
                if( nFound & FoundFlags::Bookmark )
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
                if( nFound & FoundFlags::Macro )
                {
                    pInfo->SetBookmark( aStrMacro );
                    bOk = true;
                }
                break;

            case presentation::ClickAction_VERB:
                if( nFound & FoundFlags::Verb )
                {
                    pInfo->mnVerb = static_cast<sal_uInt16>(nVerb);
                    bOk = true;
                }
                break;

            case presentation::ClickAction_VANISH:
                if( !( nFound & FoundFlags::Effect ) )
                    break;

                pInfo->meSecondEffect = eEffect;
                pInfo->meSecondSpeed = nFound & FoundFlags::Speed ? eSpeed : presentation::AnimationSpeed_MEDIUM;

                bOk = true;

                [[fallthrough]];

            case presentation::ClickAction_SOUND:
                if( nFound & FoundFlags::SoundUrl )
                {
                    pInfo->SetBookmark( aStrSoundURL );
                    if( eClickAction != presentation::ClickAction_SOUND )
                        pInfo->mbSecondSoundOn = !aStrSoundURL.isEmpty();
                    pInfo->mbSecondPlayFull = (nFound & FoundFlags::PlayFull) && bPlayFull;

                    bOk = true;
                }
                break;
            default:
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
                sal_Int32 nIdx{ 0 };
                const OUString aLibName   = aStrMacro.getToken(0, '.', nIdx);
                const OUString aModulName = aStrMacro.getToken(0, '.', nIdx);
                const OUString aMacroName = aStrMacro.getToken(0, '.', nIdx);

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
{
    if( mpShape == nullptr || aName != gaStrOnClick )
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
            aAny <<= OUString(gaStrScript);
            pProperties->Name = gaStrEventType;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= pInfo->GetBookmark();
            pProperties->Name = gaStrScript;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;
        }
        else
        {
            // Old Basic macro URL
            aAny <<= OUString(gaStrStarBasic);
            pProperties->Name = gaStrEventType;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            OUString aMacro = pInfo->GetBookmark();

            // aMacro has got following format:
            // "Macroname.Modulname.Libname.Documentname" or
            // "Macroname.Modulname.Libname.Applicationname"
            sal_Int32 nIdx{ 0 };
            const OUString aMacroName = aMacro.getToken(0, '.', nIdx);
            const OUString aModulName = aMacro.getToken(0, '.', nIdx);
            const OUString aLibName   = aMacro.getToken(0, '.', nIdx);

            OUString sBuffer = aLibName +
                "."  +
                aModulName  +
                "."  +
                aMacroName;

            aAny <<= sBuffer;
            pProperties->Name = gaStrMacroName;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= OUString( "StarOffice" );
            pProperties->Name = gaStrLibrary;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
        }
    }
    else
    {
        aAny <<= OUString(gaStrPresentation);
        pProperties->Name = gaStrEventType;
        pProperties->Handle = -1;
        pProperties->Value = aAny;
        pProperties->State = beans::PropertyState_DIRECT_VALUE;
        pProperties++;

        aAny <<= eClickAction;
        pProperties->Name = gaStrClickAction;
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
                pProperties->Name = gaStrBookmark;
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
                pProperties->Name = gaStrBookmark;
                pProperties->Handle = -1;
                pProperties->Value <<= aString;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_VANISH:
            aAny <<= pInfo->meSecondEffect;
            pProperties->Name = gaStrEffect;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            aAny <<= pInfo->meSecondSpeed;
            pProperties->Name = gaStrSpeed;
            pProperties->Handle = -1;
            pProperties->Value = aAny;
            pProperties->State = beans::PropertyState_DIRECT_VALUE;
            pProperties++;

            [[fallthrough]];

        case presentation::ClickAction_SOUND:
            if( eClickAction == presentation::ClickAction_SOUND || pInfo->mbSecondSoundOn )
            {
                aAny <<= pInfo->GetBookmark();
                pProperties->Name = gaStrSoundURL;
                pProperties->Handle = -1;
                pProperties->Value = aAny;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
                pProperties++;

                pProperties->Name = gaStrPlayFull;
                pProperties->Handle = -1;
                pProperties->Value <<= pInfo->mbSecondPlayFull;
                pProperties->State = beans::PropertyState_DIRECT_VALUE;
            }
            break;

        case presentation::ClickAction_VERB:
            aAny <<= static_cast<sal_Int32>(pInfo->mnVerb);
            pProperties->Name = gaStrVerb;
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
{
    return { gaStrOnClick };
}

sal_Bool SAL_CALL SdUnoEventsAccess::hasByName( const OUString& aName )
{
    return aName == gaStrOnClick;
}

// XElementAccess
uno::Type SAL_CALL SdUnoEventsAccess::getElementType(  )
{
    return cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get();
}

sal_Bool SAL_CALL SdUnoEventsAccess::hasElements(  )
{
    return true;
}

// XServiceInfo
OUString SAL_CALL SdUnoEventsAccess::getImplementationName(  )
{
    return "SdUnoEventsAccess";
}

sal_Bool SAL_CALL SdUnoEventsAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdUnoEventsAccess::getSupportedServiceNames(  )
{
    return { gaStrServiceName };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
