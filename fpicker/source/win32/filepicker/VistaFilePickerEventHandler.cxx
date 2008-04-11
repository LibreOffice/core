/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VistaFilePickerEventHandler.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include "VistaFilePickerEventHandler.hxx"
#include "asyncrequests.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
//#include <tools/urlobj.hxx>
//#include <unotools/ucbhelper.hxx>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

namespace css = ::com::sun::star;

namespace fpicker{
namespace win32{
namespace vista{

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
VistaFilePickerEventHandler::VistaFilePickerEventHandler()
    : m_nRefCount           (0       )
    , m_nListenerHandle     (0       )
    , m_pDialog             (        )
    , m_lListener           (m_aMutex)
{
}

//-----------------------------------------------------------------------------------------
VistaFilePickerEventHandler::~VistaFilePickerEventHandler()
{
}

//-----------------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE VistaFilePickerEventHandler::QueryInterface(REFIID rIID    ,
                                                                      void** ppObject)
{
    *ppObject=NULL;

    if ( rIID == IID_IUnknown )
        *ppObject = (IUnknown*)(IFileDialogEvents*)this;

    if ( rIID == IID_IFileDialogEvents )
        *ppObject = (IFileDialogEvents*)this;

    if ( rIID == IID_IFileDialogControlEvents )
        *ppObject = (IFileDialogControlEvents*)this;

    if ( *ppObject != NULL )
    {
        ((IUnknown*)*ppObject)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

//-----------------------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE VistaFilePickerEventHandler::AddRef()
{
    return osl_incrementInterlockedCount(&m_nRefCount);
}

//-----------------------------------------------------------------------------------------
ULONG STDMETHODCALLTYPE VistaFilePickerEventHandler::Release()
{
    ULONG nReturn = --m_nRefCount;
    if ( m_nRefCount == 0 )
        delete this;

    return nReturn;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnFileOk(IFileDialog* pDialog)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnFolderChanging(IFileDialog* pDialog,
                                                           IShellItem*  pFolder)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnFolderChange(IFileDialog* pDialog)
{
    impl_sendEvent(E_DIRECTORY_CHANGED, 0);
    return S_OK;
}

//-----------------------------------------------------------------------------
::rtl::OUString lcl_getURLFromShellItem2 (IShellItem* pItem)
{
    LPOLESTR pStr = NULL;

    SIGDN   eConversion = SIGDN_URL;
    HRESULT hr          = pItem->GetDisplayName ( eConversion, &pStr );

    if ( FAILED(hr) )
        return ::rtl::OUString();

    ::rtl::OUString sURL = ::rtl::OUString(pStr);
    CoTaskMemFree (pStr);
    return sURL;
}

//-----------------------------------------------------------------------------------------
void lcl_updateVersionListDirectly(IFileDialog* pDialog)
{
    static const ::rtl::OUString SERVICENAME_REVISIONPERSISTENCE = ::rtl::OUString::createFromAscii("com.sun.star.document.DocumentRevisionListPersistence");
    static const ::sal_Int16     CONTROL_VERSIONLIST             = css::ui::dialogs::ExtendedFilePickerElementIds::LISTBOX_VERSION;

    TFileDialog          iDialog   (pDialog);
    TFileOpenDialog      iOpen     ;
    TFileDialogCustomize iCustomize;

    iDialog.query(&iOpen     );
    iDialog.query(&iCustomize);

    // make sure version list match to the current selection always ...
    // at least an empty version list will be better then the wrong one .-)
    iCustomize->RemoveAllControlItems(CONTROL_VERSIONLIST);

    HRESULT                   hResult;
    ComPtr< IShellItemArray > iItems;
    ComPtr< IShellItem >      iItem;

    if (iOpen.is())
    {
        hResult = iOpen->GetSelectedItems(&iItems);
        if (FAILED(hResult))
            return;

        DWORD nCount;
        hResult = iItems->GetCount(&nCount);
        if ( FAILED(hResult) )
            return;

        // we can show one version list only within control
        if (nCount != 1)
            return;

        hResult = iItems->GetItemAt(0, &iItem);
    }
    else
    if (iDialog.is())
        hResult = iDialog->GetCurrentSelection(&iItem);

    if ( FAILED(hResult) )
        return;

    const ::rtl::OUString sURL = lcl_getURLFromShellItem2(iItem);
    if (sURL.getLength() < 1)
        return;
/*
    INetURLObject aURL(sURL);
    if (aURL.GetProtocol() != INET_PROT_FILE)
        return;

    ::rtl::OUString sMain = aURL.GetMainURL(INetURLObject::NO_DECODE);
    if ( ! ::utl::UCBContentHelper::IsDocument(sURL))
        return;
*/
    try
    {
        css::uno::Reference< css::embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(sURL, css::embed::ElementModes::READ);
        if ( ! xStorage.is() )
            return;

        css::uno::Reference< css::lang::XMultiServiceFactory >                 xSMGR     = ::comphelper::getProcessServiceFactory();
        css::uno::Reference< css::document::XDocumentRevisionListPersistence > xReader   (xSMGR->createInstance(SERVICENAME_REVISIONPERSISTENCE), css::uno::UNO_QUERY_THROW);
        css::uno::Sequence< css::util::RevisionTag >                           lVersions = xReader->load(xStorage);

        for (::sal_Int32 i=0; i<lVersions.getLength(); ++i)
        {
            const css::util::RevisionTag& aTag = lVersions[i];
            iCustomize->AddControlItem(CONTROL_VERSIONLIST, i, aTag.Identifier);
        }
        iCustomize->SetSelectedControlItem(CONTROL_VERSIONLIST, 0);
    }
    catch(const css::uno::Exception&)
    {}
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnSelectionChange(IFileDialog* pDialog)
{
    impl_sendEvent(E_FILE_SELECTION_CHANGED, 0);
    //lcl_updateVersionListDirectly(pDialog);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnShareViolation(IFileDialog*                 pDialog  ,
                                                           IShellItem*                  pItem    ,
                                                           FDE_SHAREVIOLATION_RESPONSE* pResponse)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnTypeChange(IFileDialog* pDialog)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnOverwrite(IFileDialog*            pDialog  ,
                                                      IShellItem*             pItem    ,
                                                      FDE_OVERWRITE_RESPONSE* pResponse)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnItemSelected(IFileDialogCustomize* pCustomize,
                                                         DWORD                 nIDCtl    ,
                                                         DWORD                 nIDItem   )
{
    impl_sendEvent(E_CONTROL_STATE_CHANGED, nIDCtl);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnButtonClicked(IFileDialogCustomize* pCustomize,
                                                          DWORD                 nIDCtl    )
{
    impl_sendEvent(E_CONTROL_STATE_CHANGED, nIDCtl);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnCheckButtonToggled(IFileDialogCustomize* pCustomize,
                                                               DWORD                 nIDCtl    ,
                                                               BOOL                  bChecked  )
{
    impl_sendEvent(E_CONTROL_STATE_CHANGED, nIDCtl);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnControlActivating(IFileDialogCustomize* pCustomize,
                                                              DWORD                 nIDCtl    )
{
    impl_sendEvent(E_CONTROL_STATE_CHANGED, nIDCtl);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePickerEventHandler::addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
    throw( css::uno::RuntimeException )
{
    m_lListener.addInterface(::getCppuType( (const css::uno::Reference< css::ui::dialogs::XFilePickerListener >*)NULL ), xListener);
}

//-----------------------------------------------------------------------------------------
void SAL_CALL VistaFilePickerEventHandler::removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
    throw( css::uno::RuntimeException )
{
    m_lListener.removeInterface(::getCppuType( (const css::uno::Reference< css::ui::dialogs::XFilePickerListener >*)NULL ), xListener);
}

//-----------------------------------------------------------------------------------------
void VistaFilePickerEventHandler::startListening( const TFileDialog& pBroadcaster )
{
    static const sal_Bool STARTUP_SUSPENDED = sal_True;
    static const sal_Bool STARTUP_WORKING   = sal_False;

    if (m_pDialog.is())
        return;

    m_pDialog = pBroadcaster;
    m_pDialog->Advise(this, &m_nListenerHandle);
}

//-----------------------------------------------------------------------------------------
void VistaFilePickerEventHandler::stopListening()
{
    if (m_pDialog.is())
    {
        m_pDialog->Unadvise(m_nListenerHandle);
        m_pDialog.release();
    }
}

static const ::rtl::OUString PROP_CONTROL_ID      = ::rtl::OUString::createFromAscii("control_id");
static const ::rtl::OUString PROP_PICKER_LISTENER = ::rtl::OUString::createFromAscii("picker_listener");

//-----------------------------------------------------------------------------------------
class AsyncPickerEvents : public RequestHandler
{
public:

    AsyncPickerEvents()
    {}

    virtual ~AsyncPickerEvents()
    {}

    virtual void before()
    {}

    virtual void doRequest(const RequestRef& rRequest)
    {
        const ::sal_Int32 nEventID   = rRequest->getRequest();
        const ::sal_Int16 nControlID = rRequest->getArgumentOrDefault(PROP_CONTROL_ID, (::sal_Int16)0);
        const css::uno::Reference< css::ui::dialogs::XFilePickerListener > xListener = rRequest->getArgumentOrDefault(PROP_PICKER_LISTENER, css::uno::Reference< css::ui::dialogs::XFilePickerListener >());

        if ( ! xListener.is())
            return;

        css::ui::dialogs::FilePickerEvent aEvent;
        aEvent.ElementId = nControlID;

        switch (nEventID)
        {
            case VistaFilePickerEventHandler::E_FILE_SELECTION_CHANGED :
                    xListener->fileSelectionChanged(aEvent);
                    break;

            case VistaFilePickerEventHandler::E_DIRECTORY_CHANGED :
                    xListener->directoryChanged(aEvent);
                    break;

            case VistaFilePickerEventHandler::E_HELP_REQUESTED :
                    xListener->helpRequested(aEvent);
                    break;

            case VistaFilePickerEventHandler::E_CONTROL_STATE_CHANGED :
                    xListener->controlStateChanged(aEvent);
                    break;

            case VistaFilePickerEventHandler::E_DIALOG_SIZE_CHANGED :
                    xListener->dialogSizeChanged();
                    break;

            // no default here. Let compiler detect changes on enum set !
        }
    }

    virtual void after()
    {}
};

//-----------------------------------------------------------------------------------------
void VistaFilePickerEventHandler::impl_sendEvent(  EEventType eEventType,
                                                 ::sal_Int16  nControlID)
{
    static AsyncRequests aNotify(RequestHandlerRef(new AsyncPickerEvents()));

    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer( ::getCppuType( ( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >*) NULL ) );
    if ( ! pContainer)
        return;

    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
    while (pIterator.hasMoreElements())
    {
        try
        {
            css::uno::Reference< css::ui::dialogs::XFilePickerListener > xListener (pIterator.next(), css::uno::UNO_QUERY);

            RequestRef rRequest(new Request());
            rRequest->setRequest (eEventType);
            rRequest->setArgument(PROP_PICKER_LISTENER, xListener);

            aNotify.triggerRequestDirectly(rRequest);
            //aNotify.triggerRequestNonBlocked(rRequest);
        }
        catch(const css::uno::RuntimeException&)
        {
            pIterator.remove();
        }
    }
}

} // namespace vista
} // namespace win32
} // namespace fpicker
