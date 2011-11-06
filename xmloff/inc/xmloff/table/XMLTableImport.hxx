/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef XMLTABLEIMPORT_HXX_
#define XMLTABLEIMPORT_HXX_

#include <com/sun/star/table/XColumnRowRange.hpp>

#include "xmloff/dllapi.h"
#include "xmloff/xmlictxt.hxx"
#include "xmloff/uniref.hxx"
#include "xmloff/xmlimppr.hxx"
#include "xmloff/prhdlfac.hxx"

#include <rtl/ref.hxx>

#include <boost/shared_ptr.hpp>
#include <map>

class SvXMLStyleContext;

typedef std::map< ::rtl::OUString, ::rtl::OUString > XMLTableTemplate;
typedef std::map < ::rtl::OUString, boost::shared_ptr< XMLTableTemplate > > XMLTableTemplateMap;

class XMLTableImport : public UniRefBase
{
    friend class XMLTableImportContext;

public:
    XMLTableImport( SvXMLImport& rImport, const rtl::Reference< XMLPropertySetMapper >& xCellPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef );
    virtual ~XMLTableImport();

    SvXMLImportContext* CreateTableContext( sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::table::XColumnRowRange >& xColumnRowRange );

    SvXMLStyleContext* CreateTableTemplateContext( sal_uInt16 nPrfx, const ::rtl::OUString& rLName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    rtl::Reference< SvXMLImportPropertyMapper > GetCellImportPropertySetMapper() const { return mxCellImportPropertySetMapper; }
    rtl::Reference< SvXMLImportPropertyMapper > GetRowImportPropertySetMapper() const { return mxRowImportPropertySetMapper; }
    rtl::Reference< SvXMLImportPropertyMapper > GetColumnImportPropertySetMapper() const { return mxColumnImportPropertySetMapper; }

    void addTableTemplate( const rtl::OUString& rsStyleName, XMLTableTemplate& xTableTemplate );
    void finishStyles();

private:
    SvXMLImport&                                 mrImport;
    rtl::Reference< SvXMLImportPropertyMapper > mxCellImportPropertySetMapper;
    rtl::Reference< SvXMLImportPropertyMapper > mxRowImportPropertySetMapper;
    rtl::Reference< SvXMLImportPropertyMapper > mxColumnImportPropertySetMapper;

    XMLTableTemplateMap                         maTableTemplates;
};

#endif /*XMLTABLEIMPORT_HXX_*/
