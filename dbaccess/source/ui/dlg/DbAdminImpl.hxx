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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DBADMINIMPL_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DBADMINIMPL_HXX

#include <sal/config.h>

#include <map>
#include <set>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include "dsntypes.hxx"
#include <svl/itemset.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <svl/poolitem.hxx>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }
namespace dbaui
{
    namespace DataSourceInfoConverter
    {
        void convert(const css::uno::Reference< css::uno::XComponentContext> & xContext,
                     const ::dbaccess::ODsnTypeCollection* _pCollection,
                     const OUString& _sOldURLPrefix,
                     const OUString& _sNewURLPrefix,
                     const css::uno::Reference< css::beans::XPropertySet >& _xDatasource);
    };
    class IItemSetHelper;
    // ODbDataSourceAdministrationHelper
    class ODbDataSourceAdministrationHelper
    {
    public:
        typedef std::map<sal_Int32, OUString> MapInt2String;

    private:
        css::uno::Reference< css::uno::XComponentContext >
                                m_xContext;                 /// service factory
        css::uno::Reference< css::sdb::XDatabaseContext >
                                m_xDatabaseContext;     /// database context we're working in
        css::uno::Reference< css::beans::XPropertySet >   m_xDatasource;
        css::uno::Reference< css::frame::XModel >         m_xModel;

        css::uno::Any              m_aDataSourceOrName;
        typedef ::std::set< OUString >   StringSet;
        typedef StringSet::const_iterator       ConstStringSetIterator;

        MapInt2String           m_aDirectPropTranslator;    /// translating property id's into names (direct properties of a data source)
        MapInt2String           m_aIndirectPropTranslator;  /// translating property id's into names (indirect properties of a data source)
        VclPtr<vcl::Window>     m_pParent;
        IItemSetHelper*         m_pItemSetHelper;
    public:

        ODbDataSourceAdministrationHelper(const css::uno::Reference< css::uno::XComponentContext >& _xORB
                                        ,vcl::Window* _pParent
                                        ,IItemSetHelper* _pItemSetHelper);

        /** translate the current dialog SfxItems into driver relevant PropertyValues
            @see successfullyConnected
        */
        bool    getCurrentSettings(css::uno::Sequence< css::beans::PropertyValue >& _rDriverParams);

        /** to be called if the settings got from getCurrentSettings have been used for successfully connecting
            @see getCurrentSettings
        */
        void        successfullyConnected();

        /// clear the password in the current data source's item set
        void        clearPassword();

        const css::uno::Reference< css::uno::XComponentContext >& getORB() const { return m_xContext; }

        /** creates a new connection. The caller is responsible to dispose it !!!!
        */
        ::std::pair< css::uno::Reference< css::sdbc::XConnection >,sal_Bool>      createConnection();

        /** return the corresponding driver for the selected URL
        */
        css::uno::Reference< css::sdbc::XDriver >         getDriver();
        css::uno::Reference< css::sdbc::XDriver >         getDriver(const OUString& _sURL);

        /** returns the data source the dialog is currently working with
        */
        css::uno::Reference< css::beans::XPropertySet > const &  getCurrentDataSource();
        // returns the Url of a database document
        static OUString        getDocumentUrl(SfxItemSet& _rDest);

        void setDataSourceOrName( const css::uno::Any& _rDataSourceOrName );

        /** extracts the connection type from the given set<p/>
            The connection type is determined by the value of the DSN item, analyzed by the TypeCollection item.
        */
        static OUString getDatasourceType( const SfxItemSet& _rSet );

        /** returns the connection URL
            @return
                The connection URL
        */
        OUString getConnectionURL() const;

        /// fill the necessary information from the url line
        static void convertUrl(SfxItemSet& _rDest);

        const MapInt2String& getIndirectProperties() const { return m_aIndirectPropTranslator; }

        /** translates properties of an UNO data source into SfxItems
            @param  _rxSource
                The data source
            @param  _rDest
                The item set to fill.
        */
        void translateProperties(
                const css::uno::Reference< css::beans::XPropertySet >& _rxSource,
                SfxItemSet& _rDest);

        /** translate SfxItems into properties of an UNO data source
            @param  _rSource
                The item set to read from.
            @param  _rxDest
                The data source to fill.
        */
        void translateProperties(
                const SfxItemSet& _rSource,
                const css::uno::Reference< css::beans::XPropertySet >& _rxDest);

        bool saveChanges(const SfxItemSet& _rSource);
    protected:
        /** fill a data source info array with the settings from a given item set
        */
        void fillDatasourceInfo(const SfxItemSet& _rSource, css::uno::Sequence< css::beans::PropertyValue >& _rInfo);

        /// translate the given value into an SfxPoolItem, put this into the given set under the given id
        void        implTranslateProperty(SfxItemSet& _rSet, sal_Int32  _nId, const css::uno::Any& _rValue);

        /// translate the given SfxPoolItem into an <type scope="com.sun.star.Any">uno</type>
        static css::uno::Any implTranslateProperty(const SfxPoolItem* _pItem);

        /// translate the given SfxPoolItem into an <type scope="com.sun.star.Any">uno</type>, set it (under the given name) on the given property set
        static void implTranslateProperty(const css::uno::Reference< css::beans::XPropertySet >& _rxSet, const OUString& _rName, const SfxPoolItem* _pItem);

        /** check if the data source described by the given set needs authentication<p/>
            The return value depends on the data source type only.
        */
        static bool hasAuthentication(const SfxItemSet& _rSet);

        OString translatePropertyId( sal_Int32 _nId );
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DBADMINIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
