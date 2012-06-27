/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "VistaFilePickerEventHandler.hxx"

// Without IFileDialogCustomize we can't do much
#ifdef __IFileDialogCustomize_INTERFACE_DEFINED__

#include "asyncrequests.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/document/XDocumentRevisionListPersistence.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

#include <osl/file.hxx>

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
VistaFilePickerEventHandler::VistaFilePickerEventHandler(IVistaFilePickerInternalNotify* pInternalNotify)
    : m_nRefCount           (0       )
    , m_nListenerHandle     (0       )
    , m_pDialog             (        )
    , m_lListener           (m_aMutex)
    , m_pInternalNotify     (pInternalNotify)
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
STDMETHODIMP VistaFilePickerEventHandler::OnFileOk(IFileDialog* /*pDialog*/)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnFolderChanging(IFileDialog* /*pDialog*/,
                                                           IShellItem*  /*pFolder*/)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnFolderChange(IFileDialog* /*pDialog*/)
{
    impl_sendEvent(E_DIRECTORY_CHANGED, 0);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnSelectionChange(IFileDialog* /*pDialog*/)
{
    impl_sendEvent(E_FILE_SELECTION_CHANGED, 0);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnShareViolation(IFileDialog*                 /*pDialog*/  ,

                                                           IShellItem*                  /*pItem*/    ,

                                                           FDE_SHAREVIOLATION_RESPONSE* /*pResponse*/)
{
    impl_sendEvent(E_CONTROL_STATE_CHANGED, css::ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER);
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnTypeChange(IFileDialog* pDialog)
{
    UINT nFileTypeIndex;
    HRESULT hResult = pDialog->GetFileTypeIndex( &nFileTypeIndex );

    if ( hResult == S_OK )
    {
        if ( m_pInternalNotify->onFileTypeChanged( nFileTypeIndex ))
            impl_sendEvent(E_CONTROL_STATE_CHANGED, css::ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER);
    }

    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnOverwrite(IFileDialog*            /*pDialog*/  ,
                                                      IShellItem*             /*pItem*/    ,
                                                      FDE_OVERWRITE_RESPONSE* /*pResponse*/)
{
    return E_NOTIMPL;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnItemSelected(IFileDialogCustomize* /*pCustomize*/,

                                                         DWORD                   nIDCtl      ,

                                                         DWORD                 /*nIDItem*/   )
{

    impl_sendEvent(E_CONTROL_STATE_CHANGED, static_cast<sal_Int16>( nIDCtl ));
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnButtonClicked(IFileDialogCustomize* /*pCustomize*/,
                                                          DWORD                 nIDCtl    )
{

    impl_sendEvent(E_CONTROL_STATE_CHANGED, static_cast<sal_Int16>( nIDCtl));
    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnCheckButtonToggled(IFileDialogCustomize* /*pCustomize*/,
                                                               DWORD                 nIDCtl    ,
                                                               BOOL                  bChecked  )
{
    if (nIDCtl == css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION)
        m_pInternalNotify->onAutoExtensionChanged(bChecked);

    impl_sendEvent(E_CONTROL_STATE_CHANGED, static_cast<sal_Int16>( nIDCtl));

    return S_OK;
}

//-----------------------------------------------------------------------------------------
STDMETHODIMP VistaFilePickerEventHandler::OnControlActivating(IFileDialogCustomize* /*pCustomize*/,
                                                              DWORD                 nIDCtl    )
{
    impl_sendEvent(E_CONTROL_STATE_CHANGED, static_cast<sal_Int16>( nIDCtl));
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

static const ::rtl::OUString PROP_CONTROL_ID(RTL_CONSTASCII_USTRINGPARAM("control_id"));
static const ::rtl::OUString PROP_PICKER_LISTENER(RTL_CONSTASCII_USTRINGPARAM("picker_listener"));

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
            if ( nControlID )
                rRequest->setArgument(PROP_CONTROL_ID, nControlID);

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

#endif // __IFileDialogCustomize_INTERFACE_DEFINED__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
