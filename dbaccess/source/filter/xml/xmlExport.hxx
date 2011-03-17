/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef DBA_XMLEXPORT_HXX
#define DBA_XMLEXPORT_HXX

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
#include "apitools.hxx"
#include "dsntypes.hxx"
#include <comphelper/stl_types.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>

#include <memory>

namespace dbaxml
{
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
// -------------
// - ODBExport -
// -------------
#define PROGRESS_BAR_STEP 20

class ODBExport : public SvXMLExport
{
    typedef ::std::map< ::xmloff::token::XMLTokenEnum, ::rtl::OUString> TSettingsMap;

    typedef ::std::pair< ::rtl::OUString ,::rtl::OUString> TStringPair;
    struct TDelimiter
    {
        ::rtl::OUString sText;
        ::rtl::OUString sField;
        ::rtl::OUString sDecimal;
        ::rtl::OUString sThousand;
        bool            bUsed;

        TDelimiter() : bUsed( false ) { }
    };
    typedef ::std::map< Reference<XPropertySet> ,::rtl::OUString >          TPropertyStyleMap;
    typedef ::std::map< Reference<XPropertySet> ,Reference<XPropertySet> >  TTableColumnMap;

    struct TypedPropertyValue
    {
        ::rtl::OUString               Name;
        ::com::sun::star::uno::Type   Type;
        ::com::sun::star::uno::Any    Value;

        TypedPropertyValue( const ::rtl::OUString& _name, const ::com::sun::star::uno::Type& _type, const ::com::sun::star::uno::Any& _value )
            :Name( _name )
            ,Type( _type )
            ,Value( _value )
        {
        }
    };

    ::std::auto_ptr< TStringPair >                  m_aAutoIncrement;
    ::std::auto_ptr< TDelimiter >                   m_aDelimiter;
    ::std::vector< TypedPropertyValue >             m_aDataSourceSettings;
    ::std::vector< XMLPropertyState >               m_aCurrentPropertyStates;
    TPropertyStyleMap                               m_aAutoStyleNames;
    TPropertyStyleMap                               m_aCellAutoStyleNames;
    TPropertyStyleMap                               m_aRowAutoStyleNames;
    TTableColumnMap                                 m_aTableDummyColumns;
    ::rtl::OUString                                 m_sCharSet;
    UniReference < SvXMLExportPropertyMapper>       m_xExportHelper;
    UniReference < SvXMLExportPropertyMapper>       m_xColumnExportHelper;
    UniReference < SvXMLExportPropertyMapper>       m_xCellExportHelper;
    UniReference < SvXMLExportPropertyMapper>       m_xRowExportHelper;

    mutable UniReference < XMLPropertySetMapper >   m_xTableStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xRowStylesPropertySetMapper;

    Reference<XPropertySet>                         m_xDataSource;
    ::dbaccess::ODsnTypeCollection                  m_aTypeCollection;
    sal_Bool                                        m_bAllreadyFilled;

    void                    exportDataSource();
    void                    exportConnectionData();
    void                    exportDriverSettings(const TSettingsMap& _aSettings);
    void                    exportApplicationConnectionSettings(const TSettingsMap& _aSettings);
    void                    exportLogin();
    void                    exportSequence(const Sequence< ::rtl::OUString>& _aValue
                                        ,::xmloff::token::XMLTokenEnum _eTokenFilter
                                        ,::xmloff::token::XMLTokenEnum _eTokenType);
    void                    exportDelimiter();
    void                    exportAutoIncrement();
    void                    exportCharSet();
    void                    exportDataSourceSettings();
    void                    exportForms();
    void                    exportReports();
    void                    exportQueries(sal_Bool _bExportContext);
    void                    exportTables(sal_Bool _bExportContext);
    void                    exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt);
    void                    exportStyleName(const ::xmloff::token::XMLTokenEnum _eToken,const Reference<XPropertySet>& _xProp,SvXMLAttributeList& _rAtt,TPropertyStyleMap& _rMap);
    void                    exportCollection(const Reference< XNameAccess >& _xCollection
                                            ,enum ::xmloff::token::XMLTokenEnum _eComponents
                                            ,enum ::xmloff::token::XMLTokenEnum _eSubComponents
                                            ,sal_Bool _bExportContext
                                            ,const ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >& _aMemFunc
                                            );
    void                    exportComponent(XPropertySet* _xProp);
    void                    exportQuery(XPropertySet* _xProp);
    void                    exportTable(XPropertySet* _xProp);
    void                    exportFilter(XPropertySet* _xProp
                                        ,const ::rtl::OUString& _sProp
                                        ,enum ::xmloff::token::XMLTokenEnum _eStatementType);
    void                    exportTableName(XPropertySet* _xProp,sal_Bool _bUpdate);
    void                    exportAutoStyle(XPropertySet* _xProp);
    void                    exportColumns(const Reference<XColumnsSupplier>& _xColSup);
    void                    collectComponentStyles();

    ::rtl::OUString         implConvertAny(const Any& _rValue);

    UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper() const;

private:
                            ODBExport();
protected:

    virtual void                    _ExportStyles( sal_Bool bUsed );
    virtual void                    _ExportAutoStyles();
    virtual void                    _ExportContent();
    virtual void                    _ExportMasterStyles();
    virtual void                    _ExportFontDecls();
    virtual sal_uInt32              exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );
    virtual SvXMLAutoStylePoolP*    CreateAutoStylePool();

    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps);

    virtual                 ~ODBExport(){};
public:

    ODBExport(const Reference< XMultiServiceFactory >& _rxMSF, sal_uInt16 nExportFlag = EXPORT_CONTENT | EXPORT_AUTOSTYLES | EXPORT_PRETTY | EXPORT_FONTDECLS | EXPORT_SCRIPTS );
    // XServiceInfo
    DECLARE_SERVICE_INFO_STATIC( );

    UniReference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper() const;
    UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper() const;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    inline Reference<XPropertySet> getDataSource() const { return m_xDataSource; }
};

// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
