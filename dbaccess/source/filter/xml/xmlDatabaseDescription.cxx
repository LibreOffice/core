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
#ifndef DBA_XMLDATABASEDESCRIPTION_HXX_INCLUDED
#include "xmlDatabaseDescription.hxx"
#endif
#ifndef DBA_XMLLOGIN_HXX
#include "xmlLogin.hxx"
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
#include "xmlEnums.hxx"
#include "xmlFileBasedDatabase.hxx"
#include "xmlServerDatabase.hxx"
#include "xmlstrings.hrc"
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDatabaseDescription)

OXMLDatabaseDescription::OXMLDatabaseDescription( ODBFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_bFoundOne(false)
{
    DBG_CTOR(OXMLDatabaseDescription,NULL);
}
// -----------------------------------------------------------------------------

OXMLDatabaseDescription::~OXMLDatabaseDescription()
{

    DBG_DTOR(OXMLDatabaseDescription,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLDatabaseDescription::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDatabaseDescriptionElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_FILE_BASED_DATABASE:
            if ( !m_bFoundOne )
            {
                m_bFoundOne = true;
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFileBasedDatabase( GetOwnImport(), nPrefix, rLocalName,xAttrList );
            }
            break;
        case XML_TOK_SERVER_DATABASE:
            if ( !m_bFoundOne )
            {
                m_bFoundOne = true;
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLServerDatabase( GetOwnImport(), nPrefix, rLocalName,xAttrList );
            }
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLDatabaseDescription::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
