/*************************************************************************
 *
 *  $RCSfile: MasterPageContainer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-15 08:59:33 $
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

#include "MasterPageContainer.hxx"

#include "TemplateScanner.hxx"
#include "strings.hrc"
#include <algorithm>
#include <vector>
#include <queue>

#include <comphelper/processfactory.hxx>
#include "unomodel.hxx"
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <svtools/itemset.hxx>
#include <svtools/eitem.hxx>
#include "stlpool.hxx"
#include "unmovss.hxx"
#include "sdresid.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

class PreviewCreationRequest
{
public:
    ::sd::toolpanel::controls::MasterPageContainer::Token maToken;
    int mnWidth;
    Link maCallback;
    void* mpUserData;
    PreviewCreationRequest (
        ::sd::toolpanel::controls::MasterPageContainer::Token aToken,
        int nWidth,
        const Link& rCallback,
        void* pUserData)
        : maToken(aToken),
          mnWidth(nWidth),
          maCallback(rCallback),
          mpUserData(pUserData)
    {}
};

class MasterPageDescriptor
{
public:
    MasterPageDescriptor (
        const String& rURL,
        const String& rPageName,
        SdPage* pMasterPage,
        SdPage* pSlide,
        const Image& rPreview,
        ::sd::toolpanel::controls::MasterPageContainer::Token aToken
        )
        : msURL(rURL),
          msPageName(rPageName),
          mpMasterPage(pMasterPage),
          mpSlide(pSlide),
          maPreview(rPreview),
          maToken(aToken)
    {}

    String msURL;
    String msPageName;
    SdPage* mpMasterPage;
    SdPage* mpSlide;
    Image maPreview;
    ::sd::toolpanel::controls::MasterPageContainer::Token maToken;
};

typedef ::std::vector<MasterPageDescriptor> MasterPageContainerType;

class URLComparator { public:
    String msURL;
    URLComparator (const String& sURL) : msURL(sURL) {};
    bool operator() (const MasterPageContainerType::value_type& rDescriptor)
    { return rDescriptor.msURL.CompareTo(msURL)==0; }
};
class PageNameComparator { public:
    String msPageName;
    PageNameComparator (const String& sPageName) : msPageName(sPageName) {};
    bool operator() (const MasterPageContainerType::value_type& rDescriptor)
    { return rDescriptor.msPageName.CompareTo(msPageName)==0; }
};
class PageObjectComparator { public:
    const SdPage* mpMasterPage;
    PageObjectComparator (const SdPage* pPageObject)
        : mpMasterPage(pPageObject) {};
    bool operator() (const MasterPageContainerType::value_type& rDescriptor)
    { return rDescriptor.mpMasterPage==mpMasterPage; }
};
class AllComparator { public:
    String msURL;
    String msPageName;
    const SdPage* mpMasterPage;
    AllComparator (
        const String& sURL,
        const String& sPageName,
        const SdPage* pPageObject)
        : msURL(sURL),
          msPageName(sPageName),
          mpMasterPage(pPageObject)
    {}
    bool operator() (const MasterPageContainerType::value_type& rDescriptor)
    {
        return
            (msURL.Len()>0
                &&rDescriptor.msURL.CompareTo(msURL)==0)
            || (msPageName.Len()>0
                && rDescriptor.msPageName.CompareTo(msPageName)==0)
            || (mpMasterPage!=NULL
                && rDescriptor.mpMasterPage==mpMasterPage);
    }
};


} // end of anonymous namespace




namespace sd { namespace toolpanel { namespace controls {


class MasterPageContainer::Implementation
{
public:
    MasterPageContainerType maContainer;

    Implementation (void);
    ~Implementation (void);

    void AddPreviewCreationRequest (
        Token aToken,
         int nWidth,
        const Link& rCallback,
        void* pUserData);

    SdPage* GetPageObjectForToken (
        MasterPageContainer::Token aToken,
        bool bLoad);
    SdPage* GetSlideForToken (
        MasterPageContainer::Token aToken,
        bool bLoad);
    SdPage* CopyMasterPageToLocalDocument (SdPage* pMasterPage);
    Token PutMasterPage (
        const String& sURL,
        const String& sPageName,
        SdPage* pMasterPage,
        Image aPreview);
    Image GetPreviewForToken (
        Token aToken,
        int nWidth);
    Image GetPreviewForToken (
        Token aToken,
        int nWidth,
        const Link& rCallback,
        void* pUserData);
    void RemoveCallback (const Link& rCallback);

private:
    Timer maDelayedPreviewCreationTimer;
    /** Use a vector for the queue so that we can iterate over all
        elements and remove those that became invalid.
    */
    typedef ::std::queue<PreviewCreationRequest> RequestQueue;
    RequestQueue maRequestQueue;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel> mxModel;
    SdDrawDocument* mpDocument;
    PreviewRenderer maPreviewRenderer;

    static const int DELAYED_CREATION_TIMEOUT=250;
    DECL_LINK(DelayedPreviewCreation, Timer *);
    ::sd::DrawDocShell* LoadDocument (
        const String& sFileName,
        SfxObjectShellLock& rxDocumentShell);
    SdPage* AddMasterPage (
        SdDrawDocument* pTargetDocument,
        SdPage* pMasterPage);
    void ProvideStyles (
        SdDrawDocument* pSourceDocument,
        SdDrawDocument* pTargetDocument,
        SdPage* pPage);
    SdPage* GetSlideForMasterPage (SdPage* pMasterPage);

    /** Retrieve the preview of the document specified by the given URL.
    */
    BitmapEx LoadPreviewFromURL (const ::rtl::OUString& aURL);

    /** Fetch previews of the master pages of all relevant templates,
        styles, and layouts and put them in the container.
    */
    void FillContainer (void);

    /** This function is used by FillContainer() to add the master page of
        one of the templates.  It tries to load the thumbnail bitmap from
        the file.  If that is not present the preview remains empty save a
        short text indicating what happened.
    */
    void AddTemplate (
        const String& rsPath,
        const String& rsTitle);

    /** Also used by FillContainer() to filter out those templates among
        those provided by the ucb that can be handled by us, i.e. Impress
        templates, layouts, styles.
        @param rsFileName
            The full file name, possibly a URL, that is used to recognize
            whether a template can be handled by this class.
    */
    bool FileFilter (const String& rsFileName);
};




//===== MasterPageContainer ===================================================

MasterPageContainer* MasterPageContainer::mpInstance = NULL;
::osl::Mutex MasterPageContainer::maMutex;
int MasterPageContainer::mnReferenceCount = 0;
static const MasterPageContainer::Token NIL_TOKEN (-1);

MasterPageContainer& MasterPageContainer::Instance (void)
{
    if (mpInstance == NULL)
    {
        ::osl::MutexGuard aGuard (maMutex);
        if (mpInstance == NULL)
        {
            mpInstance = new MasterPageContainer ();
            mpInstance->LateInit();
        }
    }

    return *mpInstance;
}




void MasterPageContainer::Register (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    mnReferenceCount ++;
}




void MasterPageContainer::Unregister (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    if (mnReferenceCount > 0)
        mnReferenceCount--;
    if (mnReferenceCount == 0)
    {
        delete mpInstance;
        mpInstance = NULL;
    }
}




MasterPageContainer::MasterPageContainer (void)
    : mpImpl(new Implementation())
{
}




void MasterPageContainer::LateInit (void)
{
}




MasterPageContainer::~MasterPageContainer (void)
{
}




MasterPageContainer::Token MasterPageContainer::PutMasterPage (
    const String& sURL,
    const String& sPageName,
    SdPage* pMasterPage,
    Image aPreview)
{
    return mpImpl->PutMasterPage (
        sURL,
        sPageName,
        pMasterPage,
        aPreview);
}




int MasterPageContainer::GetTokenCount (void)
{
    return mpImpl->maContainer.size();
}




MasterPageContainer::Token MasterPageContainer::GetTokenForIndex (int nIndex)
{
    Token aResult (NIL_TOKEN);
    if (nIndex>=0 && nIndex<GetTokenCount())
        aResult = mpImpl->maContainer[nIndex].maToken;
    return aResult;
}




MasterPageContainer::Token MasterPageContainer::GetTokenForURL (
    const String& sURL)
{
    Token aResult (NIL_TOKEN);
    if (sURL.Len() > 0)
    {
        MasterPageContainerType::iterator aEntry (
            ::std::find_if (
                mpImpl->maContainer.begin(),
                mpImpl->maContainer.end(),
                URLComparator(sURL)));
        if (aEntry != mpImpl->maContainer.end())
            aResult = aEntry->maToken;
    }
    return aResult;
}




MasterPageContainer::Token MasterPageContainer::GetTokenForPageName (
    const String& sPageName)
{
    Token aResult (NIL_TOKEN);
    if (sPageName.Len() > 0)
    {
        MasterPageContainerType::iterator aEntry (
            ::std::find_if (
                mpImpl->maContainer.begin(),
                mpImpl->maContainer.end(),
                PageNameComparator(sPageName)));
        if (aEntry != mpImpl->maContainer.end())
            return aEntry->maToken;
    }
    return aResult;
}




MasterPageContainer::Token MasterPageContainer::GetTokenForPageObject (
    const SdPage* pPage)
{
    Token aResult (NIL_TOKEN);
    if (pPage != NULL)
    {
        MasterPageContainerType::iterator aEntry (
            ::std::find_if (
                mpImpl->maContainer.begin(),
                mpImpl->maContainer.end(),
                PageObjectComparator(pPage)));
        if (aEntry != mpImpl->maContainer.end())
            return aEntry->maToken;
    }
    return aResult;
}




String MasterPageContainer::GetURLForToken (
    MasterPageContainer::Token aToken)
{
    if (aToken>=0 && (unsigned)aToken<mpImpl->maContainer.size())
        return mpImpl->maContainer[aToken].msURL;
    else
        return String();
}




String MasterPageContainer::GetPageNameForToken (
    MasterPageContainer::Token aToken)
{
    if (aToken>=0 && (unsigned)aToken<mpImpl->maContainer.size())
        return mpImpl->maContainer[aToken].msPageName;
    else
        return String();
}




SdPage* MasterPageContainer::GetPageObjectForToken (
    MasterPageContainer::Token aToken,
    bool bLoad)
{
    return mpImpl->GetPageObjectForToken (aToken, bLoad);
}




SdPage* MasterPageContainer::GetSlideForToken (
    MasterPageContainer::Token aToken,
    bool bLoad)
{
    return mpImpl->GetSlideForToken (aToken, bLoad);
}




Image MasterPageContainer::GetPreviewForToken (
    MasterPageContainer::Token aToken,
    int nWidth)
{
    return mpImpl->GetPreviewForToken (aToken, nWidth);
}




Image MasterPageContainer::GetPreviewForToken (
    MasterPageContainer::Token aToken,
    int nWidth,
    const Link& rCallback,
    void* pUserData)
{
    return mpImpl->GetPreviewForToken (
            aToken,
            nWidth,
            rCallback,
            pUserData);
}




void MasterPageContainer::RemoveCallback (const Link& rCallback)
{
    mpImpl->RemoveCallback (rCallback);
}




//==== Implementation ================================================

MasterPageContainer::Implementation::Implementation (void)
    : maContainer(),
      maDelayedPreviewCreationTimer(),
      maRequestQueue(),
      mxModel(NULL),
      mpDocument(NULL),
      maPreviewRenderer()
{
    try
    {
        // Use the API to create a new document.
        ::rtl::OUString sServiceName (
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"));
        uno::Reference<frame::XComponentLoader> xDesktop (
            ::comphelper::getProcessServiceFactory()->createInstance(
                sServiceName),
            uno::UNO_QUERY);
        if (xDesktop.is())
        {
            INetURLObject aTemplURL (
                ::rtl::OUString::createFromAscii("private:factory/simpress"));
            uno::Sequence<beans::PropertyValue> aArguments (1);
            aArguments[0].Name = ::rtl::OUString::createFromAscii("Hidden");
            aArguments[0].Value <<= sal_True;
            mxModel = uno::Reference<frame::XModel>(
                xDesktop->loadComponentFromURL(
                    aTemplURL.GetMainURL(INetURLObject::NO_DECODE),
                    ::rtl::OUString::createFromAscii("_blank"),
                    0,
                    aArguments),
                uno::UNO_QUERY);
        }

        // Use its tunnel to get a pointer to its core implementation.
        uno::Reference<lang::XUnoTunnel> xUnoTunnel (mxModel, uno::UNO_QUERY);
        if (xUnoTunnel.is())
        {
            mpDocument = reinterpret_cast<SdXImpressDocument*>(
                xUnoTunnel->getSomething(
                    SdXImpressDocument::getUnoTunnelId()))->GetDoc();
        }

        // Create one slide that is used to render previews of empty pages
        // (master pages that have not been loaded).
        uno::Reference<drawing::XDrawPagesSupplier> xSlideSupplier (
            mxModel, uno::UNO_QUERY);
        if (xSlideSupplier.is())
        {
            uno::Reference<drawing::XDrawPages> xSlides (
                xSlideSupplier->getDrawPages(), uno::UNO_QUERY);
            if (xSlides.is())
                xSlides->insertNewByIndex (0);
        }
    }
    catch (...)
    {
        mxModel = NULL;
        mpDocument = NULL;
    }

    // Set up the timer for the delayed creation of preview bitmaps.
    maDelayedPreviewCreationTimer.SetTimeout (DELAYED_CREATION_TIMEOUT);
    maDelayedPreviewCreationTimer.SetTimeoutHdl (
        LINK(this,Implementation,DelayedPreviewCreation));

    FillContainer();
}




MasterPageContainer::Implementation::~Implementation (void)
{
    maDelayedPreviewCreationTimer.Stop();
    while ( ! maRequestQueue.empty())
        maRequestQueue.pop();
}




void MasterPageContainer::Implementation::AddPreviewCreationRequest (
    ::sd::toolpanel::controls::MasterPageContainer::Token aToken,
    int nWidth,
    const Link& rCallback,
    void* pUserData)
{
    maRequestQueue.push (PreviewCreationRequest(
        aToken, nWidth, rCallback, pUserData));
    maDelayedPreviewCreationTimer.Start();
}




IMPL_LINK(MasterPageContainer::Implementation,
    DelayedPreviewCreation,
    Timer*, pTimer)
{
    while (maRequestQueue.size()>0 && ! GetpApp()->AnyInput())
    {
        PreviewCreationRequest aRequest (maRequestQueue.front());
        maRequestQueue.pop();

        if (aRequest.maToken>=0
            && (unsigned)aRequest.maToken<maContainer.size())
        {
            String sURL (maContainer[aRequest.maToken].msURL);
            BitmapEx aBitmap (LoadPreviewFromURL (sURL));
            maContainer[aRequest.maToken].maPreview
                = maPreviewRenderer.ScaleBitmap (aBitmap, aRequest.mnWidth);
            aRequest.maCallback.Call (aRequest.mpUserData);
        }
    }

    if (maRequestQueue.size() > 0)
        pTimer->Start();

    return 0;
}




MasterPageContainer::Token MasterPageContainer::Implementation::PutMasterPage (
    const String& sURL,
    const String& sPageName,
    SdPage* pMasterPage,
    Image aPreview)
{
    Token aResult (NIL_TOKEN);

    MasterPageContainerType::iterator aEntry (
        ::std::find_if (
            maContainer.begin(),
            maContainer.end(),
            AllComparator(sURL,sPageName,pMasterPage)));
    if (aEntry == maContainer.end())
    {
        aResult = maContainer.size();
        SdPage* pLocalMasterPage = CopyMasterPageToLocalDocument (pMasterPage);
        SdPage* pLocalSlide = GetSlideForMasterPage(pLocalMasterPage);
        maContainer.push_back (
            MasterPageDescriptor (
                sURL,
                sPageName,
                pLocalMasterPage,
                pLocalSlide,
                aPreview,
                aResult));
    }
    else
    {
        // Update the existing entry.
        if (aEntry->msURL.Len() == 0)
            aEntry->msURL = sURL;
        if (aEntry->msPageName.Len() == 0)
            aEntry->msPageName = sPageName;
        if (aEntry->mpMasterPage == NULL)
            aEntry->mpMasterPage = pMasterPage;
        Size aImageSize (aEntry->maPreview.GetSizePixel());
        if (aImageSize.Width()<=0 || aImageSize.Height()<=0)
            aEntry->maPreview = aPreview;
        aResult = aEntry->maToken;
    }

    return aResult;
}




SdPage* MasterPageContainer::Implementation::GetPageObjectForToken (
    MasterPageContainer::Token aToken,
    bool bLoad)
{
    SdPage* pPageObject = NULL;
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
        pPageObject = maContainer[aToken].mpMasterPage;

    if (pPageObject == NULL && bLoad)
    {
        SfxObjectShellLock xDocumentShell (NULL);
        ::sd::DrawDocShell* pDocumentShell = NULL;
        try
        {
            pDocumentShell = LoadDocument (
                maContainer[aToken].msURL,
                xDocumentShell);
            if (pDocumentShell != NULL)
            {
                SdDrawDocument* pDocument = pDocumentShell->GetDoc();
                SdPage* pPage = pDocument->GetMasterSdPage (0, PK_STANDARD);
                pPageObject = CopyMasterPageToLocalDocument (pPage);
                maContainer[aToken].mpMasterPage = pPageObject;
                maContainer[aToken].mpSlide
                    = GetSlideForMasterPage(pPageObject);
                maContainer[aToken].msPageName = pPageObject->GetName();
                //AF delete pDocumentShell;
            }
        }
        catch (...)
        {
            pPageObject = NULL;
            OSL_TRACE ("caught general exception");
        }
    }
    return pPageObject;
}




SdPage* MasterPageContainer::Implementation::GetSlideForToken (
    Token aToken,
    bool bLoad=true)
{
    SdPage* pSlide = NULL;
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
        pSlide = maContainer[aToken].mpSlide;

    if (pSlide==NULL && bLoad)
    {
        GetPageObjectForToken (aToken, bLoad);
        pSlide = maContainer[aToken].mpSlide;
    }

    return pSlide;
}




DrawDocShell* MasterPageContainer::Implementation::LoadDocument (
    const String& sFileName,
    SfxObjectShellLock& rxDocumentShell)
{
    SfxApplication* pSfxApp = SFX_APP();
    SfxItemSet* pSet = new SfxAllItemSet (pSfxApp->GetPool());
    pSet->Put (SfxBoolItem (SID_TEMPLATE, TRUE));
    pSet->Put (SfxBoolItem (SID_PREVIEW, TRUE));
    if (pSfxApp->LoadTemplate (rxDocumentShell, sFileName, TRUE, pSet))
    {
        rxDocumentShell = NULL;
    }
    SfxObjectShell* pShell = rxDocumentShell;
    return PTR_CAST(DrawDocShell,pShell);
}




SdPage* MasterPageContainer::Implementation::CopyMasterPageToLocalDocument (
    SdPage* pMasterPage)
{
    SdPage* pNewMasterPage = NULL;

    do
    {
        if (pMasterPage == NULL)
            break;

        if (mpDocument == NULL)
            break;
        SdDrawDocument* pSourceDocument = static_cast<SdDrawDocument*>(
            pMasterPage->GetModel());
        if (pSourceDocument == NULL)
            break;
        if (pMasterPage != static_cast<SdPage*>(
            pSourceDocument->GetMasterPage(pMasterPage->GetPageNum())))
            break;
        // Get the notes master page.
        SdPage* pNotesMasterPage = static_cast<SdPage*>(
            pSourceDocument->GetMasterPage(pMasterPage->GetPageNum()+1));
        if (pNotesMasterPage == NULL)
            break;


        // Check if a master page with the same name as that of the given
        // master page already exists.
        bool bPageExists (false);
        USHORT nMasterPageCount(mpDocument->GetMasterSdPageCount(PK_STANDARD));
        for (USHORT nIndex=0; nIndex<nMasterPageCount; nIndex++)
        {
            SdPage* pCandidate = static_cast<SdPage*>(
                mpDocument->GetMasterSdPage (nIndex, PK_STANDARD));
            if (pMasterPage!=NULL
                && pCandidate->GetName().CompareTo(pMasterPage->GetName())==0)
            {
                bPageExists = true;
                pNewMasterPage = pCandidate;
                break;
            }
        }
        if (bPageExists)
            break;

        // Create a new slide (and its notes page.)
        uno::Reference<drawing::XDrawPagesSupplier> xSlideSupplier (
            mxModel, uno::UNO_QUERY);
        if ( ! xSlideSupplier.is())
            break;
        uno::Reference<drawing::XDrawPages> xSlides (
            xSlideSupplier->getDrawPages(), uno::UNO_QUERY);
        if ( ! xSlides.is())
            break;
        xSlides->insertNewByIndex (xSlides->getCount());

        // Set a layout.
        SdPage* pSlide = mpDocument->GetSdPage(
            mpDocument->GetSdPageCount(PK_STANDARD)-1,
            PK_STANDARD);
        if (pSlide == NULL)
            break;

        // Create a copy of the master page and the associated notes
        // master page and insert them into our document.
        pNewMasterPage = AddMasterPage(mpDocument, pMasterPage);
        if (pNewMasterPage==NULL)
            break;
        SdPage* pNewNotesMasterPage
            = AddMasterPage(mpDocument, pNotesMasterPage);
        if (pNewNotesMasterPage==NULL)
            break;

        // Make the connection from the new slide to the master page
        // (and do the same for the notes page.)
        mpDocument->SetMasterPage (
            mpDocument->GetSdPageCount(PK_STANDARD)-1,
            pNewMasterPage->GetName(),
            mpDocument,
            FALSE, // Connect the new master page with the new slide but
                   // do not modify other (master) pages.
            TRUE);
    }
    while (false);

    // We are not interested in any automatisms for our modified internal
    // document.
    mpDocument->SetChanged (sal_False);

    return pNewMasterPage;
}




SdPage* MasterPageContainer::Implementation::AddMasterPage (
    SdDrawDocument* pTargetDocument,
    SdPage* pMasterPage)
{
    SdPage* pClonedMasterPage = NULL;

    if (pTargetDocument!=NULL && pMasterPage!=NULL)
    {
        try
        {
            // Duplicate the master page.
            pClonedMasterPage = static_cast<SdPage*>(pMasterPage->Clone());

            // Copy the necessary styles.
            SdDrawDocument* pSourceDocument
                = static_cast<SdDrawDocument*>(pMasterPage->GetModel());
            ProvideStyles (pSourceDocument,
                pTargetDocument, pClonedMasterPage);

            // Now that the styles are available we can insert the cloned
            // master page.
            pTargetDocument->InsertMasterPage (pClonedMasterPage);
        }
        catch (Exception& rException)
        {
            pClonedMasterPage = NULL;
            OSL_TRACE("caught exception while adding master page: %s",
                ::rtl::OUStringToOString(rException.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }
        catch (::std::exception rException)
        {
            pClonedMasterPage = NULL;
            OSL_TRACE ("caught general exception");
        }
        catch (...)
        {
            pClonedMasterPage = NULL;
            OSL_TRACE ("caught general exception");
        }
    }

    return pClonedMasterPage;
}




void MasterPageContainer::Implementation::ProvideStyles (
    SdDrawDocument* pSourceDocument,
    SdDrawDocument* pTargetDocument,
    SdPage* pPage)
{
    // Get the layout name of the given page.
    String sLayoutName (pPage->GetLayoutName());
    sLayoutName.Erase (sLayoutName.SearchAscii (SD_LT_SEPARATOR));

    // Copy the style sheet from source to target document.
    SdStyleSheetPool* pSourceStyleSheetPool =
        static_cast<SdStyleSheetPool*>(pSourceDocument->GetStyleSheetPool());
    SdStyleSheetPool* pTargetStyleSheetPool =
        static_cast<SdStyleSheetPool*>(pTargetDocument->GetStyleSheetPool());
    List* pCreatedStyles = new List();
    pTargetStyleSheetPool->CopyLayoutSheets (
        sLayoutName,
        *pSourceStyleSheetPool,
        pCreatedStyles);

    // Add an undo action for the copied style sheets.
    if (pCreatedStyles->Count() > 0)
    {
         SfxUndoManager* pUndoManager
            = pTargetDocument->GetDocSh()->GetUndoManager();
       if (pUndoManager != NULL)
       {
           SdMoveStyleSheetsUndoAction* pMovStyles =
               new SdMoveStyleSheetsUndoAction (
                   pTargetDocument,
                   pCreatedStyles,
                   TRUE);
           pUndoManager->AddUndoAction (pMovStyles);
       }
    }
    else
    {
        delete pCreatedStyles;
    }
}




Image MasterPageContainer::Implementation::GetPreviewForToken (
    MasterPageContainer::Token aToken,
    int nWidth)
{
    Image aPreview;
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
    {
        aPreview = maContainer[aToken].maPreview;
        if (aPreview.GetSizePixel().Width() == nWidth)
        {
            // The existing preview already has the right size so use that.
        }
        else if (maContainer[aToken].mpMasterPage!=NULL)
        {
            // We have the page in memory so we can render it in the desired
            // size.
            aPreview = maPreviewRenderer.RenderPage(
                maContainer[aToken].mpMasterPage,
                nWidth,
                String::CreateFromAscii(""));
            maContainer[aToken].maPreview = aPreview;
        }
        else if (nWidth>0 && aPreview.GetSizePixel().Width()>0)
        {
            // We already have a preview so we scale that to the desired size.
            aPreview = maPreviewRenderer.ScaleBitmap (
                aPreview.GetBitmapEx(),
                nWidth);
        }
        else if (maContainer[aToken].msURL.Len() > 0)
        {
            // We have a URL.  Try to load the thumbnail from that file.
            BitmapEx aBitmap (LoadPreviewFromURL (maContainer[aToken].msURL));
            maContainer[aToken].maPreview = aBitmap;
            aPreview = maPreviewRenderer.ScaleBitmap (aBitmap, nWidth);
        }

        if (aPreview.GetSizePixel().Width()==0
            || aPreview.GetSizePixel().Height()==0)
        {
            // All else failed so create an empty preview with a text that
            // tells the user that there is no preview available.
            aPreview = maPreviewRenderer.RenderPage(
                mpDocument->GetSdPage(0, PK_STANDARD),
                nWidth,
                SdResId(STR_TASKPANEL_NOT_AVAILABLE_SUBSTITUTION));

            // Do not copy this bitmap to the container or it will not be
            // replaced when the real preview becomes available.
        }
    }

    return aPreview;
}




Image MasterPageContainer::Implementation::GetPreviewForToken (
    MasterPageContainer::Token aToken,
    int nWidth,
    const Link& rCallback,
    void* pUserData)
{
    Image aPreview;
    SdPage* pPage = NULL;
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
    {
        aPreview = maContainer[aToken].maPreview;
        pPage = maContainer[aToken].mpMasterPage;
    }

    bool bShowSubstitution = false;
    if (aPreview.GetSizePixel().Width() != nWidth)
        if (pPage != NULL)
        {
            // The page object exists so we can create the preview right now.
            aPreview = maPreviewRenderer.RenderPage(
                maContainer[aToken].mpMasterPage,
                nWidth,
                String::CreateFromAscii(""));
            maContainer[aToken].maPreview = aPreview;
            bShowSubstitution = false;
        }
        else
        {
            // The page object has to be loaded.  That takes so long that we
            // do it asynchronously.
            AddPreviewCreationRequest(
                aToken,
                nWidth,
                rCallback,
                pUserData);
            bShowSubstitution = true;
        }

    if (bShowSubstitution)
    {
        aPreview = maPreviewRenderer.RenderPage(
            mpDocument->GetSdPage(0, PK_STANDARD),
            nWidth,
            SdResId(STR_TASKPANEL_PREPARING_PREVIEW_SUBSTITUTION));
    }

    return aPreview;
}




void MasterPageContainer::Implementation::RemoveCallback (
    const Link& rCallback)
{
    maDelayedPreviewCreationTimer.Stop();

    // Remove all entries from the queue for the given callback.

    // Move all entries to the temporary queue that will not call the
    // specified callback.
    RequestQueue aTemporaryQueue;
    while ( ! maRequestQueue.empty())
    {
    PreviewCreationRequest aRequest (maRequestQueue.front());
    maRequestQueue.pop();
    if (aRequest.maCallback != rCallback)
        aTemporaryQueue.push (aRequest);
    }

    // Move the entries back to the queue member to reverse the order
    // of the temprorary queue.
    while ( ! aTemporaryQueue.empty())
    {
    maRequestQueue.push (aTemporaryQueue.front());
    aTemporaryQueue.pop();
    }

    // Start the timer again when there are entries left.
    if ( ! maRequestQueue.empty())
        maDelayedPreviewCreationTimer.Start();
}




SdPage* MasterPageContainer::Implementation::GetSlideForMasterPage (
    SdPage* pMasterPage)
{
    SdPage* pCandidate = NULL;

    // Iterate over all pages and check if it references the given master
    // page.
    if (mpDocument->GetSdPageCount(PK_STANDARD) > 0)
    {
        // In most cases a new slide has just been inserted so start with
        // the last page.
        USHORT nPageIndex (mpDocument->GetSdPageCount(PK_STANDARD)-1);
        bool bFound (false);
        while ( ! bFound)
        {
            pCandidate = mpDocument->GetSdPage(
                nPageIndex,
                PK_STANDARD);
            if (pCandidate != NULL)
            {
          //                USHORT nMasterPageCount (pCandidate->GetMasterPageCount());
          //                for (USHORT i=0; i<nMasterPageCount; i++)
          //                {
          if (static_cast<SdPage*>(&pCandidate->TRG_GetMasterPage())
          == pMasterPage)
        {
          bFound = true;
          break;
        }
          //                }
            }
            if (nPageIndex == 0)
                break;
            else
                nPageIndex --;
        }

        // If no page was found that refernced the given master page reset
        // the pointer that is returned.
        if ( ! bFound)
            pCandidate = NULL;
    }

    return pCandidate;
}




BitmapEx MasterPageContainer::Implementation::LoadPreviewFromURL (
    const ::rtl::OUString& aURL)
{
    uno::Reference<io::XInputStream> xIStream;

    uno::Reference< lang::XMultiServiceFactory > xServiceManager (
        ::comphelper::getProcessServiceFactory());
    if (xServiceManager.is())
    {
        try
        {
            uno::Reference<lang::XSingleServiceFactory> xStorageFactory(
                xServiceManager->createInstance(
                    ::rtl::OUString::createFromAscii(
                        "com.sun.star.embed.StorageFactory")),
                uno::UNO_QUERY);

            if (xStorageFactory.is())
            {
                uno::Sequence<uno::Any> aArgs (2);
                aArgs[0] <<= aURL;
                aArgs[1] <<= embed::ElementModes::READ;
                uno::Reference<embed::XStorage> xDocStorage (
                    xStorageFactory->createInstanceWithArguments(aArgs),
                    uno::UNO_QUERY);

                try
                {
                    if (xDocStorage.is())
                    {
                        // !!! IMPORTANT: The current implementation has a
                        // bug - The storage name is "Thumbnail" instead of
                        // "Thumbnails", it will be fixed soon. After it is
                        // fixed the code below must be adjusted.
                        uno::Reference<embed::XStorage> xThumbnailStor (
                            xDocStorage->openStorageElement(
                                ::rtl::OUString::createFromAscii("Thumbnail"),
                                embed::ElementModes::READ));
                        if (xThumbnailStor.is())
                        {
                            uno::Reference<io::XStream> xThumbnailCopy (
                                xThumbnailStor->cloneStreamElement(
                                    ::rtl::OUString::createFromAscii(
                                        "thumbnail.png")));
                            if (xThumbnailCopy.is())
                                xIStream = xThumbnailCopy->getInputStream();
                        }
                    }
                }
                catch (uno::Exception& rException)
                {
                    OSL_TRACE (
                        "caught exception while trying to access Thumbnail/thumbnail.png of %s: %s",
                        ::rtl::OUStringToOString(aURL,
                            RTL_TEXTENCODING_UTF8).getStr(),
                        ::rtl::OUStringToOString(rException.Message,
                            RTL_TEXTENCODING_UTF8).getStr());
                }

                try
                {
                    if ( ! xIStream.is())
                    {
                        uno::Reference<embed::XStorage> xThumbnailStor (
                            xDocStorage->openStorageElement(
                                ::rtl::OUString::createFromAscii("Thumbnails"),
                                embed::ElementModes::READ));
                        if (xThumbnailStor.is())
                        {
                            uno::Reference<io::XStream> xThumbnailCopy (
                                xThumbnailStor->cloneStreamElement(
                                    ::rtl::OUString::createFromAscii(
                                        "thumbnail.png")));
                            if (xThumbnailCopy.is())
                                xIStream = xThumbnailCopy->getInputStream();
                        }
                    }
                }
                catch (uno::Exception& rException)
                {
                    OSL_TRACE (
                        "caught exception while trying to access Thumbnails/thumbnail.png of %s: %s",
                        ::rtl::OUStringToOString(aURL,
                            RTL_TEXTENCODING_UTF8).getStr(),
                        ::rtl::OUStringToOString(rException.Message,
                            RTL_TEXTENCODING_UTF8).getStr());
                }
            }
        }
        catch (uno::Exception& rException)
        {
            OSL_TRACE (
                "caught exception while trying to access tumbnail of %s: %s",
                ::rtl::OUStringToOString(aURL,
                    RTL_TEXTENCODING_UTF8).getStr(),
                ::rtl::OUStringToOString(rException.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    BitmapEx aThumbnail;
    if (xIStream.is())
    {
        ::std::auto_ptr<SvStream> pStream (
            ::utl::UcbStreamHelper::CreateStream (xIStream));
        ::vcl::PNGReader aReader (*pStream);
        aThumbnail = aReader.Read ();
    }
    return aThumbnail;
}




void MasterPageContainer::Implementation::FillContainer (void)
{
    TemplateScanner aScanner;
    aScanner.Scan ();

    // Move all relevant entries to a local list.
    ::std::vector<TemplateDir*>::const_iterator iDirectory;
    ::std::vector<TemplateDir*>& aDirectories (aScanner.GetFolderList());
    for (iDirectory=aDirectories.begin();
         iDirectory!=aDirectories.end();
         ++iDirectory)
    {
        ::std::vector<TemplateEntry*>::iterator iEntry;
        ::std::vector<TemplateEntry*>& aEntries ((*iDirectory)->m_aEntries);
        for (iEntry=aEntries.begin(); iEntry!=aEntries.end(); ++iEntry)
        {
            if (FileFilter ((*iEntry)->m_aPath))
                AddTemplate ((*iEntry)->m_aPath, (*iEntry)->m_aTitle);
            delete *iEntry;
        }
        aEntries.clear();
    }
}




void MasterPageContainer::Implementation::AddTemplate (
    const String& rsPath,
    const String& rsTitle)
{
    PutMasterPage (
        rsPath,
        rsTitle,
        NULL,
        Image());
}




bool MasterPageContainer::Implementation::FileFilter (const String& rsFileName)
{
    return rsFileName.SearchAscii ("presnt")!=STRING_NOTFOUND
        || rsFileName.SearchAscii ("layout")!=STRING_NOTFOUND;
}

} } } // end of namespace ::sd::toolpanel::controls
