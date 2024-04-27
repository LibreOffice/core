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

#include "DrawCommandDispatch.hxx"
#include <ChartController.hxx>
#include <DrawViewWrapper.hxx>
#include <chartview/DrawModelWrapper.hxx>

#include <com/sun/star/frame/CommandGroup.hpp>
#include <o3tl/unsafe_downcast.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/fmmodel.hxx>
#include <svx/gallery.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xtable.hxx>
#include <svx/sdtagitm.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;


namespace chart
{

DrawCommandDispatch::DrawCommandDispatch( const Reference< uno::XComponentContext >& rxContext,
    ChartController* pController )
    :FeatureCommandDispatchBase( rxContext )
    ,m_pChartController( pController )
{
}

DrawCommandDispatch::~DrawCommandDispatch()
{
}

bool DrawCommandDispatch::isFeatureSupported( const OUString& rCommandURL )
{
    ChartCommandID nFeatureId = ChartCommandID::NONE;
    OUString aBaseCommand;
    OUString aCustomShapeType;
    return parseCommandURL( rCommandURL, &nFeatureId, &aBaseCommand, &aCustomShapeType );
}

static ::basegfx::B2DPolyPolygon getPolygon(TranslateId pResId, const SdrModel& rModel)
{
    ::basegfx::B2DPolyPolygon aReturn;
    XLineEndListRef pLineEndList = rModel.GetLineEndList();
    if ( pLineEndList.is() )
    {
        OUString aName(SvxResId(pResId));
        tools::Long nCount = pLineEndList->Count();
        for ( tools::Long nIndex = 0; nIndex < nCount; ++nIndex )
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if ( pEntry->GetName() == aName )
            {
                aReturn = pEntry->GetLineEnd();
                break;
            }
        }
    }
    return aReturn;
}

void DrawCommandDispatch::setAttributes( SdrObject* pObj )
{
    if ( !m_pChartController )
        return;

    DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pDrawModelWrapper && pDrawViewWrapper && pDrawViewWrapper->GetCurrentObjIdentifier() == SdrObjKind::CustomShape) )
        return;

    bool bAttributesAppliedFromGallery = false;
    if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
    {
        std::vector< OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            for ( size_t i = 0; i < aObjList.size(); ++i )
            {
                if ( aObjList[ i ].equalsIgnoreAsciiCase( m_aCustomShapeType ) )
                {
                    FmFormModel aModel;

                    if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aModel ) )
                    {
                        const SdrObject* pSourceObj = aModel.GetPage( 0 )->GetObj( 0 );
                        if ( pSourceObj )
                        {
                            const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                            SfxItemSetFixed<
                                    // Ranges from SdrAttrObj:
                                    SDRATTR_START, SDRATTR_SHADOW_LAST,
                                    SDRATTR_MISC_FIRST,
                                        SDRATTR_MISC_LAST,
                                    SDRATTR_TEXTDIRECTION,
                                        SDRATTR_TEXTDIRECTION,
                                    // Graphic attributes, 3D
                                    // properties, CustomShape
                                    // properties:
                                    SDRATTR_GRAF_FIRST,
                                        SDRATTR_CUSTOMSHAPE_LAST,
                                    // Range from SdrTextObj:
                                    EE_ITEMS_START, EE_ITEMS_END>
                                aDest(pObj->getSdrModelFromSdrObject().GetItemPool());
                            aDest.Set( rSource );
                            pObj->SetMergedItemSet( aDest );
                            Degree100 nAngle = pSourceObj->GetRotateAngle();
                            if ( nAngle )
                                pObj->NbcRotate( pObj->GetSnapRect().Center(), nAngle );
                            bAttributesAppliedFromGallery = true;
                        }
                    }
                    break;
                }
            }
        }
    }
    if ( !bAttributesAppliedFromGallery )
    {
        pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
        pObj->SetMergedItem( makeSdrTextAutoGrowHeightItem( false ) );

        o3tl::unsafe_downcast< SdrObjCustomShape* >( pObj )->MergeDefaultAttributes( &m_aCustomShapeType );
    }
}

void DrawCommandDispatch::setLineEnds( SfxItemSet& rAttr )
{
    if ( !(m_nFeatureId == ChartCommandID::DrawLineArrowEnd && m_pChartController) )
        return;

    DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !(pDrawModelWrapper && pDrawViewWrapper) )
        return;

    ::basegfx::B2DPolyPolygon aArrow( getPolygon( RID_SVXSTR_ARROW, pDrawModelWrapper->getSdrModel() ) );
    if ( !aArrow.count() )
    {
        ::basegfx::B2DPolygon aNewArrow;
        aNewArrow.append( ::basegfx::B2DPoint( 10.0, 0.0 ) );
        aNewArrow.append( ::basegfx::B2DPoint( 0.0, 30.0) );
        aNewArrow.append( ::basegfx::B2DPoint( 20.0, 30.0 ) );
        aNewArrow.setClosed( true );
        aArrow.append( aNewArrow );
    }

    SfxItemSet aSet(pDrawViewWrapper->GetModel().GetItemPool());
    pDrawViewWrapper->GetAttributes( aSet );

    tools::Long nWidth = 300; // (1/100th mm)
    if ( aSet.GetItemState( XATTR_LINEWIDTH ) != SfxItemState::INVALID )
    {
        tools::Long nValue = aSet.Get( XATTR_LINEWIDTH ).GetValue();
        if ( nValue > 0 )
        {
            nWidth = nValue * 3;
        }
    }

    rAttr.Put( XLineEndItem( SvxResId( RID_SVXSTR_ARROW ), aArrow ) );
    rAttr.Put( XLineEndWidthItem( nWidth ) );
}

// WeakComponentImplHelperBase
void DrawCommandDispatch::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
}

// XEventListener
void DrawCommandDispatch::disposing( const lang::EventObject& /* Source */ )
{
}

FeatureState DrawCommandDispatch::getState( const OUString& rCommand )
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    aReturn.aState <<= false;

    ChartCommandID nFeatureId = ChartCommandID::NONE;
    OUString aBaseCommand;
    OUString aCustomShapeType;
    if ( parseCommandURL( rCommand, &nFeatureId, &aBaseCommand, &aCustomShapeType ) )
    {
        switch ( nFeatureId )
        {
            case ChartCommandID::DrawObjectSelect:
            case ChartCommandID::DrawLine:
            case ChartCommandID::DrawLineArrowEnd:
            case ChartCommandID::DrawRect:
            case ChartCommandID::DrawEllipse:
            case ChartCommandID::DrawFreelineNoFill:
            case ChartCommandID::DrawText:
            case ChartCommandID::DrawCaption:
            case ChartCommandID::DrawToolboxCsBasic:
            case ChartCommandID::DrawToolboxCsSymbol:
            case ChartCommandID::DrawToolboxCsArrow:
            case ChartCommandID::DrawToolboxCsFlowchart:
            case ChartCommandID::DrawToolboxCsCallout:
            case ChartCommandID::DrawToolboxCsStar:
                {
                    aReturn.bEnabled = true;
                    aReturn.aState <<= false;
                }
                break;
            default:
                {
                    aReturn.bEnabled = false;
                    aReturn.aState <<= false;
                }
                break;
        }
    }

    return aReturn;
}

void DrawCommandDispatch::execute( const OUString& rCommand, const Sequence< beans::PropertyValue>& rArgs )
{
    ChartDrawMode eDrawMode = CHARTDRAW_SELECT;
    SdrObjKind eKind = SdrObjKind::NONE;

    ChartCommandID nFeatureId = ChartCommandID::NONE;
    OUString aBaseCommand;
    OUString aCustomShapeType;
    if ( !parseCommandURL( rCommand, &nFeatureId, &aBaseCommand, &aCustomShapeType ) )
        return;

    bool bCreate = false;
    m_nFeatureId = nFeatureId;
    m_aCustomShapeType = aCustomShapeType;

    switch ( nFeatureId )
    {
        case ChartCommandID::DrawObjectSelect:
            {
                eDrawMode = CHARTDRAW_SELECT;
                eKind = SdrObjKind::NONE;
            }
            break;
        case ChartCommandID::DrawLine:
        case ChartCommandID::DrawLineArrowEnd:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::Line;
            }
            break;
        case ChartCommandID::DrawRect:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::Rectangle;
            }
            break;
        case ChartCommandID::DrawEllipse:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::CircleOrEllipse;
            }
            break;
        case ChartCommandID::DrawFreelineNoFill:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::FreehandLine;
            }
            break;
        case ChartCommandID::DrawText:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::Text;
                bCreate = true;
            }
            break;
        case ChartCommandID::DrawCaption:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::Caption;
            }
            break;
        case ChartCommandID::DrawToolboxCsBasic:
        case ChartCommandID::DrawToolboxCsSymbol:
        case ChartCommandID::DrawToolboxCsArrow:
        case ChartCommandID::DrawToolboxCsFlowchart:
        case ChartCommandID::DrawToolboxCsCallout:
        case ChartCommandID::DrawToolboxCsStar:
            {
                eDrawMode = CHARTDRAW_INSERT;
                eKind = SdrObjKind::CustomShape;
            }
            break;
        default:
            {
                eDrawMode = CHARTDRAW_SELECT;
                eKind = SdrObjKind::NONE;
            }
            break;
    }

    if ( !m_pChartController )
        return;

    DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
    if ( !pDrawViewWrapper )
        return;

    SolarMutexGuard aGuard;
    m_pChartController->setDrawMode( eDrawMode );
    setInsertObj(eKind);
    if ( bCreate )
    {
        pDrawViewWrapper->SetCreateMode();
    }

    const beans::PropertyValue* pKeyModifier = std::find_if(rArgs.begin(), rArgs.end(),
                                                    [](const beans::PropertyValue& lhs)
                                                    {return lhs.Name == "KeyModifier";} );
    sal_Int16 nKeyModifier = 0;
    if ( !(pKeyModifier != rArgs.end() && ( pKeyModifier->Value >>= nKeyModifier ) && nKeyModifier == KEY_MOD1) )
        return;

    if ( eDrawMode != CHARTDRAW_INSERT )
        return;

    rtl::Reference<SdrObject> pObj = createDefaultObject( nFeatureId );
    if ( pObj )
    {
        SdrPageView* pPageView = pDrawViewWrapper->GetSdrPageView();
        if (pDrawViewWrapper->InsertObjectAtView(pObj.get(), *pPageView))
            m_pChartController->SetAndApplySelection(Reference<drawing::XShape>(pObj->getUnoShape(), uno::UNO_QUERY));
        if ( nFeatureId == ChartCommandID::DrawText )
        {
            m_pChartController->StartTextEdit();
        }
    }
}

void DrawCommandDispatch::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:SelectObject",      ChartCommandID::DrawObjectSelect,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Line",              ChartCommandID::DrawLine,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:LineArrowEnd",      ChartCommandID::DrawLineArrowEnd,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Rect",              ChartCommandID::DrawRect,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Ellipse",           ChartCommandID::DrawEllipse,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Freeline_Unfilled", ChartCommandID::DrawFreelineNoFill,    CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DrawText",          ChartCommandID::DrawText,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DrawCaption",       ChartCommandID::DrawCaption,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes",       ChartCommandID::DrawToolboxCsBasic,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes",      ChartCommandID::DrawToolboxCsSymbol,       CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes",       ChartCommandID::DrawToolboxCsArrow,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes",   ChartCommandID::DrawToolboxCsFlowchart,    CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes",     ChartCommandID::DrawToolboxCsCallout,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes",        ChartCommandID::DrawToolboxCsStar,         CommandGroup::INSERT );
}

void DrawCommandDispatch::setInsertObj(SdrObjKind eObj)
{
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : nullptr );
    if ( pDrawViewWrapper )
    {
        pDrawViewWrapper->SetCurrentObj( eObj /*, Inventor */);
    }
}

rtl::Reference<SdrObject> DrawCommandDispatch::createDefaultObject( const ChartCommandID nID )
{
    rtl::Reference<SdrObject> pObj;
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : nullptr );
    DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : nullptr );

    if ( pDrawViewWrapper && pDrawModelWrapper )
    {
        Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
        SdrPage* pPage = GetSdrPageFromXDrawPage( xDrawPage );
        if ( pPage )
        {
            SolarMutexGuard aGuard;

            pObj = SdrObjFactory::MakeNewObject(
                pDrawModelWrapper->getSdrModel(),
                pDrawViewWrapper->GetCurrentObjInventor(),
                pDrawViewWrapper->GetCurrentObjIdentifier());

            if ( pObj )
            {
                Size aObjectSize( 4000, 2500 );
                tools::Rectangle aPageRect( tools::Rectangle( Point( 0, 0 ), pPage->GetSize() ) );
                Point aObjectPos = aPageRect.Center();
                aObjectPos.AdjustX( -(aObjectSize.Width() / 2) );
                aObjectPos.AdjustY( -(aObjectSize.Height() / 2) );
                tools::Rectangle aRect( aObjectPos, aObjectSize );

                switch ( nID )
                {
                    case ChartCommandID::DrawLine:
                    case ChartCommandID::DrawLineArrowEnd:
                        {
                            if ( auto const pathObj = dynamic_cast<SdrPathObj*>( pObj.get()) )
                            {
                                Point aStart = aRect.TopLeft();
                                Point aEnd = aRect.BottomRight();
                                sal_Int32 nYMiddle( ( aRect.Top() + aRect.Bottom() ) / 2 );
                                basegfx::B2DPolygon aPoly;
                                aPoly.append( basegfx::B2DPoint( aStart.X(), nYMiddle ) );
                                aPoly.append( basegfx::B2DPoint( aEnd.X(), nYMiddle ) );
                                pathObj->SetPathPoly(basegfx::B2DPolyPolygon(aPoly));
                                SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                setLineEnds( aSet );
                                pObj->SetMergedItemSet( aSet );
                            }
                        }
                        break;
                    case ChartCommandID::DrawFreelineNoFill:
                        {
                            if ( auto const pathObj = dynamic_cast<SdrPathObj*>( pObj.get()) )
                            {
                                basegfx::B2DPolygon aInnerPoly;
                                aInnerPoly.append( basegfx::B2DPoint( aRect.Left(), aRect.Bottom() ) );
                                aInnerPoly.appendBezierSegment(
                                    basegfx::B2DPoint( aRect.Left(), aRect.Top() ),
                                    basegfx::B2DPoint( aRect.Center().X(), aRect.Top() ),
                                    basegfx::B2DPoint( aRect.Center().X(), aRect.Center().Y() ) );
                                aInnerPoly.appendBezierSegment(
                                    basegfx::B2DPoint( aRect.Center().X(), aRect.Bottom() ),
                                    basegfx::B2DPoint( aRect.Right(), aRect.Bottom() ),
                                    basegfx::B2DPoint( aRect.Right(), aRect.Top() ) );
                                basegfx::B2DPolyPolygon aPoly;
                                aPoly.append( aInnerPoly );
                                pathObj->SetPathPoly(aPoly);
                            }
                        }
                        break;
                    case ChartCommandID::DrawText:
                    case ChartCommandID::DrawTextVertical:
                        {
                            if ( SdrTextObj* pTextObj = DynCastSdrTextObj( pObj.get()) )
                            {
                                pTextObj->SetLogicRect( aRect );
                                bool bVertical = ( nID == ChartCommandID::DrawTextVertical );
                                pTextObj->SetVerticalWriting( bVertical );
                                if ( bVertical )
                                {
                                    SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                    aSet.Put( makeSdrTextAutoGrowWidthItem( true ) );
                                    aSet.Put( makeSdrTextAutoGrowHeightItem( false ) );
                                    aSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_TOP ) );
                                    aSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_RIGHT ) );
                                    pTextObj->SetMergedItemSet( aSet );
                                }
                            }
                        }
                        break;
                    case ChartCommandID::DrawCaption:
                    case ChartCommandID::DrawCaptionVertical:
                        {
                            if ( SdrCaptionObj* pCaptionObj = dynamic_cast<SdrCaptionObj*>( pObj.get()) )
                            {
                                bool bIsVertical( nID == ChartCommandID::DrawCaptionVertical );
                                pCaptionObj->SetVerticalWriting( bIsVertical );
                                if ( bIsVertical )
                                {
                                    SfxItemSet aSet( pObj->GetMergedItemSet() );
                                    aSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
                                    aSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_RIGHT ) );
                                    pObj->SetMergedItemSet( aSet );
                                }
                                pCaptionObj->SetLogicRect( aRect );
                                pCaptionObj->SetTailPos(
                                    aRect.TopLeft() - Point( aRect.GetWidth() / 2, aRect.GetHeight() / 2 ) );
                            }
                        }
                        break;
                    default:
                        {
                            pObj->SetLogicRect( aRect );
                            SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                            setAttributes( pObj.get() );
                            pObj->SetMergedItemSet( aSet );
                        }
                        break;
                }
            }
        }
    }

    return pObj;
}

bool DrawCommandDispatch::parseCommandURL( const OUString& rCommandURL, ChartCommandID* pnFeatureId,
    OUString* pBaseCommand, OUString* pCustomShapeType )
{
    bool bFound = true;
    ChartCommandID nFeatureId = ChartCommandID::NONE;
    OUString aBaseCommand;
    OUString aType;

    sal_Int32 nIndex = std::min(sal_Int32(1), rCommandURL.getLength());
    std::u16string_view aToken = o3tl::getToken(rCommandURL, 0, '.', nIndex );
    if ( nIndex == -1 || aToken.empty() )
    {
        aBaseCommand = rCommandURL;
        SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( aBaseCommand );
        if ( aIter != m_aSupportedFeatures.end() )
        {
            nFeatureId = aIter->second.nFeatureId;

            switch ( nFeatureId )
            {
                case ChartCommandID::DrawToolboxCsBasic:
                    {
                        aType = "diamond";
                    }
                    break;
                case ChartCommandID::DrawToolboxCsSymbol:
                    {
                        aType = "smiley";
                    }
                    break;
                case ChartCommandID::DrawToolboxCsArrow:
                    {
                        aType = "left-right-arrow";
                    }
                    break;
                case ChartCommandID::DrawToolboxCsFlowchart:
                    {
                        aType = "flowchart-internal-storage";
                    }
                    break;
                case ChartCommandID::DrawToolboxCsCallout:
                    {
                        aType = "round-rectangular-callout";
                    }
                    break;
                case ChartCommandID::DrawToolboxCsStar:
                    {
                        aType = "star5";
                    }
                    break;
                default:
                    {
                    }
                    break;
            }
        }
        else
        {
            bFound = false;
        }
    }
    else
    {
        aBaseCommand = rCommandURL.copy( 0, nIndex - 1 );
        SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( aBaseCommand );
        if ( aIter != m_aSupportedFeatures.end() )
        {
            nFeatureId = aIter->second.nFeatureId;
            aType = rCommandURL.getToken( 0, '.', nIndex );
        }
        else
        {
            bFound = false;
        }
    }

    *pnFeatureId = nFeatureId;
    *pBaseCommand = aBaseCommand;
    *pCustomShapeType = aType;

    return bFound;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
