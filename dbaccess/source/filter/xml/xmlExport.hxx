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

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLEXPORT_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLEXPORT_HXX

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <osl/diagnose.h>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include "dsntypes.hxx"
#include <comphelper/stl_types.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>

#include <memory>


namespace dbaxml {

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;

#define PROGRESS_BAR_STEP 20


class ODBExport : public SvXMLExport
{
    typedef ::std::map< ::xmloff::token::XMLTokenEnum, OUString> TSettingsMap;

    typedef ::std::pair< OUString ,OUString> TStringPair;
    struct TDelimiter
    {
        OUString sText;
        OUString sField;
        OUString sDecimal;
        OUString sThousand;
        bool            bUsed;

        TDelimiter() : bUsed( false ) { }
    };
    typedef ::std::map< Reference<XPropertySet> ,OUString >          TPropertyStyleMap;
    typedef ::std::map< Reference<XPropertySet> ,Reference<XPropertySet> >  TTableColumnMap;

    struct TypedPropertyValue
    {
        OUString         Name;
        css::uno::Type   Type;
        css::uno::Any    Value;

        TypedPropertyValue( const OUString& _name, const css::uno::Type& _type, const css::uno::Any& _value )
            :Name( _name )
            ,Type( _type )
            ,Value( _value )
        {
        }
    };

    ::std::unique_ptr< TStringPair >                  m_aAutoIncrement;
    ::std::unique_ptr< TDelimiter >                   m_aDelimiter;
    ::std::vector< TypedPropertyValue >             m_aDataSourceSettings;
    ::std::vector< XMLPropertyState >               m_aCurrentPropertyStates;
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

    Reference<XPropertySet>                         m_xDataSource;
    ::dbaccess::ODsnTypeCollection                  m_aTypeCollection;
    bool                                        m_bAllreadyFilled;

    void                    exportDataSource();
    void                    exportConnectionData();
    void                    exportDriverSettings(const TSettingsMap& _aSettings);
    void                    exportApplicationConnectionSettings(const TSettingsMap& _aSettings);
    void                    exportLogin();
    void                    exportSequence(const Sequence< OUString>& _aValue
                                        ,::xmloff::token::XMLTokenEnum _eTokenFilter
                                        ,::xmloff::token::XMLTokenEnum _eTokenType);
    void                    exportDelimiter();
    void                    exportAutoIncrement();
    void                    exportCharSet();
    template< typename T > void exportDataSourceSettingsSequence(
        ::std::vector< TypedPropertyValue >::iterator const & in);
    void                    exportDataSourceSettings();
    void                    exportForms();
    void                    exportReports();
    void                    exportQueries(bool _bExportContext);
    void                    exportTables(bool _bExportContext);
    void                    exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt);
    void                    exportStyleName(const ::xmloff::token::XMLTokenEnum _eToken,const Reference<XPropertySet>& _xProp,SvXMLAttributeList& _rAtt,TPropertyStyleMap& _rMap);
    void                    exportCollection(const Reference< XNameAccess >& _xCollection
                                            ,enum ::xmloff::token::XMLTokenEnum _eComponents
                                            ,enum ::xmloff::token::XMLTokenEnum _eSubComponents
                                            ,bool _bExportContext
                                            ,const ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >& _aMemFunc
                                            );
    void                    exportComponent(XPropertySet* _xProp);
    void                    exportQuery(XPropertySet* _xProp);
    void                    exportTable(XPropertySet* _xProp);
    void                    exportFilter(XPropertySet* _xProp
                                        ,const OUString& _sProp
                                        ,enum ::xmloff::token::XMLTokenEnum _eStatementType);
    void                    exportTableName(XPropertySet* _xProp,bool _bUpdate);
    void                    exportAutoStyle(XPropertySet* _xProp);
    void                    exportColumns(const Reference<XColumnsSupplier>& _xColSup);
    void                    collectComponentStyles();

    static OUString         implConvertAny(const Any& _rValue);

    rtl::Reference < XMLPropertySetMapper > const & GetTableStylesPropertySetMapper() const;

                            ODBExport() = delete;
protected:

    virtual void                    ExportStyles_( bool bUsed ) override;
    virtual void                    ExportAutoStyles_() override;
    virtual void                    ExportContent_() override;
    virtual void                    ExportMasterStyles_() override;
    virtual void                    ExportFontDecls_() override;
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool() override;

    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& aProps) override;

    virtual                 ~ODBExport(){};
public:

    ODBExport(const Reference< XComponentContext >& _rxContext, OUString const & implementationName, SvXMLExportFlags nExportFlag = SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES | SvXMLExportFlags::PRETTY | SvXMLExportFlags::FONTDECLS | SvXMLExportFlags::SCRIPTS );

    static OUString SAL_CALL getImplementationName_Static()
        throw (css::uno::RuntimeException);

    static css::uno::Sequence<OUString> SAL_CALL
    getSupportedServiceNames_Static() throw (css::uno::RuntimeException);

    static css::uno::Reference<css::uno::XInterface> SAL_CALL Create(
        css::uno::Reference<css::lang::XMultiServiceFactory> const & _rxORB);

    rtl::Reference < XMLPropertySetMapper > const & GetColumnStylesPropertySetMapper() const;
    rtl::Reference < XMLPropertySetMapper > const & GetCellStylesPropertySetMapper() const;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    const Reference<XPropertySet>& getDataSource() const { return m_xDataSource; }
};

} // dbaxml
#endif // INCLUDED_DBACCESS_SOURCE_FILTER_XML_XMLEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
