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
    return osl_atomic_increment(&m_nRefCount);
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
