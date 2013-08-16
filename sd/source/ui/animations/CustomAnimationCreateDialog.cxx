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


#include <com/sun/star/i18n/Collator.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

#include <svx/svdetc.hxx>
#include <svx/svdstr.hrc>
#include "sdresid.hxx"
#include <unotools/viewoptions.hxx>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include "CustomAnimationCreateDialog.hxx"
#include "CustomAnimation.hrc"
#include "CustomAnimationPane.hxx"
#include "optsitem.hxx"
#include "sddll.hxx"
#include "sdmod.hxx"

#include "helpids.h"

using namespace ::com::sun::star;

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;

using namespace ::com::sun::star::presentation;

namespace sd {


const int ENTRANCE = 0;
const int EMPHASIS = 1;
const int EXIT = 2;
const int MOTIONPATH = 3;
const int MISCEFFECTS = 4;

// --------------------------------------------------------------------

extern void fillDurationComboBox( ListBox* pBox );

class CategoryListBox : public ListBox
{
public:
    CategoryListBox( Window* pParent );
    ~CategoryListBox();

    virtual void        MouseButtonUp( const MouseEvent& rMEvt );

    sal_uInt16          InsertCategory( const XubString& rStr, sal_uInt16 nPos = LISTBOX_APPEND );

    void            SetDoubleClickLink( const Link& rDoubleClickHdl ) { maDoubleClickHdl = rDoubleClickHdl; }

    DECL_LINK(implDoubleClickHdl, void *);

private:
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    Link            maDoubleClickHdl;
};

CategoryListBox::CategoryListBox( Window* pParent )
: ListBox( pParent, WB_TABSTOP | WB_BORDER )
{
    EnableUserDraw( sal_True );
    SetDoubleClickHdl( LINK( this, CategoryListBox, implDoubleClickHdl ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeCategoryListBox(Window *pParent, VclBuilder::stringmap &)
{
    return new CategoryListBox( pParent );
}

CategoryListBox::~CategoryListBox()
{
}

sal_uInt16 CategoryListBox::InsertCategory( const XubString& rStr, sal_uInt16 nPos /* = LISTBOX_APPEND */ )
{
    sal_uInt16 n = ListBox::InsertEntry( rStr, nPos );
    if( n != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SetEntryFlags( n, ListBox::GetEntryFlags(n) | LISTBOX_ENTRY_FLAG_DISABLE_SELECTION );

    return n;
}

void CategoryListBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    const sal_uInt16 nItem = rUDEvt.GetItemId();

    if( ListBox::GetEntryFlags(nItem) & LISTBOX_ENTRY_FLAG_DISABLE_SELECTION )
    {
        Rectangle aOutRect( rUDEvt.GetRect() );
        OutputDevice* pDev = rUDEvt.GetDevice();

        // fill the background
        Color aColor (GetSettings().GetStyleSettings().GetDialogColor());

        pDev->SetFillColor (aColor);
        pDev->SetLineColor ();
        pDev->DrawRect(aOutRect);

        // Erase the four corner pixels to make the rectangle appear rounded.
        pDev->SetLineColor( GetSettings().GetStyleSettings().GetWindowColor());
        pDev->DrawPixel( aOutRect.TopLeft());
        pDev->DrawPixel( Point(aOutRect.Right(), aOutRect.Top()));
        pDev->DrawPixel( Point(aOutRect.Left(), aOutRect.Bottom()));
        pDev->DrawPixel( Point(aOutRect.Right(), aOutRect.Bottom()));

        // draw the category title
        pDev->DrawText (aOutRect, GetEntry(nItem), TEXT_DRAW_CENTER );
    }
    else
    {
        DrawEntry( rUDEvt, sal_True, sal_True );
    }
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(CategoryListBox, implDoubleClickHdl)
{
    CaptureMouse();
    return 0;
}

// --------------------------------------------------------------------

void CategoryListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ReleaseMouse();
    if( rMEvt.IsLeft() && (rMEvt.GetClicks() == 2) )
    {
        if( maDoubleClickHdl.IsSet() )
            maDoubleClickHdl.Call( this );
    }
    else
    {
        ListBox::MouseButtonUp( rMEvt );
    }
}

// --------------------------------------------------------------------

class CustomAnimationCreateTabPage : public TabPage
{
public:
    CustomAnimationCreateTabPage( Window* pParent, CustomAnimationCreateDialog* pDialogParent, sal_uInt16 nTabId, const PresetCategoryList& rCategoryList, bool bHasText );
    ~CustomAnimationCreateTabPage();

    PathKind getCreatePathKind() const;
    CustomAnimationPresetPtr getSelectedPreset() const;
    double getDuration() const;
    void setDuration( double fDuration );

    bool getIsPreview() const;
    void setIsPreview( bool bIsPreview );

    sal_uInt16 getId() const;

    bool select( const OUString& rsPresetId );

private:
    DECL_LINK( implSelectHdl, Control* );
    DECL_LINK( implDoubleClickHdl, Control* );

    void onSelectEffect();

    void clearEffects();

private:
    CategoryListBox*    mpLBEffects;
    FixedText*  mpFTSpeed;
    ListBox*    mpCBSpeed;
    CheckBox*   mpCBXPReview;

    CustomAnimationCreateDialog*        mpParent;

    sal_uInt16 mnId;

    sal_uInt16 mnCurvePathPos;
    sal_uInt16 mnPolygonPathPos;
    sal_uInt16 mnFreeformPathPos;

};

struct ImplStlEffectCategorySortHelper
{
    ImplStlEffectCategorySortHelper();
    bool operator()( const CustomAnimationPresetPtr& p1, const CustomAnimationPresetPtr& p2 );

private:
    uno::Reference< i18n::XCollator > mxCollator;
};

ImplStlEffectCategorySortHelper::ImplStlEffectCategorySortHelper()
{
    mxCollator = i18n::Collator::create( ::comphelper::getProcessComponentContext() );

    const lang::Locale& rLocale = Application::GetSettings().GetLanguageTag().getLocale();
    mxCollator->loadDefaultCollator(rLocale, 0);
}

bool ImplStlEffectCategorySortHelper::operator()( const CustomAnimationPresetPtr& p1, const CustomAnimationPresetPtr& p2 )
{
    return mxCollator->compareString(p1->getLabel(), p2->getLabel()) == -1;
}

CustomAnimationCreateTabPage::CustomAnimationCreateTabPage( Window* pParent, CustomAnimationCreateDialog* pDialogParent, sal_uInt16 nTabId, const PresetCategoryList& rCategoryList, bool bHasText )
: TabPage( pParent, "CustomAnimationCreateTab", "modules/simpress/ui/customanimationcreatetab.ui" )
, mpParent( pDialogParent )
, mnId( nTabId )
, mnCurvePathPos( LISTBOX_ENTRY_NOTFOUND )
, mnPolygonPathPos( LISTBOX_ENTRY_NOTFOUND )
, mnFreeformPathPos( LISTBOX_ENTRY_NOTFOUND )
{
    get( mpLBEffects, "effect_list" );
    mpLBEffects->set_height_request( mpLBEffects->GetTextHeight() * 16 );

    get( mpFTSpeed, "effect_speed_label" );
    get( mpCBSpeed, "effect_speed_list" );
    get( mpCBXPReview, "auto_preview" );

    fillDurationComboBox(mpCBSpeed);

    OUString sMotionPathLabel( SD_RESSTR( STR_CUSTOMANIMATION_USERPATH ) );

    sal_uInt16 nFirstEffect = LISTBOX_ENTRY_NOTFOUND;

    if( nTabId == MOTIONPATH )
    {
        mpLBEffects->InsertCategory( sMotionPathLabel );

        mnCurvePathPos = nFirstEffect = mpLBEffects->InsertEntry( sdr::GetResourceString(STR_ObjNameSingulCOMBLINE) );
        mnPolygonPathPos = mpLBEffects->InsertEntry( sdr::GetResourceString(STR_ObjNameSingulPOLY) );
        mnFreeformPathPos = mpLBEffects->InsertEntry( sdr::GetResourceString(STR_ObjNameSingulFREELINE) );
    };

    PresetCategoryList::const_iterator aCategoryIter( rCategoryList.begin() );
    const PresetCategoryList::const_iterator aCategoryEnd( rCategoryList.end() );
    while( aCategoryIter != aCategoryEnd )
    {
        PresetCategoryPtr pCategory( *aCategoryIter++ );
        if( pCategory.get() )
        {
            mpLBEffects->InsertCategory( pCategory->maLabel );

            std::vector< CustomAnimationPresetPtr > aSortedVector(pCategory->maEffects.size());
            std::copy( pCategory->maEffects.begin(), pCategory->maEffects.end(), aSortedVector.begin() );
            ImplStlEffectCategorySortHelper aSortHelper;
            std::sort( aSortedVector.begin(), aSortedVector.end(), aSortHelper );

            std::vector< CustomAnimationPresetPtr >::const_iterator aIter( aSortedVector.begin() );
            const std::vector< CustomAnimationPresetPtr >::const_iterator aEnd( aSortedVector.end() );
            while( aIter != aEnd )
            {
                CustomAnimationPresetPtr pDescriptor = (*aIter++);
                if( pDescriptor.get() && (bHasText || !pDescriptor->isTextOnly() ) )
                {
                    sal_uInt16 nPos = mpLBEffects->InsertEntry( pDescriptor->getLabel() );
                    mpLBEffects->SetEntryData( nPos, static_cast<void*>( new CustomAnimationPresetPtr( pDescriptor ) ) );

                    if( nFirstEffect == LISTBOX_ENTRY_NOTFOUND )
                        nFirstEffect = nPos;
                }
            }
        }
    }

    mpLBEffects->SelectEntryPos( nFirstEffect );

    if( nFirstEffect != LISTBOX_ENTRY_NOTFOUND )
        onSelectEffect();

    mpLBEffects->SetSelectHdl( LINK( this, CustomAnimationCreateTabPage, implSelectHdl ) );
    mpLBEffects->SetDoubleClickLink( LINK( this, CustomAnimationCreateTabPage, implDoubleClickHdl ) );
}

CustomAnimationCreateTabPage::~CustomAnimationCreateTabPage()
{
    clearEffects();
}

IMPL_LINK( CustomAnimationCreateTabPage, implSelectHdl, Control*, pControl )
{
    if( pControl == mpLBEffects )
        onSelectEffect();
    return 0;
}

IMPL_LINK( CustomAnimationCreateTabPage, implDoubleClickHdl, Control*, pControl )
{
    if( pControl == mpLBEffects )
    {
        if( mpLBEffects->GetSelectEntryCount() )
            mpParent->EndDialog( sal_True );
    }
    return 0;
}

void CustomAnimationCreateTabPage::onSelectEffect()
{
    CustomAnimationPresetPtr*p = static_cast< CustomAnimationPresetPtr* >( mpLBEffects->GetEntryData( mpLBEffects->GetSelectEntryPos() ) );

    if( !p )
        return;

    CustomAnimationPresetPtr pPreset( *p );

    const double fDuration = pPreset->getDuration();
    sal_uInt16 nPos = 0xffff;

    if( fDuration == 5.0 )
        nPos = 0;
    else if( fDuration == 3.0 )
        nPos = 1;
    else if( fDuration == 2.0 )
        nPos = 2;
    else if( fDuration == 1.0 )
        nPos = 3;
    else if( fDuration == 0.5 )
        nPos = 4;

    mpCBSpeed->SelectEntryPos( nPos );

    bool bHasSpeed = pPreset->getDuration() > 0.001;
    mpCBSpeed->Enable( bHasSpeed );
    mpFTSpeed->Enable( bHasSpeed );

    if( mpCBXPReview->IsChecked() )
    {
        mpParent->preview( pPreset );
    }
}

void CustomAnimationCreateTabPage::clearEffects()
{
    sal_uInt16 nPos = mpLBEffects->GetEntryCount();
    while( nPos-- )
        delete static_cast< CustomAnimationPresetPtr* >( mpLBEffects->GetEntryData( nPos ) );

    mpLBEffects->Clear();
}

CustomAnimationPresetPtr CustomAnimationCreateTabPage::getSelectedPreset() const
{
    CustomAnimationPresetPtr pPreset;

    if( mpLBEffects->GetSelectEntryCount() == 1 )
    {
        void* pEntryData = mpLBEffects->GetEntryData( mpLBEffects->GetSelectEntryPos() );
        if( pEntryData )
            pPreset = *static_cast< CustomAnimationPresetPtr* >( pEntryData );
    }

    return pPreset;
}

PathKind CustomAnimationCreateTabPage::getCreatePathKind() const
{
    PathKind eKind = NONE;

    if( mpLBEffects->GetSelectEntryCount() == 1 )
    {
        const sal_uInt16 nPos = mpLBEffects->GetSelectEntryPos();
        if( nPos == mnCurvePathPos )
        {
            eKind = CURVE;
        }
        else if( nPos == mnPolygonPathPos )
        {
            eKind = POLYGON;
        }
        else if( nPos == mnFreeformPathPos )
        {
            eKind = FREEFORM;
        }
    }

    return eKind;
}



double CustomAnimationCreateTabPage::getDuration() const
{
    sal_uInt16 nPos = mpCBSpeed->GetSelectEntryPos();
    if( (nPos == 0xffff) || !mpCBSpeed->IsEnabled() )
    {
        CustomAnimationPresetPtr pPreset = getSelectedPreset();
        if( pPreset.get() )
            return pPreset->getDuration();
    }

    switch( nPos )
    {
    case 0: return 5.0f;
    case 1: return 3.0f;
    case 2: return 2.0f;
    case 3: return 1.0f;
    case 4: return 0.5f;
    }

    return 0.0f;
}

void CustomAnimationCreateTabPage::setDuration( double fDuration )
{
    sal_uInt16 nPos = 0;
    if( fDuration < 2.0f )
    {
        if( fDuration < 1.0f )
        {
            nPos = 4;
        }
        else
        {
            nPos = 3;
        }
    }
    else if( fDuration < 5.0f )
    {
        if( fDuration < 3.0f )
        {
            nPos = 2;
        }
        else
        {
            nPos = 1;
        }
    }

    mpCBSpeed->SelectEntryPos( nPos );
}

bool CustomAnimationCreateTabPage::getIsPreview() const
{
    return mpCBXPReview->IsChecked() ? true : false;
}

void CustomAnimationCreateTabPage::setIsPreview( bool bIsPreview )
{
    mpCBXPReview->Check( bIsPreview ? sal_True : sal_False );
}

sal_uInt16 CustomAnimationCreateTabPage::getId() const
{
    return mnId;
}

bool CustomAnimationCreateTabPage::select( const OUString& rsPresetId )
{
    sal_uInt16 nPos = mpLBEffects->GetEntryCount();
    while( nPos-- )
    {
        void* pEntryData = mpLBEffects->GetEntryData( nPos );
        if( pEntryData )
        {
            CustomAnimationPresetPtr& pPtr = *static_cast< CustomAnimationPresetPtr* >(pEntryData);
            if( pPtr.get() && pPtr->getPresetId() == rsPresetId )
            {
                mpLBEffects->SelectEntryPos( nPos );
                return true;
            }
        }
    }

    return false;
}

// --------------------------------------------------------------------

CustomAnimationCreateDialog::CustomAnimationCreateDialog( Window* pParent, CustomAnimationPane* pPane, const std::vector< ::com::sun::star::uno::Any >& rTargets, bool bHasText, const OUString& rsPresetId, double fDuration  )
:   TabDialog( pParent, "CustomAnimationCreate", "modules/simpress/ui/customanimationcreatedialog.ui" )
,   mpPane( pPane )
,   mrTargets( rTargets )
,   mfDuration( fDuration )
{
    get( mpTabControl, "tabs" );

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    mbIsPreview = pOptions->IsPreviewNewEffects();

    mnEntranceId = mpTabControl->GetPageId("entrance");
    mnEmphasisId = mpTabControl->GetPageId("emphasis");
    mnExitId = mpTabControl->GetPageId("exit");
    mnMPathId = mpTabControl->GetPageId("motion_paths");
    mnMiscId = mpTabControl->GetPageId("misc_effects");

    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();
    mpTabPages[ENTRANCE] = new CustomAnimationCreateTabPage( mpTabControl, this, mnEntranceId, rPresets.getEntrancePresets(), bHasText );
    mpTabControl->SetTabPage( mnEntranceId, mpTabPages[ENTRANCE] );
    mpTabPages[EMPHASIS] = new CustomAnimationCreateTabPage( mpTabControl, this, mnEmphasisId, rPresets.getEmphasisPresets(), bHasText );
    mpTabControl->SetTabPage( mnEmphasisId, mpTabPages[EMPHASIS] );
    mpTabPages[EXIT] = new CustomAnimationCreateTabPage( mpTabControl, this, mnExitId, rPresets.getExitPresets(), bHasText );
    mpTabControl->SetTabPage( mnExitId, mpTabPages[EXIT] );
    mpTabPages[MOTIONPATH] = new CustomAnimationCreateTabPage( mpTabControl, this, mnMPathId, rPresets.getMotionPathsPresets(), bHasText );
    mpTabControl->SetTabPage( mnMPathId, mpTabPages[MOTIONPATH] );
    mpTabPages[MISCEFFECTS] = new CustomAnimationCreateTabPage( mpTabControl, this, mnMiscId, rPresets.getMiscPresets(), bHasText );
    mpTabControl->SetTabPage( mnMiscId, mpTabPages[MISCEFFECTS] );

    getCurrentPage()->setDuration( mfDuration );
    getCurrentPage()->setIsPreview( mbIsPreview );

    mpTabControl->SetActivatePageHdl( LINK( this, CustomAnimationCreateDialog, implActivatePagekHdl ) );
    mpTabControl->SetDeactivatePageHdl( LINK( this, CustomAnimationCreateDialog, implDeactivatePagekHdl ) );

    setPosition();

    // select current preset if available
    if( !rsPresetId.isEmpty() )
    {
        for( sal_uInt16 i = ENTRANCE; i <= MOTIONPATH; i++ )
        {
            if( mpTabPages[i]->select( rsPresetId ) )
            {
                mpTabControl->SetCurPageId( mpTabPages[i]->getId() );
                break;
            }
        }
    }
}

CustomAnimationCreateDialog::~CustomAnimationCreateDialog()
{
    storePosition();

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    pOptions->SetPreviewNewEffects( getCurrentPage()->getIsPreview() );

    delete mpTabPages[ENTRANCE];
    delete mpTabPages[EMPHASIS];
    delete mpTabPages[EXIT];
    delete mpTabPages[MOTIONPATH];
    delete mpTabPages[MISCEFFECTS];

}

CustomAnimationCreateTabPage* CustomAnimationCreateDialog::getCurrentPage() const
{
    sal_Int16 curPageId = mpTabControl->GetCurPageId();

    for( sal_uInt16 i = ENTRANCE; i <= MOTIONPATH; i++ )
    {
        if( mpTabPages[i]->getId() == curPageId )
            return mpTabPages[i];
    }
    return mpTabPages[MOTIONPATH];
}

PathKind CustomAnimationCreateDialog::getCreatePathKind() const
{
    return getCurrentPage()->getCreatePathKind();
}

CustomAnimationPresetPtr CustomAnimationCreateDialog::getSelectedPreset() const
{
    return getCurrentPage()->getSelectedPreset();
}

double CustomAnimationCreateDialog::getSelectedDuration() const
{
    return getCurrentPage()->getDuration();
}

IMPL_LINK_NOARG(CustomAnimationCreateDialog, implActivatePagekHdl)
{
    getCurrentPage()->setDuration( mfDuration );
    getCurrentPage()->setIsPreview( mbIsPreview );
    return 1;
}

IMPL_LINK_NOARG(CustomAnimationCreateDialog, implDeactivatePagekHdl)
{
    mfDuration = getCurrentPage()->getDuration();
    mbIsPreview = getCurrentPage()->getIsPreview();
    return 1;
}

void CustomAnimationCreateDialog::preview( const CustomAnimationPresetPtr& pPreset ) const
{
    MainSequencePtr pSequence( new MainSequence() );

    std::vector< Any >::const_iterator aIter( mrTargets.begin() );
    const std::vector< Any >::const_iterator aEnd( mrTargets.end() );

    const double fDuration = getSelectedDuration();

    bool bFirst = true;
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pNew(
            pSequence->append( pPreset, (*aIter++), fDuration ) );

        if( bFirst )
            bFirst = false;
        else
            pNew->setNodeType( EffectNodeType::WITH_PREVIOUS );
    }

    mpPane->preview( pSequence->getRootNode() );
}

namespace
{
Window * lcl_GetTopmostParent( Window * pWindow )
{
    Window * pResult = 0;
    Window * pCurrent = pWindow ? pWindow->GetParent() : 0;
    while( pCurrent )
    {
        pResult = pCurrent;
        pCurrent = pCurrent->GetParent();
    }
    return pResult;
}
}

void CustomAnimationCreateDialog::setPosition()
{
    SvtViewOptions aDlgOpt(E_TABDIALOG, OStringToOUString(GetHelpId(), RTL_TEXTENCODING_UTF8));
    if ( aDlgOpt.Exists() )
    {
        SetWindowState( OUStringToOString(aDlgOpt.GetWindowState(),
            RTL_TEXTENCODING_ASCII_US) );
    }
    else
    {
        // default position: aligned with right edge of parent
        Window * pParent = lcl_GetTopmostParent( this );
        if( pParent )
        {
            Point aPos( GetPosPixel());
            Size  aSize( GetSizePixel());
            Size  aParentSize( pParent->GetSizePixel());

            // right center
            aPos.setX( aParentSize.getWidth() - aSize.getWidth() );
            aPos.setY( (aParentSize.getHeight() - aSize.getHeight()) / 2 );
            SetPosPixel( aPos );
        }
    }
}

void CustomAnimationCreateDialog::storePosition()
{
    // save settings (screen position and current page)
    SvtViewOptions aDlgOpt(E_TABDIALOG, OStringToOUString(GetHelpId(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetWindowState(OStringToOUString(
        GetWindowState(WINDOWSTATE_MASK_POS), RTL_TEXTENCODING_ASCII_US));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
