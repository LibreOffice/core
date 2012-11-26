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


#ifndef INCLUDED__XMLOFF_CHARCONTEXT_HXX
#define INCLUDED__XMLOFF_CHARCONTEXT_HXX

#include "xmloff/dllapi.h"
#include <com/sun/star/uno/Reference.h>
#include "xmlictxt.hxx"

class XMLOFF_DLLPUBLIC XMLCharContext : public SvXMLImportContext
{
    XMLCharContext(const XMLCharContext&);
    void operator =(const XMLCharContext&);
protected:
    sal_Int16   m_nControl;
    sal_uInt16  m_nCount;
    sal_Unicode m_c;
public:

    XMLCharContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            sal_Bool bCount );
    XMLCharContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Int16 nControl );

    virtual ~XMLCharContext();

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement();

    virtual void InsertControlCharacter(sal_Int16   _nControl);
    virtual void InsertString(const ::rtl::OUString& _sString);
};
// ---------------------------------------------------------------------
#endif // INCLUDED__XMLOFF_CHARCONTEXT_HXX
