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



#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#define _XMLOFF_XMLSHAPESTYLECONTEXT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/prstylei.hxx>
#include <xmloff/families.hxx>

class SvXMLImport;

//////////////////////////////////////////////////////////////////////////////
// style:style context

class XMLOFF_DLLPUBLIC XMLShapeStyleContext: public XMLPropStyleContext
{
protected:
    ::rtl::OUString     m_sControlDataStyleName;
    ::rtl::OUString     m_sListStyleName;
    sal_Bool            m_bIsNumRuleAlreadyConverted;
    //UUUU
    sal_Bool            m_bIsFillStyleAlreadyConverted;

protected:
    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const ::rtl::OUString& rLocalName,
                               const ::rtl::OUString& rValue );
public:
    TYPEINFO();

    XMLShapeStyleContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList,
        SvXMLStylesContext& rStyles,
        sal_uInt16 nFamily = XML_STYLE_FAMILY_SD_GRAPHICS_ID);
    virtual ~XMLShapeStyleContext();

    SvXMLImportContext * CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Finish( sal_Bool bOverwrite );

    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );
};

#endif  //  _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
