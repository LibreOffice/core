/*************************************************************************
 *
 *  $RCSfile: FilePicker.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-06-28 11:13:03 $
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

#ifndef _FILEPICKER_HXX_
#define _FILEPICKER_HXX_

//_______________________________________________________________________________________________________________________
//  includes of other projects
//_______________________________________________________________________________________________________________________


#ifndef _CPPUHELPER_COMPBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPREVIEW_HPP_
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#endif

#ifndef _WINFILEOPENIMPL_HXX_
#include "WinFileOpenImpl.hxx"
#endif

#include <memory>

//----------------------------------------------------------
// Implementation class for the XFilePicker Interface
//----------------------------------------------------------

//----------------------------------------------------------
// forward declarations
//----------------------------------------------------------

//class OFPImplBase;
class CFPImplFactory;

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CFilePickerDummy
{
protected:
    osl::Mutex  m_aMutex;
    osl::Mutex  m_rbHelperMtx;
};

class CFilePicker :
    public CFilePickerDummy,
    public cppu::WeakComponentImplHelper7<
        ::com::sun::star::ui::dialogs::XFilterManager,
        ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
        ::com::sun::star::ui::dialogs::XFilePickerNotifier,
        ::com::sun::star::ui::dialogs::XFilePreview,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::lang::XEventListener,
        ::com::sun::star::lang::XServiceInfo >
{
public:

    // ctor
    CFilePicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );

    //------------------------------------------------------------------------------------
    // XFilePickerNotifier
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XExecutableDialog functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDefaultName( const ::rtl::OUString& aName )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const ::rtl::OUString& aDirectory )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getDisplayDirectory(  )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterManager functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilter( const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilter )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setCurrentFilter( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getCurrentFilter(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XFilePreview
    //------------------------------------------------

    virtual ::com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getTargetColorDepth(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight(  ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState ) throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState(  ) throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XInitialization
    //------------------------------------------------

    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XEventListener
    //------------------------------------------------

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------------------------------------------
    // FilePicker Event functions
    //------------------------------------------------------------------------------------

    void SAL_CALL fileSelectionChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
    void SAL_CALL directoryChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
    rtl::OUString SAL_CALL helpRequested( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
    void SAL_CALL controlStateChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
    void SAL_CALL dialogSizeChanged( ) const;

private:
    typedef void (SAL_CALL ::com::sun::star::ui::dialogs::XFilePickerListener::*PFNCXFPLISTENER)(const ::com::sun::star::ui::dialogs::FilePickerEvent&);
    void SAL_CALL notifyAllListener( PFNCXFPLISTENER pfncFPListener, ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;

private:
    // prevent copy and assignment
    CFilePicker( const CFilePicker& );
    CFilePicker& operator=( const CFilePicker& );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;   // to instanciate own services
    std::auto_ptr< CWinFileOpenImpl  >                                               m_pImpl;
};

#endif
