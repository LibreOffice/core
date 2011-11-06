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



#ifndef XMLOFF_FORMSTYLES_HXX
#define XMLOFF_FORMSTYLES_HXX

#include <xmloff/prstylei.hxx>

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlStyleContext
    //=====================================================================
    class OControlStyleContext : public XMLPropStyleContext
    {
    protected:
        ::rtl::OUString     m_sNumberStyleName;

    public:
        OControlStyleContext(
            SvXMLImport& _rImport,
            const sal_uInt16 _Prefix,
            const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList,
            SvXMLStylesContext& _rParentStyles,
            const sal_uInt16 _nFamily,
            const sal_Bool _bDefaultStyle
        );

        inline const ::rtl::OUString& getNumberStyleName( ) const { return m_sNumberStyleName; }

    protected:
        virtual void SetAttribute(
            sal_uInt16 _nPrefixKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue
        );
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // XMLOFF_FORMSTYLES_HXX

