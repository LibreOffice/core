/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlHierarchyCollection.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:51:25 $
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
#ifndef DBA_XMLHIERARCHYCOLLECTION_HXX
#include "xmlHierarchyCollection.hxx"
#endif
#ifndef DBA_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
#endif
#ifndef DBA_XMLQUERY_HXX
#include "xmlQuery.hxx"
#endif
#ifndef DBA_XMLCOLUMN_HXX
#include "xmlColumn.hxx"
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


namespace dbaxml
{
    using namespace ::rtl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLHierarchyCollection)

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XNameAccess >& _xParentContainer
                ,const ::rtl::OUString& _sCollectionServiceName
                ,const ::rtl::OUString& _sComponentServiceName) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xParentContainer(_xParentContainer)
    ,m_sCollectionServiceName(_sCollectionServiceName)
    ,m_sComponentServiceName(_sComponentServiceName)
{
    DBG_CTOR(OXMLHierarchyCollection,NULL);

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetComponentElemTokenMap();

    sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COMPONENT_NAME:
                m_sName = sValue;
                break;
        }
    }
    if ( m_sName.getLength() && _xParentContainer.is() )
    {
        try
        {
            Sequence< Any > aArguments(2);
            PropertyValue aValue;
            // set as folder
            aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
            aValue.Value <<= m_sName;
            aArguments[0] <<= aValue;
            //parent
            aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
            aValue.Value <<= _xParentContainer;
            aArguments[1] <<= aValue;

            Reference<XMultiServiceFactory> xORB(_xParentContainer,UNO_QUERY);
            if ( xORB.is() )
            {
                m_xContainer.set(xORB->createInstanceWithArguments(_sCollectionServiceName,aArguments),UNO_QUERY);
                Reference<XNameContainer> xNameContainer(_xParentContainer,UNO_QUERY);
                if ( xNameContainer.is() )
                    xNameContainer->insertByName(m_sName,makeAny(m_xContainer));
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"OXMLHierarchyCollection::OXMLHierarchyCollection -> exception catched");
        }
    }
}
// -----------------------------------------------------------------------------
OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const OUString& _sLocalName
                ,const Reference< XNameAccess >& _xContainer
            ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xContainer(_xContainer)
{
    DBG_CTOR(OXMLHierarchyCollection,NULL);
}
// -----------------------------------------------------------------------------

OXMLHierarchyCollection::~OXMLHierarchyCollection()
{

    DBG_DTOR(OXMLHierarchyCollection,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLHierarchyCollection::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDocumentsElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
//      case XML_TOK_QUERY:
//          pContext = new OXMLQuery( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer );
//          break;
        case XML_TOK_COMPONENT:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sComponentServiceName );
            break;
        case XML_TOK_COLUMN:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLColumn( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer);
            break;
        //  case XML_TOK_QUERY_COLLECTION:
        case XML_TOK_COMPONENT_COLLECTION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLHierarchyCollection( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sCollectionServiceName,m_sComponentServiceName);
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLHierarchyCollection::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
