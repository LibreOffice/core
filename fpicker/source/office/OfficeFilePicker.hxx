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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFILEPICKER_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFILEPICKER_HXX

#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


#include <tools/wintypes.hxx>
#include "commonpicker.hxx"
#include "pickercallbacks.hxx"

#include <list>

class Dialog;

struct FilterEntry;
struct ElementEntry_Impl;

typedef ::std::list< FilterEntry >                          FilterList;     // can be maintained more effectively
typedef ::std::list < ElementEntry_Impl >                   ElementList;

typedef ::com::sun::star::beans::StringPair                 UnoFilterEntry;
typedef ::com::sun::star::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively
typedef ::com::sun::star::uno::Sequence< OUString >  OUStringList;   // can be transported more effectively

// class SvtFilePicker ---------------------------------------------------

typedef ::cppu::ImplHelper5 <   ::com::sun::star::ui::dialogs::XFilePicker3
                            ,   ::com::sun::star::ui::dialogs::XFilePickerControlAccess
                            ,   ::com::sun::star::ui::dialogs::XFilePreview
                            ,   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::ui::dialogs::XAsynchronousExecutableDialog
                            >   SvtFilePicker_Base;

class SvtFilePicker :public SvtFilePicker_Base
                    ,public ::svt::OCommonPicker
                    ,public ::svt::IFilePickerListener
{
protected:
    FilterList*         m_pFilterList;
    ElementList*        m_pElemList;

    bool                m_bMultiSelection;
    sal_Int16           m_nServiceType;
    OUString            m_aDefaultName;
    OUString            m_aCurrentFilter;

    OUString            m_aOldDisplayDirectory;
    OUString            m_aOldHideDirectory;

    OUString            m_aStandardDir;
    OUStringList        m_aBlackList;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >
                        m_xListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >
                        m_xDlgClosedListener;

public:
                       SvtFilePicker( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual           ~SvtFilePicker();


    // disambiguate XInterface

    DECLARE_XINTERFACE( )


    // disambiguate XTypeProvider

    DECLARE_XTYPEPROVIDER( )


    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& _rTitle ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    // XAsynchronousExecutableDialog functions

    virtual void SAL_CALL setDialogTitle( const OUString& _rTitle ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startExecuteModal( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;


    // XFilePicker functions


    virtual void SAL_CALL           setMultiSelectionMode( sal_Bool bMode ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setDefaultName( const OUString& aName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setDisplayDirectory( const OUString& aDirectory ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL    getDisplayDirectory() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getFiles() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSelectedFiles() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    // XFilePickerControlAccess functions


    virtual void SAL_CALL           setValue( sal_Int16 ElementID, sal_Int16 ControlAction, const com::sun::star::uno::Any& value ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Any SAL_CALL           getValue( sal_Int16 ElementID, sal_Int16 ControlAction ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setLabel( sal_Int16 ElementID, const OUString& aValue ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL    getLabel( sal_Int16 ElementID ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           enableControl( sal_Int16 ElementID, sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    // XFilePickerNotifier functions


    virtual void SAL_CALL           addFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           removeFilePickerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XFilePickerListener >& xListener ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    // XFilePreview functions


    virtual com::sun::star::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL      getTargetColorDepth() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL      getAvailableWidth() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL      getAvailableHeight() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setImage( sal_Int16 aImageFormat, const com::sun::star::uno::Any& aImage ) throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       setShowState( sal_Bool bShowState ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       getShowState() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    // XFilterManager functions


    virtual void SAL_CALL           appendFilter( const OUString& aTitle, const OUString& aFilter ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setCurrentFilter( const OUString& aTitle ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL    getCurrentFilter() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    // XFilterGroupManager functions

    virtual void SAL_CALL           appendFilterGroup( const OUString& sGroupTitle, const com::sun::star::uno::Sequence< com::sun::star::beans::StringPair >& aFilters ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;



    // these methods are here because they're ambiguous

    virtual void SAL_CALL           cancel() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::cancel(); }
    virtual void SAL_CALL           dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::dispose(); }
    virtual void SAL_CALL           addEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& l) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::addEventListener(l); }
    virtual void SAL_CALL           removeEventListener(const com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>& l) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::removeEventListener(l); }


    // XInitialization functions


    virtual void SAL_CALL           initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments ) throw ( com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception ) override;


    // XServiceInfo functions


    /* XServiceInfo */
    virtual OUString SAL_CALL    getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       supportsService( const OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /* Helper for XServiceInfo */
    static com::sun::star::uno::Sequence< OUString >
                                    impl_getStaticSupportedServiceNames();
    static OUString          impl_getStaticImplementationName();

    /* Helper for registry */
    static ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL impl_createInstance (
        const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( com::sun::star::uno::Exception );

protected:

    // OCommonPicker overridables

    virtual VclPtr<SvtFileDialog_Base> implCreateDialog( vcl::Window* _pParent ) override;
    virtual sal_Int16       implExecutePicker( ) override;
    virtual bool            implHandleInitializationArgument(
                                const OUString& _rName,
                                const ::com::sun::star::uno::Any& _rValue
                            ) override;

protected:
    WinBits             getWinBits( WinBits& rExtraBits );
    virtual void        notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) override;

    bool                FilterNameExists( const OUString& rTitle );
    bool                FilterNameExists( const UnoFilterList& _rGroupedFilters );

    void                ensureFilterList( const OUString& _rInitialCurrentFilter );

    void                prepareExecute( );

    DECL_LINK(          DialogClosedHdl, Dialog* );
};

// SvtRemoteFilePicker

class SvtRemoteFilePicker : public SvtFilePicker
{
public:
    SvtRemoteFilePicker( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    virtual VclPtr<SvtFileDialog_Base> implCreateDialog( vcl::Window* _pParent ) override;

    // disambiguate XInterface

    DECLARE_XINTERFACE( )

    // disambiguate XTypeProvider

    DECLARE_XTYPEPROVIDER( )

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /* Helper for XServiceInfo */
    static com::sun::star::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();
    static OUString impl_getStaticImplementationName();

    /* Helper for registry */
    static ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL impl_createInstance (
        const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( com::sun::star::uno::Exception );
};

#endif // INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
