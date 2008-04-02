/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CustomAnimationCreateDialog.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:43:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_UTIL_XCOLLATOR_HPP_
#include <com/sun/star/i18n/XCollator.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif

#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif

#include <svx/svdetc.hxx>
#include <svx/svdstr.hrc>

#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif

#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif
#ifndef _SD_CUSTOMANIMATIONCREATEDIALOG_HXX
#include "CustomAnimationCreateDialog.hxx"
#endif
#ifndef _SD_CUSTOMANIMATIONCREATEDIALOG_HRC
#include "CustomAnimationCreateDialog.hrc"
#endif
#ifndef _SD_CUSTOMANIMATION_HRC
#include "CustomAnimation.hrc"
#endif
#ifndef _SD_CUSTOMANIMATIONPANE_HXX
#include "CustomAnimationPane.hxx"
#endif
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#ifndef _SDDLL_HXX
#include "sddll.hxx"
#endif

#include "helpids.h"

using namespace ::com::sun::star;

using ::rtl::OUString;
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

extern void fillDurationComboBox( ComboBox* pBox );

// --------------------------------------------------------------------

class CategoryListBox : public ListBox
{
public:
    CategoryListBox( Window* pParent, WinBits nStyle = WB_BORDER );
    CategoryListBox( Window* pParent, const ResId& rResId );
    ~CategoryListBox();

    virtual void        MouseButtonUp( const MouseEvent& rMEvt );

    USHORT          InsertCategory( const XubString& rStr, USHORT nPos = LISTBOX_APPEND );

    void            SetDoubleClickLink( const Link& rDoubleClickHdl ) { maDoubleClickHdl = rDoubleClickHdl; }

    DECL_LINK( implDoubleClickHdl, Control* );

private:
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    Link            maDoubleClickHdl;
};

CategoryListBox::CategoryListBox( Window* pParent, WinBits nStyle /* = WB_BORDER */ )
: ListBox( pParent, nStyle )
{
    EnableUserDraw( TRUE );
}

CategoryListBox::CategoryListBox( Window* pParent, const ResId& rResId )
: ListBox( pParent, rResId )
{
    EnableUserDraw( TRUE );
    SetDoubleClickHdl( LINK( this, CategoryListBox, implDoubleClickHdl ) );
}

CategoryListBox::~CategoryListBox()
{
}

USHORT CategoryListBox::InsertCategory( const XubString& rStr, USHORT nPos /* = LISTBOX_APPEND */ )
{
    USHORT n = ListBox::InsertEntry( rStr, nPos );
    if( n != LISTBOX_ENTRY_NOTFOUND )
        ListBox::SetEntryFlags( n, ListBox::GetEntryFlags(n) | LISTBOX_ENTRY_FLAG_DISABLE_SELECTION );

    return n;
}

void CategoryListBox::UserDraw( const UserDrawEvent& rUDEvt )
{
    const USHORT nItem = rUDEvt.GetItemId();

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
        DrawEntry( rUDEvt, TRUE, TRUE );
    }
}

// --------------------------------------------------------------------

IMPL_LINK( CategoryListBox, implDoubleClickHdl, Control*, EMPTYARG )
{
    CaptureMouse();
    return 0;
}

// --------------------------------------------------------------------

void CategoryListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() && (rMEvt.GetClicks() == 2) )
    {
        ReleaseMouse();
        if( maDoubleClickHdl.IsSet() )
            maDoubleClickHdl.Call( this );
    }
    else
    {
        CategoryListBox::MouseButtonUp( rMEvt );
    }
}

// --------------------------------------------------------------------

class CustomAnimationCreateTabPage : public TabPage
{
public:
    CustomAnimationCreateTabPage( Window* pParent, CustomAnimationCreateDialog* pDialogParent, int nTabId, const PresetCategoryList& rCategoryList, bool bHasText );
    ~CustomAnimationCreateTabPage();

    PathKind getCreatePathKind() const;
    CustomAnimationPresetPtr getSelectedPreset() const;
    double getDuration() const;
    void setDuration( double fDuration );

    bool getIsPreview() const;
    void setIsPreview( bool bIsPreview );

    bool select( const OUString& rsPresetId );

private:
    DECL_LINK( implSelectHdl, Control* );
    DECL_LINK( implDoubleClickHdl, Control* );

    void onSelectEffect();

    void clearEffects();

private:
    CategoryListBox*    mpLBEffects;
    FixedText*  mpFTSpeed;
    ComboBox*   mpCBSpeed;
    CheckBox*   mpCBXPReview;

    CustomAnimationCreateDialog*        mpParent;

    USHORT mnCurvePathPos;
    USHORT mnPolygonPathPos;
    USHORT mnFreeformPathPos;

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
    uno::Reference<lang::XMultiServiceFactory> xFac( ::comphelper::getProcessServiceFactory() );
    if( xFac.is() )
    {
        mxCollator.set( xFac->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.i18n.Collator" ) ), uno::UNO_QUERY );

        if( mxCollator.is() )
        {
            const lang::Locale& rLocale = Application::GetSettings().GetLocale();
            mxCollator->loadDefaultCollator(rLocale, 0);
        }
    }
}

bool ImplStlEffectCategorySortHelper::operator()( const CustomAnimationPresetPtr& p1, const CustomAnimationPresetPtr& p2 )
{
    return mxCollator.is() ? mxCollator->compareString(p1->getLabel(), p2->getLabel()) == -1 : false;
}

CustomAnimationCreateTabPage::CustomAnimationCreateTabPage( Window* pParent, CustomAnimationCreateDialog* pDialogParent, int nTabId, const PresetCategoryList& rCategoryList, bool bHasText )
: TabPage( pParent, SdResId( RID_TP_CUSTOMANIMATION_ENTRANCE ) )
, mpParent( pDialogParent )
, mnCurvePathPos( LISTBOX_ENTRY_NOTFOUND )
, mnPolygonPathPos( LISTBOX_ENTRY_NOTFOUND )
, mnFreeformPathPos( LISTBOX_ENTRY_NOTFOUND )
{
    mpLBEffects = new CategoryListBox( this, SdResId( LB_EFFECTS ) );
    mpFTSpeed = new FixedText( this, SdResId( FT_SPEED ) );
    mpCBSpeed = new ComboBox( this, SdResId( CB_SPEED ) );
    mpCBXPReview = new CheckBox( this, SdResId( CBX_PREVIEW ) );

    String sMotionPathLabel( SdResId( STR_USERPATH ) );

    FreeResource();

    USHORT nFirstEffect = LISTBOX_ENTRY_NOTFOUND;

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
                    USHORT nPos = mpLBEffects->InsertEntry( pDescriptor->getLabel() );
                    mpLBEffects->SetEntryData( nPos, static_cast<void*>( new CustomAnimationPresetPtr( pDescriptor ) ) );

                    if( nFirstEffect == LISTBOX_ENTRY_NOTFOUND )
                        nFirstEffect = nPos;
                }
            }
        }
    }

    mpLBEffects->SelectEntryPos( nFirstEffect );

    fillDurationComboBox( mpCBSpeed );

    if( nFirstEffect != LISTBOX_ENTRY_NOTFOUND )
        onSelectEffect();

    mpLBEffects->SetSelectHdl( LINK( this, CustomAnimationCreateTabPage, implSelectHdl ) );
    mpLBEffects->SetDoubleClickLink( LINK( this, CustomAnimationCreateTabPage, implDoubleClickHdl ) );
}

CustomAnimationCreateTabPage::~CustomAnimationCreateTabPage()
{
    clearEffects();

    delete mpLBEffects;
    delete mpFTSpeed;
    delete mpCBSpeed;
    delete mpCBXPReview;
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
            mpParent->EndDialog( TRUE );
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
    USHORT nPos = 0xffff;

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
    USHORT nPos = mpLBEffects->GetEntryCount();
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
        const USHORT nPos = mpLBEffects->GetSelectEntryPos();
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
    USHORT nPos = mpCBSpeed->GetSelectEntryPos();
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
    USHORT nPos = 0;
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
    mpCBXPReview->Check( bIsPreview ? TRUE : FALSE );
}

bool CustomAnimationCreateTabPage::select( const OUString& rsPresetId )
{
    USHORT nPos = mpLBEffects->GetEntryCount();
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

CustomAnimationCreateDialog::CustomAnimationCreateDialog( Window* pParent, CustomAnimationPane* pPane, const std::vector< ::com::sun::star::uno::Any >& rTargets, bool bHasText, const ::rtl::OUString& rsPresetId, double fDuration  )
:   TabDialog( pParent, SdResId( DLG_CUSTOMANIMATION_CREATE ) )
,   mpPane( pPane )
,   mrTargets( rTargets )
,   mfDuration( fDuration )
{
    mpTabControl = new TabControl( this, SdResId( 1 ) );
    mpOKButton = new OKButton(this, SdResId( 1 ) ) ;
    mpCancelButton = new CancelButton(this, SdResId( 1 ) );
    mpHelpButton = new HelpButton(this, SdResId( 1 ) );

    FreeResource();

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    mbIsPreview = pOptions->IsPreviewNewEffects();

    const CustomAnimationPresets& rPresets = CustomAnimationPresets::getCustomAnimationPresets();
    mpTabPages[ENTRANCE] = new CustomAnimationCreateTabPage( mpTabControl, this, ENTRANCE, rPresets.getEntrancePresets(), bHasText );
    mpTabPages[ENTRANCE]->SetHelpId( HID_SD_CUSTOMANIMATIONDIALOG_ENTRANCE );
    mpTabControl->SetTabPage( RID_TP_CUSTOMANIMATION_ENTRANCE, mpTabPages[ENTRANCE] );
    mpTabPages[EMPHASIS] = new CustomAnimationCreateTabPage( mpTabControl, this, EMPHASIS, rPresets.getEmphasisPresets(), bHasText );
    mpTabPages[EMPHASIS]->SetHelpId( HID_SD_CUSTOMANIMATIONDIALOG_EMPHASIS );
    mpTabControl->SetTabPage( RID_TP_CUSTOMANIMATION_EMPHASIS, mpTabPages[EMPHASIS] );
    mpTabPages[EXIT] = new CustomAnimationCreateTabPage( mpTabControl, this, EXIT, rPresets.getExitPresets(), bHasText );
    mpTabPages[EXIT]->SetHelpId( HID_SD_CUSTOMANIMATIONDIALOG_EXIT );
    mpTabControl->SetTabPage( RID_TP_CUSTOMANIMATION_EXIT, mpTabPages[EXIT] );
    mpTabPages[MOTIONPATH] = new CustomAnimationCreateTabPage( mpTabControl, this, MOTIONPATH, rPresets.getMotionPathsPresets(), bHasText );
    mpTabPages[MOTIONPATH]->SetHelpId( HID_SD_CUSTOMANIMATIONDIALOG_MOTIONPATH );
    mpTabControl->SetTabPage( RID_TP_CUSTOMANIMATION_MOTIONPATH, mpTabPages[MOTIONPATH] );

    getCurrentPage()->setDuration( mfDuration );
    getCurrentPage()->setIsPreview( mbIsPreview );

    mpTabControl->SetActivatePageHdl( LINK( this, CustomAnimationCreateDialog, implActivatePagekHdl ) );
    mpTabControl->SetDeactivatePageHdl( LINK( this, CustomAnimationCreateDialog, implDeactivatePagekHdl ) );

    setPosition();

    // select current preset if available
    if( rsPresetId.getLength() != 0 )
    {
        for( sal_uInt16 i = ENTRANCE; i <= MOTIONPATH; i++ )
        {
            if( mpTabPages[i]->select( rsPresetId ) )
            {
                mpTabControl->SetCurPageId( RID_TP_CUSTOMANIMATION_ENTRANCE + i );
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

    delete mpTabControl;
    delete mpOKButton;
    delete mpCancelButton;
    delete mpHelpButton;
}

CustomAnimationCreateTabPage* CustomAnimationCreateDialog::getCurrentPage() const
{
    switch( mpTabControl->GetCurPageId() )
    {
    case RID_TP_CUSTOMANIMATION_ENTRANCE:   return mpTabPages[ENTRANCE];
    case RID_TP_CUSTOMANIMATION_EMPHASIS:   return mpTabPages[EMPHASIS];
    case RID_TP_CUSTOMANIMATION_EXIT:       return mpTabPages[EXIT];
    //case RID_TP_CUSTOMANIMATION_MOTIONPATH:
    default:
                                            return mpTabPages[MOTIONPATH];
    }
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

IMPL_LINK( CustomAnimationCreateDialog, implActivatePagekHdl, Control*, EMPTYARG )
{
    getCurrentPage()->setDuration( mfDuration );
    getCurrentPage()->setIsPreview( mbIsPreview );
    return 1;
}

IMPL_LINK( CustomAnimationCreateDialog, implDeactivatePagekHdl, Control*, EMPTYARG )
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
    SvtViewOptions aDlgOpt(
        E_TABDIALOG, String::CreateFromInt32( DLG_CUSTOMANIMATION_CREATE ) );
    if ( aDlgOpt.Exists() )
    {
        SetWindowState( ByteString( aDlgOpt.GetWindowState().getStr(),
                                    RTL_TEXTENCODING_ASCII_US ) );
    }
    else
    {
        // default position: aligned with right edge of parent
        Window * pParent = lcl_GetTopmostParent( this );
        if( pParent )
        {
            Point aPos( GetPosPixel());
            Size  aSize( GetSizePixel());
            Point aParentPos( pParent->GetPosPixel());
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
    SvtViewOptions aDlgOpt(
        E_TABDIALOG, String::CreateFromInt32( DLG_CUSTOMANIMATION_CREATE ) );
    aDlgOpt.SetWindowState(
        OUString::createFromAscii( GetWindowState( WINDOWSTATE_MASK_POS ).GetBuffer() ) );
}

}
