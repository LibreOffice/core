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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONCONTROLLER_HXX

#include "JoinController.hxx"
#include "RelationDesignView.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

class WaitObject;
namespace dbaui
{
    class ORelationController : public OJoinController
    {
        css::uno::Reference< css::container::XNameAccess >    m_xTables;
        ::std::unique_ptr<WaitObject> m_pWaitObject;
        sal_uLong       m_nThreadEvent;
        bool            m_bRelationsPossible;
    protected:
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures() override;
        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const override;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        void loadData();
        TTableWindowData::value_type existsTable(const OUString& _rComposedTableName,bool _bCase) const;

        // load the window positions out of the datasource
        void loadLayoutInformation();
    public:
        ORelationController(const css::uno::Reference< css::uno::XComponentContext >& _rM);

        virtual ~ORelationController();

        void mergeData(const TTableConnectionData& _aConnectionData);

        virtual bool Construct(vcl::Window* pParent) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
        // need by registration
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        // OJoinController overridables
        virtual bool allowViews() const override;
        virtual bool allowQueries() const override;

    private:
        // ask the user if the design should be saved when it is modified
        virtual short saveModified() override;
        virtual void reset() override;
        virtual void impl_initialize() override;
        virtual OUString getPrivateTitle( ) const override;
        DECL_LINK_TYPED( OnThreadFinished, void*, void );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
