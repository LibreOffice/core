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
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include "CustomAnimationList.hxx"
#include "CustomAnimation.hrc"
#include "CustomAnimationPreset.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <o3tl/make_unique.hxx>

#include "sdresid.hxx"

#include "svtools/svlbitm.hxx"
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"

#include "res_bmp.hrc"
#include "glob.hrc"

#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
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

// go recursivly through all shapes in the given XShapes collection and return true as soon as the
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
    if( xSet.is() )
    {
        Reference< XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );
        const OUString aPropName( "UINameSingular");
        if( xInfo->hasPropertyByName( aPropName ) )
            xSet->getPropertyValue( aPropName ) >>= aDescription;
    }

    aDescription += " ";
    aDescription += OUString::number( getShapeIndex( xShape ) );

    if( bWithText )
    {
        Reference< XTextRange > xText( xShape, UNO_QUERY );
        if( xText.is() )
        {
            OUString aText( xText->getString() );
            if( !aText.isEmpty() )
            {
                aDescription += ": ";

                aText = aText.replace( (sal_Unicode)'\n', (sal_Unicode)' ' );
                aText = aText.replace( (sal_Unicode)'\r', (sal_Unicode)' ' );

                aDescription += aText;
            }
        }
    }
    return aDescription;
}

static OUString getDescription( const Any& rTarget, bool bWithText = true )
{
    OUString aDescription;

    if( rTarget.getValueType() == ::cppu::UnoType<ParagraphTarget>::get() )
    {
        ParagraphTarget aParaTarget;
        rTarget >>= aParaTarget;

        Reference< XEnumerationAccess > xText( aParaTarget.Shape, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xText->createEnumeration(), UNO_QUERY_THROW );
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

class CustomAnimationListEntryItem : public SvLBoxString
{
public:
    CustomAnimationListEntryItem(const OUString& aDescription,
                                 CustomAnimationEffectPtr pEffect, CustomAnimationList* pParent);
    virtual ~CustomAnimationListEntryItem();
    void InitViewData(SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* = nullptr) override;
    SvLBoxItem* Create() const override;
    void Clone(SvLBoxItem* pSource) override;

    virtual void Paint(const Point&, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView,const SvTreeListEntry& rEntry) override;
private:
    VclPtr<CustomAnimationList> mpParent;
    OUString        msDescription;
    OUString        msEffectName;
    CustomAnimationEffectPtr mpEffect;
    const CustomAnimationPresets* mpCustomAnimationPresets;
    const long nIconWidth = 19;
    const long nItemMinHeight = 38;
};

CustomAnimationListEntryItem::CustomAnimationListEntryItem( const OUString& aDescription, CustomAnimationEffectPtr pEffect, CustomAnimationList* pParent  )
: SvLBoxString( aDescription )
, mpParent( pParent )
, msDescription( aDescription )
, msEffectName( OUString() )
, mpEffect(pEffect)
, mpCustomAnimationPresets(&CustomAnimationPresets::getCustomAnimationPresets())
{
    switch(mpEffect->getPresetClass())
    {
    case EffectPresetClass::ENTRANCE:
        msEffectName = SD_RESSTR(STR_CUSTOMANIMATION_ENTRANCE); break;
    case EffectPresetClass::EXIT:
        msEffectName = SD_RESSTR(STR_CUSTOMANIMATION_EXIT); break;
    case EffectPresetClass::EMPHASIS:
        msEffectName = SD_RESSTR(STR_CUSTOMANIMATION_EMPHASIS); break;
    case EffectPresetClass::MOTIONPATH:
        msEffectName = SD_RESSTR(STR_CUSTOMANIMATION_MOTION_PATHS); break;
    }
    msEffectName = msEffectName.replaceFirst( "%1" , mpCustomAnimationPresets->getUINameForPresetId(mpEffect->getPresetId()));
}


CustomAnimationListEntryItem::~CustomAnimationListEntryItem()
{
}


void CustomAnimationListEntryItem::InitViewData( SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData )
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    long width = pView->GetTextWidth( msDescription ) + nIconWidth;
    if( width < (pView->GetTextWidth( msEffectName ) + 2*nIconWidth))
        width = pView->GetTextWidth( msEffectName ) + 2*nIconWidth;

    Size aSize( width, pView->GetTextHeight() );
    if( aSize.Height() < nItemMinHeight )
        aSize.Height() = nItemMinHeight;
    pViewData->maSize = aSize;
}

void CustomAnimationListEntryItem::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                         const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{

    const SvViewDataItem* pViewData = mpParent->GetViewDataItem(&rEntry, this);

    Point aPos(rPos);
    Size aSize(pViewData->maSize);

    sal_Int16 nNodeType = mpEffect->getNodeType();
    if (nNodeType == EffectNodeType::ON_CLICK )
    {
        rRenderContext.DrawImage( aPos, mpParent->getImage(IMG_CUSTOMANIMATION_ON_CLICK));
    }
    else if (nNodeType == EffectNodeType::AFTER_PREVIOUS)
    {
        rRenderContext.DrawImage(aPos, mpParent->getImage(IMG_CUSTOMANIMATION_AFTER_PREVIOUS));
    }
    else if (nNodeType == EffectNodeType::WITH_PREVIOUS)
    {
        //FIXME With previous image not defined in CustomAnimation.src
    }

    aPos.X() += nIconWidth;


    rRenderContext.DrawText(aPos, rRenderContext.GetEllipsisString(msDescription, rDev.GetOutputSizePixel().Width() - aPos.X()));

    aPos.Y() += nIconWidth;

    sal_uInt16 nImage;
    switch (mpEffect->getPresetClass())
    {
    case EffectPresetClass::ENTRANCE:
        nImage = IMG_CUSTOMANIMATION_ENTRANCE_EFFECT; break;
    case EffectPresetClass::EXIT:
        nImage =  IMG_CUSTOMANIMATION_EXIT_EFFECT; break;
    case EffectPresetClass::EMPHASIS:
        nImage =  IMG_CUSTOMANIMATION_EMPHASIS_EFFECT; break;
    case EffectPresetClass::MOTIONPATH:
        nImage = IMG_CUSTOMANIMATION_MOTION_PATH; break;
    case EffectPresetClass::OLEACTION:
        nImage = IMG_CUSTOMANIMATION_OLE; break;
    case EffectPresetClass::MEDIACALL:
        switch (mpEffect->getCommand())
        {
        case EffectCommands::TOGGLEPAUSE:
            nImage = IMG_CUSTOMANIMATION_MEDIA_PAUSE; break;
        case EffectCommands::STOP:
            nImage = IMG_CUSTOMANIMATION_MEDIA_STOP; break;
        case EffectCommands::PLAY:
        default:
            nImage = IMG_CUSTOMANIMATION_MEDIA_PLAY; break;
        }
        break;
    default:
        nImage = 0xffff;
    }

    if (nImage != 0xffff)
    {
        const Image& rImage = mpParent->getImage(nImage);
        Point aImagePos(aPos);
        aImagePos.Y() += (aSize.Height()/2 - rImage.GetSizePixel().Height()) >> 1;
        rRenderContext.DrawImage(aImagePos, rImage);
    }

    aPos.X() += nIconWidth;
    aPos.Y() += (aSize.Height()/2 - rDev.GetTextHeight()) >> 1;

    rRenderContext.DrawText(aPos, rRenderContext.GetEllipsisString(msEffectName, rDev.GetOutputSizePixel().Width() - aPos.X()));
}

SvLBoxItem* CustomAnimationListEntryItem::Create() const
{
    return nullptr;
}

void CustomAnimationListEntryItem::Clone( SvLBoxItem* )
{
}

class CustomAnimationListEntry : public SvTreeListEntry
{
public:
    CustomAnimationListEntry();
    explicit CustomAnimationListEntry( CustomAnimationEffectPtr pEffect );
    virtual ~CustomAnimationListEntry();

    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

private:
    CustomAnimationEffectPtr mpEffect;
};

CustomAnimationListEntry::CustomAnimationListEntry()
{
}

CustomAnimationListEntry::CustomAnimationListEntry( CustomAnimationEffectPtr pEffect )
: mpEffect( pEffect )
{
}

CustomAnimationListEntry::~CustomAnimationListEntry()
{
}

class CustomAnimationTriggerEntryItem : public SvLBoxString
{
public:
    explicit        CustomAnimationTriggerEntryItem( const OUString& aDescription );
    virtual         ~CustomAnimationTriggerEntryItem();
    void            InitViewData( SvTreeListBox*,SvTreeListEntry*,SvViewDataItem* = nullptr ) override;
    SvLBoxItem*     Create() const override;
    void            Clone( SvLBoxItem* pSource ) override;
    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;

private:
    OUString        msDescription;
    const long nIconWidth = 19;
};

CustomAnimationTriggerEntryItem::CustomAnimationTriggerEntryItem( const OUString& aDescription )
: SvLBoxString( aDescription ), msDescription( aDescription )
{
}

CustomAnimationTriggerEntryItem::~CustomAnimationTriggerEntryItem()
{
}

void CustomAnimationTriggerEntryItem::InitViewData( SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData )
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Size aSize(pView->GetTextWidth( msDescription ) + 2 * nIconWidth, pView->GetTextHeight() );
    if( aSize.Height() < nIconWidth )
        aSize.Height() = nIconWidth;
    pViewData->maSize = aSize;
}

void CustomAnimationTriggerEntryItem::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                                            const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    Size aSize(rDev.GetOutputSizePixel().Width(), rDev.GetEntryHeight());

    Point aPos(0, rPos.Y());

    Rectangle aOutRect(aPos, aSize);

    // fill the background
    Color aColor(rRenderContext.GetSettings().GetStyleSettings().GetDialogColor());

    rRenderContext.Push();
    rRenderContext.SetFillColor(aColor);
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(aOutRect);

    // Erase the four corner pixels to make the rectangle appear rounded.
    rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.DrawPixel(aOutRect.TopLeft());
    rRenderContext.DrawPixel(Point(aOutRect.Right(), aOutRect.Top()));
    rRenderContext.DrawPixel(Point(aOutRect.Left(), aOutRect.Bottom()));
    rRenderContext.DrawPixel(Point(aOutRect.Right(), aOutRect.Bottom()));

    // draw the category title

    int nVertBorder = ((aSize.Height() - rDev.GetTextHeight()) >> 1);
    int nHorzBorder = rRenderContext.LogicToPixel(Size(3, 3), MAP_APPFONT).Width();

    aOutRect.Left() += nHorzBorder;
    aOutRect.Right() -= nHorzBorder;
    aOutRect.Top() += nVertBorder;
    aOutRect.Bottom() -= nVertBorder;

    rRenderContext.DrawText(aOutRect, rRenderContext.GetEllipsisString(msDescription, aOutRect.GetWidth()));
    rRenderContext.Pop();
}

SvLBoxItem* CustomAnimationTriggerEntryItem::Create() const
{
    return nullptr;
}

void CustomAnimationTriggerEntryItem::Clone( SvLBoxItem* )
{
}

CustomAnimationList::CustomAnimationList( vcl::Window* pParent )
    : SvTreeListBox( pParent, WB_TABSTOP | WB_BORDER | WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT )
    , mbIgnorePaint(false)
    , mpController(nullptr)
    , mnLastGroupId(0)
    , mpLastParentEntry(nullptr)
{

    EnableContextMenuHandling();
    SetSelectionMode( SelectionMode::Multiple );
    SetOptimalImageIndent();
    SetNodeDefaultImages();
}

VCL_BUILDER_FACTORY(CustomAnimationList)

const Image&  CustomAnimationList::getImage( sal_uInt16 nId )
{
    DBG_ASSERT( (nId >= IMG_CUSTOMANIMATION_ON_CLICK) && (nId <= IMG_CUSTOMANIMATION_MEDIA_STOP), "sd::CustomAnimationList::getImage(), illegal index!" );

    Image& rImage = maImages[nId - IMG_CUSTOMANIMATION_ON_CLICK];

    // load on demand
    if( rImage.GetSizePixel().Width() == 0 )
        rImage = Image(SdResId( nId ) );

    return rImage;
}

CustomAnimationList::~CustomAnimationList()
{
    disposeOnce();
}

void CustomAnimationList::dispose()
{
    if( mpMainSequence.get() )
        mpMainSequence->removeListener( this );

    clear();
    SvTreeListBox::dispose();
}

void CustomAnimationList::KeyInput( const KeyEvent& rKEvt )
{
    const int nKeyCode = rKEvt.GetKeyCode().GetCode();
    switch( nKeyCode )
    {
        case KEY_DELETE:    mpController->onContextMenu( CM_REMOVE ); return;
        case KEY_INSERT:    mpController->onContextMenu( CM_CREATE ); return;
        case KEY_SPACE:
            {
                const Point aPos;
                const CommandEvent aCEvt( aPos, CommandEventId::ContextMenu );
                Command( aCEvt );
                return;
            }

    }

    ::SvTreeListBox::KeyInput( rKEvt );
}

/** selects or deselects the given effect.
    Selections of other effects are not changed */
void CustomAnimationList::select( const CustomAnimationEffectPtr& pEffect )
{
    CustomAnimationListEntry* pEntry = static_cast< CustomAnimationListEntry* >(First());
    while( pEntry )
    {
        if( pEntry->getEffect() == pEffect )
        {
            Select( pEntry );
            MakeVisible( pEntry );
            break;
        }
        pEntry = static_cast< CustomAnimationListEntry* >(Next( pEntry ));
    }

    if( !pEntry )
    {
        append( pEffect );
        select( pEffect );
    }
}

void CustomAnimationList::clear()
{
    Clear();

    mpLastParentEntry = nullptr;
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

struct stl_append_effect_func : public std::unary_function<CustomAnimationEffectPtr, void>
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
    SetUpdateMode( false );

    CustomAnimationListEntry* pEntry = nullptr;

    std::list< CustomAnimationEffectPtr > aExpanded;
    std::list< CustomAnimationEffectPtr > aSelected;

    CustomAnimationEffectPtr pFirstSelEffect;
    CustomAnimationEffectPtr pLastSelEffect;
    long nFirstVis = -1;
    long nLastVis = -1;
    long nFirstSelOld = -1;
    long nLastSelOld = -1;

    if( mpMainSequence.get() )
    {
        // save scroll position
        pEntry = static_cast<CustomAnimationListEntry*>(GetFirstEntryInView());
        if( pEntry )
            nFirstVis = GetAbsPos( pEntry );

        pEntry = static_cast<CustomAnimationListEntry*>(GetLastEntryInView());
        if( pEntry )
            nLastVis = GetAbsPos( pEntry );

        pEntry = static_cast<CustomAnimationListEntry*>(FirstSelected());
        if( pEntry )
        {
            pFirstSelEffect = pEntry->getEffect();
            nFirstSelOld = GetAbsPos( pEntry );
        }

        pEntry = static_cast<CustomAnimationListEntry*>(LastSelected());
        if( pEntry )
        {
            pLastSelEffect = pEntry->getEffect();
            nLastSelOld = GetAbsPos( pEntry );
        }

        // save selection and expand states
        pEntry = static_cast<CustomAnimationListEntry*>(First());

        while( pEntry )
        {
            CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
            if( pEffect.get() )
            {
                if( IsExpanded( pEntry ) )
                    aExpanded.push_back( pEffect );

                if( IsSelected( pEntry ) )
                    aSelected.push_back( pEffect );
            }

            pEntry = static_cast<CustomAnimationListEntry*>(Next( pEntry ));
        }
    }

    // rebuild list
    clear();
    if( mpMainSequence.get() )
    {
        long nFirstSelNew = -1;
        long nLastSelNew = -1;
        std::for_each( mpMainSequence->getBegin(), mpMainSequence->getEnd(), stl_append_effect_func( *this ) );
        mpLastParentEntry = nullptr;

        const InteractiveSequenceList& rISL = mpMainSequence->getInteractiveSequenceList();

        InteractiveSequenceList::const_iterator aIter( rISL.begin() );
        const InteractiveSequenceList::const_iterator aEnd( rISL.end() );
        while( aIter != aEnd )
        {
            InteractiveSequencePtr pIS( (*aIter++) );

            Reference< XShape > xShape( pIS->getTriggerShape() );
            if( xShape.is() )
            {
                SvTreeListEntry* pLBoxEntry = new CustomAnimationListEntry;
                pLBoxEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(Image(), Image(), false));
                OUString aDescription = SD_RESSTR(STR_CUSTOMANIMATION_TRIGGER);
                aDescription += ": ";
                aDescription += getShapeDescription( xShape, false );
                pLBoxEntry->AddItem(o3tl::make_unique<CustomAnimationTriggerEntryItem>(aDescription));
                Insert( pLBoxEntry );
                SvViewDataEntry* pViewData = GetViewData( pLBoxEntry );
                if( pViewData )
                    pViewData->SetSelectable(false);

                std::for_each( pIS->getBegin(), pIS->getEnd(), stl_append_effect_func( *this ) );
                mpLastParentEntry = nullptr;
            }
        }

        // restore selection and expand states
        pEntry = static_cast<CustomAnimationListEntry*>(First());

        while( pEntry )
        {
            CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
            if( pEffect.get() )
            {
                if( std::find( aExpanded.begin(), aExpanded.end(), pEffect ) != aExpanded.end() )
                    Expand( pEntry );

                if( std::find( aSelected.begin(), aSelected.end(), pEffect ) != aSelected.end() )
                    Select( pEntry );

                if( pEffect == pFirstSelEffect )
                    nFirstSelNew = GetAbsPos( pEntry );

                if( pEffect == pLastSelEffect )
                    nLastSelNew = GetAbsPos( pEntry );
            }

            pEntry = static_cast<CustomAnimationListEntry*>(Next( pEntry ));
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
                ScrollToAbsPos( nLastSelNew - (nLastVis - nFirstVis) );
            }
            else
                ScrollToAbsPos( nFirstSelNew );
        }
        else if( bMoved && nFirstSelOld > nLastVis && nFirstSelNew > nLastVis )
        {
            // The selection is below the visible area.
            // Scroll down to the first few selected entries.
            ScrollToAbsPos( nFirstSelNew );
        }
        else if( bMovedUp && nFirstSelOld <= nFirstVis )
        {
            // A visible entry has moved up out of view; scroll up one.
            ScrollToAbsPos( nFirstVis - 1 );
        }
        else if( bMovedDown && nLastSelOld >= nLastVis )
        {
            // An entry has moved down out of view; scroll down one.
            ScrollToAbsPos( nFirstVis + 1 );
        }
        else if ( nFirstVis != -1 )
        {
            // The selection is still in view, or it hasn't moved.
            ScrollToAbsPos( nFirstVis );
        }
    }

    mbIgnorePaint = false;
    SetUpdateMode( true );
    Invalidate();
}

void CustomAnimationList::append( CustomAnimationEffectPtr pEffect )
{
    // create a ui description
    OUString aDescription;

    Any aTarget( pEffect->getTarget() );
    if( aTarget.hasValue() ) try
    {
        aDescription = getDescription( aTarget, pEffect->getTargetSubItem() != ShapeAnimationSubType::ONLY_BACKGROUND );

        SvTreeListEntry* pParentEntry = nullptr;

        Reference< XShape > xTargetShape( pEffect->getTargetShape() );
        sal_Int32 nGroupId = pEffect->getGroupId();

        // if this effect has the same target and group-id as the last root effect,
        // the last root effect is also this effects parent
        if( mpLastParentEntry && (nGroupId != -1) && (mxLastTargetShape == xTargetShape) && (mnLastGroupId == nGroupId) )
            pParentEntry = mpLastParentEntry;

        // create an entry for the effect
        SvTreeListEntry* pEntry = new CustomAnimationListEntry( pEffect );

        pEntry->AddItem(o3tl::make_unique<SvLBoxContextBmp>(Image(), Image(), false));
        pEntry->AddItem(o3tl::make_unique<CustomAnimationListEntryItem>(aDescription, pEffect, this));

        if( pParentEntry )
        {
            // add a subentry
            Insert( pEntry, pParentEntry );
        }
        else
        {
            // add a root entry
            Insert( pEntry );

            // and the new root entry becomes the possible next group header
            mxLastTargetShape = xTargetShape;
            mnLastGroupId = nGroupId;
            mpLastParentEntry = pEntry;
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationList::append(), exception caught!" );
    }
}

void selectShape( SvTreeListBox* pTreeList, const Reference< XShape >& xShape )
{
    CustomAnimationListEntry* pEntry = static_cast< CustomAnimationListEntry* >(pTreeList->First());
    while( pEntry )
    {
        CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
        if( pEffect.get() )
        {
            if( pEffect->getTarget() == xShape )
                pTreeList->Select( pEntry );
        }

        pEntry = static_cast< CustomAnimationListEntry* >(pTreeList->Next( pEntry ));
    }
}

void CustomAnimationList::onSelectionChanged(const Any& rSelection)
{
    try
    {
        SelectAll(false);

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
                        selectShape( this, xShape );
                }
            }
            else
            {
                Reference< XShape > xShape(rSelection, UNO_QUERY);
                if( xShape.is() )
                    selectShape( this, xShape );
            }
        }

        SelectHdl();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationList::onSelectionChanged(), Exception caught!" );
    }
}

void CustomAnimationList::SelectHdl()
{
    if( mbIgnorePaint )
        return;
    SvTreeListBox::SelectHdl();
    mpController->onSelect();
}

bool CustomAnimationList::isExpanded( const CustomAnimationEffectPtr& pEffect ) const
{
    CustomAnimationListEntry* pEntry = static_cast<CustomAnimationListEntry*>(First());

    while( pEntry )
    {
        if( pEntry->getEffect() == pEffect )
            break;

        pEntry = static_cast<CustomAnimationListEntry*>(Next( pEntry ));
    }

    if( pEntry )
        pEntry = static_cast<CustomAnimationListEntry*>(GetParent( pEntry ));

    return (pEntry == nullptr) || IsExpanded( pEntry );
}

EffectSequence CustomAnimationList::getSelection() const
{
    EffectSequence aSelection;

    CustomAnimationListEntry* pEntry = dynamic_cast< CustomAnimationListEntry* >(FirstSelected());
    while( pEntry )
    {
        CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
        if( pEffect.get() )
            aSelection.push_back( pEffect );

        // if the selected effect is not expanded and has children
        // we say that the children are automatically selected
        if( !IsExpanded( pEntry ) )
        {
            CustomAnimationListEntry* pChild = dynamic_cast< CustomAnimationListEntry* >( FirstChild( pEntry ) );
            while( pChild )
            {
                if( !IsSelected( pChild ) )
                {
                    CustomAnimationEffectPtr pChildEffect( pChild->getEffect() );
                    if( pChildEffect.get() )
                        aSelection.push_back( pChildEffect );
                }

                pChild = dynamic_cast< CustomAnimationListEntry* >(  NextSibling( pChild ) );
            }
        }

        pEntry = static_cast< CustomAnimationListEntry* >(NextSelected( pEntry ));
    }

    return aSelection;
}

bool CustomAnimationList::DoubleClickHdl()
{
    mpController->onDoubleClick();
    return false;
}

VclPtr<PopupMenu> CustomAnimationList::CreateContextMenu()
{
    VclPtrInstance<PopupMenu> pMenu(SdResId( RID_EFFECT_CONTEXTMENU ));

    sal_Int16 nNodeType = -1;
    sal_Int16 nEntries = 0;

    CustomAnimationListEntry* pEntry = static_cast< CustomAnimationListEntry* >(FirstSelected());
    while( pEntry )
    {
        nEntries++;
        CustomAnimationEffectPtr pEffect( pEntry->getEffect() );
        if( pEffect.get() )
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
                    break;
                }
            }
        }

        pEntry = static_cast< CustomAnimationListEntry* >(NextSelected( pEntry ));
    }

    pMenu->CheckItem( CM_WITH_CLICK, nNodeType == EffectNodeType::ON_CLICK );
    pMenu->CheckItem( CM_WITH_PREVIOUS, nNodeType == EffectNodeType::WITH_PREVIOUS );
    pMenu->CheckItem( CM_AFTER_PREVIOUS, nNodeType == EffectNodeType::AFTER_PREVIOUS );
    pMenu->EnableItem( CM_OPTIONS, nEntries == 1 );
    pMenu->EnableItem( CM_DURATION, nEntries == 1 );

    return pMenu;
}

void CustomAnimationList::ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    mpController->onContextMenu( nSelectedPopupEntry );
}

void CustomAnimationList::notify_change()
{
    update();
    mpController->onSelect();
}

void CustomAnimationList::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if( mbIgnorePaint )
        return;

    SvTreeListBox::Paint(rRenderContext, rRect);

    // draw help text if list box is still empty
    if( First() == nullptr )
    {
        Color aOldColor(rRenderContext.GetTextColor());
        rRenderContext.SetTextColor(rRenderContext.GetSettings().GetStyleSettings().GetDisableColor());
        ::Point aOffset(rRenderContext.LogicToPixel(Point(6, 6), MAP_APPFONT));

        Rectangle aRect(Point(0,0), GetOutputSizePixel());

        aRect.Left() += aOffset.X();
        aRect.Top() += aOffset.Y();
        aRect.Right() -= aOffset.X();
        aRect.Bottom() -= aOffset.Y();

        rRenderContext.DrawText(aRect, SD_RESSTR(STR_CUSTOMANIMATION_LIST_HELPTEXT),
                                DrawTextFlags::MultiLine | DrawTextFlags::WordBreak | DrawTextFlags::Center | DrawTextFlags::VCenter );

        rRenderContext.SetTextColor(aOldColor);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
