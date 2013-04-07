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

#ifndef _BIB_DATMAN_HXX
#define _BIB_DATMAN_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/form/XLoadable.hpp>
#include <comphelper/broadcasthelper.hxx>
// #100312# --------------------
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <cppuhelper/implbase1.hxx>

class Window;

//-----------------------------------------------------------------------------
namespace bib
{
    class BibView;
    // #100312# -----------
    class BibBeamer;
}

class BibToolBar;
struct BibDBDescriptor;

// #100312# ---------------------
class BibInterceptorHelper
    :public cppu::WeakImplHelper1< ::com::sun::star::frame::XDispatchProviderInterceptor >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > xMasterDispatchProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > xSlaveDispatchProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xFormDispatch;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception > xInterception;

protected:
    ~BibInterceptorHelper( );

public:
    BibInterceptorHelper( ::bib::BibBeamer* pBibBeamer, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch);

    void ReleaseInterceptor();

    // XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw (::com::sun::star::uno::RuntimeException);
    // XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSlaveDispatchProvider ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewMasterDispatchProvider ) throw (::com::sun::star::uno::RuntimeException);
};

typedef cppu::WeakComponentImplHelper2  <   ::com::sun::star::beans::XPropertyChangeListener
                                        ,   ::com::sun::star::form::XLoadable
                                        >   BibDataManager_Base;
class BibDataManager
            :public ::comphelper::OMutexAndBroadcastHelper
            ,public BibDataManager_Base
{
private:
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >                       m_xForm;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >                m_xGridModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               m_xSourceProps;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >   m_xParser;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >    m_xFormCtrl;
        // #100312# -------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      m_xFormDispatch;
        BibInterceptorHelper* m_pInterceptorHelper;

        OUString                     aActiveDataTable;
        OUString                     aDataSourceURL;
        OUString                     aQuoteChar;
        ::com::sun::star::uno::Any                      aUID;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >              xBibCursor;

        ::cppu::OInterfaceContainerHelper   m_aLoadListeners;

        ::bib::BibView*             pBibView;
        BibToolBar*                 pToolbar;

        OUString               sIdentifierMapping;
protected:

        void                        InsertFields(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);
        void                        SetMeAsUidListener();
        void                        RemoveMeAsUidListener();

        void                        UpdateAddressbookCursor(OUString aSourceName);

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                                    updateGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xDbForm);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                                    createGridModel( const OUString& rName );

        // XLoadable
        virtual void SAL_CALL load(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unload(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reload(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isLoaded(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addLoadListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeLoadListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL disposing();

public:

        BibDataManager();
        ~BibDataManager();

        virtual void                SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
                                                                throw( ::com::sun::star::uno::RuntimeException );
        virtual void                SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
                                                                throw( ::com::sun::star::uno::RuntimeException );



        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >                   createDatabaseForm( BibDBDescriptor&    aDesc);

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            updateGridModel();

        ::com::sun::star::uno::Sequence< OUString>           getDataSources();

        OUString             getActiveDataSource() {return aDataSourceURL;}
        void                        setActiveDataSource(const OUString& rURL);

        OUString             getActiveDataTable();
        void                        setActiveDataTable(const OUString& rTable);

        void                        setFilter(const OUString& rQuery);
        OUString                     getFilter();

        ::com::sun::star::uno::Sequence< OUString>           getQueryFields();
        OUString                     getQueryField();
        void                        startQueryWith(const OUString& rQuery);

        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >&    getParser() { return m_xParser; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >&                        getForm()   { return m_xForm; }


        OUString                     getControlName(sal_Int32 nFormatKey );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            loadControlModel(const OUString& rName,
                                                        sal_Bool bForceListBox = sal_False);

        void                        CreateMappingDialog(Window* pParent);
        OUString             CreateDBChangeDialog(Window* pParent);

        void                        DispatchDBChangeDialog();
        sal_Bool                    HasActiveConnection() const;

        void                        SetView( ::bib::BibView* pView ) { pBibView = pView; }

        void                        SetToolbar(BibToolBar* pSet);

        const OUString&        GetIdentifierMapping();
        void                        ResetIdentifierMapping() {sIdentifierMapping = OUString();}

        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > GetFormController();
        // #100312# ----------
        void RegisterInterceptor( ::bib::BibBeamer* pBibBeamer);

        sal_Bool                    HasActiveConnection();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
