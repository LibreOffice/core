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

#include "singledoccontroller.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "TypeInfo.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace dbaui
{
    class OTableRow;
    typedef OSingleDocumentController   OTableController_BASE;
    class OTableController final : public OTableController_BASE
    {
    private:
        std::vector< std::shared_ptr<OTableRow> > m_vRowList;
        OTypeInfoMap                                    m_aTypeInfo;
        std::vector<OTypeInfoMap::iterator>           m_aTypeInfoIndex;

        css::uno::Reference< css::beans::XPropertySet >       m_xTable;

        OUString        m_sName;                // table for update data
        OUString        m_sAutoIncrementValue;  // the autoincrement value set in the datasource
        OUString        m_sTypeNames;           // these type names are the ones out of the resource file
        TOTypeInfoSP    m_pTypeInfo;            // fall back when type is unknown because database driver has a failure

        bool            m_bAllowAutoIncrementValue; // no : 1 NO BIT , is true when the datasource has an AutoIncrementValue property in their info property
        bool            m_bNew      : 1;        // is true when we create a new table


        void reSyncRows();
        void assignTable();                 // set the table if a name is given
        void loadData();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        bool checkColumns(bool _bNew);      // check if we have double column names
        void appendColumns(css::uno::Reference< css::sdbcx::XColumnsSupplier> const & _rxColSup, bool _bNew, bool _bKeyColumns = false);
        void appendPrimaryKey(css::uno::Reference< css::sdbcx::XKeysSupplier> const & _rxSup, bool _bNew);
        void alterColumns();
        void dropPrimaryKey();
        css::uno::Reference< css::container::XNameAccess> getKeyColumns() const;
        OUString createUniqueName(const OUString& _rName);

        void reload();

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures() override;
        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const override;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        virtual void losingConnection( ) override;

        virtual OUString getPrivateTitle( ) const override;

        void        doEditIndexes();
        bool        doSaveDoc(bool _bSaveAs);

        virtual ~OTableController() override;
    public:
        OTableController(const css::uno::Reference< css::uno::XComponentContext >& _rM);

        const css::uno::Reference< css::beans::XPropertySet >&  getTable() const { return m_xTable;}

        bool     isAddAllowed()     const;
        bool     isDropAllowed()    const;
        bool     isAlterAllowed()   const;
        bool     isAutoIncrementPrimaryKey() const;

        bool             isAutoIncrementValueEnabled()   const { return m_bAllowAutoIncrementValue; }
        const OUString&   getAutoIncrementValue()         const { return m_sAutoIncrementValue; }

        virtual void impl_onModifyChanged() override;

        std::vector< std::shared_ptr<OTableRow> >& getRows() { return m_vRowList; }

        /// returns the position of the first empty row
        sal_Int32                           getFirstEmptyRowPosition();

        const OTypeInfoMap&          getTypeInfo() const { return m_aTypeInfo; }

        TOTypeInfoSP const &                getTypeInfo(sal_Int32 _nPos) const { return m_aTypeInfoIndex[_nPos]->second; }
        TOTypeInfoSP                        getTypeInfoByType(sal_Int32 _nDataType) const;

        const TOTypeInfoSP&                 getTypeInfoFallBack() const { return m_pTypeInfo; }

        virtual bool                        Construct(vcl::Window* pParent) override;
        // XEventListener
        virtual void SAL_CALL               disposing( const css::lang::EventObject& Source ) override;

        // css::frame::XController
        virtual sal_Bool SAL_CALL           suspend(sal_Bool bSuspend) override;

        // css::lang::XComponent
        virtual void        SAL_CALL disposing() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

    private:
        void startTableListening();
        void stopTableListening();
        virtual void impl_initialize() override;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
