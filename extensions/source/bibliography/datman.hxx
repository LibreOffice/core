/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datman.hxx,v $
 * $Revision: 1.16 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BIB_DATMAN_HXX
#define _BIB_DATMAN_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/form/XFormController.hpp>
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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw (::com::sun::star::uno::RuntimeException);
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
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >             m_xFormCtrl;
        // #100312# -------------------
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      m_xFormDispatch;
        BibInterceptorHelper* m_pInterceptorHelper;

        ::rtl::OUString                     aActiveDataTable;
        ::rtl::OUString                     aDataSourceURL;
        ::rtl::OUString                     aQuoteChar;
        ::com::sun::star::uno::Any                      aUID;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >              xBibCursor;

        ::cppu::OInterfaceContainerHelper   m_aLoadListeners;

        ::bib::BibView*             pBibView;
        BibToolBar*                 pToolbar;

        rtl::OUString               sIdentifierMapping;
protected:

        void                        InsertFields(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);
        void                        SetMeAsUidListener();
        void                        RemoveMeAsUidListener();

        void                        UpdateAddressbookCursor(::rtl::OUString aSourceName);

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                                    updateGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xDbForm);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                                    createGridModel( const ::rtl::OUString& rName );

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

        ::com::sun::star::uno::Sequence< ::rtl::OUString>           getDataSources();

        ::rtl::OUString             getActiveDataSource() {return aDataSourceURL;}
        void                        setActiveDataSource(const ::rtl::OUString& rURL);

        ::rtl::OUString             getActiveDataTable();
        void                        setActiveDataTable(const ::rtl::OUString& rTable);

        void                        setFilter(const ::rtl::OUString& rQuery);
        ::rtl::OUString                     getFilter();

        ::com::sun::star::uno::Sequence< ::rtl::OUString>           getQueryFields();
        ::rtl::OUString                     getQueryField();
        void                        startQueryWith(const ::rtl::OUString& rQuery);

        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >&    getParser() { return m_xParser; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >&                        getForm()   { return m_xForm; }


        ::rtl::OUString                     getControlName(sal_Int32 nFormatKey );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >            loadControlModel(const ::rtl::OUString& rName,
                                                        sal_Bool bForceListBox = sal_False);
        void                        saveCtrModel(const ::rtl::OUString& rName,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & rCtrModel);

        sal_Bool                        moveRelative(sal_Int32 nMove);

        void                        CreateMappingDialog(Window* pParent);
        ::rtl::OUString             CreateDBChangeDialog(Window* pParent);

        void                        DispatchDBChangeDialog();
        sal_Bool                    HasActiveConnection() const;

        void                        SetView( ::bib::BibView* pView ) { pBibView = pView; }

        void                        SetToolbar(BibToolBar* pSet);

        const rtl::OUString&        GetIdentifierMapping();
        void                        ResetIdentifierMapping() {sIdentifierMapping = rtl::OUString();}

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > GetFormController();
        // #100312# ----------
        void RegisterInterceptor( ::bib::BibBeamer* pBibBeamer);

        sal_Bool                    HasActiveConnection();
};


#endif
