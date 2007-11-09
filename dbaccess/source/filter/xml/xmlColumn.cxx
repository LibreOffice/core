/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlColumn.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 08:12:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
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
                ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
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
                    SvXMLUnitConverter::convertAny(m_aDefaultValue,sType,sValue);
                break;
            case XML_TOK_COLUMN_VISIBLE:
                m_bHidden = sValue.equalsAscii("false");
                break;
        }
    }
    OSL_ENSURE(m_sName.getLength(),"Invalid column name of length: ZERO");
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
            }
            Reference<XAppend> xAppend(m_xParentContainer,UNO_QUERY);
            if ( xAppend.is() )
                xAppend->appendByDescriptor(xProp);
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
