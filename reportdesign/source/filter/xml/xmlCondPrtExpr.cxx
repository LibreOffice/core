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


#include "precompiled_rptxml.hxx"
#include "xmlCondPrtExpr.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlControlProperty.hxx"
#include "xmlComponent.hxx"
#ifndef REPORTDESIGN_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#include <tools/debug.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLCondPrtExpr )

OXMLCondPrtExpr::OXMLCondPrtExpr( ORptFilter& _rImport,
                sal_uInt16 nPrfx
                ,const ::rtl::OUString& rLName
                ,const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const Reference< XPropertySet > & _xComponent ) :
    SvXMLImportContext( _rImport, nPrfx, rLName )
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLCondPrtExpr,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetFunctionElemTokenMap();
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_FUNCTION_FORMULA:
                    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,uno::makeAny(ORptFilter::convertFormula(sValue)));
                    break;
                default:
                    break;
            }

        }
    }
    catch(const Exception&)
    {
        OSL_ENSURE(0,"Exception catched while putting Function props!");
    }
}
// -----------------------------------------------------------------------------

OXMLCondPrtExpr::~OXMLCondPrtExpr()
{

    DBG_DTOR( rpt_OXMLCondPrtExpr,NULL);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void OXMLCondPrtExpr::Characters( const ::rtl::OUString& rChars )
{
    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,makeAny(rChars));
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
