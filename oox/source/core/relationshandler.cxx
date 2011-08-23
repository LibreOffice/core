/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/core/relationshandler.hxx"
#include <rtl/ustrbuf.hxx>
#include "tokens.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace core {

// ============================================================================

namespace {

/*  Build path to relations file from passed fragment path, e.g.:
        'path/path/file.xml'    -> 'path/path/_rels/file.xml.rels'
        'file.xml'              -> '_rels/file.xml.rels'
        ''                      -> '_rels/.rels'
 */
OUString lclGetRelationsPath( const OUString& rFragmentPath )
{
    sal_Int32 nPathLen = ::std::max< sal_Int32 >( rFragmentPath.lastIndexOf( '/' ) + 1, 0 );
    return
        OUStringBuffer( rFragmentPath.copy( 0, nPathLen ) ).    // file path including slash
        appendAscii( "_rels/" ).                                // additional '_rels/' path
        append( rFragmentPath.copy( nPathLen ) ).               // file name after path
        appendAscii( ".rels" ).                                 // '.rels' suffix
        makeStringAndClear();
}

} // namespace

// ============================================================================

RelationsFragment::RelationsFragment( XmlFilterBase& rFilter, RelationsRef xRelations ) :
    FragmentHandler( rFilter, lclGetRelationsPath( xRelations->getFragmentPath() ), xRelations ),
    mxRelations( xRelations )
{
}

Reference< XFastContextHandler > RelationsFragment::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( rxAttribs );
    switch( nElement )
    {
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationship:
        {
            Relation aRelation;
            aRelation.maId     = aAttribs.getString( XML_Id, OUString() );
            aRelation.maType   = aAttribs.getString( XML_Type, OUString() );
            aRelation.maTarget = aAttribs.getString( XML_Target, OUString() );
            if( (aRelation.maId.getLength() > 0) && (aRelation.maType.getLength() > 0) && (aRelation.maTarget.getLength() > 0) )
            {
                sal_Int32 nTargetMode = aAttribs.getToken( XML_TargetMode, XML_Internal );
                OSL_ENSURE( (nTargetMode == XML_Internal) || (nTargetMode == XML_External),
                    "RelationsFragment::createFastChildContext - unexpected target mode, assuming external" );
                aRelation.mbExternal = nTargetMode != XML_Internal;

                OSL_ENSURE( mxRelations->count( aRelation.maId ) == 0,
                    "RelationsFragment::createFastChildContext - relation identifier exists already" );
                mxRelations->insert( Relations::value_type( aRelation.maId, aRelation ) );
            }
        }
        break;
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationships:
            xRet = getFastContextHandler();
        break;
    }
    return xRet;
}

// ============================================================================

} // namespace core
} // namespace oox

