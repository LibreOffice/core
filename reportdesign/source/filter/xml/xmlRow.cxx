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

#ifndef RPT_XMLCOLUMN_HXX
#include "xmlColumn.hxx"
#endif
#include <xmloff/xmluconv.hxx>
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include <tools/debug.hxx>
#include "xmlStyleImport.hxx"
#include <comphelper/namecontainer.hxx>
#include <comphelper/genericpropertyset.hxx>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME( rpt_OXMLRow )

OXMLRow::OXMLRow( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLTable* _pContainer) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
{
    DBG_CTOR( rpt_OXMLRow,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetColumnTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
     ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COLUMN_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_NUMBER_ROWS_SPANNED:
                m_pContainer->setRowSpanned(sValue.toInt32());
                break;
            default:
                break;
        }
    }

}
// -----------------------------------------------------------------------------

OXMLRow::~OXMLRow()
{
    DBG_DTOR( rpt_OXMLRow,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLRow::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetControlPropertyElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_COLUMN:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_ROW:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_CELL:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_COV_CELL:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRow( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLRow::EndElement()
{
    if ( m_sStyleName.getLength() )
    {
        const SvXMLStylesContext* pAutoStyles = GetImport().GetAutoStyles();
        if ( pAutoStyles )
        {
            PropertySetInfo* pInfo = new PropertySetInfo();
            static PropertyMapEntry pMap[] =
            {
                {PROPERTY_WIDTH,        static_cast<sal_uInt16>(PROPERTY_WIDTH.length),     0,          &::getCppuType(static_cast< sal_Int32* >( NULL ))       ,PropertyAttribute::BOUND,0},
                {PROPERTY_HEIGHT,       static_cast<sal_uInt16>(PROPERTY_HEIGHT.length),    0,          &::getCppuType(static_cast< sal_Int32* >( NULL ))       ,PropertyAttribute::BOUND,0},
                { NULL, 0, 0, NULL, 0, 0 }
            };
            pInfo->add(pMap);
            Reference<XPropertySet> xProp = GenericPropertySet_CreateInstance(pInfo);
            XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_COLUMN,m_sStyleName));
            if ( pAutoStyle )
            {
                pAutoStyle->FillPropertySet(xProp);
                sal_Int32 nWidth = 0;
                xProp->getPropertyValue(PROPERTY_WIDTH) >>= nWidth;
                m_pContainer->addWidth(nWidth);
            }
            else
            {
                pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_ROW,m_sStyleName));
                if ( pAutoStyle )
                {
                    pAutoStyle->FillPropertySet(xProp);
                    sal_Int32 nHeight = 0;
                    xProp->getPropertyValue(PROPERTY_HEIGHT) >>= nHeight;
                    m_pContainer->addHeight(nHeight);
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLRow::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
