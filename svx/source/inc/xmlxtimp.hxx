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



#ifndef _SVX_XMLXTIMP_HXX
#define _SVX_XMLXTIMP_HXX

#include <xmloff/xmlimp.hxx>

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace document { class XGraphicObjectResolver; }
    namespace container { class XNameContainer; }

} } }

class SvxXMLXTableImport : public SvXMLImport
{
public:
    // #110680#
    SvxXMLXTableImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & rTable,
        com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);

    virtual ~SvxXMLXTableImport() throw ();

    static sal_Bool load( const rtl::OUString& rUrl, const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xTable ) throw();
protected:
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & mrTable;
};

#endif

