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

typedef ::std::list< FilterEntry >             FilterList;     // can be maintained more effectively
typedef ::std::list < ElementEntry_Impl >      ElementList;

typedef css::beans::StringPair                 UnoFilterEntry;
typedef css::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively

// class SvtFilePicker ---------------------------------------------------

typedef ::cppu::ImplHelper5 <   css::ui::dialogs::XFilePicker3
                            ,   css::ui::dialogs::XFilePickerControlAccess
                            ,   css::ui::dialogs::XFilePreview
                            ,   css::lang::XServiceInfo
                            ,   css::ui::dialogs::XAsynchronousExecutableDialog
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
    css::uno::Sequence< OUString >
                        m_aBlackList;

    css::uno::Reference< css::ui::dialogs::XFilePickerListener >
                        m_xListener;
    css::uno::Reference< css::ui::dialogs::XDialogClosedListener >
                        m_xDlgClosedListener;

public:
                       SvtFilePicker( const css::uno::Reference < css::lang::XMultiServiceFactory >& xFactory );
    virtual           ~SvtFilePicker();


    // disambiguate XInterface

    DECLARE_XINTERFACE( )


    // disambiguate XTypeProvider

    DECLARE_XTYPEPROVIDER( )


    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& _rTitle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute(  ) throw (css::uno::RuntimeException, std::exception) override;


    // XAsynchronousExecutableDialog functions

    virtual void SAL_CALL setDialogTitle( const OUString& _rTitle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startExecuteModal( const css::uno::Reference< css::ui::dialogs::XDialogClosedListener >& xListener )
        throw (css::uno::RuntimeException,
               std::exception) override;


    // XFilePicker functions


    virtual void SAL_CALL           setMultiSelectionMode( sal_Bool bMode ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setDefaultName( const OUString& aName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setDisplayDirectory( const OUString& aDirectory ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL    getDisplayDirectory() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getFiles() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles() throw( css::uno::RuntimeException, std::exception ) override;


    // XFilePickerControlAccess functions


    virtual void SAL_CALL           setValue( sal_Int16 ElementID, sal_Int16 ControlAction, const css::uno::Any& value ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL           getValue( sal_Int16 ElementID, sal_Int16 ControlAction ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setLabel( sal_Int16 ElementID, const OUString& aValue ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL    getLabel( sal_Int16 ElementID ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           enableControl( sal_Int16 ElementID, sal_Bool bEnable ) throw( css::uno::RuntimeException, std::exception ) override;


    // XFilePickerNotifier functions


    virtual void SAL_CALL           addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) throw ( css::uno::RuntimeException, std::exception ) override;


    // XFilePreview functions


    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL      getTargetColorDepth() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL      getAvailableWidth() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL      getAvailableHeight() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       setShowState( sal_Bool bShowState ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       getShowState() throw ( css::uno::RuntimeException, std::exception ) override;


    // XFilterManager functions


    virtual void SAL_CALL           appendFilter( const OUString& aTitle, const OUString& aFilter ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setCurrentFilter( const OUString& aTitle ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL       getCurrentFilter() throw( css::uno::RuntimeException, std::exception ) override;


    // XFilterGroupManager functions

    virtual void SAL_CALL           appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;



    // these methods are here because they're ambiguous

    virtual void SAL_CALL           cancel() throw( css::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::cancel(); }
    virtual void SAL_CALL           dispose() throw( css::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::dispose(); }
    virtual void SAL_CALL           addEventListener(const css::uno::Reference<css::lang::XEventListener>& l) throw( css::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::addEventListener(l); }
    virtual void SAL_CALL           removeEventListener(const css::uno::Reference<css::lang::XEventListener>& l) throw( css::uno::RuntimeException, std::exception ) override
     { ::svt::OCommonPicker::removeEventListener(l); }


    // XInitialization functions


    virtual void SAL_CALL           initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;


    // XServiceInfo functions


    /* XServiceInfo */
    virtual OUString SAL_CALL       getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    /* Helper for XServiceInfo */
    static css::uno::Sequence< OUString >
                                    impl_getStaticSupportedServiceNames();
    static OUString                 impl_getStaticImplementationName();

    /* Helper for registry */
    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance (
        const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        throw( css::uno::Exception );

protected:

    // OCommonPicker overridables

    virtual VclPtr<SvtFileDialog_Base> implCreateDialog( vcl::Window* _pParent ) override;
    virtual sal_Int16       implExecutePicker( ) override;
    virtual bool            implHandleInitializationArgument(
                                const OUString& _rName,
                                const css::uno::Any& _rValue
                            ) override;

protected:
    WinBits             getWinBits( WinBits& rExtraBits );
    virtual void        notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) override;

    bool                FilterNameExists( const OUString& rTitle );
    bool                FilterNameExists( const UnoFilterList& _rGroupedFilters );

    void                ensureFilterList( const OUString& _rInitialCurrentFilter );

    void                prepareExecute( );

    DECL_LINK_TYPED(    DialogClosedHdl, Dialog&, void );
};

// SvtRemoteFilePicker

class SvtRemoteFilePicker : public SvtFilePicker
{
public:
    SvtRemoteFilePicker( const css::uno::Reference < css::lang::XMultiServiceFactory >& xFactory );

    virtual VclPtr<SvtFileDialog_Base> implCreateDialog( vcl::Window* _pParent ) override;

    // disambiguate XInterface

    DECLARE_XINTERFACE( )

    // disambiguate XTypeProvider

    DECLARE_XTYPEPROVIDER( )

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    /* Helper for XServiceInfo */
    static css::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();
    static OUString impl_getStaticImplementationName();

    /* Helper for registry */
    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance (
        const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        throw( css::uno::Exception );
};

#endif // INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFILEPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
