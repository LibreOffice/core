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
#ifndef DBA_XMLFILEBASEDDATABASE_HXX_INCLUDED
#include "xmlFileBasedDatabase.hxx"
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#include <comphelper/sequence.hxx>
#include <svl/filenotation.hxx>
#include <unotools/pathoptions.hxx>
#include "dsntypes.hxx"
namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLFileBasedDatabase)

OXMLFileBasedDatabase::OXMLFileBasedDatabase( ODBFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    DBG_CTOR(OXMLFileBasedDatabase,NULL);

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceElemTokenMap();

    Reference<XPropertySet> xDataSource = rImport.getDataSource();

    PropertyValue aProperty;

    const sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    ::rtl::OUString sLocation,sMediaType,sFileTypeExtension;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name = ::rtl::OUString();
        aProperty.Value = Any();

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_DB_HREF:
                {
                    SvtPathOptions aPathOptions;
                    rtl::OUString sFileName = aPathOptions.SubstituteVariable(sValue);
                    if ( sValue == sFileName )
                    {
                        const sal_Int32 nFileNameLength = sFileName.getLength();
                        if ( ( nFileNameLength > 0 ) && ( sFileName.getStr()[ nFileNameLength - 1 ] == '/' ) )
                            sFileName = sFileName.copy( 0, nFileNameLength - 1 );

                        sLocation = ::svt::OFileNotation( rImport.GetAbsoluteReference( sFileName ) ).get( ::svt::OFileNotation::N_SYSTEM );
                    }

                    if ( sLocation.getLength() == 0 )
                        sLocation = sValue;
                }
                break;
            case XML_TOK_MEDIA_TYPE:
                sMediaType = sValue;
                break;
            case XML_TOK_EXTENSION:
                aProperty.Name = INFO_TEXTFILEEXTENSION;
                sFileTypeExtension = sValue;
                break;
        }
        if ( aProperty.Name.getLength() )
        {
            if ( !aProperty.Value.hasValue() )
                aProperty.Value <<= sValue;
            rImport.addInfo(aProperty);
        }
    }
    if ( sLocation.getLength() && sMediaType.getLength() )
    {
        ::dbaccess::ODsnTypeCollection aTypeCollection(rImport.getORB());
        ::rtl::OUString sURL(aTypeCollection.getDatasourcePrefixFromMediaType(sMediaType,sFileTypeExtension));
        sURL += sLocation;
        try
        {
            xDataSource->setPropertyValue(PROPERTY_URL,makeAny(sURL));
        }
        catch(Exception)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------

OXMLFileBasedDatabase::~OXMLFileBasedDatabase()
{

    DBG_DTOR(OXMLFileBasedDatabase,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
