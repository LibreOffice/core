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

#ifndef _DBAUI_DBADMINIMPL_HXX_
#define _DBAUI_DBADMINIMPL_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <comphelper/stl_types.hxx>
#include "dsntypes.hxx"
#include <svl/itemset.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <svl/poolitem.hxx>

class Window;
namespace dbaui
{
    namespace DataSourceInfoConverter
    {
        void convert(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> & xContext,
                     const ::dbaccess::ODsnTypeCollection* _pCollection,
                     const OUString& _sOldURLPrefix,
                     const OUString& _sNewURLPrefix,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _xDatasource);
    };
    class IItemSetHelper;
    // ODbDataSourceAdministrationHelper
    class ODbDataSourceAdministrationHelper
    {
    public:
        DECLARE_STL_MAP(sal_Int32, OUString, ::std::less< sal_Int32 >, MapInt2String);

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                            m_xContext;                 /// service factory
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseContext >
                                m_xDatabaseContext;     /// database context we're working in
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xDatasource;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >         m_xModel;

        ::com::sun::star::uno::Any              m_aDataSourceOrName;
        typedef ::std::set< OUString >   StringSet;
        typedef StringSet::const_iterator       ConstStringSetIterator;

        MapInt2String           m_aDirectPropTranslator;    /// translating property id's into names (direct properties of a data source)
        MapInt2String           m_aIndirectPropTranslator;  /// translating property id's into names (indirect properties of a data source)
        Window*                 m_pParent;
        IItemSetHelper*         m_pItemSetHelper;
    public:

        ODbDataSourceAdministrationHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xORB
                                        ,Window* _pParent
                                        ,IItemSetHelper* _pItemSetHelper);

        /** translate the current dialog SfxItems into driver relevant PropertyValues
            @see successfullyConnected
        */
        sal_Bool    getCurrentSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDriverParams);

        /** to be called if the settings got from getCurrentSettings have been used for successfully connecting
            @see getCurrentSettings
        */
        void        successfullyConnected();

        /// clear the password in the current data source's item set
        void        clearPassword();

        inline ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getORB() const { return m_xContext; }

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseContext > getDatabaseContext() const { return m_xDatabaseContext; }

        /** creates a new connection. The caller is responsible to dispose it !!!!
        */
        ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool>      createConnection();

        /** return the corresponding driver for the selected URL
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >         getDriver();
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >         getDriver(const OUString& _sURL);

        /** returns the data source the dialog is currently working with
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getCurrentDataSource();
        // returns the Url of a database document
        String              getDocumentUrl(SfxItemSet& _rDest);

        void setDataSourceOrName( const ::com::sun::star::uno::Any& _rDataSourceOrName );

        /** extracts the connection type from the given set<p/>
            The connection type is determined by the value of the DSN item, analyzed by the TypeCollection item.
        */
        static OUString getDatasourceType( const SfxItemSet& _rSet );

        /** returns the connection URL
            @return
                The connection URL
        */
        String getConnectionURL() const;

        /// fill the necessary information from the url line
        void convertUrl(SfxItemSet& _rDest);

        const MapInt2String& getIndirectProperties() const { return m_aIndirectPropTranslator; }

        /** translates properties of an UNO data source into SfxItems
            @param  _rxSource
                The data source
            @param  _rDest
                The item set to fill.
        */
        void translateProperties(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSource,
                SfxItemSet& _rDest);

        /** translate SfxItems into properties of an UNO data source
            @param  _rSource
                The item set to read from.
            @param  _rxDest
                The data source to fill.
        */
        void translateProperties(
                const SfxItemSet& _rSource,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDest);

        sal_Bool saveChanges(const SfxItemSet& _rSource);
    protected:
        /** fill a data source info array with the settings from a given item set
        */
        void fillDatasourceInfo(const SfxItemSet& _rSource, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo);

        /// translate the given value into an SfxPoolItem, put this into the given set under the given id
        void        implTranslateProperty(SfxItemSet& _rSet, sal_Int32  _nId, const ::com::sun::star::uno::Any& _rValue);

        /// translate the given SfxPoolItem into an <type scope="com.sun.star.Any">uno</type>
        ::com::sun::star::uno::Any implTranslateProperty(const SfxPoolItem* _pItem);

        /// translate the given SfxPoolItem into an <type scope="com.sun.star.Any">uno</type>, set it (under the given name) on the given property set
        void        implTranslateProperty(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxSet, const OUString& _rName, const SfxPoolItem* _pItem);

        /** check if the data source described by the given set needs authentication<p/>
            The return value depends on the data source type only.
        */
        sal_Bool            hasAuthentication(const SfxItemSet& _rSet) const;

#if OSL_DEBUG_LEVEL > 0
        OString translatePropertyId( sal_Int32 _nId );
#endif
    };

}   // namespace dbaui

#endif // _DBAUI_DBADMINIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
