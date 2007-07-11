/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SalAquaPicker.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 10:59:54 $
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

#ifndef _SALAQUAFPICKER_HXX_
#define _SALAQUAFPICKER_HXX_

//_____________________________________________________________________________
//  includes of other projects
//_____________________________________________________________________________

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _CONTROLHELPER_HXX_
#include "ControlHelper.hxx"
#endif

#include <unistd.h>
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalAquaPicker
{
public:
    // constructor
    SalAquaPicker();
    virtual ~SalAquaPicker();

    OSStatus run();
    OSStatus runandwaitforresult();

    virtual void implHandleNavDialogCustomize(NavCBRecPtr callBackParms);

    virtual void implHandleNavDialogStart(NavCBRecPtr callBackParms);

    virtual void implHandleNavDialogEvent(NavCBRecPtr callBackParms);

    virtual sal_Bool implFilterHandler(AEDesc *theItem, void *info,
                                       void *callBackUD,
                                       NavFilterModes filterMode);

    virtual void implHandlePopupMenuSelect(NavMenuItemSpec* menuItem);
    virtual void implHandleNavDialogSelectEntry(NavCBRecPtr callBackParms);
    virtual sal_Bool implPreviewHandler(NavCBRecPtr callBackParms);

    inline rtl::OUString getDisplayDirectory() { return m_sDisplayDirectory; }
    inline NavDialogRef getDialogRef() { return m_pDialog; }

    inline NavEventCallbackMessage getLatestEvent() {
        return m_aLatestEvent;
    }

    inline void setLatestEvent(NavEventCallbackMessage eventType) {
        m_aLatestEvent = eventType;
    }

    inline ControlHelper* getControlHelper() const {
        return m_pControlHelper;
    }

protected:

    rtl::OUString m_sDisplayDirectory;
    NavDialogRef  m_pDialog;
    ControlHelper *m_pControlHelper;

    osl::Mutex m_rbHelperMtx;
    //::vos::OGuard guard;

    NavDialogCreationOptions m_pDialogOptions;
    NavEventUPP              m_pEventHandler;
    NavObjectFilterUPP       m_pFilterHandler;
    NavPreviewUPP            m_pPreviewHandler;
    NavReplyRecord           m_pReplyRecord;

    // The type of dialog
    enum NavigationServices_DialogType {
        NAVIGATIONSERVICES_OPEN,
        NAVIGATIONSERVICES_SAVE,
        NAVIGATIONSERVICES_DIRECTORY
    };

    NavigationServices_DialogType m_nDialogType;

    NavEventCallbackMessage m_aLatestEvent;

protected:
    void implsetTitle( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    void implsetDisplayDirectory( const rtl::OUString& rDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    rtl::OUString implgetDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    void implInitialize( );

};

#endif
