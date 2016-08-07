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
#include "xmlFormatCondition.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlEnums.hxx"
#include "xmlHelper.hxx"
#include <com/sun/star/report/XReportControlFormat.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#include "xmlStyleImport.hxx"
#include <tools/debug.hxx>

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
    using namespace ::com::sun::star::beans;

DBG_NAME( rpt_OXMLFormatCondition )

OXMLFormatCondition::OXMLFormatCondition( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,const Reference< XFormatCondition > & _xComponent ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
,m_rImport(rImport)
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLFormatCondition,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetFormatElemTokenMap();
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
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
                case XML_TOK_ENABLED:
                    m_xComponent->setEnabled(sValue == s_sTRUE);
                    break;
                case XML_TOK_FORMULA:
                    m_xComponent->setFormula(ORptFilter::convertFormula(sValue));
                    break;
                case XML_TOK_FORMAT_STYLE_NAME:
                    m_sStyleName = sValue;
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------

OXMLFormatCondition::~OXMLFormatCondition()
{

    DBG_DTOR( rpt_OXMLFormatCondition,NULL);
}
// -----------------------------------------------------------------------------
void OXMLFormatCondition::EndElement()
{
    OXMLHelper::copyStyleElements(m_rImport.isOldFormat(),m_sStyleName,GetImport().GetAutoStyles(),m_xComponent.get());
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
