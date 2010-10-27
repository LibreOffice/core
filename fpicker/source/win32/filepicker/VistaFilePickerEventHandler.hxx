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

#ifndef FPICKER_WIN32_VISTA_FILEPICKER_EVENTHANDLER_HXX
#define FPICKER_WIN32_VISTA_FILEPICKER_EVENTHANDLER_HXX

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4917 )
#endif

#include "comptr.hxx"
#include "vistatypes.h"
#include "IVistaFilePickerInternalNotify.hxx"

#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <osl/interlck.h>

#include <shobjidl.h>

//-----------------------------------------------------------------------------
// namespace
//-----------------------------------------------------------------------------

#ifdef css
    #error "Clash on using CSS as namespace define."
#else
    #define css ::com::sun::star
#endif

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
// types, const etcpp.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/** todo document me
 */
class VistaFilePickerEventHandler : public ::cppu::BaseMutex
                                  , public IFileDialogEvents
                                  , public IFileDialogControlEvents
{
    public:

        //------------------------------------------------------------------------------------
        // ctor/dtor
        //------------------------------------------------------------------------------------

                 VistaFilePickerEventHandler(IVistaFilePickerInternalNotify* pInternalNotify);
        virtual ~VistaFilePickerEventHandler();

        //------------------------------------------------------------------------------------
        // IUnknown
        //------------------------------------------------------------------------------------
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID rIID    ,
                                                         void** ppObject);
        virtual ULONG STDMETHODCALLTYPE AddRef();
        virtual ULONG STDMETHODCALLTYPE Release();

        //------------------------------------------------------------------------------------
        // IFileDialogEvents
        //------------------------------------------------------------------------------------

        STDMETHODIMP OnFileOk(IFileDialog* pDialog);

        STDMETHODIMP OnFolderChanging(IFileDialog* pDialog,
                                      IShellItem*  pFolder);

        STDMETHODIMP OnFolderChange(IFileDialog* pDialog);

        STDMETHODIMP OnSelectionChange(IFileDialog* pDialog);

        STDMETHODIMP OnShareViolation(IFileDialog*                 pDialog  ,
                                      IShellItem*                  pItem    ,
                                      FDE_SHAREVIOLATION_RESPONSE* pResponse);

        STDMETHODIMP OnTypeChange(IFileDialog* pDialog);

        STDMETHODIMP OnOverwrite(IFileDialog*            pDialog  ,
                                 IShellItem*             pItem    ,
                                 FDE_OVERWRITE_RESPONSE* pResponse);

        //------------------------------------------------------------------------------------
        // IFileDialogControlEvents
        //------------------------------------------------------------------------------------

        STDMETHODIMP OnItemSelected(IFileDialogCustomize* pCustomize,
                                    DWORD                 nIDCtl    ,
                                    DWORD                 nIDItem   );

        STDMETHODIMP OnButtonClicked(IFileDialogCustomize* pCustomize,
                                     DWORD                 nIDCtl    );

        STDMETHODIMP OnCheckButtonToggled(IFileDialogCustomize* pCustomize,
                                          DWORD                 nIDCtl    ,
                                          BOOL                  bChecked  );

        STDMETHODIMP OnControlActivating(IFileDialogCustomize* pCustomize,
                                         DWORD                 nIDCtl    );

        //------------------------------------------------------------------------------------
        // XFilePickerNotifier
        //------------------------------------------------------------------------------------

        virtual void SAL_CALL addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
            throw( css::uno::RuntimeException );

        virtual void SAL_CALL removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener )
            throw( css::uno::RuntimeException );

        //------------------------------------------------------------------------------------
        // native interface
        //------------------------------------------------------------------------------------

        //------------------------------------------------------------------------------------
        /** start listening for file picker events on the given file open dialog COM object.
         *
         *  The broadcaster will be cached internaly so deregistration will be easy.
         *  Further all needed informations are capsulated within this class (e.g. the listener handler).
         *  Nobody outside must know such informations.
         *
         *  Nothing will happen if an inconsistent state will be detected
         *  (means: double registration will be ignored).
         *
         *  @param  pBroadcaster
         *          reference to the dialog, where we should start listening.
         */
        void startListening( const TFileDialog& pBroadcaster );

        //------------------------------------------------------------------------------------
        /** stop listening for file picker events on the internaly cached dialog COM object.
         *
         *  The  COM dialog provided on the startListeneing() call was cached internaly.
         *  And now its used to deregister this listener. Doing so the also internaly cached
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

        //------------------------------------------------------------------------------------
        /// @todo document me
        void impl_sendEvent(  EEventType eEventType,
                            ::sal_Int16  nControlID);

    private:

        //------------------------------------------------------------------------------------
        /// ref count for AddRef/Release()
        oslInterlockedCount m_nRefCount;

        //------------------------------------------------------------------------------------
        /// unique handle for this listener provided by the broadcaster on registration time
        DWORD m_nListenerHandle;

        //------------------------------------------------------------------------------------
        /// cached file dialog instance (there we listen for events)
        TFileDialog m_pDialog;

        //---------------------------------------------------------------------
        IVistaFilePickerInternalNotify* m_pInternalNotify;

        //---------------------------------------------------------------------
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

#undef css

#endif  // FPICKER_WIN32_VISTA_FILEPICKER_EVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
