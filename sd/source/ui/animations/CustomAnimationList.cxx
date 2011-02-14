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
#include "precompiled_sd.hxx"
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
#include <svx/ShapeTypeHandler.hxx>
#include "CustomAnimationList.hxx"
#ifndef _SD_CUSTOMANIMATIONPANE_HRC
#include "CustomAnimationPane.hrc"
#endif
#ifndef _SD_CUSTOMANIMATION_HRC
#include "CustomAnimation.hrc"
#endif
#include "CustomAnimationPreset.hxx"
#include <svtools/treelist.hxx>
#include <vcl/svapp.hxx>
#include "sdresid.hxx"

#include "res_bmp.hrc"
#include "glob.hrc"

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::rtl::OUString;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
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
using ::accessibility::ShapeTypeHandler;

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
        xChild = Reference< XChild >::query( x );
        Reference< XDrawPage > xTestPage( x, UNO_QUERY );
        if( xTestPage.is() )
            xPage = Reference< XShapes >::query( x );
    }

    sal_Int32 nIndex = 1;

    if( xPage.is() && getShapeIndex( xPage, xShape, nIndex ) )
        return nIndex;
    else
        return -1;
}

OUString getShapeDescription( const Reference< XShape >& xShape, bool bWithText = true )
{
    OUString aDescription;
    Reference< XPropertySet > xSet( xShape, UNO_QUERY );
    if( xSet.is() )
    {
        Reference< XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );
        const OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("UINameSingular") );
        if( xInfo->hasPropertyByName( aPropName ) )
            xSet->getPropertyValue( aPropName ) >>= aDescription;
    }

    aDescription += OUString( RTL_CONSTASCII_USTRINGPARAM(" "));
    aDescription += OUString::valueOf( getShapeIndex( xShape ) );

    if( bWithText )
    {
        Reference< XTextRange > xText( xShape, UNO_QUERY );
        if( xText.is() )
        {
            OUString aText( xText->getString() );
            if( aText.getLength() )
            {
                aDescription += OUString(RTL_CONSTASCII_USTRINGPARAM(": "));

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

    if( rTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
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

// ====================================================================

class CustomAnimationListEntryItem : public SvLBoxString
{
public:
                    CustomAnimationListEntryItem( SvLBoxEntry*,sal_uInt16 nFlags, OUString aDescription, CustomAnimationEffectPtr pEffect, CustomAnimationList* pParent  );
    virtual         ~CustomAnimationListEntryItem();
    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    void            Paint( const Point&, SvLBox& rDev, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
    void            Clone( SvLBoxItem* pSource );

private:
    CustomAnimationList* mpParent;
    OUString        maDescription;
    CustomAnimationEffectPtr mpEffect;
};

// --------------------------------------------------------------------

CustomAnimationListEntryItem::CustomAnimationListEntryItem( SvLBoxEntry* pEntry, sal_uInt16 nFlags, OUString aDescription, CustomAnimationEffectPtr pEffect, CustomAnimationList* pParent  )
: SvLBoxString( pEntry, nFlags, aDescription )
, mpParent( pParent )
, maDescription( aDescription )
, mpEffect(pEffect)
{
}

// --------------------------------------------------------------------

CustomAnimationListEntryItem::~CustomAnimationListEntryItem()
{
}

// --------------------------------------------------------------------

void CustomAnimationListEntryItem::InitViewData( SvLBox* pView, SvLBoxEntry* pEntry, SvViewDataItem* pViewData )
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Size aSize(pView->GetTextWidth( maDescription ) + 2 * 19, pView->GetTextHeight() );
    if( aSize.Height() < 19 )
        aSize.Height() = 19;
    pViewData->aSize = aSize;
}

// --------------------------------------------------------------------

void CustomAnimationListEntryItem::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16, SvLBoxEntry* pEntry )
{
    const bool bHighContrast = Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    SvViewDataItem* pViewData = mpParent->GetViewDataItem( pEntry, this );

    Point aPos( rPos );
    Size aSize( pViewData->aSize );

    sal_Int16 nNodeType = mpEffect->getNodeType();
    if( nNodeType == EffectNodeType::ON_CLICK )
    {
        rDev.DrawImage( aPos, mpParent->getImage( IMG_CUSTOMANIMATION_ON_CLICK, bHighContrast ) );
    }
    else if( nNodeType == EffectNodeType::AFTER_PREVIOUS )
    {
        rDev.DrawImage( aPos, mpParent->getImage( IMG_CUSTOMANIMATION_AFTER_PREVIOUS, bHighContrast ) );
    }

    aPos.X() += 19;

    sal_uInt16 nImage;
    switch( mpEffect->getPresetClass() )
    {
    case EffectPresetClass::ENTRANCE:   nImage = IMG_CUSTOMANIMATION_ENTRANCE_EFFECT; break;
    case EffectPresetClass::EXIT:       nImage =  IMG_CUSTOMANIMATION_EXIT_EFFECT; break;
    case EffectPresetClass::EMPHASIS:   nImage =  IMG_CUSTOMANIMATION_EMPHASIS_EFFECT; break;
    case EffectPresetClass::MOTIONPATH: nImage = IMG_CUSTOMANIMATION_MOTION_PATH; break;
    case EffectPresetClass::OLEACTION:  nImage = IMG_CUSTOMANIMATION_OLE; break;
    case EffectPresetClass::MEDIACALL:
        switch( mpEffect->getCommand() )
        {
        case EffectCommands::TOGGLEPAUSE:   nImage = IMG_CUSTOMANIMATION_MEDIA_PAUSE; break;
        case EffectCommands::STOP:          nImage = IMG_CUSTOMANIMATION_MEDIA_STOP; break;
        case EffectCommands::PLAY:
        default:                            nImage = IMG_CUSTOMANIMATION_MEDIA_PLAY; break;
        }
        break;
    default:                            nImage = 0xffff;
    }

    if( nImage != 0xffff )
    {
        const Image& rImage = mpParent->getImage( nImage, bHighContrast );
        Point aImagePos( aPos );
        aImagePos.Y() += ( aSize.Height() - rImage.GetSizePixel().Height() ) >> 1;
        rDev.DrawImage( aImagePos, rImage );
    }

    aPos.X() += 19;
    aPos.Y() += ( aSize.Height() - rDev.GetTextHeight()) >> 1;

    rDev.DrawText( aPos, rDev.GetEllipsisString( maDescription, rDev.GetOutputSizePixel().Width() - aPos.X() ) );
}

// --------------------------------------------------------------------

SvLBoxItem* CustomAnimationListEntryItem::Create() const
{
    return NULL;
}

// --------------------------------------------------------------------

void CustomAnimationListEntryItem::Clone( SvLBoxItem* )
{
}

// ====================================================================

class CustomAnimationListEntry : public SvLBoxEntry
{
public:
    CustomAnimationListEntry();
    CustomAnimationListEntry( CustomAnimationEffectPtr pEffect );
    virtual ~CustomAnimationListEntry();

    CustomAnimationEffectPtr getEffect() const { return mpEffect; }

private:
    CustomAnimationEffectPtr mpEffect;
};

// --------------------------------------------------------------------

CustomAnimationListEntry::CustomAnimationListEntry()
{
}

// --------------------------------------------------------------------

CustomAnimationListEntry::CustomAnimationListEntry( CustomAnimationEffectPtr pEffect )
: mpEffect( pEffect )
{
}

// --------------------------------------------------------------------

CustomAnimationListEntry::~CustomAnimationListEntry()
{
}

// ====================================================================

class CustomAnimationTriggerEntryItem : public SvLBoxString
{
public:
                    CustomAnimationTriggerEntryItem( SvLBoxEntry*,sal_uInt16 nFlags, OUString aDescription, CustomAnimationList* pParent  );
    virtual         ~CustomAnimationTriggerEntryItem();
    virtual sal_uInt16  IsA();
    void            InitViewData( SvLBox*,SvLBoxEntry*,SvViewDataItem* );
    void            Paint( const Point&, SvLBox& rDev, sal_uInt16 nFlags,SvLBoxEntry* );
    SvLBoxItem*     Create() const;
    void            Clone( SvLBoxItem* pSource );

private:
    CustomAnimationList* mpParent;
    OUString        maDescription;
};

// --------------------------------------------------------------------

CustomAnimationTriggerEntryItem::CustomAnimationTriggerEntryItem( SvLBoxEntry* pEntry, sal_uInt16 nFlags, OUString aDescription, CustomAnimationList* pParent  )
: SvLBoxString( pEntry, nFlags, aDescription ), mpParent( pParent ), maDescription( aDescription )
{
}

// --------------------------------------------------------------------

CustomAnimationTriggerEntryItem::~CustomAnimationTriggerEntryItem()
{
}

// --------------------------------------------------------------------

sal_uInt16 CustomAnimationTriggerEntryItem::IsA()
{
    return (sal_uInt16)-1;
}

// --------------------------------------------------------------------

void CustomAnimationTriggerEntryItem::InitViewData( SvLBox* pView, SvLBoxEntry* pEntry, SvViewDataItem* pViewData )
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    Size aSize(pView->GetTextWidth( maDescription ) + 2 * 19, pView->GetTextHeight() );
    if( aSize.Height() < 19 )
        aSize.Height() = 19;
    pViewData->aSize = aSize;

/*
        SvViewData* pViewData = pView->GetViewData( pEntry );
        if( pViewData )
            pViewData->SetSelectable(false);
*/
}

// --------------------------------------------------------------------

void CustomAnimationTriggerEntryItem::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16, SvLBoxEntry* )
{
    Size aSize( rDev.GetOutputSizePixel().Width(), static_cast< SvTreeListBox* >(&rDev)->GetEntryHeight() );

    Point aPos( 0, rPos.Y() );

    Rectangle aOutRect( aPos, aSize );

    // fill the background
    Color aColor (rDev.GetSettings().GetStyleSettings().GetDialogColor());

    rDev.Push();
    rDev.SetFillColor (aColor);
    rDev.SetLineColor ();
    rDev.DrawRect(aOutRect);

    // Erase the four corner pixels to make the rectangle appear rounded.
    rDev.SetLineColor( rDev.GetSettings().GetStyleSettings().GetWindowColor());
    rDev.DrawPixel( aOutRect.TopLeft());
    rDev.DrawPixel( Point(aOutRect.Right(), aOutRect.Top()));
    rDev.DrawPixel( Point(aOutRect.Left(), aOutRect.Bottom()));
    rDev.DrawPixel( Point(aOutRect.Right(), aOutRect.Bottom()));

    // draw the category title

    int nVertBorder = (( aSize.Height() - rDev.GetTextHeight()) >> 1);
    int nHorzBorder = rDev.LogicToPixel( Size( 3, 3 ), MAP_APPFONT ).Width();

    aOutRect.nLeft += nHorzBorder;
    aOutRect.nRight -= nHorzBorder;
    aOutRect.nTop += nVertBorder;
    aOutRect.nBottom -= nVertBorder;

    rDev.DrawText (aOutRect, rDev.GetEllipsisString( maDescription, aOutRect.GetWidth() ) );
    rDev.Pop();
}

// --------------------------------------------------------------------

SvLBoxItem* CustomAnimationTriggerEntryItem::Create() const
{
    return NULL;
}

// --------------------------------------------------------------------

void CustomAnimationTriggerEntryItem::Clone( SvLBoxItem* )
{
}

// ====================================================================

CustomAnimationList::CustomAnimationList( ::Window* pParent, const ResId& rResId, ICustomAnimationListController* pController )
:   SvTreeListBox( pParent, rResId )
,   mbIgnorePaint( false )
,   mpController( pController )
,   mpLastParentEntry(0)
{
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES | WB_HASBUTTONS | WB_HASBUTTONSATROOT );

    EnableContextMenuHandling();
    SetSelectionMode( MULTIPLE_SELECTION );
    SetIndent(16);
    SetNodeDefaultImages();
}

// --------------------------------------------------------------------

const Image&  CustomAnimationList::getImage( sal_uInt16 nId, bool bHighContrast )
{
    DBG_ASSERT( (nId >= IMG_CUSTOMANIMATION_ON_CLICK) && (nId <= IMG_CUSTOMANIMATION_MEDIA_STOP), "sd::CustomAnimationList::getImage(), illegal index!" );

    if( bHighContrast )
        nId += 1;

    Image& rImage = maImages[nId - IMG_CUSTOMANIMATION_ON_CLICK];

    // load on demand
    if( rImage.GetSizePixel().Width() == 0 )
        rImage = Image(SdResId( nId ) );

    return rImage;
}

// --------------------------------------------------------------------

CustomAnimationList::~CustomAnimationList()
{
    if( mpMainSequence.get() )
        mpMainSequence->removeListener( this );

    clear();
}

// --------------------------------------------------------------------

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
                const CommandEvent aCEvt( aPos, COMMAND_CONTEXTMENU );
                Command( aCEvt );
                return;
            }

    }

    ::SvTreeListBox::KeyInput( rKEvt );
}

// --------------------------------------------------------------------

/** selects or deselects the given effect.
    Selections of other effects are not changed */
void CustomAnimationList::select( CustomAnimationEffectPtr pEffect, bool bSelect /* = true */ )
{
    CustomAnimationListEntry* pEntry = static_cast< CustomAnimationListEntry* >(First());
    while( pEntry )
    {
        if( pEntry->getEffect() == pEffect )
        {
            Select( pEntry, bSelect );
            break;
        }
        pEntry = static_cast< CustomAnimationListEntry* >(Next( pEntry ));
    }

    if( !pEntry && bSelect )
    {
        append( pEffect );
        select( pEffect );
    }
}

// --------------------------------------------------------------------

void CustomAnimationList::clear()
{
    Clear();

    mpLastParentEntry = 0;
    mxLastTargetShape = 0;
}

// --------------------------------------------------------------------

void CustomAnimationList::update( MainSequencePtr pMainSequence )
{
    if( mpMainSequence.get() )
        mpMainSequence->removeListener( this );

    mpMainSequence = pMainSequence;
    update();

    if( mpMainSequence.get() )
        mpMainSequence->addListener( this );
}

// --------------------------------------------------------------------

struct stl_append_effect_func : public std::unary_function<CustomAnimationEffectPtr, void>
{
    stl_append_effect_func( CustomAnimationList& rList ) : mrList( rList ) {}
    void operator()(CustomAnimationEffectPtr pEffect);
    CustomAnimationList& mrList;
};

void stl_append_effect_func::operator()(CustomAnimationEffectPtr pEffect)
{
    mrList.append( pEffect );
}
// --------------------------------------------------------------------

void CustomAnimationList::update()
{
    mbIgnorePaint = true;

    CustomAnimationListEntry* pEntry = 0;

    std::list< CustomAnimationEffectPtr > aExpanded;
    std::list< CustomAnimationEffectPtr > aSelected;

    CustomAnimationEffectPtr pFirstVisibleEffect;

    if( mpMainSequence.get() )
    {
        // save selection and expand states
        pEntry = static_cast<CustomAnimationListEntry*>(FirstVisible());
        if( pEntry )
            pFirstVisibleEffect = pEntry->getEffect();

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
        std::for_each( mpMainSequence->getBegin(), mpMainSequence->getEnd(), stl_append_effect_func( *this ) );
        mpLastParentEntry = 0;

        const InteractiveSequenceList& rISL = mpMainSequence->getInteractiveSequenceList();

        InteractiveSequenceList::const_iterator aIter( rISL.begin() );
        const InteractiveSequenceList::const_iterator aEnd( rISL.end() );
        while( aIter != aEnd )
        {
            InteractiveSequencePtr pIS( (*aIter++) );

            Reference< XShape > xShape( pIS->getTriggerShape() );
            if( xShape.is() )
            {
                SvLBoxEntry* pLBoxEntry = new CustomAnimationListEntry;
                pLBoxEntry->AddItem( new SvLBoxContextBmp( pLBoxEntry, 0, Image(), Image(), 0));
                OUString aDescription = String( SdResId( STR_CUSTOMANIMATION_TRIGGER ) );
                aDescription += OUString( RTL_CONSTASCII_USTRINGPARAM(": ") );
                aDescription += getShapeDescription( xShape, false );
                pLBoxEntry->AddItem( new CustomAnimationTriggerEntryItem( pLBoxEntry, 0, aDescription, this ) );
                Insert( pLBoxEntry );
                SvViewData* pViewData = GetViewData( pLBoxEntry );
                if( pViewData )
                    pViewData->SetSelectable(false);

                std::for_each( pIS->getBegin(), pIS->getEnd(), stl_append_effect_func( *this ) );
                mpLastParentEntry = 0;
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

                if( pFirstVisibleEffect == pEffect )
                    MakeVisible( pEntry );
            }

            pEntry = static_cast<CustomAnimationListEntry*>(Next( pEntry ));
        }
    }

    mbIgnorePaint = false;
    Invalidate();
}

// --------------------------------------------------------------------

/*
void CustomAnimationList::update( CustomAnimationEffectPtr pEffect )
{
    SvLBoxEntry* pEntry = First();
    while( pEntry )
    {
        if( static_cast< CustomAnimationEffectPtr * >( pEntry->GetUserData() )->get() == pEffect.get() )
        {
            CustomAnimationPresetsPtr pPresets = mpController->getPresets();
            const CustomAnimationPresetPtr pPreset = pPresets->getEffectDescriptor( pEffect->getPresetId() );
            if( pPreset.get() )
                pEffect->setName( pPresets->getUINameForPresetId( pPreset->getPresetId() ) );
            else
                pEffect->setName( pEffect->getPresetId() );
            break;
        }
        pEntry = Next( pEntry );
    }

    Invalidate();
}
*/

// --------------------------------------------------------------------

void CustomAnimationList::append( CustomAnimationEffectPtr pEffect )
{
    // create a ui description
    OUString aDescription;

    Any aTarget( pEffect->getTarget() );
    if( aTarget.hasValue() ) try
    {
        aDescription = getDescription( aTarget, pEffect->getTargetSubItem() != ShapeAnimationSubType::ONLY_BACKGROUND );

        SvLBoxEntry* pParentEntry = 0;

        Reference< XShape > xTargetShape( pEffect->getTargetShape() );
        sal_Int32 nGroupId = pEffect->getGroupId();

        // if this effect has the same target and group-id as the last root effect,
        // the last root effect is also this effects parent
        if( mpLastParentEntry && (nGroupId != -1) && (mxLastTargetShape == xTargetShape) && (mnLastGroupId == nGroupId) )
            pParentEntry = mpLastParentEntry;

        // create an entry for the effect
        SvLBoxEntry* pEntry = new CustomAnimationListEntry( pEffect );

        pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));
        pEntry->AddItem( new CustomAnimationListEntryItem( pEntry, 0, aDescription, pEffect, this ) );

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
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::CustomAnimationList::append(), exception catched!" );
    }
}

// --------------------------------------------------------------------

/*
void CustomAnimationList::remove( CustomAnimationEffectPtr pEffect )
{
    SvLBoxEntry* pEntry = First();
    while( pEntry )
    {
        if( static_cast< CustomAnimationEffectPtr * >( pEntry->GetUserData() )->get() == pEffect.get() )
        {
            GetModel()->Remove( pEntry );
            if( pEntry == mpLastParentEntry )
            {
                mpLastParentEntry = 0;
                mxLastTargetShape = 0;
            }
            break;
        }
        pEntry = Next( pEntry );
    }

    Invalidate();
}
*/

// --------------------------------------------------------------------

void selectShape( SvTreeListBox* pTreeList, Reference< XShape > xShape )
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

// --------------------------------------------------------------------

void CustomAnimationList::onSelectionChanged( Any aSelection )
{
    try
    {
        SelectAll(sal_False);

        if( aSelection.hasValue() )
        {
            Reference< XIndexAccess > xShapes( aSelection, UNO_QUERY );
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
                Reference< XShape > xShape( aSelection, UNO_QUERY );
                if( xShape.is() )
                    selectShape( this, xShape );
            }
        }

        SelectHdl();
    }
    catch( Exception& )
    {
        DBG_ERROR( "sd::CustomAnimationList::onSelectionChanged(), Exception catched!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationList::SelectHdl()
{
    SvTreeListBox::SelectHdl();
    mpController->onSelect();
}

// --------------------------------------------------------------------

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

    return (pEntry == 0) || IsExpanded( pEntry );
}

// --------------------------------------------------------------------
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
        // we say that the children are automaticly selected
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

// --------------------------------------------------------------------

sal_Bool CustomAnimationList::DoubleClickHdl()
{
    mpController->onDoubleClick();
    return false;
}

// --------------------------------------------------------------------

PopupMenu* CustomAnimationList::CreateContextMenu()
{
    PopupMenu* pMenu = new PopupMenu(SdResId( RID_EFFECT_CONTEXTMENU ));

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

// --------------------------------------------------------------------

void CustomAnimationList::ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    mpController->onContextMenu( nSelectedPopupEntry );
}

// --------------------------------------------------------------------

void CustomAnimationList::SetTabs()
{
    SvTreeListBox::SetTabs();
}

// --------------------------------------------------------------------

void CustomAnimationList::notify_change()
{
    update();
    mpController->onSelect();
}

void CustomAnimationList::Paint( const Rectangle& rRect )
{
    if( mbIgnorePaint )
        return;

    SvTreeListBox::Paint( rRect );

    // draw help text if list box is still empty
    if( First() == 0 )
    {
        Color aOldColor( GetTextColor() );
        SetTextColor( GetSettings().GetStyleSettings().GetDisableColor() );
        ::Point aOffset( LogicToPixel( Point( 6, 6 ), MAP_APPFONT ) );

        Rectangle aRect( Point( 0,0 ), GetOutputSizePixel() );

        aRect.Left() += aOffset.X();
        aRect.Top() += aOffset.Y();
        aRect.Right() -= aOffset.X();
        aRect.Bottom() -= aOffset.Y();

        DrawText( aRect, String( SdResId( STR_CUSTOMANIMATION_LIST_HELPTEXT ) ),
            TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK | TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER );

        SetTextColor( aOldColor );
    }
}

}
