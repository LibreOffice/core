/*************************************************************************
 *
 *  $RCSfile: xmlComponent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:19:00 $
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
#ifndef DBA_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
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

OXMLComponent::OXMLComponent( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const ::rtl::OUString& _sComponentServiceName
                ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_bAsTemplate(sal_False)
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetComponentElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    sal_Bool bAutoEnabled = sal_False;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_HREF:
                m_sHREF = sValue;
                break;
            case XML_TOK_COMPONENT_NAME:
                m_sName = sValue;
                break;
            case XML_TOK_AS_TEMPLATE:
                m_bAsTemplate = (sValue == s_sTRUE ? sal_True : sal_False);
                break;
        }
    }
    if ( m_sHREF.getLength() && m_sName.getLength() && _xParentContainer.is() )
    {
        Sequence< Any > aArguments(3);
        PropertyValue aValue;
        // set as folder
        aValue.Name = PROPERTY_NAME;
        aValue.Value <<= m_sName;
        aArguments[0] <<= aValue;

        aValue.Name = PROPERTY_PERSISTENT_NAME;
        sal_Int32 nIndex = m_sHREF.lastIndexOf('/')+1;
        aValue.Value <<= m_sHREF.getToken(0,'/',nIndex);
        aArguments[1] <<= aValue;

        aValue.Name = PROPERTY_AS_TEMPLATE;
        aValue.Value <<= m_bAsTemplate;
        aArguments[2] <<= aValue;

        try
        {

            Reference<XMultiServiceFactory> xORB(_xParentContainer,UNO_QUERY);
            if ( xORB.is() )
            {
                Reference<XInterface> xComponent = xORB->createInstanceWithArguments(_sComponentServiceName,aArguments);
                Reference<XNameContainer> xNameContainer(_xParentContainer,UNO_QUERY);
                if ( xNameContainer.is() )
                    xNameContainer->insertByName(m_sName,makeAny(xComponent));
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"OXMLComponent::OXMLComponent -> exception catched");
        }
    }
}
// -----------------------------------------------------------------------------

OXMLComponent::~OXMLComponent()
{
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
