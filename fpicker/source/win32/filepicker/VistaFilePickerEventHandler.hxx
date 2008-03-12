/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VistaFilePickerEventHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:32:50 $
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

#ifndef FPICKER_WIN32_VISTA_FILEPICKER_EVENTHANDLER_HXX
#define FPICKER_WIN32_VISTA_FILEPICKER_EVENTHANDLER_HXX

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "comptr.hxx"
#include "vistatypes.h"

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

                 VistaFilePickerEventHandler();
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
