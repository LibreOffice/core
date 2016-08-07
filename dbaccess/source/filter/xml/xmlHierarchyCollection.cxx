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
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLHierarchyCollection)

OXMLHierarchyCollection::OXMLHierarchyCollection( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
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
        ::rtl::OUString sLocalName;
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
                if ( xNameContainer.is() && !xNameContainer->hasByName(m_sName) )
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
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XNameAccess >& _xContainer
                ,const Reference< XPropertySet >& _xTable
            ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_xContainer(_xContainer)
    ,m_xTable(_xTable)
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
        const ::rtl::OUString& rLocalName,
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
            pContext = new OXMLColumn( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_xTable);
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
