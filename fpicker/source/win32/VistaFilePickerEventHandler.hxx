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

#ifndef INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKEREVENTHANDLER_HXX
#define INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKEREVENTHANDLER_HXX

#include <shobjidl.h>

#include "vistatypes.h"
#include "IVistaFilePickerInternalNotify.hxx"

#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <osl/interlck.h>

namespace fpicker{
namespace win32{
namespace vista{


// types, const etcpp.


/** todo document me
 */
class VistaFilePickerEventHandler : public ::cppu::BaseMutex
                                  , public IFileDialogEvents
                                  , public IFileDialogControlEvents
{
    public:


        // ctor/dtor


        explicit VistaFilePickerEventHandler(IVistaFilePickerInternalNotify* pInternalNotify);
        virtual ~VistaFilePickerEventHandler();


        // IUnknown

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID rIID    ,
                                                         void** ppObject) override;
        virtual ULONG STDMETHODCALLTYPE AddRef() override;
        virtual ULONG STDMETHODCALLTYPE Release() override;


        // IFileDialogEvents


        STDMETHODIMP OnFileOk(IFileDialog* pDialog) override;

        STDMETHODIMP OnFolderChanging(IFileDialog* pDialog,
                                      IShellItem*  pFolder) override;

        STDMETHODIMP OnFolderChange(IFileDialog* pDialog) override;

        STDMETHODIMP OnSelectionChange(IFileDialog* pDialog) override;

        STDMETHODIMP OnShareViolation(IFileDialog*                 pDialog  ,
                                      IShellItem*                  pItem    ,
                                      FDE_SHAREVIOLATION_RESPONSE* pResponse) override;

        STDMETHODIMP OnTypeChange(IFileDialog* pDialog) override;

        STDMETHODIMP OnOverwrite(IFileDialog*            pDialog  ,
                                 IShellItem*             pItem    ,
                                 FDE_OVERWRITE_RESPONSE* pResponse) override;


        // IFileDialogControlEvents


        STDMETHODIMP OnItemSelected(IFileDialogCustomize* pCustomize,
                                    DWORD                 nIDCtl    ,
                                    DWORD                 nIDItem   ) override;

        STDMETHODIMP OnButtonClicked(IFileDialogCustomize* pCustomize,
                                     DWORD                 nIDCtl    ) override;

        STDMETHODIMP OnCheckButtonToggled(IFileDialogCustomize* pCustomize,
                                          DWORD                 nIDCtl    ,
                                          BOOL                  bChecked  ) override;

        STDMETHODIMP OnControlActivating(IFileDialogCustomize* pCustomize,
                                         DWORD                 nIDCtl    ) override;


        // XFilePickerNotifier

        /// @throws css::uno::RuntimeException
        virtual void addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener );

        /// @throws css::uno::RuntimeException
        virtual void removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener );


        // native interface


        /** start listening for file picker events on the given file open dialog COM object.
         *
         *  The broadcaster will be cached internally so deregistration will be easy.
         *  Further all needed information is capsulated within this class (e.g. the listener handler).
         *  Nobody outside must know such information.
         *
         *  Nothing will happen if an inconsistent state will be detected
         *  (means: double registration will be ignored).
         *
         *  @param  pBroadcaster
         *          reference to the dialog, where we should start listening.
         */
        void startListening( const TFileDialog& pBroadcaster );


        /** stop listening for file picker events on the internally cached dialog COM object.
         *
         *  The COM dialog provided on the startListening() call was cached internally.
         *  And now it's used to deregister this listener. Doing so the also internally cached
         *  listener handle is used. If listener was not already registered - nothing will happen.
         */
        void stopListening();

    public:

        enum EEventType
        {
            E_FILE_SELECTION_CHANGED,
            E_DIRECTORY_CHANGED,
            E_HELP_REQUESTED,
            E_CONTROL_STATE_CHANGED,
            E_DIALOG_SIZE_CHANGED
        };

    private:


        /// @todo document me
        void impl_sendEvent(  EEventType eEventType,
                            ::sal_Int16  nControlID);

    private:


        /// ref count for AddRef/Release()
        oslInterlockedCount m_nRefCount;


        /// unique handle for this listener provided by the broadcaster on registration time
        DWORD m_nListenerHandle;


        /// cached file dialog instance (there we listen for events)
        TFileDialog m_pDialog;


        IVistaFilePickerInternalNotify* m_pInternalNotify;


        /** used to inform file picker listener asynchronously.
         *  Those listener must be called asynchronously .. because
         *  every request will block the caller thread. Mostly that will be
         *  the main thread of the office. Further the global SolarMutex will
         *  be locked during this time. If we call our listener back now synchronously ..
         *  we will block on SolarMutex.acquire() forever .-))
         */
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;
};

} // namespace vista
} // namespace win32
} // namespace fpicker

#endif // INCLUDED_FPICKER_SOURCE_WIN32_FILEPICKER_VISTAFILEPICKEREVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
