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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "formstyles.hxx"
#include <xmloff/xmltoken.hxx>

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace token;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml;

    //=====================================================================
    //= OControlStyleContext
    //=====================================================================
    //---------------------------------------------------------------------
    OControlStyleContext::OControlStyleContext( SvXMLImport& _rImport, const sal_uInt16 _Prefix, const ::rtl::OUString& _rLocalName,
        const Reference< sax::XAttributeList >& _rxAttrList, SvXMLStylesContext& _rParentStyles,
        const sal_uInt16 _nFamily, const sal_Bool _bDefaultStyle )
        :XMLPropStyleContext( _rImport, _Prefix, _rLocalName, _rxAttrList, _rParentStyles, _nFamily, _bDefaultStyle )
    {
    }

    //---------------------------------------------------------------------
    void OControlStyleContext::SetAttribute( sal_uInt16 _nPrefixKey, const ::rtl::OUString& _rLocalName, const ::rtl::OUString& _rValue )
    {
        if ( _rLocalName == GetXMLToken( XML_DATA_STYLE_NAME ) )
            m_sNumberStyleName = _rValue;
        else
            XMLPropStyleContext::SetAttribute( _nPrefixKey, _rLocalName, _rValue );
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................


