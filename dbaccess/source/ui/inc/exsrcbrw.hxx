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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_EXSRCBRW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_EXSRCBRW_HXX

#include "brwctrlr.hxx"

#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase2.hxx>

// SbaExternalSourceBrowser

namespace dbaui
{
    class SbaXFormAdapter;
    class SbaExternalSourceBrowser
                :public SbaXDataBrowserController
                ,public ::com::sun::star::util::XModifyBroadcaster
    {
        ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
            // for multiplexing the modify events
        SbaXFormAdapter*                    m_pDataSourceImpl;
        bool                            m_bInQueryDispatch;
            // our queryDispatch will ask our frame, which first will ask our queryDispatch, so we need to protect against
            // recursion

    public:
        SbaExternalSourceBrowser(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM);

        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaExternalSourceBrowser, OGenericUnoController)
        virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        //  virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass > >  getIdlClasses();

        //  static ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >  getStaticIdlClass();

        // ::com::sun::star::frame::XDispatch
        virtual void SAL_CALL dispatch(const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::util::XModifyListener
        virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::util::XModifyBroadcaster
        virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // ::com::sun::star::form::XLoadListener
        virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual ~SbaExternalSourceBrowser();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  CreateForm() SAL_OVERRIDE;
        virtual bool InitializeForm( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_formProperties ) SAL_OVERRIDE;

        virtual bool LoadForm() SAL_OVERRIDE;

        void Attach(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xMaster);

        void ClearView();

        void startListening();
        void stopListening();
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_EXSRCBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
