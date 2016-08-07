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
#include "xmlComponent.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlHelper.hxx"
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#include "xmlStyleImport.hxx"
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>
#include <comphelper/namecontainer.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/report/XReportControlModel.hpp>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#include <tools/debug.hxx>

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;
    DBG_NAME( rpt_OXMLComponent )
OXMLComponent::OXMLComponent( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XReportComponent > & _xComponent
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLComponent,NULL);
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetComponentElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);

    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        try
        {
            ::rtl::OUString sLocalName;
            const ::rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const ::rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_NAME:
                    m_xComponent->setName(sValue);
                    break;
                case XML_TOK_TEXT_STYLE_NAME:
                    m_sTextStyleName = sValue;
                    break;
                case XML_TOK_TRANSFORM:
                    break;
                default:
                    break;
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exception catched while putting props into report component!");
        }
    }
}
// -----------------------------------------------------------------------------

OXMLComponent::~OXMLComponent()
{
    DBG_DTOR( rpt_OXMLComponent,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
