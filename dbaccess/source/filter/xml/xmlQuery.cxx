/*************************************************************************
 *
 *  $RCSfile: xmlQuery.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:22:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBA_XMLQUERY_HXX
#include "xmlQuery.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

namespace dbaxml
{
    using namespace ::rtl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;


OXMLQuery::OXMLQuery( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                ) :
    OXMLTable( rImport, nPrfx, _sLocalName,_xAttrList,_xParentContainer,SERVICE_SDB_COMMAND_DEFINITION )
        ,m_bEscapeProcessing(sal_True)
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetQueryElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    sal_Bool bAutoEnabled = sal_False;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMMAND:
                m_sCommand = sValue;
                break;
            case XML_TOK_ESCAPE_PROCESSING:
                m_bEscapeProcessing = sValue.equalsAscii("true");
                break;
        }
    }
}
// -----------------------------------------------------------------------------

OXMLQuery::~OXMLQuery()
{
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLQuery::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext* pContext = OXMLTable::CreateChildContext(nPrefix, rLocalName,xAttrList );
    if ( !pContext )
    {
        const SvXMLTokenMap& rTokenMap = GetOwnImport().GetQueryElemTokenMap();

        switch( rTokenMap.Get( nPrefix, rLocalName ) )
        {
            case XML_TOK_UPDATE_TABLE:
                {
                    GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                    ::rtl::OUString s1;
                    fillAttributes(nPrefix, rLocalName,xAttrList,s1,m_sTable,m_sSchema,m_sCatalog);
                }
                break;
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLQuery::setProperties(Reference< XPropertySet > & _xProp )
{
    try
    {
        if ( _xProp.is() )
        {
            OXMLTable::setProperties(_xProp);

            _xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(m_sCommand));
            _xProp->setPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING,makeAny(m_bEscapeProcessing));

            if ( m_sTable.getLength() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_TABLENAME,makeAny(m_sTable));
            if ( m_sCatalog.getLength() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,makeAny(m_sCatalog));
            if ( m_sSchema.getLength() )
                _xProp->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,makeAny(m_sSchema));

            const ODBFilter::TPropertyNameMap& rSettings = GetOwnImport().getQuerySettings();
            ODBFilter::TPropertyNameMap::const_iterator aFind = rSettings.find(m_sName);
            if ( aFind != rSettings.end() )
                _xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aFind->second));
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"OXMLTable::EndElement -> exception catched");
    }
}
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
