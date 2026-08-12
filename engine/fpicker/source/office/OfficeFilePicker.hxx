/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

struct FilterEntry;
struct ElementEntry_Impl;
enum class PickerFlags;

typedef ::std::vector< FilterEntry >           FilterList;     // can be maintained more effectively
typedef ::std::vector< ElementEntry_Impl >     ElementList;

typedef css::beans::StringPair                 UnoFilterEntry;
typedef cpo::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively

// class SvtFilePicker ---------------------------------------------------

typedef cppu::ImplInheritanceHelper <svt::OCommonPicker
                            ,   css::ui::dialogs::XFilePicker3
                            ,   css::ui::dialogs::XFilePickerControlAccess
                            ,   css::ui::dialogs::XFilePreview
                            ,   css::lang::XServiceInfo
                            ,   css::ui::dialogs::XAsynchronousExecutableDialog
                            >   SvtFilePicker_Base;

class SvtFilePicker :public SvtFilePicker_Base
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

    cpo::uno::Sequence< OUString >
                        m_aDenyList;

    css::uno::Reference< css::ui::dialogs::XFilePickerListener >
                        m_xListener;
    css::uno::Reference< css::ui::dialogs::XDialogClosedListener >
                        m_xDlgClosedListener;

public:
                       SvtFilePicker();
    virtual           ~SvtFilePicker() override;


    // XExecutableDialog functions

    virtual void setTitle( const OUString& _rTitle ) override;
    virtual sal_Int16 execute(  ) override;


    // XAsynchronousExecutableDialog functions

    virtual void setDialogTitle( const OUString& _rTitle ) override;
    virtual void startExecuteModal( const css::uno::Reference< css::ui::dialogs::XDialogClosedListener >& xListener ) override;


    // XFilePicker functions


    virtual void           setMultiSelectionMode( bool bMode ) override;
    virtual void           setDefaultName( const OUString& aName ) override;
    virtual void           setDisplayDirectory( const OUString& aDirectory ) override;
    virtual OUString    getDisplayDirectory() override;
    virtual cpo::uno::Sequence< OUString > getFiles() override;
    virtual cpo::uno::Sequence< OUString > getSelectedFiles() override;


    // XFilePickerControlAccess functions


    virtual void           setValue( sal_Int16 ElementID, sal_Int16 ControlAction, const cpo::uno::Any& value ) override;
    virtual cpo::uno::Any           getValue( sal_Int16 ElementID, sal_Int16 ControlAction ) override;
    virtual void           setLabel( sal_Int16 ElementID, const OUString& aValue ) override;
    virtual OUString    getLabel( sal_Int16 ElementID ) override;
    virtual void           enableControl( sal_Int16 ElementID, bool bEnable ) override;


    // XFilePickerNotifier functions


    virtual void           addFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;
    virtual void           removeFilePickerListener( const css::uno::Reference< css::ui::dialogs::XFilePickerListener >& xListener ) override;


    // XFilePreview functions


    virtual cpo::uno::Sequence< sal_Int16 > getSupportedImageFormats() override;
    virtual sal_Int32      getTargetColorDepth() override;
    virtual sal_Int32      getAvailableWidth() override;
    virtual sal_Int32      getAvailableHeight() override;
    virtual void           setImage( sal_Int16 aImageFormat, const cpo::uno::Any& aImage ) override;
    virtual bool       setShowState( bool bShowState ) override;
    virtual bool       getShowState() override;


    // XFilterManager functions


    virtual void           appendFilter( const OUString& aTitle, const OUString& aFilter ) override;
    virtual void           setCurrentFilter( const OUString& aTitle ) override;
    virtual OUString       getCurrentFilter() override;


    // XFilterGroupManager functions

    virtual void           appendFilterGroup( const OUString& sGroupTitle, const cpo::uno::Sequence< css::beans::StringPair >& aFilters ) override;


    // these methods are here because they're ambiguous

    virtual void           cancel() override
     { ::svt::OCommonPicker::cancel(); }
    virtual void           dispose() override
     { ::svt::OCommonPicker::dispose(); }
    virtual void           addEventListener(const css::uno::Reference<css::lang::XEventListener>& l) override
     { ::svt::OCommonPicker::addEventListener(l); }
    virtual void           removeEventListener(const css::uno::Reference<css::lang::XEventListener>& l) override
     { ::svt::OCommonPicker::removeEventListener(l); }


    // XInitialization functions


    virtual void           initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;


    // XServiceInfo functions


    /* XServiceInfo */
    virtual OUString       getImplementationName() override;
    virtual bool       supportsService( const OUString& sServiceName ) override;
    virtual cpo::uno::Sequence< OUString >
                                    getSupportedServiceNames() override;

protected:

    // OCommonPicker overridables

    virtual std::shared_ptr<SvtFileDialog_Base> implCreateDialog( weld::Window* pParent ) override;
    virtual sal_Int16       implExecutePicker( ) override;
    virtual bool            implHandleInitializationArgument(
                                const OUString& _rName,
                                const cpo::uno::Any& _rValue
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

    /* XServiceInfo */
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& sServiceName ) override;
    virtual cpo::uno::Sequence< OUString >
                                    getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
