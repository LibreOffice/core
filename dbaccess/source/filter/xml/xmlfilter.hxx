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

#ifndef DBA_XMLFILTER_HXX
#define DBA_XMLFILTER_HXX

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
#include <xmloff/xmlimp.hxx>
#include "apitools.hxx"
#include <comphelper/stl_types.hxx>

#include <memory>

namespace dbaxml
{
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;

// - ODBFilter -
class ODBFilter : public SvXMLImport
{
public:
    DECLARE_STL_USTRINGACCESS_MAP(Sequence<PropertyValue>,TPropertyNameMap);
    typedef ::std::vector< ::com::sun::star::beans::PropertyValue> TInfoSequence;
private:
    TPropertyNameMap                                m_aQuerySettings;
    TPropertyNameMap                                m_aTablesSettings;
    TInfoSequence                                   m_aInfoSequence;
    Reference< XComponent >                         m_xSrcDoc;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDocElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDatabaseElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDataSourceElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pLoginElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDatabaseDescriptionElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDataSourceInfoElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDocumentsElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pComponentElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pQueryElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pColumnElemTokenMap;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    mutable UniReference < XMLPropertySetMapper >   m_xTableStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xCellStylesPropertySetMapper;
    Reference<XPropertySet>                         m_xDataSource;
    sal_Int32                                       m_nPreviewMode;
    bool                                            m_bNewFormat;

    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);

    /** fills the map with the Properties
        @param  _rValue
            The Any where the sequence resists in.
        @param  _rMap
            The map to fill.
    */
    void fillPropertyMap(const Any& _rValue,TPropertyNameMap& _rMap);

    SvXMLImportContext* CreateStylesContext(sal_uInt16 nPrefix,const OUString& rLocalName,
                                     const Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle );
    SvXMLImportContext* CreateScriptContext( const OUString& rLocalName );

protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual ~ODBFilter()  throw();
public:

    ODBFilter( const Reference< XComponentContext >& _rxContext );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);

    // XServiceInfo
    DECLARE_SERVICE_INFO_STATIC( );

    // helper class
    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps);

    inline Reference<XPropertySet> getDataSource() const { return m_xDataSource; }

    inline const TPropertyNameMap& getQuerySettings() const { return m_aQuerySettings;}
    inline const TPropertyNameMap& getTableSettings() const { return m_aTablesSettings;}

    const SvXMLTokenMap& GetDocElemTokenMap() const;
    const SvXMLTokenMap& GetDatabaseElemTokenMap() const;
    const SvXMLTokenMap& GetDataSourceElemTokenMap() const;
    const SvXMLTokenMap& GetLoginElemTokenMap() const;
    const SvXMLTokenMap& GetDatabaseDescriptionElemTokenMap() const;
    const SvXMLTokenMap& GetDataSourceInfoElemTokenMap() const;
    const SvXMLTokenMap& GetDocumentsElemTokenMap() const;
    const SvXMLTokenMap& GetComponentElemTokenMap() const;
    const SvXMLTokenMap& GetQueryElemTokenMap() const;
    const SvXMLTokenMap& GetColumnElemTokenMap() const;

    UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper() const;
    UniReference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper() const;
    UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper() const;

    /** add a Info to the sequence which will be appened to the data source
        @param  _rInfo The property to append.
    */
    inline void addInfo(const ::com::sun::star::beans::PropertyValue& _rInfo)
    {
        m_aInfoSequence.push_back(_rInfo);
    }

    void setPropertyInfo();

    const ::std::map< sal_uInt16,com::sun::star::beans::Property>& GetDataSourceInfoDefaulValueMap() const;

    inline bool isNewFormat() const { return m_bNewFormat; }
    inline void setNewFormat(bool _bNewFormat) { m_bNewFormat = _bNewFormat; }
};

} // dbaxml
#endif // DBA_XMLFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
