/*************************************************************************
 *
 *  $RCSfile: slideshowimpl.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:45:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XMASTERPAGETARGET_HPP_
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SYSTEMPOINTER_HPP_
#include <com/sun/star/awt/SystemPointer.hpp>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#include "sfx2/docfile.hxx"

#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif

// for child window ids
#include <sfx2/templdlg.hxx>
#include <svx/f3dchild.hxx>
#include <svx/imapdlg.hxx>
#include <svx/fontwork.hxx>
#include <svx/colrctrl.hxx>
#include <svx/bmpmask.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyprlink.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/galbrws.hxx>

#ifndef SD_NAVIGATOR_CHILD_WINDOW_HXX
#include "NavigatorChildWindow.hxx"
#endif
#ifndef SD_ANIMATION_CHILD_WINDOW_HXX
#include "AnimationChildWindow.hxx"
#endif

#ifndef _SD_SLIDESHOWIMPL_HXX_
#include <slideshowimpl.hxx>
#endif
#ifndef _SD_SLIDESHOWVIEWIMPL_HXX_
#include <slideshowviewimpl.hxx>
#endif
#ifndef _SD_PGJUMP_HXX
#include <pgjump.hxx>
#endif
#include "PaneHider.hxx"

#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "vcl/canvastools.hxx"
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "rtl/ref.hxx"

#include "canvas/elapsedtime.hxx"
#include "canvas/prioritybooster.hxx"

// TODO(Q3): This breaks encapsulation. Either export
// these strings from avmedia, or provide an XManager
// factory there
#ifdef WNT
#   define AVMEDIA_MANAGER_SERVICE_NAME "com.sun.star.media.Manager_DirectX"
#else
#   define AVMEDIA_MANAGER_SERVICE_NAME "com.sun.star.media.Manager_Java"
#endif

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;

using ::rtl::OUString;
using ::rtl::OString;
using ::comphelper::ImplementationReference;

using namespace ::com::sun::star;
using namespace ::com::sun::star;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing;


extern String getUiNameFromPageApiNameImpl( const ::rtl::OUString& rApiName );
namespace sd
{
///////////////////////////////////////////////////////////////////////

// Slots, welche im Sfx verwaltet werden und in der SlideShow disabled
// werden sollen (muss in Reihenfolge der SIDs geordnet sein)
static USHORT __READONLY_DATA pAllowed[] =
{
    SID_OPENDOC                             , //     5501   // damit interne Spruenge klappen
    SID_JUMPTOMARK                          , //     5598
//  SID_SHOWPOPUPS                          , //     5929
//    SID_GALLERY                             , //     5960
//    SID_GALLERY_FORMATS                     , //    10280
    SID_NAVIGATOR                           , //    10366
//  SID_FM_DESIGN_MODE                      , //    10629
    SID_PRESENTATION_END                    , //    27218
    SID_NAVIGATOR_PAGENAME                  , //    27287
    SID_NAVIGATOR_STATE                     , //    27288
    SID_NAVIGATOR_INIT                      , //    27289
    SID_NAVIGATOR_PEN                       , //    27291
    SID_NAVIGATOR_PAGE                      , //    27292
    SID_NAVIGATOR_OBJECT                      //    27293
};

///////////////////////////////////////////////////////////////////////
// AnimationPageList
///////////////////////////////////////////////////////////////////////

class AnimationPageList
{
public:
    AnimationPageList( SdDrawDocument* pDoc );

    void setStartPageNumber( sal_Int32 nPageNumber ) { mnStartPageNumber = nPageNumber; }
    sal_Int32 getStartPageIndex() const;

    bool isPageNumberIncluded( sal_Int32 nPageNumber ) const;

    sal_Int32 getCurrentPageNumber() const { return mnCurrentPageIndex >= 0 ? maPageNumbers[mnCurrentPageIndex] : -1; }
    sal_Int32 getCurrentPageIndex() const { return mnCurrentPageIndex; }

    sal_Int32 getPageIndexCount() const { return maPageNumbers.size(); }
    sal_Int32 getPageNumberCount() const { return mnPageCount; }

    sal_Int32 findPageIndex( sal_Int32 nPageNumber ) const;
    sal_Int32 getPageNumber( sal_Int32 nPageIndex ) const;

    void insertPageNumber( sal_Int32 nPageNumber );

    void fillSequences( Sequence< Reference< XDrawPage > >& rSlides, Sequence< Reference< animations::XAnimationNode > >& rRootNodes );

    void slideChange( sal_Int32 nNewSlideIndex );

private:
    sal_Int32 mnStartPageNumber;
    std::vector< sal_Int32 > maPageNumbers;
    SdDrawDocument* mpDoc;
    sal_Int32 mnPageCount;
    sal_Int32 mnCurrentPageIndex;
};

AnimationPageList::AnimationPageList( SdDrawDocument* pDoc )
    : mpDoc( pDoc ), mnCurrentPageIndex(0), mnStartPageNumber(-1)
{
    mnPageCount = mpDoc->GetSdPageCount( PK_STANDARD );
}

sal_Int32 AnimationPageList::getStartPageIndex() const
{
    if( mnStartPageNumber >= 0 )
    {
        sal_Int32 nIndex;
        const sal_Int32 nCount = maPageNumbers.size();

        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            if( maPageNumbers[nIndex] == mnStartPageNumber )
                return nIndex;
        }
    }

    return 0;
}

sal_Int32 AnimationPageList::getPageNumber( sal_Int32 nPageIndex ) const
{
    if( nPageIndex >= 0 && (sal_uInt32)nPageIndex < maPageNumbers.size() )
        return maPageNumbers[nPageIndex];
    else
        return -1;
}

void AnimationPageList::insertPageNumber( sal_Int32 nPageNumber )
{
    DBG_ASSERT( ( nPageNumber < mnPageCount ), "sd::AnimationPageList::insertPageNumber(), illegal index" );
    if( nPageNumber < mnPageCount )
        maPageNumbers.push_back( nPageNumber );
}

bool AnimationPageList::isPageNumberIncluded( sal_Int32 nPageNumber ) const
{
    return findPageIndex( nPageNumber ) != -1;
}

sal_Int32 AnimationPageList::findPageIndex( sal_Int32 nPageNumber ) const
{
    sal_Int32 nIndex;
    const sal_Int32 nCount = maPageNumbers.size();

    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        if( maPageNumbers[nIndex] == nPageNumber )
            return nIndex;
    }

    return -1;
}

void AnimationPageList::fillSequences( Sequence< Reference< XDrawPage > >& rSlides, Sequence< Reference< animations::XAnimationNode > >& rRootNodes )
{
    try
    {
        Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xPages( xDrawPages->getDrawPages(), UNO_QUERY_THROW );

        Reference< XDrawPage > xDrawPage;
        const int nPageCount( maPageNumbers.size() );

        rSlides.realloc( nPageCount );
        rRootNodes.realloc( nPageCount );

        for( sal_Int32 i=0; i<nPageCount; ++i )
        {
            xPages->getByIndex(maPageNumbers[i]) >>= xDrawPage;

            rSlides[i] = xDrawPage;

            Reference< animations::XAnimationNodeSupplier > xAnimNodeSupplier( xDrawPage, UNO_QUERY_THROW );
            rRootNodes[i] = xAnimNodeSupplier->getAnimationNode();
        }
    }
    catch( uno::Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::AnimationPageList::fillSequences(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void AnimationPageList::slideChange( sal_Int32 nNewSlideIndex )
{
    mnCurrentPageIndex = nNewSlideIndex;
}


///////////////////////////////////////////////////////////////////////
// class SlideshowImpl
///////////////////////////////////////////////////////////////////////

SlideshowImpl::SlideshowImpl(
    ViewShell* pViewSh,
    ::sd::View* pView,
    SdDrawDocument* pDoc )
:   SlideshowImpl_base( m_aMutex ),
    mxModel(pDoc->getUnoModel(),UNO_QUERY_THROW),
    mpView(pView),
    mpViewShell(pViewSh),
    mpDocSh(pDoc->GetDocSh()),
    mpDoc(pDoc),
    mpShowWindow(0),
    mpTimeButton(0),
    mpSaveOptions( new SvtSaveOptions ),
    mnRestorePage(0),
    mbRehearseTimings(false),
    mbAutoSaveSuppressed(false),
    meAnimationMode(ANIMATIONMODE_SHOW),
    maPresSize( -1, -1 ),
    mpOldActiveWindow(0),
    mbGridVisible(false),
    mbBordVisible(false),
    mbPageBorderVisible(false),
    mpNewAttr(0),
    mbSetOnlineSpelling(false),
    mbDisposed(false),
    mnChildMask( 0 ),
    maPresSettings( pDoc->getPresentationSettings() ),
    mbDesignMode(false),
    msOnClick( RTL_CONSTASCII_USTRINGPARAM("OnClick") ),
    msBookmark( RTL_CONSTASCII_USTRINGPARAM("Bookmark") ),
    msVerb( RTL_CONSTASCII_USTRINGPARAM("Verb") ),
    mnEntryCounter(0),
    mnLastPageNumber(-1),
    mbIsPaused(false),
    mbInputFreeze(false),
    mnEndShowEvent(0)
{
    if( mpViewShell )
        mpOldActiveWindow = mpViewShell->GetActiveWindow();

    maUpdateTimer.SetTimeoutHdl(LINK(this, SlideshowImpl, updateHdl));

    maInputFreezeTimer.SetTimeoutHdl( LINK( this, SlideshowImpl, ReadyForNextInputHdl ) );
    maInputFreezeTimer.SetTimeout( 20 );
}

SlideshowImpl::~SlideshowImpl()
{
    if( mnEndShowEvent )
        Application::RemoveUserEvent( mnEndShowEvent );

    stopShow();

    delete mpSaveOptions;
}

bool SlideshowImpl::startPreview(
        const Reference< XDrawPage >& xDrawPage,
        const Reference< animations::XAnimationNode >& xAnimationNode,
        ::Window* pParent )
{
    bool bRet = false;

    try
    {
        const Reference<lang::XServiceInfo> xServiceInfo( xDrawPage, UNO_QUERY );
        if (xServiceInfo.is()) {
            const Sequence<OUString> supportedServices(
                xServiceInfo->getSupportedServiceNames() );
            for ( sal_Int32 pos = supportedServices.getLength(); pos--; ) {
                if (supportedServices[pos].equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM(
                            "com.sun.star.drawing.MasterPage") )) {
                    DBG_ERROR("sd::SlideshowImpl::startPreview() "
                              "not allowed on master page!");
                    return false;
                }
            }
        }

        mxPreviewDrawPage = xDrawPage;
        mxPreviewAnimationNode = xAnimationNode;
        meAnimationMode = ANIMATIONMODE_PREVIEW;

        maPresSettings.mbAll = sal_False;
        maPresSettings.mbEndless = sal_False;
        maPresSettings.mbCustomShow = sal_False;
        maPresSettings.mbManual = sal_False;
        maPresSettings.mbMouseVisible = sal_False;
        maPresSettings.mbMouseAsPen = sal_False;
        maPresSettings.mbLockedPages = sal_False;
        maPresSettings.mbAlwaysOnTop = sal_False;
        maPresSettings.mbFullScreen = sal_False;
        maPresSettings.mbAnimationAllowed = sal_True;
        maPresSettings.mnPauseTimeout = 0;
        maPresSettings.mbShowPauseLogo = sal_False;
        maPresSettings.mbStartWithNavigator = sal_False;

        mpAnimationPageList.reset( new AnimationPageList( mpDoc ) );

        sal_Int32 nPageNumber;
        Reference< XPropertySet > xSet( mxPreviewDrawPage, UNO_QUERY_THROW );
        xSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) >>= nPageNumber;
        mpAnimationPageList->insertPageNumber( nPageNumber-1 );

        mpShowWindow = new ShowWindow( ((pParent == 0) && mpViewShell) ?  mpViewShell->GetParentWindow() : pParent );
        if( mpViewShell )
        {
            mpViewShell->SetActiveWindow( mpShowWindow );
            mpShowWindow->SetViewShell (mpViewShell);
            mpViewShell->ShowUIControls (false);
        }

        if( mpView )
            mpView->AddWin( mpShowWindow );

        // call resize handler
        if( pParent )
        {
            maPresSize = pParent->GetSizePixel();
        }
        else if( mpViewShell )
        {
            const Rectangle& aContentRect = mpViewShell->GetViewShellBase().getClientRectangle();
            maPresSize = aContentRect.GetSize();
            mpShowWindow->SetPosPixel( aContentRect.TopLeft() );
        }
        else
        {
            DBG_ERROR("sd::SlideshowImpl::startPreview(), I need either a parent window or a viewshell!");
        }
        resize( maPresSize );

        sal_Int32 nPropertyCount = 1;
        if( mxPreviewAnimationNode.is() )
            nPropertyCount++;

        Sequence< beans::PropertyValue > aProperties(nPropertyCount);
        aProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("AutomaticAdvancement") );
        aProperties[0].Value = uno::makeAny( (double)1.0 ); // one second timeout

        if( mxPreviewAnimationNode.is() )
        {
            aProperties[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("NoSlideTransitions") );
            aProperties[1].Value = uno::makeAny( sal_True );
        }

        Sequence< Reference< XDrawPage > > aSlides(&mxPreviewDrawPage, 1);
        Sequence< Reference< animations::XAnimationNode > > aRootNodes(&mxPreviewAnimationNode, 1);

        bRet = startShowImpl( aSlides, aRootNodes, aProperties );

        if( mpShowWindow != 0 && meAnimationMode == ANIMATIONMODE_PREVIEW )
            mpShowWindow->SetPreviewMode();

    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::SlideshowImpl::startPreview(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShow( PresentationSettings* pPresSettings )
{
    const rtl::Reference<SlideshowImpl> this_(this);

    DBG_ASSERT( !mxShow.is(), "sd::SlideshowImpl::startShow(), called twice!" );
    if( mxShow.is() )
        return true;

    bool bRet = false;

    try
    {
        if( pPresSettings )
            maPresSettings = *pPresSettings;

        // ---

        String  aPresPage( maPresSettings.maPresPage );
        SdPage* pStartPage = mpViewShell ? mpViewShell->GetActualPage() : 0;
        bool    bStartWithActualPage =  pStartPage &&
                                        ( (meAnimationMode != ANIMATIONMODE_SHOW) ||
                                           SD_MOD()->GetSdOptions( mpDoc->GetDocumentType() )->IsStartWithActualPage() );

        // sollen Zeiten gestoppt werden?
        if( mbRehearseTimings )
        {
            maPresSettings.mbEndless = sal_False;
            maPresSettings.mbManual = sal_True;
            maPresSettings.mbMouseVisible = sal_True;
            maPresSettings.mbMouseAsPen = sal_False;
            maPresSettings.mnPauseTimeout = 0;
            maPresSettings.mbShowPauseLogo = sal_False;
            maPresSettings.mbStartWithNavigator = sal_False;
        }

        if( pStartPage )
        {
            if( pStartPage->GetPageKind() == PK_NOTES )
            {
                // we are in notes page mode, so get
                // the corresponding draw page
                const USHORT nPgNum = ( pStartPage->GetPageNum() - 2 ) >> 1;
                pStartPage = mpDoc->GetSdPage( nPgNum, PK_STANDARD );
            }
        }

        if( bStartWithActualPage )
        {
            if( meAnimationMode != ANIMATIONMODE_SHOW )
            {
                if( pStartPage->GetPageKind() == PK_STANDARD )
                {
                    aPresPage = pStartPage->GetName();
                    maPresSettings.mbAll = false;
                }
                else
                {
                    bStartWithActualPage = false;
                }
            }
        }
        else
        {
            if( pStartPage->GetPageKind() != PK_STANDARD )
            {
                bStartWithActualPage = false;
            }
        }

        mpAnimationPageList.reset( new AnimationPageList( mpDoc ) );

        // build page list
        createPageList( maPresSettings.mbAll, false, aPresPage );

        if( bStartWithActualPage )
        {
            const USHORT nPageNum = ( pStartPage->GetPageNum() - 1 ) >> 1;

            if( !maPresSettings.mbAll && !maPresSettings.mbCustomShow )
            {
                // its start from dia, find out if it is located before our current page
                const long nPageCount = mpDoc->GetSdPageCount( PK_STANDARD );
                long nPage;
                for( nPage = 0; nPage < nPageCount; nPage++ )
                {
                    if( mpDoc->GetSdPage( (USHORT) nPage, PK_STANDARD )->GetName() == aPresPage )
                        break;
                }

                if( nPage < nPageNum )
                    mpAnimationPageList->setStartPageNumber( nPageNum );
            }
            else if( mpAnimationPageList->isPageNumberIncluded( nPageNum ) )
            {
                mpAnimationPageList->setStartPageNumber( nPageNum );
            }
        }

        // remember page number from where the show was started
        if( pStartPage )
            mnRestorePage = ( pStartPage->GetPageNum() - 1 ) / 2;

        // hide child windows
        hideChildWindows();

        ::Window* pParent;
        pParent = &getViewFrame()->GetWindow();

        mpShowWindow = new ShowWindow( pParent );
        if( mpViewShell )
        {
            mpViewShell->SetActiveWindow( mpShowWindow );
            mpShowWindow->SetViewShell (mpViewShell);
            mpViewShell->GetViewShellBase().ShowUIControls (false);
            mpPaneHider.reset(new PaneHider(*mpViewShell));

            mpViewShell->GetViewFrame()->SetChildWindow( SID_NAVIGATOR, maPresSettings.mbStartWithNavigator );
        }

        // these Slots are forbiden in other views for this document
        mpDocSh->SetSlotFilter( TRUE, sizeof( pAllowed ) / sizeof( USHORT ), pAllowed );
        mpDocSh->ApplySlotFilter();

        Help::DisableContextHelp();
        Help::DisableExtHelp();

    //  mpTimeButton = new PushButton( mpShowWindow, SdResId( RID_TIME_BUTTON ) );
    //  maPencil = Pointer( POINTER_PEN );
    //  mpTimeButton->Hide();

        if( maPresSettings.mbFullScreen )
        {
            // disable basic ide error handling
            maStarBASICGlobalErrorHdl = StarBASIC::GetGlobalErrorHdl();
            StarBASIC::SetGlobalErrorHdl( Link() );
        }

        // call resize handler
        maPresSize = pParent->GetSizePixel();
        if( !maPresSettings.mbFullScreen )
        {
            const Rectangle& aClientRect = mpViewShell->GetViewShellBase().getClientRectangle();
            maPresSize = aClientRect.GetSize();
            mpShowWindow->SetPosPixel( aClientRect.TopLeft() );
            resize( maPresSize );
        }

        // #i41824#
        // Note: In FullScreen Mode the OS (window manager) sends a resize to
        // the WorkWindow once it actually resized it to full size.  The
        // WorkWindow propagates the resize to the DrawViewShell which calls
        // resize() at the SlideShow (this).  Calling resize here results in a
        // temporary display of a black window in the window's default size

/*
        if ( mbRehearseTimings )
        {
            Size  aButtonSizePixel( pTimeButton->GetSizePixel() );
            Point aButtonPosPixel( aButtonSizePixel.Width() >> 1, pShowWindow->GetSizePixel().Height() - aButtonSizePixel.Height() * 5 / 2);

            pTimeButton->SetPosPixel( aButtonPosPixel );
            aTimer.SetTimeoutHdl( LINK( this,FuSlideShow, TimeButtonTimeOutHdl ) );
            pTimeButton->SetClickHdl( LINK( this, FuSlideShow, TimeButtonHdl ) );
        }
*/

        if( mpView )
            mpView->AddWin( mpShowWindow );

        SfxBindings& rBindings = getViewFrame()->GetBindings();
        rBindings.Invalidate( SID_PRESENTATION );
        rBindings.Invalidate( SID_REHEARSE_TIMINGS );

        mpShowWindow->GrabFocus();

        std::vector<beans::PropertyValue> aProperties;
        aProperties.reserve( 4 );

        aProperties.push_back(
            beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("AdvanceOnClick") ),
                -1, Any( ! (maPresSettings.mbLockedPages != sal_False) ),
                beans::PropertyState_DIRECT_VALUE ) );

        aProperties.push_back(
            beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("ImageAnimationsAllowed") ),
                -1, Any( maPresSettings.mbAnimationAllowed != sal_False ),
                beans::PropertyState_DIRECT_VALUE ) );

        aProperties.push_back(
            beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("MouseVisible") ),
                -1, Any( maPresSettings.mbMouseVisible != sal_False ),
                beans::PropertyState_DIRECT_VALUE ) );

        aProperties.push_back(
            beans::PropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("ForceManualAdvance") ),
                -1, Any( maPresSettings.mbManual != sal_False ),
                beans::PropertyState_DIRECT_VALUE ) );

        if( maPresSettings.mbMouseAsPen )
         {
            aProperties.push_back(
                beans::PropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("UserPaintColor") ),
                    -1, Any( static_cast<sal_Int32>(0x0000FF00L) ),
                    beans::PropertyState_DIRECT_VALUE ) );
        }

        if (mbRehearseTimings) {
            aProperties.push_back(
                beans::PropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("RehearseTimings") ),
                    -1, Any(true), beans::PropertyState_DIRECT_VALUE ) );
        }

        Sequence< Reference< XDrawPage > > aSlides;
        Sequence< Reference< animations::XAnimationNode > > aRootNodes;
        mpAnimationPageList->fillSequences( aSlides, aRootNodes );

        bRet = startShowImpl( aSlides, aRootNodes,
                              Sequence<beans::PropertyValue>(
                                  &aProperties[0], aProperties.size() ) );
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::SlideshowImpl::startShow(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
        stopShow();
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShowImpl(
    const Sequence< Reference< XDrawPage > >& aSlides,
    const Sequence< Reference< animations::XAnimationNode > >& aRootNodes,
    const Sequence< beans::PropertyValue >& aProperties_ )
{
    try
    {
        Reference< XSlideShow > xShow( createSlideShow() );
        mxShow = xShow;

        if( mxShow.is() )
        {
            mxView = mxView.createFromQuery( new SlideShowView( *mpShowWindow, mpDoc, meAnimationMode, this ) );

            Sequence<beans::PropertyValue> aProperties(aProperties_);
            // try add wait symbol to properties:
            const Reference<rendering::XSpriteCanvas> xSpriteCanvas(
                mxView->getCanvas() );
            if (xSpriteCanvas.is()) {
                BitmapEx waitSymbolBitmap( SdResId(BMP_WAIT_ICON) );
                const Reference<rendering::XBitmap> xBitmap(
                    vcl::unotools::xBitmapFromBitmapEx(
                        xSpriteCanvas->getDevice(), waitSymbolBitmap ) );
                if (xBitmap.is()) {
                    aProperties.realloc( aProperties.getLength() + 1 );
                    aProperties[ aProperties.getLength() - 1 ] =
                        beans::PropertyValue(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(
                                          "WaitSymbolBitmap") ),
                            -1, Any( xBitmap ),
                            beans::PropertyState_DIRECT_VALUE );
                }
            }

            mxShow->addView( mxView.getRef() );
            mxShow->addSlideShowListener( Reference< XSlideShowListener >( this ) );
            const sal_Int32 nStartIndex = mpAnimationPageList->getStartPageIndex();
            mxShow->show( aSlides, aRootNodes, ( nStartIndex > 0) ? nStartIndex : 0, aProperties );

//          update is now started in onFirstPaint()
//          update();
        }

        return true;
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::SlideshowImpl::startShowImpl(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
        stopShow();
        return false;
    }
}

void SlideshowImpl::stopShow()
{
    if( !mxShow.is() )
        return;

    maUpdateTimer.Stop();

    removeShapeEvents();

    try
    {

        mxShow->removeSlideShowListener( Reference< XSlideShowListener >(this) );

        if( mxView.is() )
            mxShow->removeView( mxView.getRef() );

        Reference< XComponent > xComponent( mxShow, UNO_QUERY );
        if( xComponent.is() )
        {
            xComponent->dispose();
            xComponent.clear();
        }
        mxShow.clear();

        if( mxView.is() )
        {
            mxView->dispose();
            mxView.reset();
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::stopShow(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );

        mxShow.clear();
        mxView.reset();
    }

/*
    if( mpShowWindow )
    {
        if(maPresSettings.mbMouseAsPen)
            mpShowWindow->SetPointer( maOldPointer );

        mpShowWindow->ShowPointer( TRUE );
    }
*/

    if( mpAnimationPageList.get() )
    {
        if( ANIMATIONMODE_SHOW == meAnimationMode )
        {
            if( mpAnimationPageList->getCurrentPageNumber() != -1 )
                mnRestorePage = mpAnimationPageList->getCurrentPageNumber();
        }

        mpAnimationPageList.reset();
    }

    // der DrawView das Praesentationfenster wegnehmen und ihr dafuer ihre alten Fenster wiedergeben
    if( mpShowWindow && mpView )
        mpView->DelWin( mpShowWindow );

    if( mpViewShell )
    {
        mpViewShell->SetActiveWindow(mpOldActiveWindow);
        mpShowWindow->SetViewShell( NULL );
    }

    if( mpView )
        mpView->InvalidateAllWin();

    if( maPresSettings.mbFullScreen )
    {
        // restore StarBASICErrorHdl
        StarBASIC::SetGlobalErrorHdl(maStarBASICGlobalErrorHdl);
        maStarBASICGlobalErrorHdl = Link();
    }
    else
    {
        if( mpShowWindow )
            mpShowWindow->Hide();
    }

    if( meAnimationMode == ANIMATIONMODE_SHOW )
    {
        mpDocSh->SetSlotFilter();
        mpDocSh->ApplySlotFilter();

        Help::EnableContextHelp();
        Help::EnableExtHelp();

        showChildWindows();
        mnChildMask = 0UL;
    }

    // aktuelle Fenster wieder einblenden
    if( mpViewShell && !mpViewShell->ISA(PresentationViewShell))
    {
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            mpViewShell->GetViewShellBase().ShowUIControls (true);
            mpPaneHider.reset();
        }
        else if( meAnimationMode == ANIMATIONMODE_PREVIEW )
        {
            mpViewShell->ShowUIControls (true);
        }
    }

    if( mpTimeButton )
    {
        mpTimeButton->Hide();
        delete mpTimeButton;
        mpTimeButton = 0;
    }

    if( mpShowWindow )
    {
        mpShowWindow->Hide();
        delete mpShowWindow;
        mpShowWindow = 0;
    }

    if ( mpViewShell )
    {
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            // switch to the previously visible page
            static_cast<DrawViewShell*>(mpViewShell)->SwitchPage( (USHORT)mnRestorePage );

            // invalidate the view shell so the presentation slot will be re-enabled
            // and the rehersing will be updated
            mpViewShell->Invalidate();

            ::sd::Window* pActWin = mpViewShell->GetActiveWindow();

            if (pActWin)
            {
                Size aVisSizePixel = pActWin->GetOutputSizePixel();
                Rectangle aVisAreaWin = pActWin->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
                mpViewShell->VisAreaChanged(aVisAreaWin);
                mpView->VisAreaChanged(pActWin);
                pActWin->GrabFocus();
            }
        }

        // restart the custom show dialog if he started us
        if( mpViewShell->IsStartShowWithDialog() )
        {
            mpViewShell->SetStartShowWithDialog( FALSE );
            getViewFrame()->GetDispatcher()->Execute( SID_CUSTOMSHOW_DLG, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }

        mpViewShell->GetViewShellBase().UpdateBorder(true);
    }
}

/** called only by the slideshow view when the first paint event occurs.
    This actually starts the slideshow. */
void SlideshowImpl::onFirstPaint()
{
    if( mpShowWindow )
    {
        mpShowWindow->SetBackground( Wallpaper( Color( COL_BLACK ) ) );
        mpShowWindow->Erase();
        mpShowWindow->SetBackground();
    }

    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    maUpdateTimer.SetTimeout( (ULONG)100 );
    maUpdateTimer.Start();
}

void SlideshowImpl::paint( const Rectangle& rRect )
{
    if( mxView.is() ) try
    {
        awt::PaintEvent aEvt;
        // aEvt.UpdateRect = TODO
        mxView->paint( aEvt );
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::paint(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::slideChange()
{
    try
    {
        if( mxPlayer.is() )
        {
            mxPlayer->stop();
            mxPlayer.clear();
        }
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::SlideshowImpl::slideChange(), exception caught!" );
    }

    // if this is a show, ignore user inputs and
    // start 20ms timer to reenable inputs to fiter
    // buffered inputs during slide transition
    if( meAnimationMode == ANIMATIONMODE_SHOW )
    {
        mbInputFreeze = true;
        maInputFreezeTimer.Start();
    }

    if( mpAnimationPageList.get() )
    {
        sal_Int32 nCurrentPageIndex = mpAnimationPageList->getCurrentPageIndex();
        sal_Int32 nCurrentPageNumber = mpAnimationPageList->getCurrentPageNumber();

        if( nCurrentPageNumber != mnLastPageNumber )
        {
            removeShapeEvents();
            registerShapeEvents(nCurrentPageNumber);
        }
        mnLastPageNumber = nCurrentPageNumber;

        if( (nCurrentPageIndex == -1) && mpShowWindow )
        {
            if( meAnimationMode == ANIMATIONMODE_PREVIEW )
            {
                endPresentation();
            }
            else if( maPresSettings.mbEndless )
            {
                if( maPresSettings.mnPauseTimeout )
                {
                    Graphic* pGraphic = 0;

                    if( maPresSettings.mbShowPauseLogo )
                    {
                        // load about image from module path
                        String aBmpFileName( RTL_CONSTASCII_USTRINGPARAM("about.bmp") );
                        INetURLObject aObj( SvtPathOptions().GetModulePath(), INET_PROT_FILE );
                        aObj.insertName( aBmpFileName );
                        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
                        if ( !aStrm.GetError() )
                        {
                            Bitmap aBmp;
                            aStrm >> aBmp;
                            pGraphic = new Graphic(aBmp);
                            pGraphic->SetPrefMapMode(MAP_PIXEL);
                        }
                        else
                        {
                            //if no image is located in the module path
                            //use default logo from iso resource:

                            String aMgrName( RTL_CONSTASCII_USTRINGPARAM( "iso" ) );
                            aMgrName += String::CreateFromInt32(SUPD);
                            ResMgr* pResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));
                            DBG_ASSERT(pResMgr,"No ResMgr found");
                            if(pResMgr)
                            {
                                pGraphic = new Graphic( Bitmap( ResId( RID_DEFAULT_ABOUT_BMP_LOGO, pResMgr ) ) );
                                pGraphic->SetPrefMapMode(MAP_PIXEL);
                                delete pResMgr;
                            }
                        }
                    }
                    mpShowWindow->SetPauseMode( 0, maPresSettings.mnPauseTimeout, pGraphic );
                    delete pGraphic;
                }
                else
                {
                    jumpToPageIndex( 0 );
                }
            }
            else
            {
                mpShowWindow->SetEndMode();
            }
        }
    }
}

void SlideshowImpl::removeShapeEvents()
{
    try
    {
        Reference< XShapeEventListener > xListener( this );

        WrappedShapeEventImplMap::iterator aIter;
        const WrappedShapeEventImplMap::iterator aEnd( maShapeEventMap.end() );

        for( aIter = maShapeEventMap.begin(); aIter != aEnd; aIter++ )
        {
            mxShow->removeShapeEventListener( xListener, (*aIter).first );
            mxShow->setShapeCursor( (*aIter).first, awt::SystemPointer::ARROW );
        }

        maShapeEventMap.clear();
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::SlideshowImpl::removeShapeEvents(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::registerShapeEvents(sal_Int32 nPageNumber)
{
    if( nPageNumber >= 0 ) try
    {
        Reference< XDrawPagesSupplier > xDrawPages( mxModel, UNO_QUERY_THROW );
        Reference< XIndexAccess > xPages( xDrawPages->getDrawPages(), UNO_QUERY_THROW );

        Reference< XShapes > xDrawPage;
        xPages->getByIndex(nPageNumber) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XMasterPageTarget > xMasterPageTarget( xDrawPage, UNO_QUERY );
            if( xMasterPageTarget.is() )
            {
                Reference< XShapes > xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY );
                if( xMasterPage.is() )
                    registerShapeEvents( xMasterPage );
            }
            registerShapeEvents( xDrawPage );
        }
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::SlideshowImpl::registerShapeEvents(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::registerShapeEvents( Reference< XShapes >& xShapes )
    throw( Exception )
{
    try
    {
        Reference< XShapeEventListener > xListener( this );

        const sal_Int32 nShapeCount = xShapes->getCount();
        sal_Int32 nShape;
        for( nShape = 0; nShape < nShapeCount; nShape++ )
        {
            Reference< XShape > xShape;
            xShapes->getByIndex( nShape ) >>= xShape;

            if( xShape.is() &&
                xShape->getShapeType().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GroupShape") ) )
            {
                Reference< XShapes > xSubShapes( xShape, UNO_QUERY );
                if( xSubShapes.is() )
                    registerShapeEvents( xSubShapes );
            }

            Reference< XPropertySet > xSet( xShape, UNO_QUERY );
            if( !xSet.is() )
                continue;

            Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
            if( !xSetInfo.is() || !xSetInfo->hasPropertyByName( msOnClick ) )
                continue;

            WrappedShapeEventImplPtr pEvent( new WrappedShapeEventImpl );
            xSet->getPropertyValue( msOnClick ) >>= pEvent->meClickAction;

            switch( pEvent->meClickAction )
            {
            case ClickAction_PREVPAGE:
            case ClickAction_NEXTPAGE:
            case ClickAction_FIRSTPAGE:
            case ClickAction_LASTPAGE:
            case ClickAction_STOPPRESENTATION:
                break;
            case ClickAction_BOOKMARK:
                if( xSetInfo->hasPropertyByName( msBookmark ) )
                    xSet->getPropertyValue( msBookmark ) >>= pEvent->maStrBookmark;
                if( getPageNumberForBookmark( pEvent->maStrBookmark ) == -1 )
                    continue;
                break;
            case ClickAction_DOCUMENT:
            case ClickAction_SOUND:
            case ClickAction_PROGRAM:
            case ClickAction_MACRO:
                if( xSetInfo->hasPropertyByName( msBookmark ) )
                    xSet->getPropertyValue( msBookmark ) >>= pEvent->maStrBookmark;
                break;
            case ClickAction_VERB:
                if( xSetInfo->hasPropertyByName( msVerb ) )
                    xSet->getPropertyValue( msVerb ) >>= pEvent->mnVerb;
                break;
            default:
                continue; // skip all others
            }

            maShapeEventMap[ xShape ] = pEvent;

            mxShow->addShapeEventListener( xListener, xShape );
                mxShow->setShapeCursor( xShape, awt::SystemPointer::REFHAND );
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::registerShapeEvents(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::gotoPreviousSlide()
{
    if( mxShow.is() ) try
    {
        if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_END )
        {
            const sal_Int32 nLastPageIndex = mpAnimationPageList->getPageIndexCount() - 1;
            if( nLastPageIndex > 0 )
                mpShowWindow->RestartShow( nLastPageIndex );
        }
        else if( mpAnimationPageList->getCurrentPageIndex() > 0 )
        {
            mxShow->previousSlide();
            update();
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::gotoPreviousSlide(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::gotoNextSlide()
{
    if( mxShow.is() ) try
    {
        if( mpAnimationPageList->getCurrentPageIndex() == (mpAnimationPageList->getPageIndexCount() - 1) )
        {
            slideChange( mpAnimationPageList->getCurrentPageIndex(), -1 );
        }
        else
        {
            mxShow->nextSlide();
            update();
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::gotoNextSlide(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::gotoFirstSlide()
{
    if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_END )
    {
        if( mpAnimationPageList->getPageIndexCount() )
            mpShowWindow->RestartShow( 0);
    }
    else
    {
        gotoSlideIndex( 0 );
    }
}

void SlideshowImpl::gotoLastSlide()
{
    const sal_Int32 nLastPageIndex = mpAnimationPageList->getPageIndexCount() - 1;
    if( mpAnimationPageList.get() && (nLastPageIndex > 0) )
    {
        if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_END )
        {
            mpShowWindow->RestartShow( nLastPageIndex );
        }
        else
        {
            gotoSlideIndex( nLastPageIndex );
        }
    }
}

void SlideshowImpl::endPresentation()
{
    if( !mnEndShowEvent )
        mnEndShowEvent = Application::PostUserEvent( LINK(this, SlideshowImpl, endPresentationHdl) );
}

IMPL_LINK( SlideshowImpl, endPresentationHdl, void*, EMPTYARG )
{
    mnEndShowEvent = 0;

    if( mpViewShell )
        mpViewShell->GetViewShellBase().StopPresentation();
    else
        stopShow();

    return 0;
}

void SlideshowImpl::gotoSlideIndex( sal_Int32 nPageIndex )
{
    if( (nPageIndex >= 0) && mxShow.is() ) try
    {
        mxShow->displaySlide( nPageIndex );
        update();
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::gotoSlideIndex(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::enablePen()
{
    if( mxShow.is()) try
    {
        uno::Any aValue;
        if( maPresSettings.mbMouseAsPen )
            // todo: take color from configuration
            aValue <<= (sal_Int32)0x0000FF00L;

        beans::PropertyValue aPenProp;
        aPenProp.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "UserPaintColor" ));
        aPenProp.Value = aValue;

        mxShow->setProperty( aPenProp );
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::enablePen(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

bool SlideshowImpl::pause( bool bPause )
{
    if( bPause != mbIsPaused ) try
    {
        mbIsPaused = bPause;
        if( mxShow.is() )
        {
            bool bRet = mxShow->pause(bPause);
            if( !bPause )
                update();

            return bRet;
        }
        else
        {
            return false;
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::pause(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    return false;
}

// XShapeEventListener
void SAL_CALL SlideshowImpl::click( const Reference< XShape >& xShape, sal_Int32 nSlideIndex, const ::com::sun::star::awt::MouseEvent& aOriginalEvent ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    WrappedShapeEventImplPtr pEvent = maShapeEventMap[xShape];
    if( !pEvent.get() )
        return;

    switch( pEvent->meClickAction )
    {
    case ClickAction_PREVPAGE:          gotoPreviousSlide();        break;
    case ClickAction_NEXTPAGE:          gotoNextSlide();            break;
    case ClickAction_FIRSTPAGE:         gotoFirstSlide();           break;
    case ClickAction_LASTPAGE:          gotoLastSlide();            break;
    case ClickAction_STOPPRESENTATION:  endPresentation();          break;
    case ClickAction_BOOKMARK:
    {
        sal_Int32 nPageNumber = getPageNumberForBookmark( pEvent->maStrBookmark );
        if( nPageNumber == -1 )
            break;

        if( mpAnimationPageList.get() )
        {
            sal_Int32 nPageIndex = mpAnimationPageList->findPageIndex( nPageNumber );
            if( nPageIndex == -1 )
                break;

            gotoSlideIndex( nPageIndex );
        }
    }
    break;
    case ClickAction_SOUND:
    {
        try
        {
            if( !mxManager.is() )
            {
                uno::Reference<lang::XMultiServiceFactory> xFac( ::comphelper::getProcessServiceFactory() );

                mxManager.set(
                    xFac->createInstance(
                        ::rtl::OUString::createFromAscii( AVMEDIA_MANAGER_SERVICE_NAME ) ),
                    uno::UNO_QUERY_THROW );
            }

            mxPlayer.set( mxManager->createPlayer( pEvent->maStrBookmark ), uno::UNO_QUERY_THROW );
            mxPlayer->start();
        }
        catch( uno::Exception& e )
        {
            (void)e;
            DBG_ERROR("sd::SlideshowImpl::click(), exception caught!" );
        }
    }
    break;

    case ClickAction_DOCUMENT:
    {
        OUString aBookmark( pEvent->maStrBookmark );

        sal_Int32 nPos = aBookmark.lastIndexOf( sal_Unicode('#') );
        if( nPos >= 0 )
        {
            OUString aURL( aBookmark.copy( 0, nPos+1 ) );
            OUString aName( aBookmark.copy( nPos+1 ) );
            aURL += getUiNameFromPageApiNameImpl( aName );
            aBookmark = aURL;
        }

        mpDocSh->OpenBookmark( aBookmark );
    }
    break;

    case ClickAction_PROGRAM:
    {
        INetURLObject aURL(
            ::URIHelper::SmartRel2Abs(
                INetURLObject(mpDocSh->GetMedium()->GetBaseURL()),
                pEvent->maStrBookmark, ::URIHelper::GetMaybeFileHdl(), true,
                false, INetURLObject::WAS_ENCODED,
                INetURLObject::DECODE_UNAMBIGUOUS ) );

        if( INET_PROT_FILE == aURL.GetProtocol() )
        {
            SfxStringItem aUrl( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            SfxBoolItem aBrowsing( SID_BROWSE, TRUE );

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if (pViewFrm)
                pViewFrm->GetDispatcher()->Execute( SID_OPENDOC,
                                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                            &aUrl,
                                            &aBrowsing,
                                            0L );
        }
    }
    break;

    case presentation::ClickAction_MACRO:
    {
        const String aMacro( pEvent->maStrBookmark );

        if ( SfxApplication::IsXScriptURL( aMacro ) )
        {
            Any aRet;
            Sequence< sal_Int16 > aOutArgsIndex;
            Sequence< Any > aOutArgs;
            Sequence< Any >* pInArgs = new Sequence< Any >(0);
            mpDocSh->CallXScript( aMacro, *pInArgs, aRet, aOutArgsIndex, aOutArgs);
        }
        else
        {
            // aMacro has the following syntax:
            // "Macroname.Modulname.Libname.Dokumentname" or
            // "Macroname.Modulname.Libname.Applikationsname"
            String aMacroName = aMacro.GetToken(0, sal_Unicode('.'));
            String aModulName = aMacro.GetToken(1, sal_Unicode('.'));
            String aLibName   = aMacro.GetToken(2, sal_Unicode('.'));
            String aDocName   = aMacro.GetToken(3, sal_Unicode('.'));

            // todo: is the limitation still given that only
            // Modulname+Macroname can be used here?
            String aExecMacro(aModulName);
            aExecMacro.Append( sal_Unicode('.') );
            aExecMacro.Append( aMacroName );
            mpDocSh->GetBasic()->Call(aExecMacro);
        }
    }
    break;

    case ClickAction_VERB:
    {
        // todo, better do it async?
        SdrObject* pObj = GetSdrObjectFromXShape( xShape );
        SdrOle2Obj* pOleObject = PTR_CAST(SdrOle2Obj, pObj);
        if (pOleObject && mpViewShell )
            mpViewShell->ActivateObject(pOleObject, pEvent->mnVerb);
    }
    break;
    }
}

sal_Int32 SlideshowImpl::getPageNumberForBookmark( const OUString& rStrBookmark )
{
    BOOL bIsMasterPage;
    OUString aBookmark = getUiNameFromPageApiNameImpl( rStrBookmark );
    USHORT nPgNum = mpDoc->GetPageByName( aBookmark, bIsMasterPage );
    SdrObject* pObj = NULL;

    if( nPgNum == SDRPAGE_NOTFOUND )
    {
        // Ist das Bookmark ein Objekt?
        SdrObject* pObj = mpDoc->GetObj( aBookmark );

        if( pObj )
        {
            nPgNum = pObj->GetPage()->GetPageNum();
            bIsMasterPage = pObj->GetPage()->IsMasterPage();
        }
    }

    if( (nPgNum == SDRPAGE_NOTFOUND) || bIsMasterPage || static_cast<SdPage*>(mpDoc->GetPage(nPgNum))->GetPageKind() != PK_STANDARD )
        return -1;

    return ( nPgNum - 1) >> 1;
}

void SAL_CALL SlideshowImpl::slideChange( sal_Int32 nOldSlideIndex, sal_Int32 nNewSlideIndex ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    mpAnimationPageList->slideChange( nNewSlideIndex );
    slideChange();
}

void SAL_CALL SlideshowImpl::showEnded( ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    mpAnimationPageList->slideChange(-1);
    slideChange();
}

void SAL_CALL SlideshowImpl::disposing(  const EventObject& Source ) throw (RuntimeException)
{
}

void SlideshowImpl::jumpToPageNumber( sal_Int32 nPageNumber )
{
    if( mpAnimationPageList.get() )
    {
        sal_Int32 nPageIndex = mpAnimationPageList->findPageIndex( nPageNumber );

        DBG_ASSERT( nPageIndex != -1, "sd::SlideshowImpl::jumpToPageNumber(), illegal page number!");

        if( nPageIndex != -1 )
            jumpToPageIndex( nPageIndex );
    }
}

void SlideshowImpl::jumpToPageIndex( sal_Int32 nPageIndex )
{
    if( mpAnimationPageList.get() ) try
    {
        DBG_ASSERT( (nPageIndex >= 0) && (nPageIndex < mpAnimationPageList->getPageIndexCount()), "sd::SlideshowImpl::jumpToPageIndex(), illegal page index!" );

        if( (nPageIndex >= 0) && (nPageIndex < mpAnimationPageList->getPageIndexCount()) )
        {
            mxShow->displaySlide(nPageIndex);
            update();
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::jumpToPageIndex(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

sal_Int32 SlideshowImpl::getCurrentPageNumber()
{
    return mpAnimationPageList.get() ? mpAnimationPageList->getCurrentPageNumber() : -1;
}

sal_Int32 SlideshowImpl::getCurrentPageIndex()
{
    return mpAnimationPageList.get() ? mpAnimationPageList->getCurrentPageIndex() : -1;
}

sal_Int32 SlideshowImpl::getFirstPageNumber()
{
    sal_Int32 nRet = 0;
    if( mpAnimationPageList.get() )
    {
        sal_Int32 nPageIndexCount = mpAnimationPageList->getPageIndexCount() - 1;
        if( nPageIndexCount >= 0 )
        {
            nRet = mpAnimationPageList->getPageNumber( nPageIndexCount );
            while( nPageIndexCount-- )
            {
                sal_Int32 nTemp = mpAnimationPageList->getPageNumber( nPageIndexCount );
                if( nRet > nTemp )
                    nRet = nTemp;
            }
        }
    }

    return nRet;
}

sal_Int32 SlideshowImpl::getLastPageNumber()
{
    sal_Int32 nRet = 0;
    if( mpAnimationPageList.get() )
    {
        sal_Int32 nPageIndexCount = mpAnimationPageList->getPageIndexCount() - 1;
        if( nPageIndexCount >= 0 )
        {
            nRet = mpAnimationPageList->getPageNumber( nPageIndexCount );
            while( nPageIndexCount-- )
            {
                sal_Int32 nTemp = mpAnimationPageList->getPageNumber( nPageIndexCount );
                if( nRet < nTemp )
                    nRet = nTemp;
            }
        }
    }

    return nRet;
}

bool SlideshowImpl::isEndless()
{
    return maPresSettings.mbEndless;
}

bool SlideshowImpl::isDrawingPossible()
{
    return maPresSettings.mbMouseAsPen;
}

double SlideshowImpl::update()
{
    startUpdateTimer();
    return -1;
}

void SlideshowImpl::startUpdateTimer()
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    maUpdateTimer.SetTimeout( 0 );
    maUpdateTimer.Start();
}

/** this timer is called 20ms after a new slide was displayed.
    This is used to unfreeze user input that was disabled after
    slide change to skip input that was buffered during slide
    transition preperation */
IMPL_LINK( SlideshowImpl, ReadyForNextInputHdl, Timer*, EMPTYARG )
{
    mbInputFreeze = false;
    return 0;
}

/** if I catch someone someday who calls this method by hand
    and not by using the timer, I will personaly punish this
    person seriously, even if this person is me.
*/
IMPL_LINK( SlideshowImpl, updateHdl, Timer*, EMPTYARG )
{
    // doing some nMagic
    const rtl::Reference<SlideshowImpl> this_(this);

    // prevent recursive calls
    if(mnEntryCounter)
        return 0;
    mnEntryCounter++;

    try
    {
        // TODO(Q3): Evaluate under various systems and setups,
        // whether this is really necessary. Under WinXP and Matrox
        // G550, the frame rates were much more steadier with this
        // tweak, although.

// currently no solution, because this kills sound (at least on Windows)
//         // Boost our prio, as long as we're in the render loop
//         ::canvas::tools::PriorityBooster aBooster(2);

        Reference< XSlideShow > xShow( mxShow );

        double fUpdate = -1.0;
        while(mxShow.is() && ( fUpdate < 1.0 ))
        {
             if( !xShow->update(fUpdate) )
             {
                 fUpdate = -1.0;
                 break;
             }

             // if UI input pending: quit busy loop (and setup timer
             // below)
             if( Application::AnyInput(INPUT_MOUSE|INPUT_KEYBOARD|INPUT_PAINT) )
                 break;
        }
        if( mxShow.is() && ( fUpdate >= 0.0 ) )
        {
            maUpdateTimer.SetTimeout(
                ::std::max( 1UL, static_cast<ULONG>(fUpdate * 1000.0) ) );
            maUpdateTimer.Start();
        }
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::updateHdl(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    --mnEntryCounter;

    return 0;
}

bool SlideshowImpl::keyInput(const KeyEvent& rKEvt)
{
    if( !mxShow.is() || mbInputFreeze )
        return false;

    bool bRet = true;

    try
    {
        const int nKeyCode = rKEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            // cancel show
            case KEY_ESCAPE:
            case KEY_BACKSPACE:
            case KEY_SUBTRACT:
                endPresentation();
                break;

            // advance show
            case KEY_SPACE:
                mxShow->nextEffect();
                update();
                break;

            case KEY_RETURN:
            {
                if( maCharBuffer.Len() )
                {
                    if( mpAnimationPageList.get() )
                    {
                        sal_Int32 nPageIndex = mpAnimationPageList->findPageIndex( maCharBuffer.ToInt32() - 1 );
                        if( nPageIndex != -1 )
                            jumpToPageIndex( nPageIndex );
                    }
                    maCharBuffer.Erase();
                }
                else
                {
                    mxShow->nextEffect();
                    update();
                }
            }
            break;

            // numeric: add to buffer
            case KEY_0:
            case KEY_1:
            case KEY_2:
            case KEY_3:
            case KEY_4:
            case KEY_5:
            case KEY_6:
            case KEY_7:
            case KEY_8:
            case KEY_9:
                maCharBuffer.Append( rKEvt.GetCharCode() );
                break;

            case KEY_PAGEDOWN:
            case KEY_RIGHT:
            case KEY_DOWN:
            case KEY_N:
                gotoNextSlide();
                break;

            case KEY_PAGEUP:
            case KEY_LEFT:
            case KEY_UP:
            case KEY_P:
                gotoPreviousSlide();
                break;

            case KEY_HOME:
                gotoFirstSlide();
                break;

            case KEY_END:
                gotoLastSlide();
                break;

            case( KEY_B ):
            case( KEY_W ):
            {
                if( mpShowWindow )
                {
                    const Color aBlankColor( (nKeyCode == KEY_B ) ? COL_BLACK : COL_WHITE );
                    if( mpShowWindow->SetBlankMode( mpAnimationPageList->getCurrentPageIndex(), aBlankColor ) )
                        pause( true );
                }
            }
            break;

            default:
                bRet = false;
            break;
        }
    }
    catch( Exception& e )
    {
        bRet = false;
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::keyInput(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    return bRet;
}

void SlideshowImpl::mouseButtonUp(const MouseEvent& rMEvt)
{
    if( rMEvt.IsRight() )
        gotoPreviousSlide();
}

Reference< XSlideShow > SlideshowImpl::createSlideShow() const
{
    Reference< XSlideShow > xShow;

    try
    {
        Reference< lang::XMultiServiceFactory > xFactory(
            ::comphelper::getProcessServiceFactory(),
            UNO_QUERY_THROW );

        Reference< XInterface > xInt( xFactory->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.SlideShow")) ) );

        xShow.set( xInt, UNO_QUERY_THROW );
    }
    catch( uno::Exception& e )
    {
        (void)e;
        DBG_ERROR(
            (OString("sd::SlideshowImpl::createSlideShow(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    return xShow;
}

void SlideshowImpl::createPageList( bool bAll, bool bStartWithActualPage, const String& rPresPage )
{
    const long nPageCount = mpDoc->GetSdPageCount( PK_STANDARD );

    if( nPageCount )
    {
        SdCustomShow*   pCustomShow;

        if( !bStartWithActualPage && mpDoc->GetCustomShowList() && maPresSettings.mbCustomShow )
            pCustomShow = (SdCustomShow*) mpDoc->GetCustomShowList()->GetCurObject();
        else
            pCustomShow = NULL;

        if( !pCustomShow || !pCustomShow->Count() )
        {
            sal_Int32 nFirstPage = 0;

            // normale Praesentation
            if( !bAll )
            {
                if( rPresPage.Len() )
                {
                    sal_Int32 nPage;
                    BOOL bTakeNextAvailable = FALSE;

                    for( nPage = 0, nFirstPage = -1; ( nPage < nPageCount ) && ( -1 == nFirstPage ); nPage++ )
                    {
                        SdPage* pTestPage = mpDoc->GetSdPage( (USHORT)nPage, PK_STANDARD );

                        if( pTestPage->GetName() == rPresPage )
                        {
                            if( pTestPage->IsExcluded() )
                                bTakeNextAvailable = TRUE;
                            else
                                nFirstPage = nPage;
                        }
                        else if( bTakeNextAvailable && !pTestPage->IsExcluded() )
                            nFirstPage = nPage;
                    }

                    if( -1 == nFirstPage )
                        nFirstPage = 0;
                }
            }

            for( sal_Int32 i = 0; i < nPageCount; i++ )
                if( !( mpDoc->GetSdPage( (USHORT)i, PK_STANDARD ) )->IsExcluded() )
                    mpAnimationPageList->insertPageNumber( i );

            mpAnimationPageList->setStartPageNumber( nFirstPage );
        }
        else
        {
            if( meAnimationMode != ANIMATIONMODE_SHOW && rPresPage.Len() )
            {
                sal_Int32 nPage;
                for( nPage = 0; nPage < nPageCount; nPage++ )
                    if( rPresPage == mpDoc->GetSdPage( (USHORT) nPage, PK_STANDARD )->GetName() )
                        break;

                if( nPage < nPageCount )
                    mpAnimationPageList->insertPageNumber( (USHORT) nPage );
            }

            void* pCustomPage;
            sal_Int32 nPageIndex;
            for( pCustomPage = pCustomShow->First(),nPageIndex=0; pCustomPage; pCustomPage = pCustomShow->Next(), nPageIndex++ )
            {
                const USHORT nSdPage = ( ( (SdPage*) pCustomPage )->GetPageNum() - 1 ) / 2;

                if( !( mpDoc->GetSdPage( nSdPage, PK_STANDARD ) )->IsExcluded())
                    mpAnimationPageList->insertPageNumber( nSdPage );
            }
        }
    }
}

typedef USHORT (*FncGetChildWindowId)();

FncGetChildWindowId aShowChilds[] =
{
    &AnimationChildWindow::GetChildWindowId,
    &Svx3DChildWindow::GetChildWindowId,
    &SvxFontWorkChildWindow::GetChildWindowId,
    &SvxColorChildWindow::GetChildWindowId,
    &SvxSearchDialogWrapper::GetChildWindowId,
    &SvxBmpMaskChildWindow::GetChildWindowId,
    &SvxIMapDlgChildWindow::GetChildWindowId,
    &SvxHyperlinkDlgWrapper::GetChildWindowId,
    &SvxHlinkDlgWrapper::GetChildWindowId,
    &SfxTemplateDialogWrapper::GetChildWindowId,
    &GalleryChildWindow::GetChildWindowId
};

#define NAVIGATOR_CHILD_MASK        0x80000000UL

void SlideshowImpl::hideChildWindows()
{
    mnChildMask = 0UL;

    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        SfxViewFrame* pViewFrame = getViewFrame();

        if( pViewFrame->GetChildWindow( SID_NAVIGATOR ) != NULL )
            mnChildMask |= NAVIGATOR_CHILD_MASK;

        for( ULONG i = 0, nCount = sizeof( aShowChilds ) / sizeof( FncGetChildWindowId ); i < nCount; i++ )
        {
            const USHORT nId = ( *aShowChilds[ i ] )();

            if( pViewFrame->GetChildWindow( nId ) )
            {
                pViewFrame->SetChildWindow( nId, FALSE );
                mnChildMask |= 1 << i;
            }
        }
    }
}

void SlideshowImpl::showChildWindows()
{
    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        SfxViewFrame* pViewFrame = getViewFrame();
        pViewFrame->SetChildWindow( SID_NAVIGATOR, ( mnChildMask & NAVIGATOR_CHILD_MASK ) != 0 );

        for( ULONG i = 0, nCount = sizeof( aShowChilds ) / sizeof( FncGetChildWindowId ); i < nCount; i++ )
        {
            if( mnChildMask & ( 1 << i ) )
                pViewFrame->SetChildWindow( ( *aShowChilds[ i ] )(), TRUE );
        }
    }
}

SfxViewFrame* SlideshowImpl::getViewFrame() const
{
    return mpViewShell ? mpViewShell->GetViewFrame() : SfxViewFrame::Current();
}

void SlideshowImpl::resize( const Size& rSize )
{
    maPresSize = rSize;

    if( mpShowWindow && (ANIMATIONMODE_VIEW != meAnimationMode) )
    {
        mpShowWindow->SetSizePixel( maPresSize );
        mpShowWindow->Show();

        // Call ToTop() to bring the window to top if
        // a) the old size is not degenerate (then the window will be closed
        // soon) and
        // b) the animation mode is not that of a preview (on the one hand
        // this leaves the old behaviour for the slide show mode unmodified
        // and on the other hand does not move the focus from the document
        // to the (preview) window; the ToTop() seems not to be necessary at
        // least for the preview).
//        if( !aOldSize.Width() && !aOldSize.Height() )
//          mpShowWindow->ToTop();
    }

    if( mxView.is() ) try
    {
        awt::WindowEvent aEvt;
        mxView->windowResized(aEvt);
    }
    catch( Exception& e )
    {
        static_cast<void>(e);
        DBG_ERROR(
            (OString("sd::SlideshowImpl::resize(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

void SlideshowImpl::activate()
{
    if(!mxShow.is())
        return;

    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        // no autosave during show
        if( mpSaveOptions->IsAutoSave() )
        {
            mpSaveOptions->SetAutoSave( FALSE );
            mbAutoSaveSuppressed = TRUE;
        }

        if( mpShowWindow )
        {
            SfxDispatcher* pDispatcher = getViewFrame()->GetDispatcher();

/* ???
            if( mpViewShell )
            {
                mpViewShell->Invalidate(SID_OBJECT_ALIGN);
                mpViewShell->Invalidate(SID_ZOOM_TOOLBOX);
                mpViewShell->Invalidate(SID_OBJECT_CHOOSE_MODE);
                mpViewShell->Invalidate(SID_POSITION);
                mpViewShell->Invalidate(SID_DRAWTBX_TEXT);
                mpViewShell->Invalidate(SID_DRAWTBX_RECTANGLES);
                mpViewShell->Invalidate(SID_DRAWTBX_ELLIPSES);
                mpViewShell->Invalidate(SID_DRAWTBX_LINES);
                mpViewShell->Invalidate(SID_DRAWTBX_ARROWS);
                mpViewShell->Invalidate(SID_DRAWTBX_3D_OBJECTS);
                mpViewShell->Invalidate(SID_DRAWTBX_CONNECTORS);
                mpViewShell->Invalidate(SID_DRAWTBX_INSERT);
            }
*/
            hideChildWindows();

            // filter all forbiden slots
            pDispatcher->SetSlotFilter( TRUE, sizeof(pAllowed) / sizeof(USHORT), pAllowed );

            getViewFrame()->GetBindings().InvalidateAll(TRUE);
            mpShowWindow->GrabFocus();
        }
    }

    pause( false );
}

// -----------------------------------------------------------------------------

void SlideshowImpl::deactivate()
{
    if( !mxShow.is() )
        return;

    pause( true );

    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        // restore autosave
        if( mbAutoSaveSuppressed )
        {
            SfxAllItemSet   aSet( SFX_APP()->GetPool() );
            SfxBoolItem     aItem( SID_ATTR_AUTOSAVE, sal_True );

            // set options at SFX_APP() to restart autosave timer
            aSet.Put( aItem );
            SFX_APP()->SetOptions( aSet );
            mbAutoSaveSuppressed = FALSE;
        }

        if( mpShowWindow )
        {
            showChildWindows();
        }
    }
}

// -----------------------------------------------------------------------------

void SlideshowImpl::receiveRequest(SfxRequest& rReq)
{
    const SfxItemSet* pArgs      = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_NAVIGATOR_PEN:
            maPresSettings.mbMouseAsPen = !maPresSettings.mbMouseAsPen;
            enablePen();
        break;

        case SID_NAVIGATOR_PAGE:
        {
            PageJump    eJump = (PageJump)((SfxAllEnumItem&) pArgs->Get(SID_NAVIGATOR_PAGE)).GetValue();
            switch( eJump )
            {
                case PAGE_FIRST:        gotoFirstSlide(); break;
                case PAGE_LAST:         gotoLastSlide(); break;
                case PAGE_NEXT:         gotoNextSlide(); break;
                case PAGE_PREVIOUS:     gotoPreviousSlide(); break;
            }
        }
        break;

        case SID_NAVIGATOR_OBJECT:
        {
            const String aTarget( ((SfxStringItem&) pArgs->Get(SID_NAVIGATOR_OBJECT)).GetValue() );

            // is the bookmark a page?
            BOOL        bIsMasterPage;
            USHORT      nPgNum = mpDoc->GetPageByName( aTarget, bIsMasterPage );
            SdrObject*  pObj   = NULL;

            if( nPgNum == SDRPAGE_NOTFOUND )
            {
                // is the bookmark an object?
                pObj = mpDoc->GetObj( aTarget );

                if( pObj )
                    nPgNum = pObj->GetPage()->GetPageNum();
            }

            if( nPgNum != SDRPAGE_NOTFOUND )
            {
                nPgNum = ( nPgNum - 1 ) >> 1;
                jumpToPageNumber( nPgNum );
            }
        }
        break;
    }
}

SlideShowImplGuard::SlideShowImplGuard( SlideshowImpl* pImpl )
{
    mpImpl = pImpl;
    if( mpImpl )
        mpImpl->acquire();
}

SlideShowImplGuard::~SlideShowImplGuard()
{
    if( mpImpl )
        mpImpl->release();
}

} // namespace ::sd
