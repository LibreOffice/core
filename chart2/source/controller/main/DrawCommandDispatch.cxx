/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "DrawCommandDispatch.hxx"
#include "DrawCommandDispatch.hrc"
#include "ChartController.hxx"
#include "DrawViewWrapper.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "macros.hxx"

#include <vos/mutex.hxx>
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
#include <svx/svdlegacy.hxx>

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
    XLineEndListSharedPtr aLineEndList = rModel.GetLineEndListFromSdrModel();
    if ( aLineEndList.get() )
    {
        String aName( SVX_RES( nResId ) );
        long nCount = aLineEndList->Count();
        for ( long nIndex = 0; nIndex < nCount; ++nIndex )
        {
            XLineEndEntry* pEntry = aLineEndList->GetLineEnd( nIndex );
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
        if ( pDrawModelWrapper && pDrawViewWrapper && pDrawViewWrapper->getSdrObjectCreationInfo().getIdent() == OBJ_CUSTOMSHAPE )
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
                                    SfxItemSet aDest( pObj->GetObjectItemPool(), // ranges from SdrAttrObj
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

                                    const long aOldRotation(sdr::legacy::GetRotateAngle(*pSourceObj));
                                    if ( aOldRotation )
                                    {
                                        sdr::legacy::RotateSdrObject(*pObj, sdr::legacy::GetSnapRect(*pObj).Center(), aOldRotation);
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
                pObj->SetMergedItem( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False ) );
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

            SfxItemSet aSet( pDrawViewWrapper->getSdrModelFromSdrView().GetItemPool() );
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
    SdrObjKind eKind(OBJ_NONE);
    bool bFreehandMode(false);
    SdrPathObjType aSdrPathObjType(PathType_Line);
    SdrCircleObjType aSdrCircleObjType(CircleType_Circle);
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
                    eKind = OBJ_POLY;
                    aSdrPathObjType = PathType_Line;
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
                    aSdrCircleObjType = CircleType_Circle;
                }
                break;
            case COMMAND_ID_DRAW_FREELINE_NOFILL:
                {
                    eDrawMode = CHARTDRAW_INSERT;
                    eKind = OBJ_POLY;
                    aSdrPathObjType = PathType_OpenBezier;
                    bFreehandMode = true;
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
                ::vos::OGuard aGuard( Application::GetSolarMutex() );
                m_pChartController->setDrawMode( eDrawMode );
                setInsertObj(SdrObjectCreationInfo(static_cast< sal_uInt16 >(eKind), SdrInventor, aSdrPathObjType, aSdrCircleObjType, bFreehandMode));

                if ( bCreate )
                {
                    pDrawViewWrapper->SetViewEditMode(SDREDITMODE_CREATE);
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
                            pDrawViewWrapper->InsertObjectAtView( *pObj );
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

void DrawCommandDispatch::setInsertObj(const SdrObjectCreationInfo& rSdrObjectCreationInfo)
{
    DrawViewWrapper* pDrawViewWrapper = ( m_pChartController ? m_pChartController->GetDrawViewWrapper() : NULL );
    if ( pDrawViewWrapper )
    {
        pDrawViewWrapper->setSdrObjectCreationInfo(rSdrObjectCreationInfo);
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
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            pObj = SdrObjFactory::MakeNewObject(
                pDrawViewWrapper->getSdrModelFromSdrView(),
                pDrawViewWrapper->getSdrObjectCreationInfo());
            if ( pObj )
            {
                const basegfx::B2DVector aObjectSize(4000.0, 2500.0);
                const basegfx::B2DRange aPageRange(basegfx::B2DPoint(0.0, 0.0), pPage->GetPageScale());
                const basegfx::B2DPoint aObjectPos(aPageRange.getCenter() - (aObjectSize * 0.5));
                const basegfx::B2DRange aRange(aObjectPos, aObjectPos + aObjectSize);

                switch ( nID )
                {
                    case COMMAND_ID_DRAW_LINE:
                    case COMMAND_ID_LINE_ARROW_END:
                        {
                            SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

                            if ( pSdrPathObj )
                            {
                                const double nYMiddle((aRange.getMinY() + aRange.getMaxY()) * 0.5);
                                basegfx::B2DPolygon aPoly;
                                aPoly.append(basegfx::B2DPoint(aRange.getMinX(), nYMiddle));
                                aPoly.append(basegfx::B2DPoint(aRange.getMaxX(), nYMiddle));
                                pSdrPathObj->setB2DPolyPolygonInObjectCoordinates(basegfx::B2DPolyPolygon(aPoly));
                                SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                setLineEnds( aSet );
                                pObj->SetMergedItemSet( aSet );
                            }
                        }
                        break;
                    case COMMAND_ID_DRAW_FREELINE_NOFILL:
                        {
                            SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

                            if ( pSdrPathObj )
                            {
                                basegfx::B2DPolygon aInnerPoly;
                                aInnerPoly.append(basegfx::B2DPoint(aRange.getMinX(), aRange.getMaxY()));
                                aInnerPoly.appendBezierSegment(
                                    aRange.getMinimum(),
                                    basegfx::B2DPoint(aRange.getCenterX(), aRange.getMinY()),
                                    aRange.getCenter());
                                aInnerPoly.appendBezierSegment(
                                    basegfx::B2DPoint(aRange.getCenterX(), aRange.getMaxY()),
                                    aRange.getMaximum(),
                                    basegfx::B2DPoint(aRange.getMaxX(), aRange.getMinY()));
                                const basegfx::B2DPolyPolygon aPoly(aInnerPoly);
                                pSdrPathObj->setB2DPolyPolygonInObjectCoordinates( aPoly );
                            }
                        }
                        break;
                    case COMMAND_ID_DRAW_TEXT:
                    case COMMAND_ID_DRAW_TEXT_VERTICAL:
                        {
                                SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );

                                if ( pTextObj )
                                {
                                sdr::legacy::SetLogicRange(*pTextObj, aRange );
                                const sal_Bool bVertical = ( nID == SID_DRAW_TEXT_VERTICAL );
                                    pTextObj->SetVerticalWriting( bVertical );

                                    if ( bVertical )
                                    {
                                        SfxItemSet aSet( pDrawModelWrapper->GetItemPool() );
                                    aSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, sal_True ) );
                                    aSet.Put( SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, sal_False ) );
                                        aSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_TOP ) );
                                        aSet.Put( SdrTextHorzAdjustItem( SDRTEXTHORZADJUST_RIGHT ) );
                                        pTextObj->SetMergedItemSet( aSet );
                                    }
                                }
                            }
                        break;
                    case COMMAND_ID_DRAW_CAPTION:
                    case COMMAND_ID_DRAW_CAPTION_VERTICAL:
                        {
                            SdrCaptionObj* pSdrCaptionObj = dynamic_cast< SdrCaptionObj* >(pObj);

                            if ( pSdrCaptionObj )
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

                                sdr::legacy::SetLogicRange(*pSdrCaptionObj, aRange );
                                pSdrCaptionObj->SetTailPos(aRange.getMinimum() - (aRange.getRange() * 0.5));
                            }
                        }
                        break;
                    default:
                        {
                            sdr::legacy::SetLogicRange(*pObj, aRange );
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
    if ( nIndex == -1 || aToken.isEmpty() )
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
