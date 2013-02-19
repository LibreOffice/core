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

#ifndef _SALAQUAFILEPICKER_HXX_
#define _SALAQUAFILEPICKER_HXX_

#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include "SalAquaPicker.hxx"

#include <memory>
#include <list>

#include <rtl/ustring.hxx>
#include "FilterHelper.hxx"
#include "AquaFilePickerDelegate.hxx"

//----------------------------------------------------------
// Implementation class for the XFilePicker Interface
//----------------------------------------------------------

//----------------------------------------------------------
// forward declarations
//----------------------------------------------------------

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

typedef ::cppu::WeakComponentImplHelper4 <
            ::com::sun::star::ui::dialogs::XFilePicker3,
            ::com::sun::star::ui::dialogs::XFilePickerControlAccess,
            ::com::sun::star::lang::XInitialization,
           ::com::sun::star::lang::XServiceInfo >  SalAquaFilePicker_Base;

class SalAquaFilePicker :
    public SalAquaPicker,
    public SalAquaFilePicker_Base
{
public:

    // constructor
    SalAquaFilePicker();

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

    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilePicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setMultiSelectionMode( sal_Bool bMode )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDefaultName( const OUString& aName )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDisplayDirectory( const OUString& aDirectory )
        throw( com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );

    virtual OUString SAL_CALL getDisplayDirectory(  )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getFiles(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterManager functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setCurrentFilter( const OUString& aTitle )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    virtual OUString SAL_CALL getCurrentFilter(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFilterGroupManager functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL enableControl( sal_Int16 nControlId, sal_Bool bEnable )
        throw(::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setLabel( sal_Int16 nControlId, const OUString& aLabel )
        throw (::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getLabel( sal_Int16 nControlId )
        throw (::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XInitialization
    //------------------------------------------------

    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XCancellable
    //------------------------------------------------

    virtual void SAL_CALL cancel( )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XEventListener
    //------------------------------------------------

    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------------------------------------------
    // FilePicker Event functions
    //------------------------------------------------------------------------------------

   void SAL_CALL fileSelectionChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
   void SAL_CALL directoryChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
   // OUString SAL_CALL helpRequested( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent ) const;
   void SAL_CALL controlStateChanged( ::com::sun::star::ui::dialogs::FilePickerEvent aEvent );
   void SAL_CALL dialogSizeChanged( );

   inline AquaFilePickerDelegate * getDelegate() {
       return m_pDelegate;
   }

   inline OUString getSaveFileName() {
       return m_sSaveFileName;
   }

private:
    // prevent copy and assignment
    SalAquaFilePicker( const SalAquaFilePicker& );
    SalAquaFilePicker& operator=( const SalAquaFilePicker& );

    virtual void ensureFilterHelper();

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >
    m_xListener;

    FilterHelper *m_pFilterHelper;

    OUString m_sSaveFileName;

    AquaFilePickerDelegate *m_pDelegate;

    void updateFilterUI();
    void updateSaveFileNameExtension();

public:

    virtual ~SalAquaFilePicker();

    void filterControlChanged();

    void implInitialize();

};

#endif // _SALAQUAFILEPICKER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
