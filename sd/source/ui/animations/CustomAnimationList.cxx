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

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/scopeguard.hxx>
#include <CustomAnimationList.hxx>
#include <CustomAnimationPreset.hxx>
#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <sdresid.hxx>

#include <strings.hrc>
#include <bitmaps.hlst>

#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::container::XChild;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;

namespace sd {

// go recursively through all shapes in the given XShapes collection and return true as soon as the
// given shape is found. nIndex is incremented for each shape with the same shape type as the given
// shape is found until the given shape is found.
static bool getShapeIndex(  const Reference< XShapes >& xShapes, const Reference< XShape >& xShape, sal_Int32& nIndex )
{
    const sal_Int32 nCount = xShapes->getCount();
    sal_Int32 n;
    for( n = 0; n < nCount; n++ )
    {
        Reference< XShape > xChild;
        xShapes->getByIndex( n ) >>= xChild;
        if( xChild == xShape )
            return true;

        if( xChild->getShapeType() == xShape->getShapeType() )
            nIndex++;

        Reference< XShapes > xChildContainer( xChild, UNO_QUERY );
        if( xChildContainer.is() )
        {
            if( getShapeIndex( xChildContainer, xShape, nIndex ) )
                return true;
        }
    }

    return false;
}

// returns the index of the shape type from the given shape
static sal_Int32 getShapeIndex( const Reference< XShape >& xShape )
{
    Reference< XChild > xChild( xShape, UNO_QUERY );
    Reference< XShapes > xPage;

    while( xChild.is() && !xPage.is() )
    {
        Reference< XInterface > x( xChild->getParent() );
        xChild.set( x, UNO_QUERY );
        Reference< XDrawPage > xTestPage( x, UNO_QUERY );
        if( xTestPage.is() )
            xPage.set( x, UNO_QUERY );
    }

    sal_Int32 nIndex = 1;

    if( xPage.is() && getShapeIndex( xPage, xShape, nIndex ) )
        return nIndex;
    else
        return -1;
}

OUString getShapeDescription( const Reference< XShape >& xShape, bool bWithText )
{
    OUString aDescription;
    Reference< XPropertySet > xSet( xShape, UNO_QUERY );
    bool bAppendIndex = true;

    if(xSet.is()) try
    {
        Reference<XPropertySetInfo> xInfo(xSet->getPropertySetInfo());
        if (xInfo.is())
        {
            static constexpr OUString aPropName1(u"Name"_ustr);
            if(xInfo->hasPropertyByName(aPropName1))
                xSet->getPropertyValue(aPropName1) >>= aDescription;

            bAppendIndex = aDescription.isEmpty();

            static constexpr OUString aPropName2(u"UINameSingular"_ustr);
            if(xInfo->hasPropertyByName(aPropName2))
                xSet->getPropertyValue(aPropName2) >>= aDescription;
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::getShapeDescription()" );
    }

    if (bAppendIndex)
    {
        aDescription += " " + OUString::number(getShapeIndex(xShape));
    }

    if( bWithText )
    {
        Reference< XTextRange > xText( xShape, UNO_QUERY );
        if( xText.is() )
        {
            OUString aText( xText->getString() );
            if( !aText.isEmpty() )
            {
                aDescription += ": ";

                aText = aText.replace( '\n', ' ' );
                aText = aText.replace( '\r', ' ' );

                aDescription += aText;
            }
        }
    }
    return aDescription;
}

static OUString getDescription( const Any& rTarget, bool bWithText )
{
    OUString aDescription;

    if( rTarget.getValueType() == ::cppu::UnoType<ParagraphTarget>::get() )
    {
        ParagraphTarget aParaTarget;
        rTarget >>= aParaTarget;

        css::uno::Reference<css::document::XActionLockable> xLockable(aParaTarget.Shape, css::uno::UNO_QUERY);
        if (xLockable.is())
            xLockable->addActionLock();
        comphelper::ScopeGuard aGuard([&xLockable]()
        {
            if (xLockable.is())
                xLockable->removeActionLock();
        });

        Reference< XEnumerationAccess > xText( aParaTarget.Shape, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xText->createEnumeration(), css::uno::UNO_SET_THROW );
        sal_Int32 nPara = aParaTarget.Paragraph;

        while( xEnumeration->hasMoreElements() && nPara )
        {
            xEnumeration->nextElement();
            nPara--;
        }

        DBG_ASSERT( xEnumeration->hasMoreElements(), "sd::CustomAnimationEffect::prepareText(), paragraph out of range!" );

        if( xEnumeration->hasMoreElements() )
        {
            Reference< XTextRange > xParagraph;
            xEnumeration->nextElement() >>= xParagraph;

            if( xParagraph.is() )
                aDescription = xParagraph->getString();
        }
    }
    else
    {
        Reference< XShape > xShape;
        rTarget >>= xShape;
        if( xShape.is() )
            aDescription = getShapeDescription( xShape, bWithText );
    }

    return aDescription;
}

class CustomAnimationListEntryItem
{
public:
    CustomAnimationListEntryItem(OUString aDescription,
                                 CustomAnimationEffectPtr pEffect);
    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

    Size GetSize(const vcl::RenderContext& rRenderContext);
    void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect, bool bSelected);
    void PaintEffect(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect, bool bSelected);
    void PaintTrigger(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect);

private:
    OUString        msDescription;
    OUString        msEffectName;
    CustomAnimationEffectPtr mpEffect;

public:
    static const ::tools::Long nIconWidth = 19;
    static const ::tools::Long nItemMinHeight = 38;
};

CustomAnimationListEntryItem::CustomAnimationListEntryItem(OUString aDescription, CustomAnimationEffectPtr pEffect)
    : msDescription(std::move(aDescription))
    , mpEffect(std::move(pEffect))
{
    if (!mpEffect)
        return;
    switch (mpEffect->getPresetClass())
    {
    case EffectPresetClass::ENTRANCE:
        msEffectName = SdResId(STR_CUSTOMANIMATION_ENTRANCE); break;
    case EffectPresetClass::EXIT:
        msEffectName = SdResId(STR_CUSTOMANIMATION_EXIT); break;
    case EffectPresetClass::EMPHASIS:
        msEffectName = SdResId(STR_CUSTOMANIMATION_EMPHASIS); break;
    case EffectPresetClass::MOTIONPATH:
        msEffectName = SdResId(STR_CUSTOMANIMATION_MOTION_PATHS); break;
    default:
        msEffectName = SdResId(STR_CUSTOMANIMATION_MISC); break;
    }
    msEffectName = msEffectName.replaceFirst( "%1" , CustomAnimationPresets::getCustomAnimationPresets().getUINameForPresetId(mpEffect->getPresetId()));
}

IMPL_STATIC_LINK(CustomAnimationList, CustomRenderHdl, weld::TreeView::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    CustomAnimationListEntryItem* pItem = weld::fromId<CustomAnimationListEntryItem*>(rId);

    pItem->Paint(rRenderContext, rRect, bSelected);
}

IMPL_STATIC_LINK(CustomAnimationList, CustomGetSizeHdl, weld::TreeView::get_size_args, aPayload, Size)
{
    vcl::RenderContext& rRenderContext = aPayload.first;
    const OUString& rId = aPayload.second;

    CustomAnimationListEntryItem* pItem = weld::fromId<CustomAnimationListEntryItem*>(rId);
    if (!pItem)
        return Size(CustomAnimationListEntryItem::nIconWidth, CustomAnimationListEntryItem::nItemMinHeight);
    return pItem->GetSize(rRenderContext);
}

Size CustomAnimationListEntryItem::GetSize(const vcl::RenderContext& rRenderContext)
{
    auto width = rRenderContext.GetTextWidth( msDescription ) + nIconWidth;
    if (width < (rRenderContext.GetTextWidth( msEffectName ) + 2*nIconWidth))
        width = rRenderContext.GetTextWidth( msEffectName ) + 2*nIconWidth;

    Size aSize(width, rRenderContext.GetTextHeight());
    if (aSize.Height() < nItemMinHeight)
        aSize.setHeight(nItemMinHeight);
    return aSize;
}

void CustomAnimationListEntryItem::PaintTrigger(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect)
{
    Size aSize(rRect.GetSize());

    ::tools::Rectangle aOutRect(rRect);

    rRenderContext.Push();
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetDialogColor());
    rRenderContext.SetLineColor();
    // fill the background with the dialog bg color
    rRenderContext.DrawRect(aOutRect);

    // Erase the four corner pixels to make the rectangle appear rounded.
    rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.DrawPixel(aOutRect.TopLeft());
    rRenderContext.DrawPixel(Point(aOutRect.Right(), aOutRect.Top()));
    rRenderContext.DrawPixel(Point(aOutRect.Left(), aOutRect.Bottom()));
    rRenderContext.DrawPixel(Point(aOutRect.Right(), aOutRect.Bottom()));

    // draw the category title

    int nVertBorder = ((aSize.Height() - rRenderContext.GetTextHeight()) >> 1);
    int nHorzBorder = rRenderContext.LogicToPixel(Size(3, 3), MapMode(MapUnit::MapAppFont)).Width();

    aOutRect.AdjustLeft(nHorzBorder );
    aOutRect.AdjustRight( -nHorzBorder );
    aOutRect.AdjustTop( nVertBorder );
    aOutRect.AdjustBottom( -nVertBorder );

    // Draw the text with the dialog text color
    rRenderContext.SetTextColor(rRenderContext.GetSettings().GetStyleSettings().GetDialogTextColor());
    rRenderContext.DrawText(aOutRect, rRenderContext.GetEllipsisString(msDescription, aOutRect.GetWidth()));
    rRenderContext.Pop();
}

void CustomAnimationListEntryItem::PaintEffect(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect, bool bSelected)
{
    rRenderContext.Push(vcl::PushFlags::TEXTCOLOR);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (bSelected)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());

    Point aPos(rRect.TopLeft());
    int nItemHeight = rRect.GetHeight();

    sal_Int16 nNodeType = mpEffect->getNodeType();
    if (nNodeType == EffectNodeType::ON_CLICK )
    {
        rRenderContext.DrawImage(aPos, Image(StockImage::Yes, BMP_CUSTOMANIMATION_ON_CLICK));
    }
    else if (nNodeType == EffectNodeType::AFTER_PREVIOUS)
    {
        rRenderContext.DrawImage(aPos, Image(StockImage::Yes, BMP_CUSTOMANIMATION_AFTER_PREVIOUS));
    }
    else if (nNodeType == EffectNodeType::WITH_PREVIOUS)
    {
        //FIXME With previous image not defined in CustomAnimation.src
    }

    aPos.AdjustX(nIconWidth);

    //TODO, full width of widget ?
    rRenderContext.DrawText(aPos, rRenderContext.GetEllipsisString(msDescription, rRect.GetWidth()));

    aPos.AdjustY(nIconWidth);

    OUString sImage;
    switch (mpEffect->getPresetClass())
    {
    case EffectPresetClass::ENTRANCE:
        sImage = BMP_CUSTOMANIMATION_ENTRANCE_EFFECT; break;
    case EffectPresetClass::EXIT:
        sImage = BMP_CUSTOMANIMATION_EXIT_EFFECT; break;
    case EffectPresetClass::EMPHASIS:
        sImage = BMP_CUSTOMANIMATION_EMPHASIS_EFFECT; break;
    case EffectPresetClass::MOTIONPATH:
        sImage = BMP_CUSTOMANIMATION_MOTION_PATH; break;
    case EffectPresetClass::OLEACTION:
        sImage = BMP_CUSTOMANIMATION_OLE; break;
    case EffectPresetClass::MEDIACALL:
        switch (mpEffect->getCommand())
        {
        case EffectCommands::TOGGLEPAUSE:
            sImage = BMP_CUSTOMANIMATION_MEDIA_PAUSE; break;
        case EffectCommands::STOP:
            sImage = BMP_CUSTOMANIMATION_MEDIA_STOP; break;
        case EffectCommands::PLAY:
        default:
            sImage = BMP_CUSTOMANIMATION_MEDIA_PLAY; break;
        }
        break;
    default:
        break;
    }

    if (!sImage.isEmpty())
    {
        Image aImage(StockImage::Yes, sImage);
        Point aImagePos(aPos);
        aImagePos.AdjustY((nItemHeight/2 - aImage.GetSizePixel().Height()) >> 1 );
        rRenderContext.DrawImage(aImagePos, aImage);
    }

    aPos.AdjustX(nIconWidth );
    aPos.AdjustY((nItemHeight/2 - rRenderContext.GetTextHeight()) >> 1 );

    rRenderContext.DrawText(aPos, rRenderContext.GetEllipsisString(msEffectName, rRect.GetWidth()));
    rRenderContext.Pop();
}

void CustomAnimationListEntryItem::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect, bool bSelected)
{
    if (mpEffect)
        PaintEffect(rRenderContext, rRect, bSelected);
    else
        PaintTrigger(rRenderContext, rRect);
}

CustomAnimationList::CustomAnimationList(std::unique_ptr<weld::TreeView> xTreeView,
                                         std::unique_ptr<weld::Label> xLabel,
                                         std::unique_ptr<weld::Widget> xScrolledWindow)
    : mxTreeView(std::move(xTreeView))
    , maDropTargetHelper(*this)
    , mxEmptyLabel(std::move(xLabel))
    , mxEmptyLabelParent(std::move(xScrolledWindow))
    , mbIgnorePaint(false)
    , mpController(nullptr)
    , mnLastGroupId(0)
    , mnPostExpandEvent(nullptr)
    , mnPostCollapseEvent(nullptr)
{
    mxEmptyLabel->set_stack_background();

    mxTreeView->set_selection_mode(SelectionMode::Multiple);
    mxTreeView->connect_changed(LINK(this, CustomAnimationList, SelectHdl));
    mxTreeView->connect_key_press(LINK(this, CustomAnimationList, KeyInputHdl));
    mxTreeView->connect_popup_menu(LINK(this, CustomAnimationList, CommandHdl));
    mxTreeView->connect_row_activated(LINK(this, CustomAnimationList, DoubleClickHdl));
    mxTreeView->connect_expanding(LINK(this, CustomAnimationList, ExpandHdl));
    mxTreeView->connect_collapsing(LINK(this, CustomAnimationList, CollapseHdl));
    mxTreeView->connect_drag_begin(LINK(this, CustomAnimationList, DragBeginHdl));
    mxTreeView->connect_custom_get_size(LINK(this, CustomAnimationList, CustomGetSizeHdl));
    mxTreeView->connect_custom_render(LINK(this, CustomAnimationList, CustomRenderHdl));
    mxTreeView->set_column_custom_renderer(1, true);
}

CustomAnimationListDropTarget::CustomAnimationListDropTarget(CustomAnimationList& rTreeView)
    : DropTargetHelper(rTreeView.get_widget().get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 CustomAnimationListDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    sal_Int8 nAccept = m_rTreeView.AcceptDrop(rEvt);

    if (nAccept != DND_ACTION_NONE)
    {
        // to enable the autoscroll when we're close to the edges
        weld::TreeView& rWidget = m_rTreeView.get_widget();
        rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    }

    return nAccept;
}

sal_Int8 CustomAnimationListDropTarget::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    return m_rTreeView.ExecuteDrop(rEvt);
}

// D'n'D #1: Record selected effects for drag'n'drop.
IMPL_LINK(CustomAnimationList, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;

    // Record which effects are selected:
    // Since NextSelected(..) iterates through the selected items in the order they
    // were selected, create a sorted list for simpler drag'n'drop algorithms.
    mDndEffectsSelected.clear();
    mxTreeView->selected_foreach([this](weld::TreeIter& rEntry){
        mDndEffectsSelected.emplace_back(mxTreeView->make_iterator(&rEntry));
        return false;
    });

    // Note: pEntry is the effect with focus (if multiple effects are selected)
    mxDndEffectDragging = mxTreeView->make_iterator();
    if (!mxTreeView->get_cursor(mxDndEffectDragging.get()))
        mxDndEffectDragging.reset();

    // Allow normal processing.
    return false;
}

// D'n'D #3: Called each time mouse moves during drag
sal_Int8 CustomAnimationList::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 ret = DND_ACTION_NONE;

    const bool bIsMove = DND_ACTION_MOVE == rEvt.mnAction;
    if (mxDndEffectDragging && !rEvt.mbLeaving && bIsMove)
        ret = DND_ACTION_MOVE;
    return ret;
}

// D'n'D #5: Tell model to update effect order.
sal_Int8 CustomAnimationList::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    std::unique_ptr<weld::TreeIter> xDndEffectInsertBefore(mxTreeView->make_iterator());
    if (!mxTreeView->get_dest_row_at_pos(rEvt.maPosPixel, xDndEffectInsertBefore.get(), true))
        xDndEffectInsertBefore.reset();

    const bool bMovingEffect = ( mxDndEffectDragging != nullptr );
    const bool bMoveNotSelf  = !xDndEffectInsertBefore || (mxDndEffectDragging && mxTreeView->iter_compare(*xDndEffectInsertBefore, *mxDndEffectDragging) != 0);
    const bool bHaveSequence(mpMainSequence);

    if( bMovingEffect && bMoveNotSelf && bHaveSequence )
    {
        CustomAnimationListEntryItem* pTarget = xDndEffectInsertBefore ?
                                                    weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xDndEffectInsertBefore)) :
                                                    nullptr;

        // Build list of effects
        std::vector< CustomAnimationEffectPtr > aEffects;
        for( const auto &pEntry : mDndEffectsSelected )
        {
            CustomAnimationListEntryItem* pCustomAnimationEffect = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*pEntry));
            aEffects.push_back(pCustomAnimationEffect->getEffect());
        }

        // Callback to observer to have it update the model.
        // If pTarget is null, pass nullptr to indicate end of list.
        mpController->onDragNDropComplete(
            std::move(aEffects),
            pTarget ? pTarget->getEffect() : nullptr );

        // Reset selection
        mxTreeView->select(*mxDndEffectDragging);
        Select();
    }

    // NOTE: Don't call default handler because all required
    //       move operations have been completed here to update the model.
    return DND_ACTION_NONE;
}

CustomAnimationList::~CustomAnimationList()
{
    if (mnPostExpandEvent)
    {
        Application::RemoveUserEvent(mnPostExpandEvent);
        mnPostExpandEvent = nullptr;
    }

    if (mnPostCollapseEvent)
    {
        Application::RemoveUserEvent(mnPostCollapseEvent);
        mnPostCollapseEvent = nullptr;
    }

    if( mpMainSequence )
        mpMainSequence->removeListener( this );

    clear();
}

IMPL_LINK(CustomAnimationList, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const int nKeyCode = rKEvt.GetKeyCode().GetCode();
    switch (nKeyCode)
    {
        case KEY_DELETE:
            mpController->onContextMenu("remove");
            return true;
        case KEY_INSERT:
            mpController->onContextMenu("create");
            return true;
        case KEY_SPACE:
        {
            std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
            if (mxTreeView->get_cursor(xEntry.get()))
            {
                auto aRect = mxTreeView->get_row_area(*xEntry);
                const Point aPos(aRect.getOpenWidth() / 2, aRect.getOpenHeight() / 2);
                const CommandEvent aCEvt(aPos, CommandEventId::ContextMenu);
                CommandHdl(aCEvt);
                return true;
            }
        }
    }
    return false;
}

/** selects or deselects the given effect.
    Selections of other effects are not changed */
void CustomAnimationList::select( const CustomAnimationEffectPtr& pEffect )
{
    CustomAnimationListEntryItem* pEntry = nullptr;

    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
    if (mxTreeView->get_iter_first(*xEntry))
    {
        do
        {
            CustomAnimationListEntryItem* pTestEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry));
            if (pTestEntry->getEffect() == pEffect)
            {
                mxTreeView->select(*xEntry);
                mxTreeView->scroll_to_row(*xEntry);
                pEntry = pTestEntry;
                break;
            }
        } while (mxTreeView->iter_next(*xEntry));
    }

    if( !pEntry )
    {
        append( pEffect );
        select( pEffect );
    }
}

void CustomAnimationList::clear()
{
    mxEntries.clear();
    mxTreeView->clear();

    mxEmptyLabelParent->show();
    mxTreeView->hide();

    mxLastParentEntry.reset();
    mxLastTargetShape = nullptr;
}

void CustomAnimationList::update( const MainSequencePtr& pMainSequence )
{
    if( mpMainSequence )
        mpMainSequence->removeListener( this );

    mpMainSequence = pMainSequence;
    update();

    if( mpMainSequence )
        mpMainSequence->addListener( this );
}

struct stl_append_effect_func
{
    explicit stl_append_effect_func( CustomAnimationList& rList ) : mrList( rList ) {}
    void operator()(const CustomAnimationEffectPtr& pEffect);
    CustomAnimationList& mrList;
};

void stl_append_effect_func::operator()(const CustomAnimationEffectPtr& pEffect)
{
    mrList.append( pEffect );
}

void CustomAnimationList::update()
{
    mbIgnorePaint = true;

    std::vector< CustomAnimationEffectPtr > aVisible;
    std::vector< CustomAnimationEffectPtr > aSelected;
    CustomAnimationEffectPtr aCurrent;

    CustomAnimationEffectPtr pFirstSelEffect;
    CustomAnimationEffectPtr pLastSelEffect;
    ::tools::Long nFirstVis = -1;
    ::tools::Long nLastVis = -1;
    ::tools::Long nFirstSelOld = -1;
    ::tools::Long nLastSelOld = -1;

    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();

    if( mpMainSequence )
    {
        std::unique_ptr<weld::TreeIter> xLastSelectedEntry;
        std::unique_ptr<weld::TreeIter> xLastVisibleEntry;

        // save selection, current, and expand (visible) states
        mxTreeView->all_foreach([this, &aVisible, &nFirstVis, &xLastVisibleEntry,
                                 &aSelected, &nFirstSelOld, &pFirstSelEffect, &xLastSelectedEntry](weld::TreeIter& rEntry){
            CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(rEntry));
            CustomAnimationEffectPtr pEffect(pEntry->getEffect());
            if (pEffect)
            {
                if (weld::IsEntryVisible(*mxTreeView, rEntry))
                {
                    aVisible.push_back(pEffect);
                    // save scroll position
                    if (nFirstVis == -1)
                        nFirstVis = weld::GetAbsPos(*mxTreeView, rEntry);
                    if (!xLastVisibleEntry)
                        xLastVisibleEntry = mxTreeView->make_iterator(&rEntry);
                    else
                        mxTreeView->copy_iterator(rEntry, *xLastVisibleEntry);
                }

                if (mxTreeView->is_selected(rEntry))
                {
                    aSelected.push_back(pEffect);
                    if (nFirstSelOld == -1)
                    {
                        pFirstSelEffect = pEffect;
                        nFirstSelOld = weld::GetAbsPos(*mxTreeView, rEntry);
                    }
                    if (!xLastSelectedEntry)
                        xLastSelectedEntry = mxTreeView->make_iterator(&rEntry);
                    else
                        mxTreeView->copy_iterator(rEntry, *xLastSelectedEntry);
                }
            }

            return false;
        });

        if (xLastSelectedEntry)
        {
            CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xLastSelectedEntry));
            pLastSelEffect = pEntry->getEffect();
            nLastSelOld = weld::GetAbsPos(*mxTreeView, *xLastSelectedEntry);
        }

        if (xLastVisibleEntry)
            nLastVis = weld::GetAbsPos(*mxTreeView, *xLastVisibleEntry);

        if (mxTreeView->get_cursor(xEntry.get()))
        {
            CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry));
            aCurrent = pEntry->getEffect();
        }
    }

    // rebuild list

    mxTreeView->freeze();

    clear();

    if (mpMainSequence)
    {
        std::for_each( mpMainSequence->getBegin(), mpMainSequence->getEnd(), stl_append_effect_func( *this ) );
        mxLastParentEntry.reset();

        auto rInteractiveSequenceVector = mpMainSequence->getInteractiveSequenceVector();

        for (InteractiveSequencePtr const& pIS : rInteractiveSequenceVector)
        {
            Reference< XShape > xShape( pIS->getTriggerShape() );
            if( xShape.is() )
            {
                OUString aDescription = SdResId(STR_CUSTOMANIMATION_TRIGGER) + ": " +
                    getShapeDescription( xShape, false );

                mxEntries.emplace_back(std::make_unique<CustomAnimationListEntryItem>(aDescription, nullptr));

                OUString sId(weld::toId(mxEntries.back().get()));
                mxTreeView->insert(nullptr, -1, &aDescription, &sId, nullptr, nullptr, false, nullptr);
                std::for_each( pIS->getBegin(), pIS->getEnd(), stl_append_effect_func( *this ) );
                mxLastParentEntry.reset();
            }
        }
    }

    mxTreeView->thaw();

    if (mxTreeView->n_children())
    {
        mxEmptyLabelParent->hide();
        mxTreeView->show();
    }

    if (mpMainSequence)
    {
        ::tools::Long nFirstSelNew = -1;
        ::tools::Long nLastSelNew = -1;

        std::vector<std::unique_ptr<weld::TreeIter>> aNewSelection;

        // restore selection state, expand state, and current-entry (under cursor)
        if (mxTreeView->get_iter_first(*xEntry))
        {
            do
            {
                CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry));

                CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
                if (pEffect)
                {
                    // Any effects that were visible should still be visible, so expand their parents.
                    // (a previously expanded parent may have moved leaving a child to now be the new parent to expand)
                    if( std::find( aVisible.begin(), aVisible.end(), pEffect ) != aVisible.end() )
                    {
                        if (mxTreeView->get_iter_depth(*xEntry))
                        {
                            std::unique_ptr<weld::TreeIter> xParentEntry = mxTreeView->make_iterator(xEntry.get());
                            mxTreeView->iter_parent(*xParentEntry);
                            mxTreeView->expand_row(*xParentEntry);
                        }
                    }

                    if( std::find( aSelected.begin(), aSelected.end(), pEffect ) != aSelected.end() )
                        aNewSelection.emplace_back(mxTreeView->make_iterator(xEntry.get()));

                    // Restore the cursor, as it may deselect other effects wait until
                    // after the loop to reset the selection
                    if( pEffect == aCurrent )
                        mxTreeView->set_cursor(*xEntry);

                    if (pEffect == pFirstSelEffect)
                        nFirstSelNew = weld::GetAbsPos(*mxTreeView, *xEntry);

                    if (pEffect == pLastSelEffect)
                        nLastSelNew = weld::GetAbsPos(*mxTreeView, *xEntry);
                }
            } while (mxTreeView->iter_next(*xEntry));
        }

        // tdf#147032 unselect what previous set_cursor may have caused to get selected as a side-effect
        mxTreeView->unselect_all();
        for (const auto& rEntry : aNewSelection)
            mxTreeView->select(*rEntry);

        // Scroll to a selected entry, depending on where the selection moved.
        const bool bMoved = nFirstSelNew != nFirstSelOld;
        const bool bMovedUp = nFirstSelNew < nFirstSelOld;
        const bool bMovedDown = nFirstSelNew > nFirstSelOld;

        if( bMoved && nLastSelOld < nFirstVis && nLastSelNew < nFirstVis )
        {
            // The selection is above the visible area.
            // Scroll up to show the last few selected entries.
            if( nLastSelNew - (nLastVis - nFirstVis) > nFirstSelNew)
            {
                // The entries in the selection range can't fit in view.
                // Scroll so the last selected entry is last in view.
                mxTreeView->vadjustment_set_value(nLastSelNew - (nLastVis - nFirstVis));
            }
            else
                mxTreeView->vadjustment_set_value(nFirstSelNew);
        }
        else if( bMoved && nFirstSelOld > nLastVis && nFirstSelNew > nLastVis )
        {
            // The selection is below the visible area.
            // Scroll down to the first few selected entries.
            mxTreeView->vadjustment_set_value(nFirstSelNew);
        }
        else if( bMovedUp && nFirstSelOld <= nFirstVis )
        {
            // A visible entry has moved up out of view; scroll up one.
            mxTreeView->vadjustment_set_value(nFirstVis - 1);
        }
        else if( bMovedDown && nLastSelOld >= nLastVis )
        {
            // An entry has moved down out of view; scroll down one.
            mxTreeView->vadjustment_set_value(nFirstVis + 1);
        }
        else if ( nFirstVis != -1 )
        {
            // The selection is still in view, or it hasn't moved.
            mxTreeView->vadjustment_set_value(nFirstVis);
        }
    }

    mbIgnorePaint = false;

    Select();
}

void CustomAnimationList::append( CustomAnimationEffectPtr pEffect )
{
    Any aTarget( pEffect->getTarget() );
    if( !aTarget.hasValue() )
        return;

    try
    {
        // create a ui description
        OUString aDescription = getDescription(aTarget, pEffect->getTargetSubItem() != ShapeAnimationSubType::ONLY_BACKGROUND);

        std::unique_ptr<weld::TreeIter> xParentEntry;

        Reference< XShape > xTargetShape( pEffect->getTargetShape() );
        sal_Int32 nGroupId = pEffect->getGroupId();

        // if this effect has the same target and group-id as the last root effect,
        // the last root effect is also this effects parent
        if (mxLastParentEntry && nGroupId != -1 && mxLastTargetShape == xTargetShape && mnLastGroupId == nGroupId)
            xParentEntry = mxTreeView->make_iterator(mxLastParentEntry.get());

        // create an entry for the effect
        std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();

        mxEntries.emplace_back(std::make_unique<CustomAnimationListEntryItem>(aDescription, pEffect));

        OUString sId(weld::toId(mxEntries.back().get()));

        if (xParentEntry)
        {
            // add a subentry
            mxTreeView->insert(xParentEntry.get(), -1, &aDescription, &sId, nullptr, nullptr, false, xEntry.get());
        }
        else
        {
            // add a root entry
            mxTreeView->insert(nullptr, -1, &aDescription, &sId, nullptr, nullptr, false, xEntry.get());

            // and the new root entry becomes the possible next group header
            mxLastTargetShape = xTargetShape;
            mnLastGroupId = nGroupId;
            mxLastParentEntry = std::move(xEntry);
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationList::append()" );
    }
}

static void selectShape(weld::TreeView* pTreeList, const Reference< XShape >& xShape )
{
    std::unique_ptr<weld::TreeIter> xEntry = pTreeList->make_iterator();
    if (!pTreeList->get_iter_first(*xEntry))
        return;

    bool bFirstEntry = true;

    do
    {
        CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(pTreeList->get_id(*xEntry));
        CustomAnimationEffectPtr pEffect(pEntry->getEffect());
        if (pEffect)
        {
            if (pEffect->getTarget() == xShape)
            {
                pTreeList->select(*xEntry);
                if (bFirstEntry)
                {
                    pTreeList->scroll_to_row(*xEntry);
                    bFirstEntry = false;
                }
            }
        }
    } while (pTreeList->iter_next(*xEntry));
}

void CustomAnimationList::onSelectionChanged(const Any& rSelection)
{
    try
    {
        mxTreeView->unselect_all();

        if (rSelection.hasValue())
        {
            Reference< XIndexAccess > xShapes(rSelection, UNO_QUERY);
            if( xShapes.is() )
            {
                sal_Int32 nCount = xShapes->getCount();
                sal_Int32 nIndex;
                for( nIndex = 0; nIndex < nCount; nIndex++ )
                {
                    Reference< XShape > xShape( xShapes->getByIndex( nIndex ), UNO_QUERY );
                    if( xShape.is() )
                        selectShape(mxTreeView.get(), xShape);
                }
            }
            else
            {
                Reference< XShape > xShape(rSelection, UNO_QUERY);
                if( xShape.is() )
                    selectShape(mxTreeView.get(), xShape);
            }
        }

        Select();
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationList::onSelectionChanged()" );
    }
}

IMPL_LINK_NOARG(CustomAnimationList, SelectHdl, weld::TreeView&, void)
{
    Select();
}

// Notify controller to refresh UI when we are notified of selection change from base class
void CustomAnimationList::Select()
{
    if( mbIgnorePaint )
        return;
    mpController->onSelect();
}

IMPL_LINK_NOARG(CustomAnimationList, PostExpandHdl, void*, void)
{
    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
    if (mxTreeView->get_selected(xEntry.get()))
    {
        for (bool bChild = mxTreeView->iter_children(*xEntry); bChild; bChild = mxTreeView->iter_next_sibling(*xEntry))
        {
            if (!mxTreeView->is_selected(*xEntry))
                mxTreeView->select(*xEntry);
        }
    }

    // Notify controller that selection has changed (it should update the UI)
    mpController->onSelect();

    mnPostExpandEvent = nullptr;
}

IMPL_LINK(CustomAnimationList, ExpandHdl, const weld::TreeIter&, rParent, bool)
{
    // If expanded entry is selected, then select its children too afterwards.
    if (mxTreeView->is_selected(rParent) && !mnPostExpandEvent) {
        mnPostExpandEvent = Application::PostUserEvent(LINK(this, CustomAnimationList, PostExpandHdl));
    }

    return true;
}

IMPL_LINK_NOARG(CustomAnimationList, PostCollapseHdl, void*, void)
{
    // Deselect all entries as SvTreeListBox::Collapse selects the last
    // entry to have focus (or its parent), which is not desired
    mxTreeView->unselect_all();

    // Restore selection state for entries which are still visible
    for (const auto &pEntry : lastSelectedEntries)
    {
        if (weld::IsEntryVisible(*mxTreeView, *pEntry))
            mxTreeView->select(*pEntry);
    }

    lastSelectedEntries.clear();

    // Notify controller that selection has changed (it should update the UI)
    mpController->onSelect();

    mnPostCollapseEvent = nullptr;
}

IMPL_LINK_NOARG(CustomAnimationList, CollapseHdl, const weld::TreeIter&, bool)
{
    if (!mnPostCollapseEvent)
    {
        // weld::TreeView::collapse() discards multi-selection state
        // of list entries, so first save current selection state
        mxTreeView->selected_foreach([this](weld::TreeIter& rEntry){
            lastSelectedEntries.emplace_back(mxTreeView->make_iterator(&rEntry));
            return false;
        });

        mnPostCollapseEvent = Application::PostUserEvent(LINK(this, CustomAnimationList, PostCollapseHdl));
    }

    // Execute collapse on base class
    return true;
}

bool CustomAnimationList::isExpanded( const CustomAnimationEffectPtr& pEffect ) const
{
    bool bExpanded = true; // we assume expanded by default

    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
    if (mxTreeView->get_iter_first(*xEntry))
    {
        do
        {
            CustomAnimationListEntryItem* pEntry =
                weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry));
            if (pEntry->getEffect() == pEffect)
            {
                if (mxTreeView->get_iter_depth(*xEntry)) // no parent, keep expanded default of true
                {
                    std::unique_ptr<weld::TreeIter> xParentEntry = mxTreeView->make_iterator(xEntry.get());
                    if (mxTreeView->iter_parent(*xParentEntry))
                        bExpanded = mxTreeView->get_row_expanded(*xParentEntry);
                }
                break;
            }
        } while (mxTreeView->iter_next(*xEntry));
    }

    return bExpanded;
}

bool CustomAnimationList::isVisible(const CustomAnimationEffectPtr& pEffect) const
{
    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
    if (mxTreeView->get_iter_first(*xEntry))
    {
        do
        {
            CustomAnimationListEntryItem* pTestEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry));
            if (pTestEntry->getEffect() == pEffect)
                return weld::IsEntryVisible(*mxTreeView, *xEntry);
        } while (mxTreeView->iter_next(*xEntry));
    }
    return true;
}

EffectSequence CustomAnimationList::getSelection() const
{
    EffectSequence aSelection;

    mxTreeView->selected_foreach([this, &aSelection](weld::TreeIter& rEntry){
        CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(rEntry));
        CustomAnimationEffectPtr pEffect(pEntry->getEffect());
        if (pEffect)
            aSelection.push_back(pEffect);

        // if the selected effect is not expanded and has children
        // we say that the children are automatically selected
        if (!mxTreeView->get_row_expanded(rEntry) && mxTreeView->iter_has_child(rEntry))
        {
            std::unique_ptr<weld::TreeIter> xChild = mxTreeView->make_iterator(&rEntry);
            (void)mxTreeView->iter_children(*xChild);

            do
            {
                if (!mxTreeView->is_selected(*xChild))
                {
                    CustomAnimationListEntryItem* pChild = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xChild));
                    const CustomAnimationEffectPtr& pChildEffect( pChild->getEffect() );
                    if( pChildEffect )
                        aSelection.push_back( pChildEffect );
                }
            } while (mxTreeView->iter_next_sibling(*xChild));
        }

        return false;
    });

    return aSelection;
}

IMPL_LINK_NOARG(CustomAnimationList, DoubleClickHdl, weld::TreeView&, bool)
{
    mpController->onDoubleClick();
    return false;
}

IMPL_LINK(CustomAnimationList, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    if (rCEvt.IsMouseEvent())
    {
        ::Point aPos = rCEvt.GetMousePosPixel();
        std::unique_ptr<weld::TreeIter> xIter(mxTreeView->make_iterator());
        if (mxTreeView->get_dest_row_at_pos(aPos, xIter.get(), false) && !mxTreeView->is_selected(*xIter))
        {
            mxTreeView->unselect_all();
            mxTreeView->set_cursor(*xIter);
            mxTreeView->select(*xIter);
            SelectHdl(*mxTreeView);
        }
    }

    if (!mxTreeView->get_selected(nullptr))
        return false;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(mxTreeView.get(), "modules/simpress/ui/effectmenu.ui"));
    std::unique_ptr<weld::Menu> xMenu = xBuilder->weld_menu("menu");

    sal_Int16 nNodeType = -1;
    sal_Int16 nEntries = 0;

    mxTreeView->selected_foreach([this, &nNodeType, &nEntries](weld::TreeIter& rEntry){
        CustomAnimationListEntryItem* pEntry = weld::fromId<CustomAnimationListEntryItem*>(mxTreeView->get_id(rEntry));
        CustomAnimationEffectPtr pEffect(pEntry->getEffect());

        nEntries++;
        if (pEffect)
        {
            if( nNodeType == -1 )
            {
                nNodeType = pEffect->getNodeType();
            }
            else
            {
                if( nNodeType != pEffect->getNodeType() )
                {
                    nNodeType = -1;
                    return true;
                }
            }
        }

        return false;
    });

    xMenu->set_active("onclick", nNodeType == EffectNodeType::ON_CLICK);
    xMenu->set_active("withprev", nNodeType == EffectNodeType::WITH_PREVIOUS);
    xMenu->set_active("afterprev", nNodeType == EffectNodeType::AFTER_PREVIOUS);
    xMenu->set_sensitive("options", nEntries == 1);
    xMenu->set_sensitive("timing", nEntries == 1);

    OUString sCommand = xMenu->popup_at_rect(mxTreeView.get(), ::tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));
    if (!sCommand.isEmpty())
        ExecuteContextMenuAction(sCommand);

    return true;
}

void CustomAnimationList::ExecuteContextMenuAction(const OUString& rIdent)
{
    mpController->onContextMenu(rIdent);
}

void CustomAnimationList::notify_change()
{
    update();
    mpController->onSelect();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
