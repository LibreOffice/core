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
#ifndef DBA_XMLDOCUMENTS_HXX
#include "xmlDocuments.hxx"
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
#ifndef DBA_XMLQUERY_HXX
#include "xmlQuery.hxx"
#endif
#ifndef DBA_XMLTABLE_HXX
#include "xmlTable.hxx"
#endif
#ifndef DBA_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
#endif
#ifndef DBA_XMLHIERARCHYCOLLECTION_HXX
#include "xmlHierarchyCollection.hxx"
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDocuments)

OXMLDocuments::OXMLDocuments( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                , const ::rtl::OUString& rLName
                ,const Reference< XNameAccess >& _xContainer
                ,const ::rtl::OUString& _sCollectionServiceName
                ,const ::rtl::OUString& _sComponentServiceName) :
    SvXMLImportContext( rImport, nPrfx, rLName )
        ,m_xContainer(_xContainer)
        ,m_sCollectionServiceName(_sCollectionServiceName)
        ,m_sComponentServiceName(_sComponentServiceName)
{
    DBG_CTOR(OXMLDocuments,NULL);

}
// -----------------------------------------------------------------------------
OXMLDocuments::OXMLDocuments( ODBFilter& rImport
                ,sal_uInt16 nPrfx
                , const ::rtl::OUString& rLName
                ,const Reference< XNameAccess >& _xContainer
                ,const ::rtl::OUString& _sCollectionServiceName
                ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
        ,m_xContainer(_xContainer)
        ,m_sCollectionServiceName(_sCollectionServiceName)
{
    DBG_CTOR(OXMLDocuments,NULL);
}
// -----------------------------------------------------------------------------

OXMLDocuments::~OXMLDocuments()
{

    DBG_DTOR(OXMLDocuments,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLDocuments::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDocumentsElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_TABLE:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTable( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,SERVICE_SDB_TABLEDEFINITION);
            break;
        case XML_TOK_QUERY:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLQuery( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer );
            break;
        case XML_TOK_COMPONENT:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLComponent( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sComponentServiceName );
            break;
        //  case XML_TOK_QUERY_COLLECTION:
        case XML_TOK_COMPONENT_COLLECTION:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLHierarchyCollection( GetOwnImport(), nPrefix, rLocalName,xAttrList,m_xContainer,m_sCollectionServiceName,m_sComponentServiceName );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );


    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLDocuments::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
