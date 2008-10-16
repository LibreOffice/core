/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlfilter.hxx,v $
 * $Revision: 1.8.2.1 $
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

#ifndef DBA_XMLFILTER_HXX
#define DBA_XMLFILTER_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

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

// -------------
// - ODBFilter -
// -------------
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

    mutable UniReference < XMLPropertySetMapper >   m_xTableStylesPropertySetMapper;
    mutable UniReference < XMLPropertySetMapper >   m_xColumnStylesPropertySetMapper;
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

    SvXMLImportContext* CreateStylesContext(sal_uInt16 nPrefix,const ::rtl::OUString& rLocalName,
                                     const Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle );
    SvXMLImportContext* CreateScriptContext( const ::rtl::OUString& rLocalName );

protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual ~ODBFilter()  throw();
public:

    ODBFilter( const Reference< XMultiServiceFactory >& _rxMSF );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);

    // XServiceInfo
    DECLARE_SERVICE_INFO_STATIC( );

    // helper class
    virtual void SetViewSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aViewProps);
    virtual void SetConfigurationSettings(const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aConfigProps);

    inline Reference< XMultiServiceFactory > getORB() { return getServiceFactory(); }
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

// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
#endif // DBA_XMLFILTER_HXX
