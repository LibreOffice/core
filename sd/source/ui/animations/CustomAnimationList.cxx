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

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include "CustomAnimationList.hxx"
#include <CustomAnimationPreset.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <osl/diagnose.h>

#include <sdresid.hxx>

#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>


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
            const OUString aPropName1("Name");
            if(xInfo->hasPropertyByName(aPropName1))
                xSet->getPropertyValue(aPropName1) >>= aDescription;

            bAppendIndex = aDescription.isEmpty();

            const OUString aPropName2("UINameSingular");
            if(xInfo->hasPropertyByName(aPropName2))
                xSet->getPropertyValue(aPropName2) >>= aDescription;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::getShapeDescription(), exception caught!" );
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
    CustomAnimationListEntryItem(const OUString& aDescription,
                                 const CustomAnimationEffectPtr& pEffect);
    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }
private:
    OUString        msDescription;
    OUString        msEffectName;
    CustomAnimationEffectPtr mpEffect;
};

CustomAnimationListEntryItem::CustomAnimationListEntryItem(const OUString& aDescription, const CustomAnimationEffectPtr& pEffect)
    : msDescription(aDescription)
    , msEffectName(OUString())
    , mpEffect(pEffect)
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

#if 0
void CustomAnimationListEntryItem::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                         const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{

    const SvViewDataItem* pViewData = mpParent->GetViewDataItem(&rEntry, this);

    Point aPos(rPos);
    int nItemHeight = pViewData->mnHeight;

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

    aPos.AdjustX(nIconWidth );


    rRenderContext.DrawText(aPos, rRenderContext.GetEllipsisString(msDescription, rDev.GetOutputSizePixel().Width() - aPos.X()));

    aPos.AdjustY(nIconWidth );

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
    aPos.AdjustY((nItemHeight/2 - rDev.GetTextHeight()) >> 1 );

    rRenderContext.DrawText(aPos, rRenderContext.GetEllipsisString(msEffectName, rDev.GetOutputSizePixel().Width() - aPos.X()));
}

#endif

CustomAnimationList::CustomAnimationList(std::unique_ptr<weld::TreeView> xTreeView,
                                         std::unique_ptr<weld::Label> xLabel,
                                         std::unique_ptr<weld::Widget> xScrolledWindow)
    : mxTreeView(std::move(xTreeView))
    , mxEmptyLabel(std::move(xLabel))
    , mxEmptyLabelParent(std::move(xScrolledWindow))
    , mbIgnorePaint(false)
    , mpController(nullptr)
    , mnLastGroupId(0)
    , mnPostExpandEvent(nullptr)
    , mnPostCollapseEvent(nullptr)
#if 0
    , mpDndEffectDragging(nullptr)
    , mpDndEffectInsertBefore(nullptr)
#endif
{
    mxEmptyLabel->set_stack_background();

    mxTreeView->set_selection_mode(SelectionMode::Multiple);
#if 0
    SetDragDropMode(DragDropMode::CTRL_MOVE);
#endif
    mxTreeView->connect_changed(LINK(this, CustomAnimationList, SelectHdl));
    mxTreeView->connect_key_press(LINK(this, CustomAnimationList, KeyInputHdl));
    mxTreeView->connect_popup_menu(LINK(this, CustomAnimationList, CommandHdl));
    mxTreeView->connect_row_activated(LINK(this, CustomAnimationList, DoubleClickHdl));
    mxTreeView->connect_expanding(LINK(this, CustomAnimationList, ExpandHdl));
    mxTreeView->connect_collapsing(LINK(this, CustomAnimationList, CollapseHdl));
}

#if 0
// D'n'D #1: Record selected effects for drag'n'drop.
void CustomAnimationList::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    // Record which effects are selected:
    // Since NextSelected(..) iterates through the selected items in the order they
    // were selected, create a sorted list for simpler drag'n'drop algorithms.
    mDndEffectsSelected.clear();
    for( SvTreeListEntry* pEntry = First(); pEntry; pEntry = Next(pEntry) )
    {
        if( IsSelected(pEntry) )
        {
            mDndEffectsSelected.push_back( pEntry );
        }
    }

    // Allow normal processing; this calls our NotifyStartDrag().
    SvTreeListBox::StartDrag( nAction, rPosPixel );
}

// D'n'D #2: Prepare selected element for moving.
DragDropMode CustomAnimationList::NotifyStartDrag( TransferDataContainer& /*rData*/, SvTreeListEntry* pEntry )
{
    // Restore selection for multiple selected effects.
    // Do it here to remove a flicker on the UI with effects being unselected and reselected.
    for( auto &pEffect : mDndEffectsSelected )
        SelectListEntry( pEffect, true);

    // Note: pEntry is the effect with focus (if multiple effects are selected)
    mpDndEffectDragging = pEntry;
    mpDndEffectInsertBefore = pEntry;

    return DragDropMode::CTRL_MOVE;
}

// D'n'D #3: Called each time mouse moves during drag
sal_Int8 CustomAnimationList::AcceptDrop( const AcceptDropEvent& rEvt )
{
    /*
        Don't call SvTreeListBox::AcceptDrop because it puts an unnecessary
        highlight via ImplShowTargetEmphasis()
    */

    sal_Int8 ret = DND_ACTION_NONE;

    const bool bIsMove = ( DND_ACTION_MOVE == rEvt.mnAction );
    if( mpDndEffectDragging && !rEvt.mbLeaving && bIsMove )
    {
        SvTreeListEntry* pEntry = GetDropTarget( rEvt.maPosPixel );

        const bool bOverASelectedEffect =
            std::find( mDndEffectsSelected.begin(), mDndEffectsSelected.end(), pEntry ) != mDndEffectsSelected.end();
        if( pEntry && !bOverASelectedEffect )
        {
            ReparentChildrenDuringDrag();

            ReorderEffectsInUiDuringDragOver( pEntry );
        }

        // Return DND_ACTION_MOVE on internal drag'n'drops so that ExecuteDrop() is called.
        // Return MOVE even if we are over other dragged effect because dragged effect moves.
        ret = DND_ACTION_MOVE;
    }

    return ret;
}

// D'n'D: For each dragged effect, re-parent (only in the UI) non-selected
//        visible children so they are not dragged with the parent.
void CustomAnimationList::ReparentChildrenDuringDrag()
{
    /*
        Re-parent (only in the UI!):
          a) the dragged effect's first non-selected child to the root, and
          b) the remaining non-selected children to that re-parented 1st child.
    */
    for( auto &pEffect : mDndEffectsSelected )
    {
        const bool bExpandedWithChildren = GetVisibleChildCount( pEffect ) > 0;
        if( bExpandedWithChildren )
        {
            SvTreeListEntry* pEntryParent = GetParent( pEffect );

            SvTreeListEntry* pFirstNonSelectedChild = nullptr;
            sal_uLong nInsertNextChildPos = 0;

            // Process all children of this effect
            SvTreeListEntry* pChild = FirstChild( pEffect );
            while( pChild && ( GetParent( pChild ) == pEffect ) )
            {
                // Start by finding next child because if pChild moves, we cannot then
                // ask it what the next child is because it's no longer with its siblings.
                SvTreeListEntry* pNextChild = Next( pChild );

                // Skip selected effects: they stay with their previous parent to be moved.
                // During drag, the IsSelected() set changes, so use mDndEffectsSelected instead
                const bool bIsSelected = std::find( mDndEffectsSelected.begin(), mDndEffectsSelected.end(), pChild ) != mDndEffectsSelected.end();
                if( !bIsSelected )
                {
                    // Re-parent 1st non-selected child to root, below all the other children.
                    if( !pFirstNonSelectedChild )
                    {
                        pFirstNonSelectedChild = pChild;
                        sal_uLong nInsertAfterPos = SvTreeList::GetRelPos( pEffect ) + 1;
                        pModel->Move( pFirstNonSelectedChild, pEntryParent, nInsertAfterPos );
                    }
                    else
                    {
                        // Re-parent remaining non-selected children to 1st child
                        ++nInsertNextChildPos;
                        pModel->Move( pChild, pFirstNonSelectedChild, nInsertNextChildPos );
                    }
                }

                pChild = pNextChild;
            }

            // Expand all children (they were previously visible)
            if( pFirstNonSelectedChild )
                Expand( pFirstNonSelectedChild );

        }
    }
}

// D'n'D: Update UI to show where dragged event will appear if dropped now.
void CustomAnimationList::ReorderEffectsInUiDuringDragOver( SvTreeListEntry* pOverEntry )
{
    /*
        Update the order of effects in *just the UI* while the user is dragging.
        The model (MainSequence) will only be changed after the user drops
        the effect so that there is minimal work to do if the drag is canceled.
        Plus only one undo record should be created per drag, and changing
        the model recreates all effects (on a background timer) which invalidates
        all effect pointers.
    */

    // Compute new location in *UI*
    SvTreeListEntry* pNewParent = nullptr;
    sal_uLong nInsertAfterPos = 0;

    Point aPosOverEffect( GetEntryPosition(pOverEntry) );
    Point aPosDraggedEffect( GetEntryPosition(mpDndEffectDragging) );
    const bool bDraggingUp = (aPosDraggedEffect.Y() - aPosOverEffect.Y()) > 0;

    if( bDraggingUp )
    {
        // Drag up   --> place above the element we are over
        pNewParent = GetParent( pOverEntry );
        nInsertAfterPos = SvTreeList::GetRelPos( pOverEntry );
        mpDndEffectInsertBefore = pOverEntry;
    }
    else
    {
        // Drag down -->  place below the element we are over
        SvTreeListEntry* pNextVisBelowTarget = NextVisible( pOverEntry );
        if( pNextVisBelowTarget )
        {
            // Match parent of NEXT visible effect (works for sub-items too)
            pNewParent = GetParent( pNextVisBelowTarget );
            nInsertAfterPos = SvTreeList::GetRelPos( pNextVisBelowTarget );
            mpDndEffectInsertBefore = pNextVisBelowTarget;
        }
        else
        {
            // Over the last element: no next to work with
            pNewParent = GetParent( pOverEntry );
            nInsertAfterPos = SvTreeList::GetRelPos( pOverEntry ) + 1;
            mpDndEffectInsertBefore = nullptr;
        }
    }

    // Move each selected effect in *just* the UI to show where it would be if dropped.
    // This leaves the exist parent relationships in the non-dragged elements so that
    // the list does not seem to change structure during drag. Parent relationships will
    // be correctly recreated on drop.
    for( auto aItr = mDndEffectsSelected.rbegin();
         aItr != mDndEffectsSelected.rend();
         ++aItr)
    {
        SvTreeListEntry* pEffect = *aItr;

        // Move only effects whose parents is not selected because
        // they will automatically move when their parent is moved.
        const bool bParentIsSelected =
            std::find(mDndEffectsSelected.begin(), mDndEffectsSelected.end(), GetParent(pEffect)) != mDndEffectsSelected.end();

        if( !bParentIsSelected )
        {
            // If the current effect is being moved down, the insert position must be decremented
            // after move if it will have the same parent as it currently does because it moves
            // from above the insertion point to below it, hence changing its index.
            // Must decide move-up vs move-down for each effect being dragged because we may be
            // processing a discontinuous set of selected effects (some below, some above insertion point)
            Point aCurPosOverEffect( GetEntryPosition( pOverEntry ) );
            Point aCurPosMovedEffect( GetEntryPosition( pEffect ) );
            const bool bCurDraggingDown = ( aCurPosMovedEffect.Y() - aCurPosOverEffect.Y() ) < 0;
            const bool bWillHaveSameParent = ( pNewParent == GetParent(pEffect) );

            pModel->Move( pEffect, pNewParent, nInsertAfterPos );

            if( bCurDraggingDown && bWillHaveSameParent )
                --nInsertAfterPos;
        }
    }

    // Restore selection (calling Select() is slow; SelectListEntry() is faster)
    for( auto &pEffect : mDndEffectsSelected )
        SelectListEntry( pEffect, true);
}

// D'n'D #5: Tell model to update effect order.
sal_Int8 CustomAnimationList::ExecuteDrop( const ExecuteDropEvent& /*rEvt*/ )
{
    // NOTE: We cannot just override NotifyMoving() because it's not called
    //       since we dynamically reorder effects during drag.

    sal_Int8 ret = DND_ACTION_NONE;

    const bool bMovingEffect = ( mpDndEffectDragging != nullptr );
    const bool bMoveNotSelf  = ( mpDndEffectInsertBefore != mpDndEffectDragging );
    const bool bHaveSequence = ( mpMainSequence.get() != nullptr );

    if( bMovingEffect && bMoveNotSelf && bHaveSequence )
    {
        CustomAnimationListEntry*  pTarget = static_cast< CustomAnimationListEntry* >( mpDndEffectInsertBefore );

        // Build list of effects
        std::vector< CustomAnimationEffectPtr > aEffects;
        for( const auto &pEntry : mDndEffectsSelected )
        {
            CustomAnimationListEntry* pCustomAnimationEffect = static_cast< CustomAnimationListEntry* >( pEntry );
            aEffects.push_back( pCustomAnimationEffect->getEffect() );
        }

        // Callback to observer to have it update the model.
        // If pTarget is null, pass nullptr to indicate end of list.
        mpController->onDragNDropComplete(
            aEffects,
            pTarget ? pTarget->getEffect() : nullptr );

        // Reset selection
        Select( mpDndEffectDragging );

        ret = DND_ACTION_MOVE;
    }

    // NOTE: Don't call SvTreeListBox::ExecuteDrop(...) because all required
    //       move operations have been completed here to update the model.
    return ret;
}

// D'n'D #6: Cleanup (regardless of if we were target of drop or not)
void CustomAnimationList::DragFinished( sal_Int8 /*nDropAction*/ )
{
    mpDndEffectDragging = nullptr;
    mpDndEffectInsertBefore = nullptr;
    mDndEffectsSelected.clear();

    // Rebuild because we may have re-parented the dragged effect's first child.
    // Can hit this without running ExecuteDrop(...) when drag canceled.
    mpMainSequence->rebuild();

    // Note: Don't call SvTreeListBox::DragFinished(...) because we don't call
    //       SvTreeListBox::ExecuteDrop(...) which sets variables that are
    //       needed in its DragFinished(...) method.
}

#endif

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

    if( mpMainSequence.get() )
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
                const Point aPos(aRect.getWidth() / 2, aRect.getHeight() / 2);
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
            CustomAnimationListEntryItem* pTestEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry).toInt64());
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
    if( mpMainSequence.get() )
        mpMainSequence->removeListener( this );

    mpMainSequence = pMainSequence;
    update();

    if( mpMainSequence.get() )
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

//TODO    CustomAnimationListEntry* pEntry = nullptr;

    std::vector< CustomAnimationEffectPtr > aVisible;
    std::vector< CustomAnimationEffectPtr > aSelected;
    CustomAnimationEffectPtr aCurrent;

    CustomAnimationEffectPtr pFirstSelEffect;
    CustomAnimationEffectPtr pLastSelEffect;
    long nFirstVis = -1;
    long nLastVis = -1;
    long nFirstSelOld = -1;
    long nLastSelOld = -1;

    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();

    if( mpMainSequence.get() )
    {
        std::unique_ptr<weld::TreeIter> xLastSelectedEntry;
        std::unique_ptr<weld::TreeIter> xLastVisibleEntry;

        // save selection, current, and expand (visible) states
        mxTreeView->all_foreach([this, &aVisible, &nFirstVis, &xLastVisibleEntry,
                                 &aSelected, &nFirstSelOld, &pFirstSelEffect, &xLastSelectedEntry](weld::TreeIter& rEntry){
            CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(rEntry).toInt64());
            CustomAnimationEffectPtr pEffect(pEntry->getEffect());
            if (pEffect.get())
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
            CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xLastSelectedEntry).toInt64());
            pLastSelEffect = pEntry->getEffect();
            nLastSelOld = weld::GetAbsPos(*mxTreeView, *xLastSelectedEntry);
        }

        if (xLastVisibleEntry)
            nLastVis = weld::GetAbsPos(*mxTreeView, *xLastVisibleEntry);

        if (mxTreeView->get_cursor(xEntry.get()))
        {
            CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry).toInt64());
            aCurrent = pEntry->getEffect();
        }
    }

    // rebuild list

    mxTreeView->freeze();

    clear();

    if (mpMainSequence.get())
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

                OUString sId(OUString::number(reinterpret_cast<sal_Int64>(mxEntries.back().get())));
                fprintf(stderr, "this insert\n");
                mxTreeView->insert(nullptr, -1, &aDescription, &sId, nullptr, nullptr, nullptr, false, nullptr);
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

    if (mpMainSequence.get())
    {
        long nFirstSelNew = -1;
        long nLastSelNew = -1;

        // restore selection state, expand state, and current-entry (under cursor)
        if (mxTreeView->get_iter_first(*xEntry))
        {
            do
            {
                CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry).toInt64());

                CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
                if (pEffect.get())
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
                        mxTreeView->select(*xEntry);

#if 0
                    // Restore the cursor; don't use SetCurEntry() as it may deselect other effects
                    if( pEffect == aCurrent )
                        mxTreeView->set_cursor(*xEntry); // against comment: TODO
#endif

                    if (pEffect == pFirstSelEffect)
                        nFirstSelNew = weld::GetAbsPos(*mxTreeView, *xEntry);

                    if (pEffect == pLastSelEffect)
                        nLastSelNew = weld::GetAbsPos(*mxTreeView, *xEntry);
                }
            } while (mxTreeView->iter_next(*xEntry));
        }

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

    Select();   // maybe TODO
}

namespace
{
    OUString GetImage(CustomAnimationEffectPtr pEffect)
    {
        OUString sImage;
        sal_Int16 nNodeType = pEffect->getNodeType();
        if (nNodeType == EffectNodeType::ON_CLICK )
        {
            sImage = BMP_CUSTOMANIMATION_ON_CLICK;
        }
        else if (nNodeType == EffectNodeType::AFTER_PREVIOUS)
        {
            sImage = BMP_CUSTOMANIMATION_AFTER_PREVIOUS;
        }
        else if (nNodeType == EffectNodeType::WITH_PREVIOUS)
        {
            //FIXME With previous image not defined in CustomAnimation.src
        }
        return sImage;
    }
}

void CustomAnimationList::append( CustomAnimationEffectPtr pEffect )
{
    fprintf(stderr, "append\n");

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

        fprintf(stderr, "group %d\n", nGroupId);

        // if this effect has the same target and group-id as the last root effect,
        // the last root effect is also this effects parent
        if (mxLastParentEntry && nGroupId != -1 && mxLastTargetShape == xTargetShape && mnLastGroupId == nGroupId)
            xParentEntry = mxTreeView->make_iterator(mxLastParentEntry.get());

        // create an entry for the effect
        std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();

        mxEntries.emplace_back(std::make_unique<CustomAnimationListEntryItem>(aDescription, pEffect));

        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(mxEntries.back().get())));

        fprintf(stderr, "the other insert\n");

        OUString sImage = GetImage(pEffect);

        if (xParentEntry)
        {
            // add a subentry
            mxTreeView->insert(xParentEntry.get(), -1, &aDescription, &sId, nullptr, nullptr, &sImage, false, xEntry.get());
        }
        else
        {
            // add a root entry
            mxTreeView->insert(nullptr, -1, &aDescription, &sId, nullptr, nullptr, &sImage, false, xEntry.get());

            // and the new root entry becomes the possible next group header
            mxLastTargetShape = xTargetShape;
            mnLastGroupId = nGroupId;
            mxLastParentEntry = std::move(xEntry);
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("sd::CustomAnimationList::append(), exception caught!" );
    }
}

static void selectShape(weld::TreeView* pTreeList, const Reference< XShape >& xShape )
{
    std::unique_ptr<weld::TreeIter> xEntry = pTreeList->make_iterator();
    if (pTreeList->get_iter_first(*xEntry))
    {
        do
        {
            CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(pTreeList->get_id(*xEntry).toInt64());
            CustomAnimationEffectPtr pEffect(pEntry->getEffect());
            if (pEffect)
            {
                if (pEffect->getTarget() == xShape)
                    pTreeList->select(*xEntry);
            }
        } while (pTreeList->iter_next(*xEntry));
    }
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
        OSL_FAIL( "sd::CustomAnimationList::onSelectionChanged(), Exception caught!" );
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
    for (auto &pEntry : lastSelectedEntries)
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
    // TODO, odd true if not found thing ?
    bool bExpanded = false;

    std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
    if (mxTreeView->get_iter_first(*xEntry))
    {
        do
        {
            CustomAnimationListEntryItem* pEntry =
                reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry).toInt64());
            if (pEntry->getEffect() == pEffect)
            {
                std::unique_ptr<weld::TreeIter> xParentEntry = mxTreeView->make_iterator(xEntry.get());
                if (mxTreeView->iter_parent(*xParentEntry))
                    bExpanded = mxTreeView->get_row_expanded(*xParentEntry);
                else
                    bExpanded = true;
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
            CustomAnimationListEntryItem* pTestEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xEntry).toInt64());
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
        CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(rEntry).toInt64());
        CustomAnimationEffectPtr pEffect(pEntry->getEffect());
        if (pEffect)
            aSelection.push_back(pEffect);

        // if the selected effect is not expanded and has children
        // we say that the children are automatically selected
        if (!mxTreeView->get_row_expanded(rEntry) && mxTreeView->iter_has_child(rEntry))
        {
            std::unique_ptr<weld::TreeIter> xChild = mxTreeView->make_iterator(&rEntry);
            mxTreeView->iter_children(*xChild);

            do
            {
                if (!mxTreeView->is_selected(*xChild))
                {
                    CustomAnimationListEntryItem* pChild = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(*xChild).toInt64());
                    const CustomAnimationEffectPtr& pChildEffect( pChild->getEffect() );
                    if( pChildEffect.get() )
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

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(mxTreeView.get(), "modules/simpress/ui/effectmenu.ui"));
    std::unique_ptr<weld::Menu> xMenu = xBuilder->weld_menu("menu");

    sal_Int16 nNodeType = -1;
    sal_Int16 nEntries = 0;

    mxTreeView->selected_foreach([this, &nNodeType, &nEntries](weld::TreeIter& rEntry){
        CustomAnimationListEntryItem* pEntry = reinterpret_cast<CustomAnimationListEntryItem*>(mxTreeView->get_id(rEntry).toInt64());
        CustomAnimationEffectPtr pEffect(pEntry->getEffect());

        nEntries++;
        if (pEffect.get())
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

    OString sCommand = xMenu->popup_at_rect(mxTreeView.get(), ::tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));
    if (!sCommand.isEmpty())
        ExecuteContextMenuAction(sCommand);

    return true;
}

void CustomAnimationList::ExecuteContextMenuAction(const OString& rIdent)
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
