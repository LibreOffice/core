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
#pragma once

#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XEventListener.hpp>


#include "commonpicker.hxx"
#include "pickercallbacks.hxx"

#include <vector>

class Dialog;
struct FilterEntry;
struct ElementEntry_Impl;
enum class PickerFlags;

typedef ::std::vector< FilterEntry >           FilterList;     // can be maintained more effectively
typedef ::std::vector< ElementEntry_Impl >     ElementList;

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
    std::unique_ptr<FilterList>
                        m_pFilterList;
    std::unique_ptr<ElementList>
                        m_pElemList;

    bool                m_bMultiSelection;
    sal_Int16           m_nServiceType;
    OUString            m_aDefaultName;
    OUString            m_aCurrentFilter;

    OUString            m_aOldDisplayDirectory;
    OUString            m_aOldHideDirectory;

    OUString            m_aStandardDir;
    css::uno::Sequence< OUString >
                        m_aDenyList;

    css::uno::Reference< css::ui::dialogs::XFilePickerListener >
                        m_xListener;
    css::uno::Reference< css::ui::dialogs::XDialogClosedListener >
                        m_xDlgClosedListener;

public:
                       SvtFilePicker();
    virtual           ~SvtFilePicker() override;


    // disambiguate XInterface

    DECLARE_XINTERFACE( )


    // disambiguate XTypeProvider

    DECLARE_XTYPEPROVIDER( )


    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& _rTitle ) override;
    virtual sal_Int16 SAL_CALL execute(  ) override;


    // XAsynchronousExecutableDialog functions

    virtual void SAL_CALL setDialogTitle( const OUString& _rTitle ) override;
    virtual void SAL_CALL startExecuteModal( const css::uno::Reference< css::ui::dialogs::XDialogClosedListener >& xListener ) override;


    // XFilePicker functions


    virtual void SAL_CALL           setMultiSelectionMode( sal_Bool bMode ) override;
    virtual void SAL_CALL           setDefaultName( const OUString& aName ) override;
    virtual void SAL_CALL           setDisplayDirectory( const OUString& aDirectory ) override;
    virtual OUString SAL_CALL    getDisplayDirectory() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getFiles() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSelectedFiles() override;


    // XFilePickerControlAccess functions


    virtual void SAL_CALL           setValue( sal_Int16 ElementID, sal_Int16 ControlAction, const css::uno::Any& value ) override;
    virtual css::uno::Any SAL_CALL           getValue( sal_Int16 ElementID, sal_Int16 ControlAction ) override;
    virtual void SAL_CALL           setLabel( sal_Int16 ElementID, const OUString& aValue ) override;
    virtual OUString SAL_CALL    getLabel( sal_Int16 ElementID ) override;
    virtual void SAL_CALL           enableControl( sal_Int16 ElementID, sal_Bool bEnable ) override;


    // XFilePickerNotifier functions


    virtual void SAL_CALL           addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;
    virtual void SAL_CALL           removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;


    // XFilePreview functions


    virtual css::uno::Sequence< sal_Int16 > SAL_CALL getSupportedImageFormats() override;
    virtual sal_Int32 SAL_CALL      getTargetColorDepth() override;
    virtual sal_Int32 SAL_CALL      getAvailableWidth() override;
    virtual sal_Int32 SAL_CALL      getAvailableHeight() override;
    virtual void SAL_CALL           setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage ) override;
    virtual sal_Bool SAL_CALL       setShowState( sal_Bool bShowState ) override;
    virtual sal_Bool SAL_CALL       getShowState() override;


    // XFilterManager functions


    virtual void SAL_CALL           appendFilter( const OUString& aTitle, const OUString& aFilter ) override;
    virtual void SAL_CALL           setCurrentFilter( const OUString& aTitle ) override;
    virtual OUString SAL_CALL       getCurrentFilter() override;


    // XFilterGroupManager functions

    virtual void SAL_CALL           appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters ) override;


    // these methods are here because they're ambiguous

    virtual void SAL_CALL           cancel() override
     { ::svt::OCommonPicker::cancel(); }
    virtual void SAL_CALL           dispose() override
     { ::svt::OCommonPicker::dispose(); }
    virtual void SAL_CALL           addEventListener(const css::uno::Reference<css::lang::XEventListener>& l) override
     { ::svt::OCommonPicker::addEventListener(l); }
    virtual void SAL_CALL           removeEventListener(const css::uno::Reference<css::lang::XEventListener>& l) override
     { ::svt::OCommonPicker::removeEventListener(l); }


    // XInitialization functions


    virtual void SAL_CALL           initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;


    // XServiceInfo functions


    /* XServiceInfo */
    virtual OUString SAL_CALL       getImplementationName() override;
    virtual sal_Bool SAL_CALL       supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() override;

protected:

    // OCommonPicker overridables

    virtual std::shared_ptr<SvtFileDialog_Base> implCreateDialog( weld::Window* pParent ) override;
    virtual sal_Int16       implExecutePicker( ) override;
    virtual bool            implHandleInitializationArgument(
                                const OUString& _rName,
                                const css::uno::Any& _rValue
                            ) override;

protected:
    PickerFlags         getPickerFlags() const;
    virtual void        notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) override;

    bool                FilterNameExists( const OUString& rTitle );
    bool                FilterNameExists( const UnoFilterList& _rGroupedFilters );

    void                ensureFilterList( const OUString& _rInitialCurrentFilter );

    void                prepareExecute( );

    void                DialogClosedHdl(sal_Int32 nResult);
};

// SvtRemoteFilePicker

class SvtRemoteFilePicker : public SvtFilePicker
{
public:
    SvtRemoteFilePicker();

    virtual std::shared_ptr<SvtFileDialog_Base> implCreateDialog( weld::Window* pParent ) override;

    // disambiguate XInterface

    DECLARE_XINTERFACE( )

    // disambiguate XTypeProvider

    DECLARE_XTYPEPROVIDER( )

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
