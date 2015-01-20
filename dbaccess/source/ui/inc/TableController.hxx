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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECONTROLLER_HXX

#include "singledoccontroller.hxx"
#include "moduledbu.hxx"
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "TypeInfo.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace dbaui
{
    class OTableRow;
    typedef OSingleDocumentController   OTableController_BASE;
    class OTableController : public OTableController_BASE
    {
    private:
        OModuleClient                                   m_aModuleClient;
        ::std::vector< ::boost::shared_ptr<OTableRow> > m_vRowList;
        OTypeInfoMap                                    m_aTypeInfo;
        ::std::vector<OTypeInfoMap::iterator>           m_aTypeInfoIndex;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;

        OUString m_sCatalogName;         // catalog for update data
        OUString m_sSchemaName;          // schema for update data
        OUString m_sName;                // table for update data
        OUString m_sAutoIncrementValue;  // the autoincrement value set in the datasource
        OUString        m_sTypeNames;           // these type names are the ones out of the resource file
        TOTypeInfoSP    m_pTypeInfo;            // fall back when type is unknown because database driver has a failure

        bool        m_bAllowAutoIncrementValue; // no : 1 NO BIT , is true when the datasource has a AutoIncrementValue property in their info property
        bool        m_bNew      : 1;        // is true when we create a new table


        void reSyncRows();
        void assignTable();                 // set the table if a name is given
        void loadData();
        bool checkColumns(bool _bNew)
            throw (css::sdbc::SQLException,
                   css::uno::RuntimeException, std::exception);      // check if we have double column names
        OUString createUniqueName(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _rxTables,const OUString& _rDefault);
        void appendColumns(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>& _rxColSup, bool _bNew, bool _bKeyColumns = false);
        void appendPrimaryKey(::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XKeysSupplier>& _rxSup, bool _bNew);
        void alterColumns();
        void dropPrimaryKey();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> getKeyColumns() const;
        OUString createUniqueName(const OUString& _rName);

        void reload();

        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures() SAL_OVERRIDE;
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const SAL_OVERRIDE;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) SAL_OVERRIDE;

        virtual void losingConnection( ) SAL_OVERRIDE;

        virtual OUString getPrivateTitle( ) const SAL_OVERRIDE;

        void        doEditIndexes();
        bool        doSaveDoc(bool _bSaveAs);

        virtual ~OTableController();
    public:
        OTableController(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM);

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       getTable() { return m_xTable;}

        bool     isAddAllowed()     const;
        bool     isDropAllowed()    const;
        bool     isAlterAllowed()   const;
        bool     isAutoIncrementPrimaryKey() const;

        inline bool             isAutoIncrementValueEnabled()   const { return m_bAllowAutoIncrementValue; }
        inline const OUString&   getAutoIncrementValue()         const { return m_sAutoIncrementValue; }

        virtual void impl_onModifyChanged() SAL_OVERRIDE;

        inline ::std::vector< ::boost::shared_ptr<OTableRow> >& getRows() { return m_vRowList; }

        /// returns the position of the first empty row
        sal_Int32                           getFirstEmptyRowPosition();

        inline const OTypeInfoMap&          getTypeInfo() const { return m_aTypeInfo; }

        inline TOTypeInfoSP                 getTypeInfo(sal_Int32 _nPos) const { return m_aTypeInfoIndex[_nPos]->second; }
        TOTypeInfoSP                        getTypeInfoByType(sal_Int32 _nDataType) const;

        inline TOTypeInfoSP                 getTypeInfoFallBack() const { return m_pTypeInfo; }

        virtual bool                        Construct(vcl::Window* pParent) SAL_OVERRIDE;
        // XEventListener
        virtual void SAL_CALL               disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL           suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing() SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // need by registration
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);


        void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxIn);
        void Save(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOut);

    protected:
        void startTableListening();
        void stopTableListening();
        virtual void impl_initialize() SAL_OVERRIDE;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECONTROLLER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
