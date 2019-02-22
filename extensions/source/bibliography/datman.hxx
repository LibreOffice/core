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

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_DATMAN_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_DATMAN_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/form/XLoadable.hpp>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }
namespace weld { class Window; }

namespace bib
{
    class BibView;
    class BibBeamer;
}

class BibToolBar;
struct BibDBDescriptor;

class BibInterceptorHelper
    :public cppu::WeakImplHelper< css::frame::XDispatchProviderInterceptor >
{
private:
    css::uno::Reference< css::frame::XDispatchProvider > xMasterDispatchProvider;
    css::uno::Reference< css::frame::XDispatchProvider > xSlaveDispatchProvider;
    css::uno::Reference< css::frame::XDispatch > xFormDispatch;
    css::uno::Reference< css::frame::XDispatchProviderInterception > xInterception;

protected:
    virtual ~BibInterceptorHelper( ) override;

public:
    BibInterceptorHelper( ::bib::BibBeamer* pBibBeamer, css::uno::Reference< css::frame::XDispatch > const & xDispatch);

    void ReleaseInterceptor();

    // XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) override;
    // XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) override;
    virtual void SAL_CALL setSlaveDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewSlaveDispatchProvider ) override;
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) override;
    virtual void SAL_CALL setMasterDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewMasterDispatchProvider ) override;
};

typedef cppu::WeakComponentImplHelper  <   css::form::XLoadable
                                        >   BibDataManager_Base;
class BibDataManager final
            :public ::comphelper::OMutexAndBroadcastHelper
            ,public BibDataManager_Base
{
private:
        css::uno::Reference< css::form::XForm >                       m_xForm;
        css::uno::Reference< css::awt::XControlModel >                m_xGridModel;
        css::uno::Reference< css::sdb::XSingleSelectQueryComposer >   m_xParser;
        css::uno::Reference< css::form::runtime::XFormController >    m_xFormCtrl;
        css::uno::Reference< css::frame::XDispatch >                  m_xFormDispatch;
        rtl::Reference<BibInterceptorHelper>                          m_xInterceptorHelper;

        OUString                     aActiveDataTable;
        OUString                     aDataSourceURL;
        OUString                     aQuoteChar;

        ::comphelper::OInterfaceContainerHelper2   m_aLoadListeners;

        VclPtr< ::bib::BibView>      pBibView;
        VclPtr<BibToolBar>           pToolbar;

        OUString                     sIdentifierMapping;

        void                        InsertFields(const css::uno::Reference< css::form::XFormComponent > & xGrid);

        css::uno::Reference< css::awt::XControlModel > const &
                                    updateGridModel(const css::uno::Reference< css::form::XForm > & xDbForm);
        static css::uno::Reference< css::awt::XControlModel >
                                    createGridModel( const OUString& rName );

        // XLoadable
        virtual void SAL_CALL load(  ) override;
        virtual void SAL_CALL unload(  ) override;
        virtual void SAL_CALL reload(  ) override;
        virtual sal_Bool SAL_CALL isLoaded(  ) override;
        virtual void SAL_CALL addLoadListener( const css::uno::Reference< css::form::XLoadListener >& aListener ) override;
        virtual void SAL_CALL removeLoadListener( const css::uno::Reference< css::form::XLoadListener >& aListener ) override;

        using WeakComponentImplHelperBase::disposing;

public:

        BibDataManager();
        virtual ~BibDataManager() override;

        css::uno::Reference< css::form::XForm >                   createDatabaseForm( BibDBDescriptor&    aDesc);

        css::uno::Reference< css::awt::XControlModel >            updateGridModel();

        css::uno::Sequence< OUString>           getDataSources();

        const OUString&             getActiveDataSource() {return aDataSourceURL;}
        void                        setActiveDataSource(const OUString& rURL);

        const OUString&             getActiveDataTable() { return aActiveDataTable;}
        void                        setActiveDataTable(const OUString& rTable);

        void                        setFilter(const OUString& rQuery);
        OUString                    getFilter();

        css::uno::Sequence< OUString> getQueryFields();
        OUString                    getQueryField();
        void                        startQueryWith(const OUString& rQuery);

        const css::uno::Reference< css::sdb::XSingleSelectQueryComposer >&    getParser() { return m_xParser; }
        const css::uno::Reference< css::form::XForm >&                        getForm()   { return m_xForm; }


        static OUString             getControlName(sal_Int32 nFormatKey );

        css::uno::Reference< css::awt::XControlModel > loadControlModel(const OUString& rName,
                                                        bool bForceListBox);

        void                        CreateMappingDialog(vcl::Window* pParent);
        OUString                    CreateDBChangeDialog(weld::Window* pParent);

        void                        DispatchDBChangeDialog();

        void                        SetView( ::bib::BibView* pView ) { pBibView = pView; }

        void                        SetToolbar(BibToolBar* pSet);

        const OUString&             GetIdentifierMapping();
        void                        ResetIdentifierMapping() {sIdentifierMapping.clear();}

        css::uno::Reference< css::form::runtime::XFormController > const & GetFormController();
        void                        RegisterInterceptor( ::bib::BibBeamer* pBibBeamer);

        bool                        HasActiveConnection();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
