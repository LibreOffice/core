/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlDocuments.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:43:34 $
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
