/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <optional>
#include <utility>
#include <xmloff/maptype.hxx>
#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlexppr.hxx>
#include <dsntypes.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>

#include <memory>

namespace dbaxml {

class ODBExport : public SvXMLExport
{
    typedef std::map< ::xmloff::token::XMLTokenEnum, OUString> TSettingsMap;

    typedef std::pair< OUString ,OUString> TStringPair;
    struct TDelimiter
    {
        OUString sText;
        OUString sField;
        OUString sDecimal;
        OUString sThousand;
        bool            bUsed;

        TDelimiter() : bUsed( false ) { }
    };
    typedef std::map< css::uno::Reference<css::beans::XPropertySet> ,OUString >          TPropertyStyleMap;
    typedef std::map< css::uno::Reference<css::beans::XPropertySet> ,css::uno::Reference<css::beans::XPropertySet> >  TTableColumnMap;

    struct TypedPropertyValue
    {
        OUString         Name;
        css::uno::Type   Type;
        cpo::uno::Any    Value;

        TypedPropertyValue( OUString _name, const css::uno::Type& _type, cpo::uno::Any _value )
            :Name(std::move( _name ))
            ,Type( _type )
            ,Value(std::move( _value ))
        {
        }
    };

    std::optional< TStringPair >                  m_oAutoIncrement;
    std::unique_ptr< TDelimiter >                   m_aDelimiter;
    std::vector< TypedPropertyValue >             m_aDataSourceSettings;
    std::vector< XMLPropertyState >               m_aCurrentPropertyStates;
    TPropertyStyleMap                               m_aAutoStyleNames;
    TPropertyStyleMap                               m_aCellAutoStyleNames;
    TPropertyStyleMap                               m_aRowAutoStyleNames;
    TTableColumnMap                                 m_aTableDummyColumns;
    OUString                                 m_sCharSet;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xExportHelper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xColumnExportHelper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xCellExportHelper;
    rtl::Reference < SvXMLExportPropertyMapper>       m_xRowExportHelper;

    mutable rtl::Reference < XMLPropertySetMapper >   m_xTableStylesPropertySetMapper;
    mutable rtl::Reference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
    mutable rtl::Reference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;

    css::uno::Reference<css::beans::XPropertySet>     m_xDataSource;
    ::dbaccess::ODsnTypeCollection                  m_aTypeCollection;
    bool                                        m_bAllreadyFilled;

    void                    exportDataSource();
    void                    exportConnectionData();
    void                    exportDriverSettings(const TSettingsMap& _aSettings);
    void                    exportApplicationConnectionSettings(const TSettingsMap& _aSettings);
    void                    exportLogin();
    void                    exportSequence(const css::uno::Sequence< OUString>& _aValue
                                        ,::xmloff::token::XMLTokenEnum _eTokenFilter
                                        ,::xmloff::token::XMLTokenEnum _eTokenType);
    void                    exportDelimiter();
    void                    exportAutoIncrement();
    void                    exportCharSet();
    template< typename T > void exportDataSourceSettingsSequence(
        std::vector< TypedPropertyValue >::iterator const & in);
    void                    exportDataSourceSettings();
    void                    exportForms();
    void                    exportReports();
    void                    exportQueries(bool _bExportContext);
    void                    exportTables(bool _bExportContext);
    void                    exportStyleName(css::beans::XPropertySet* _xProp,comphelper::AttributeList& _rAtt);
    void                    exportStyleName(const ::xmloff::token::XMLTokenEnum _eToken,const css::uno::Reference<css::beans::XPropertySet>& _xProp,comphelper::AttributeList& _rAtt,TPropertyStyleMap& _rMap);
    void                    exportCollection(const css::uno::Reference< css::container::XNameAccess >& _xCollection
                                            ,enum ::xmloff::token::XMLTokenEnum _eComponents
                                            ,enum ::xmloff::token::XMLTokenEnum _eSubComponents
                                            ,bool _bExportContext
                                            ,const ::comphelper::mem_fun1_t<ODBExport,css::beans::XPropertySet* >& _aMemFunc
                                            );
    void                    exportComponent(css::beans::XPropertySet* _xProp);
    void                    exportQuery(css::beans::XPropertySet* _xProp);
    void                    exportTable(css::beans::XPropertySet* _xProp);
    void                    exportFilter(css::beans::XPropertySet* _xProp
                                        ,const OUString& _sProp
                                        ,enum ::xmloff::token::XMLTokenEnum _eStatementType);
    void                    exportTableName(css::beans::XPropertySet* _xProp,bool _bUpdate);
    void                    exportAutoStyle(css::beans::XPropertySet* _xProp);
    void                    exportColumns(const css::uno::Reference<css::sdbcx::XColumnsSupplier>& _xColSup);
    void                    collectComponentStyles();

    static OUString         implConvertAny(const cpo::uno::Any& _rValue);

    rtl::Reference < XMLPropertySetMapper > const & GetTableStylesPropertySetMapper() const;

                            ODBExport() = delete;
protected:

    virtual void                    ExportAutoStyles_() override;
    virtual void                    ExportContent_() override;
    virtual void                    ExportMasterStyles_() override;
    virtual void                    ExportFontDecls_() override;
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool() override;

    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;

    virtual                 ~ODBExport() override {};
public:

    ODBExport(const css::uno::Reference< css::uno::XComponentContext >& _rxContext, OUString const & implementationName, SvXMLExportFlags nExportFlag = SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::PRETTY | SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::SCRIPTS );

    rtl::Reference < XMLPropertySetMapper > const & GetColumnStylesPropertySetMapper() const;
    rtl::Reference < XMLPropertySetMapper > const & GetCellStylesPropertySetMapper() const;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    const css::uno::Reference<css::beans::XPropertySet>& getDataSource() const { return m_xDataSource; }
};

} // dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
