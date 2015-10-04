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

// SbaExternalSourceBrowser

namespace dbaui
{
    class SbaXFormAdapter;
    class SbaExternalSourceBrowser
                :public SbaXDataBrowserController
                ,public css::util::XModifyBroadcaster
    {
        ::cppu::OInterfaceContainerHelper   m_aModifyListeners;
            // for multiplexing the modify events
        SbaXFormAdapter*                    m_pDataSourceImpl;
        bool                            m_bInQueryDispatch;
            // our queryDispatch will ask our frame, which first will ask our queryDispatch, so we need to protect against
            // recursion

    public:
        SbaExternalSourceBrowser(const css::uno::Reference< css::uno::XComponentContext >& _rM);

        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        // UNO
        DECLARE_UNO3_DEFAULTS(SbaExternalSourceBrowser, OGenericUnoController)
        virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;
        //  virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > >  getIdlClasses();

        //  static css::uno::Reference< css::reflection::XIdlClass >  getStaticIdlClass();

        // css::frame::XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) throw(css::uno::RuntimeException, std::exception) override;

        // css::frame::XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( css::uno::RuntimeException, std::exception ) override;

        // css::util::XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

        // css::util::XModifyBroadcaster
        virtual void SAL_CALL addModifyListener(const css::uno::Reference< css::util::XModifyListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;

        // css::lang::XComponent
        virtual void SAL_CALL disposing() override;

        // css::form::XLoadListener
        virtual void SAL_CALL unloading(const css::lang::EventObject& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~SbaExternalSourceBrowser();

        virtual css::uno::Reference< css::sdbc::XRowSet >  CreateForm() override;
        virtual bool InitializeForm( const css::uno::Reference< css::beans::XPropertySet >& i_formProperties ) override;

        virtual bool LoadForm() override;

        void Attach(const css::uno::Reference< css::sdbc::XRowSet > & xMaster);

        void ClearView();

        void startListening();
        void stopListening();
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_EXSRCBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
