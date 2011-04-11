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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "DrawCommandDispatch.hxx"
#include "DrawCommandDispatch.hrc"
#include "ChartController.hxx"
#include "DrawViewWrapper.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "macros.hxx"

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/itempool.hxx>
#include <editeng/adjitem.hxx>
#include <svx/dialogs.hrc>
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
#include <basegfx/polygon/b2dpolygon.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;


//.............................................................................
namespace
{
//.............................................................................

    // comparing two PropertyValue instances
    struct PropertyValueCompare : public ::std::binary_function< beans::PropertyValue, ::rtl::OUString, bool >
    {
        bool operator() ( const beans::PropertyValue& rPropValue, const ::rtl::OUString& rName ) const
        {
            return rPropValue.Name.equals( rName );
        }
        bool operator() ( const ::rtl::OUString& rName, const beans::PropertyValue& rPropValue ) const
        {
            return rName.equals( rPropValue.Name );
        }
    };

//.............................................................................
} // anonymous namespace
//.............................................................................


//.............................................................................
namespace chart
{
//.............................................................................

DrawCommandDispatch::DrawCommandDispatch( const Reference< uno::XComponentContext >& rxContext,
    ChartController* pController )
    :FeatureCommandDispatchBase( rxContext )
    ,m_pChartController( pController )
{
}

DrawCommandDispatch::~DrawCommandDispatch()
{
}

void DrawCommandDispatch::initialize()
{
    FeatureCommandDispatchBase::initialize();
}

bool DrawCommandDispatch::isFeatureSupported( const ::rtl::OUString& rCommandURL )
{
    sal_uInt16 nFeatureId = 0;
    ::rtl::OUString aBaseCommand;
    ::rtl::OUString aCustomShapeType;
    return parseCommandURL( rCommandURL, &nFeatureId, &aBaseCommand, &aCustomShapeType );
}

::basegfx::B2DPolyPolygon getPolygon( sal_uInt16 nResId, SdrModel& rModel )
{
    ::basegfx::B2DPolyPolygon aReturn;
    XLineEndList* pLineEndList = rModel.GetLineEndList();
    if ( pLineEndList )
    {
        String aName( SVX_RES( nResId ) );
        long nCount = pLineEndList->Count();
        for ( long nIndex = 0; nIndex < nCount; ++nIndex )
        {
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nIndex );
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
    if ( m_pChartController )
    {
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawModelWrapper && pDrawViewWrapper && pDrawViewWrapper->GetCurrentObjIdentifier() == OBJ_CUSTOMSHAPE )
        {
            sal_Bool bAttributesAppliedFromGallery = sal_False;
            if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
            {
                ::std::vector< ::rtl::OUString > aObjList;
                if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
                {
                    for ( sal_uInt16 i = 0; i < aObjList.size(); ++i )
                    {
                        if ( aObjList[ i ].equalsIgnoreAsciiCase( m_aCustomShapeType ) )
                        {
                            FmFormModel aModel;
                            SfxItemPool& rPool = aModel.GetItemPool();
                            rPool.FreezeIdRanges();
                            if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aModel ) )
                            {
                                const SdrObject* pSourceObj = aModel.GetPage( 0 )->GetObj( 0 );
                                if ( pSourceObj )
                                {
                                    const SfxItemSet& rSource = pSourceObj->GetMergedItemSet();
                                    SfxItemSet aDest( pObj->GetModel()->GetItemPool(),          // ranges from SdrAttrObj
                                        SDRATTR_START, SDRATTR_SHADOW_LAST,
                                        SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                                        SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                                        // Graphic Attributes
                                        SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,
                                        // 3d Properties
                                        SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                                        // CustomShape properties
                                        SDRATTR_CUSTOMSHAPE_FIRST, SDRATTR_CUSTOMSHAPE_LAST,
                                        // range from SdrTextObj
                                        EE_ITEMS_START, EE_ITEMS_END,
                                        // end
                                        0, 0);
                                    aDest.Set( rSource );
                                    pObj->SetMergedItemSet( aDest );
                                    sal_Int32 nAngle = pSourceObj->GetRotateAngle();
                                    if ( nAngle )
                                    {
                                        double a = nAngle * F_PI18000;
                                        pObj->NbcRotate( pObj->GetSnapRect().Center(), nAngle, sin( a ), cos( a ) );
                                    }
                                    bAttributesAppliedFromGallery = sal_True;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            if ( !bAttributesAppliedFromGallery )
            {
                pObj->SetMergedItem( SvxAdjustItem( SVX_ADJUST_CENTER, 0 ) );
                pObj->SetMergedItem( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
                pObj->SetMergedItem( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_BLOCK ) );
                pObj->SetMergedItem( SdrTextAutoGrowHeightItem( sal_False ) );
                ( dynamic_cast< SdrObjCustomShape* >( pObj ) )->MergeDefaultAttributes( &m_aCustomShapeType );
            }
        }
    }
}

void DrawCommandDispatch::setLineEnds( SfxItemSet& rAttr )
{
    if ( m_nFeatureId == COMMAND_ID_LINE_ARROW_END && m_pChartController )
    {
        DrawModelWrapper* pDrawModelWrapper = m_pChartController->GetDrawModelWrapper();
        DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
        if ( pDrawModelWrapper && pDrawViewWrapper )
        {
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

            SfxItemSet aSet( pDrawViewWrapper->GetModel()->GetItemPool() );
            pDrawViewWrapper->GetAttributes( aSet );

            long nWidth = 300; // (1/100th mm)
            if ( aSet.GetItemState( XATTR_LINEWIDTH ) != SFX_ITEM_DONTCARE )
            {
                long nValue = ( ( const XLineWidthItem& ) aSet.Get( XATTR_LINEWIDTH ) ).GetValue();
                if ( nValue > 0 )
                {
                    nWidth = nValue * 3;
                }
            }

            rAttr.Put( XLineEndItem( SVX_RESSTR( RID_SVXSTR_ARROW ), aArrow ) );
            rAttr.Put( XLineEndWidthItem( nWidth ) );
        }
    }
}

// WeakComponentImplHelperBase
void DrawCommandDispatch::disposing()
{
}

// XEventListener
void DrawCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
}

FeatureState DrawCommandDispatch::getState( const ::rtl::OUString& rCommand )
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    aReturn.aState <<= false;

    sal_uInt16 nFeatureId = 0;
    ::rtl::OUString aBaseCommand;
    ::rtl::OUString aCustomShapeType;
    if ( parseCommandURL( rCommand, &nFeatureId, &aBaseCommand, &aCustomShapeType ) )
    {
        switch ( nFeatureId )
        {
            case COMMAND_ID_OBJECT_SELECT:
            case COMMAND_ID_DRAW_LINE:
            case COMMAND_ID_LINE_ARROW_END:
            case COMMAND_ID_DRAW_RECT:
            case COMMAND_ID_DRAW_ELLIPSE:
            case COMMAND_ID_DRAW_FREELINE_NOFILL:
            case COMMAND_ID_DRAW_TEXT:
            case COMMAND_ID_DRAW_CAPTION:
            case COMMAND_ID_DRAWTBX_CS_BASIC:
            case COMMAND_ID_DRAWTBX_CS_SYMBOL:
            case COMMAND_ID_DRAWTBX_CS_ARROW:
            case COMMAND_ID_DRAWTBX_CS_FLOWCHART:
            case COMMAND_ID_DRAWTBX_CS_CALLOUT:
            case COMMAND_ID_DRAWTBX_CS_STAR:
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

void DrawCommandDispatch::execute( const ::rtl::OUString& rCommand, const Sequence< beans::PropertyValue>& rArgs )
{
    (void)rArgs;

    ChartDrawMode eDrawMode = CHARTDRAW_SELECT;
    SdrObjKind eKind = OBJ_NONE;
    bool bCreate = false;

    sal_uInt16 nFeatureId = 0;
    ::rtl::OUString aBaseCommand;
    ::rtl::OUString aCustomShapeType;
    if ( parseCommandURL( rCommand, &nFeatureId, &aBaseCommand, &aCustomShapeType ) )
    {
        m_nFeatureId = nFeatureId;
        m_aCustomShapeType = aCustomShapeType;

        switch ( nFeatureId )
        {
            case COMMAND_ID_OBJECT_SELECT:
                {
                    eDrawMode = CHARTDRAW_SELECT;
                    eKind = OBJ_NONE;
                }
                break;
            case COMMAND_ID_DRAW_LINE:
            case COMMAND_ID_LINE_ARROW_END:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_LINE;
                }
                break;
            case COMMAND_ID_DRAW_RECT:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_RECT;
                }
                break;
            case COMMAND_ID_DRAW_ELLIPSE:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_CIRC;
                }
                break;
            case COMMAND_ID_DRAW_FREELINE_NOFILL:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_FREELINE;
                }
                break;
            case COMMAND_ID_DRAW_TEXT:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_TEXT;
                    bCreate = true;
                }
                break;
            case COMMAND_ID_DRAW_CAPTION:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_CAPTION;
                }
                break;
            case COMMAND_ID_DRAWTBX_CS_BASIC:
            case COMMAND_ID_DRAWTBX_CS_SYMBOL:
            case COMMAND_ID_DRAWTBX_CS_ARROW:
            case COMMAND_ID_DRAWTBX_CS_FLOWCHART:
            case COMMAND_ID_DRAWTBX_CS_CALLOUT:
            case COMMAND_ID_DRAWTBX_CS_STAR:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_CUSTOMSHAPE;
                }
                break;
            default:
                {
                    eDrawMode = CHARTDRAW_SELECT;
                    eKind = OBJ_NONE;
                }
                break;
        }

        if ( m_pChartController )
        {
            DrawViewWrapper* pDrawViewWrapper = m_pChartController->GetDrawViewWrapper();
            if ( pDrawViewWrapper )
            {
                SolarMutexGuard aGuard;
                m_pChartController->setDrawMode( eDrawMode );
                setInsertObj( sal::static_int_cast< sal_uInt16 >( eKind ) );
                if ( bCreate )
                {
                    pDrawViewWrapper->SetCreateMode();
                }

                const ::rtl::OUString sKeyModifier( C2U( "KeyModifier" ) );
                const beans::PropertyValue* pIter = rArgs.getConstArray();
                const beans::PropertyValue* pEnd  = pIter + rArgs.getLength();
                const beans::PropertyValue* pKeyModifier = ::std::find_if(
                    pIter, pEnd, ::std::bind2nd( PropertyValueCompare(), boost::cref( sKeyModifier ) ) );
                sal_Int16 nKeyModifier = 0;
                if ( pKeyModifier && ( pKeyModifier->Value >>= nKeyModifier ) && nKeyModifier == KEY_MOD1 )
                {
                    if ( eDrawMode == CHARTDRAW_INSERT )
                    {
                        SdrObject* pObj = createDefaultObject( nFeatureId );
                        if ( pObj )
                        {
                            SdrPageView* pPageView = pDrawViewWrapper->GetSdrPageView();
                            pDrawViewWrapper->InsertObjectAtView( pObj, *pPageView );
                            Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
                            if ( xShape.is() )
                            {
                                m_pChartController->m_aSelection.setSelection( xShape );
                                m_pChartController->m_aSelection.applySelection( pDrawViewWrapper );
                            }
                            if ( nFeatureId == SID_DRAW_TEXT )
                            {
                                m_pChartController->StartTextEdit();
                            }
                        }
                    }
                }
            }
        }
    }
}

void DrawCommandDispatch::describeSupportedFeatures()
{
    implDescribeSupportedFeature( ".uno:SelectObject",      COMMAND_ID_OBJECT_SELECT,           CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Line",              COMMAND_ID_DRAW_LINE,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:LineArrowEnd",      COMMAND_ID_LINE_ARROW_END,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Rect",              COMMAND_ID_DRAW_RECT,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Ellipse",           COMMAND_ID_DRAW_ELLIPSE,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:Freeline_Unfilled", COMMAND_ID_DRAW_FREELINE_NOFILL,    CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DrawText",          COMMAND_ID_DRAW_TEXT,               CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DrawCaption",       COMMAND_ID_DRAW_CAPTION,            CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:BasicShapes",       COMMAND_ID_DRAWTBX_CS_BASIC,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:SymbolShapes",      COMMAND_ID_DRAWTBX_CS_SYMBOL,       CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:ArrowShapes",       COMMAND_ID_DRAWTBX_CS_ARROW,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:FlowChartShapes",   COMMAND_ID_DRAWTBX_CS_FLOWCHART,    CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:CalloutShapes",     COMMAND_ID_DRAWTBX_CS_CALLOUT,      CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:StarShapes",        COMMAND_ID_DRAWTBX_CS_STAR,         CommandGroup::INSERT );
}

void DrawCommandDispatch::setInsertObj( sal_uInt16 eObj )
{
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
    if ( pDrawViewWrapper )
    {
        pDrawViewWrapper->SetCurrentObj( eObj /*, Inventor */);
    }
}

SdrObject* DrawCommandDispatch::createDefaultObject( const sal_uInt16 nID )
{
    SdrObject* pObj = NULL;
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
    DrawModelWrapper* pDrawModelWrapper = ( m_pChartController ? m_pChartController->GetDrawModelWrapper() : NULL );

    if ( pDrawViewWrapper && pDrawModelWrapper )
    {
        Reference< drawing::XDrawPage > xDrawPage( pDrawModelWrapper->getMainDrawPage() );
        SdrPage* pPage = GetSdrPageFromXDrawPage( xDrawPage );
        if ( pPage )
        {
            SolarMutexGuard aGuard;
            pObj = SdrObjFactory::MakeNewObject( pDrawViewWrapper->GetCurrentObjInventor(),
                pDrawViewWrapper->GetCurrentObjIdentifier(), pPage );
            if ( pObj )
            {
                long nDefaultObjectSizeWidth = 4000;
                long nDefaultObjectSizeHeight = 2500;
                Size aObjectSize( nDefaultObjectSizeWidth, nDefaultObjectSizeHeight );
                Rectangle aPageRect( Rectangle( Point( 0, 0 ), pPage->GetSize() ) );
                Point aObjectPos = aPageRect.Center();
                aObjectPos.X() -= aObjectSize.Width() / 2;
                aObjectPos.Y() -= aObjectSize.Height() / 2;
                Rectangle aRect( aObjectPos, aObjectSize );

                switch ( nID )
                {
                    case COMMAND_ID_DRAW_LINE:
                    case COMMAND_ID_LINE_ARROW_END:
                        {
                            if ( pObj->ISA( SdrPathObj ) )
                            {
                                Point aStart = aRect.TopLeft();
                                Point aEnd = aRect.BottomRight();
                                sal_Int32 nYMiddle( ( aRect.Top() + aRect.Bottom() ) / 2 );
                                basegfx::B2DPolygon aPoly;
                                aPoly.append( basegfx::B2DPoint( aStart.X(), nYMiddle ) );
                                aPoly.append( basegfx::B2DPoint( aEnd.X(), nYMiddle ) );
                                ( dynamic_cast< SdrPathObj* >( pObj ) )->SetPathPoly( basegfx::B2DPolyPolygon( aPoly ) );
                                SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                setLineEnds( aSet );
                                pObj->SetMergedItemSet( aSet );
                            }
                        }
                        break;
                    case COMMAND_ID_DRAW_FREELINE_NOFILL:
                        {
                            if ( pObj->ISA( SdrPathObj ) )
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
                                ( dynamic_cast< SdrPathObj* >( pObj ) )->SetPathPoly( aPoly );
                            }
                        }
                        break;
                    case COMMAND_ID_DRAW_TEXT:
                    case COMMAND_ID_DRAW_TEXT_VERTICAL:
                        {
                            if ( pObj->ISA( SdrTextObj ) )
                            {
                                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                                if ( pTextObj )
                                {
                                    pTextObj->SetLogicRect( aRect );
                                    sal_Bool bVertical = ( nID == SID_DRAW_TEXT_VERTICAL );
                                    pTextObj->SetVerticalWriting( bVertical );
                                    if ( bVertical )
                                    {
                                        SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                        aSet.Put( SdrTextAutoGrowWidthItem( sal_True ) );
                                        aSet.Put( SdrTextAutoGrowHeightItem( sal_False ) );
                                        aSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_TOP ) );
                                        aSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_RIGHT ) );
                                        pTextObj->SetMergedItemSet( aSet );
                                    }
                                }
                            }
                        }
                        break;
                    case COMMAND_ID_DRAW_CAPTION:
                    case COMMAND_ID_DRAW_CAPTION_VERTICAL:
                        {
                            if ( pObj->ISA( SdrCaptionObj ) )
                            {
                                sal_Bool bIsVertical( SID_DRAW_CAPTION_VERTICAL == nID );
                                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                                if ( pTextObj )
                                {
                                    pTextObj->SetVerticalWriting( bIsVertical );
                                }
                                if ( bIsVertical )
                                {
                                    SfxItemSet aSet( pObj->GetMergedItemSet() );
                                    aSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
                                    aSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_RIGHT ) );
                                    pObj->SetMergedItemSet( aSet );
                                }
                                SdrCaptionObj* pCaptionObj = dynamic_cast< SdrCaptionObj* >( pObj );
                                if ( pCaptionObj )
                                {
                                    pCaptionObj->SetLogicRect( aRect );
                                    pCaptionObj->SetTailPos(
                                        aRect.TopLeft() - Point( aRect.GetWidth() / 2, aRect.GetHeight() / 2 ) );
                                }
                            }
                        }
                        break;
                    default:
                        {
                            pObj->SetLogicRect( aRect );
                            SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                            setAttributes( pObj );
                            pObj->SetMergedItemSet( aSet );
                        }
                        break;
                }
            }
        }
    }

    return pObj;
}

bool DrawCommandDispatch::parseCommandURL( const ::rtl::OUString& rCommandURL, sal_uInt16* pnFeatureId,
    ::rtl::OUString* pBaseCommand, ::rtl::OUString* pCustomShapeType )
{
    bool bFound = true;
    sal_uInt16 nFeatureId = 0;
    ::rtl::OUString aBaseCommand;
    ::rtl::OUString aType;

    sal_Int32 nIndex = 1;
    ::rtl::OUString aToken = rCommandURL.getToken( 0, '.', nIndex );
    if ( nIndex == -1 || !aToken.getLength() )
    {
        aBaseCommand = rCommandURL;
        SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( aBaseCommand );
        if ( aIter != m_aSupportedFeatures.end() )
        {
            nFeatureId = aIter->second.nFeatureId;

            switch ( nFeatureId )
            {
                case COMMAND_ID_DRAWTBX_CS_BASIC:
                    {
                        aType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "diamond" ) );
                    }
                    break;
                case COMMAND_ID_DRAWTBX_CS_SYMBOL:
                    {
                        aType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "smiley" ) );
                    }
                    break;
                case COMMAND_ID_DRAWTBX_CS_ARROW:
                    {
                        aType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "left-right-arrow" ) );
                    }
                    break;
                case COMMAND_ID_DRAWTBX_CS_FLOWCHART:
                    {
                        aType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "flowchart-internal-storage" ) );
                    }
                    break;
                case COMMAND_ID_DRAWTBX_CS_CALLOUT:
                    {
                        aType = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "round-rectangular-callout" ) );
                    }
                    break;
                case COMMAND_ID_DRAWTBX_CS_STAR:
                    {
                        aType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "star5" ) );
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

//.............................................................................
} //  namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
