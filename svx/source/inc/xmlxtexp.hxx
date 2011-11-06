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



#ifndef _SVX_XMLXTEXP_HXX
#define _SVX_XMLXTEXP_HXX

#include "xmloff/xmlexp.hxx"

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace container { class XNameContainer; }
    namespace document { class XGraphicObjectResolver; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }

class SvxXMLXTableExportComponent : public SvXMLExport
{
public:
    // #110680#
    SvxXMLXTableExportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const rtl::OUString& rFileName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & xHandler,
        const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & xTable,
        com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);

    ~SvxXMLXTableExportComponent();

    static sal_Bool save( const rtl::OUString& rURL, const com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xTable ) throw();

    sal_Bool exportTable() throw();

    // methods without content:
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

private:
    const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > & mxTable;
};

#endif

