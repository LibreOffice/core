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
#include "precompiled_dbaxml.hxx"
#ifndef DBA_XMLCOLUMN_HXX
#include "xmlColumn.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#include <com/sun/star/container/XChild.hpp>
#ifndef DBA_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLColumn)

OXMLColumn::OXMLColumn( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const Reference< XPropertySet >& _xTable
                ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
    ,m_xTable(_xTable)
    ,m_bHidden(sal_False)
{
    DBG_CTOR(OXMLColumn,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetColumnElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    ::rtl::OUString sType;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COLUMN_NAME:
                m_sName = sValue;
                break;
            case XML_TOK_COLUMN_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_COLUMN_HELP_MESSAGE:
                m_sHelpMessage = sValue;
                break;
            case XML_TOK_COLUMN_VISIBILITY:
                m_bHidden = !sValue.equalsAscii("visible");
                break;
            case XML_TOK_COLUMN_TYPE_NAME:
                sType = sValue;
                OSL_ENSURE(sType.getLength(),"No type name set");
                break;
            case XML_TOK_COLUMN_DEFAULT_VALUE:
                if ( sValue.getLength() && sType.getLength() )
                    m_aDefaultValue <<= sValue;
                //    SvXMLUnitConverter::convertAny(m_aDefaultValue,sType,sValue);
                break;
            case XML_TOK_COLUMN_VISIBLE:
                m_bHidden = sValue.equalsAscii("false");
                break;
            case XML_TOK_DEFAULT_CELL_STYLE_NAME:
                m_sCellStyleName = sValue;
                break;
        }
    }
}
// -----------------------------------------------------------------------------

OXMLColumn::~OXMLColumn()
{

    DBG_DTOR(OXMLColumn,NULL);
}
// -----------------------------------------------------------------------------
void OXMLColumn::EndElement()
{
    Reference<XDataDescriptorFactory> xFac(m_xParentContainer,UNO_QUERY);
    if ( xFac.is() && m_sName.getLength() )
    {
        Reference<XPropertySet> xProp(xFac->createDataDescriptor());
        if ( xProp.is() )
        {
            xProp->setPropertyValue(PROPERTY_NAME,makeAny(m_sName));
            xProp->setPropertyValue(PROPERTY_HIDDEN,makeAny(m_bHidden));
            if ( m_sHelpMessage.getLength() )
                xProp->setPropertyValue(PROPERTY_HELPTEXT,makeAny(m_sHelpMessage));

            if ( m_aDefaultValue.hasValue() )
                xProp->setPropertyValue(PROPERTY_CONTROLDEFAULT,m_aDefaultValue);

            Reference<XAppend> xAppend(m_xParentContainer,UNO_QUERY);
            if ( xAppend.is() )
                xAppend->appendByDescriptor(xProp);
            m_xParentContainer->getByName(m_sName) >>= xProp;

            if ( m_sStyleName.getLength() )
            {
                const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
                if ( pAutoStyles )
                {
                    OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_COLUMN,m_sStyleName));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(xProp);
                    }
                }
            } // if ( m_sStyleName.getLength() )
            if ( m_sCellStyleName.getLength() )
            {
                const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
                if ( pAutoStyles )
                {
                    OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,m_sCellStyleName));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(xProp);
                        // we also have to do this on the table to import text-properties
                        pAutoStyle->FillPropertySet(m_xTable);
                    }
                }
            }

        }
    } // if ( xFac.is() && m_sName.getLength() )
    else if ( m_sCellStyleName.getLength() )
    {
        const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
        if ( pAutoStyles )
        {
            OTableStyleContext* pAutoStyle = PTR_CAST(OTableStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,m_sCellStyleName));
            if ( pAutoStyle )
            {
                // we also have to do this on the table to import text-properties
                pAutoStyle->FillPropertySet(m_xTable);
            }
        }
    }
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLColumn::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
