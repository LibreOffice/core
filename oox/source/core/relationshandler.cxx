/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: relationshandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:15:12 $
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

#include "oox/core/relationshandler.hxx"
#include <rtl/ustrbuf.hxx>
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace core {

// ============================================================================

namespace {

/*  Build path to relations file from passed path, e.g.:
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

RelationsFragmentHandler::RelationsFragmentHandler( XmlFilterBase& rFilter, RelationsRef xRelations ) :
    FragmentHandler( rFilter, lclGetRelationsPath( xRelations->getBasePath() ), xRelations ),
    mxRelations( xRelations )
{
}

Reference< XFastContextHandler > RelationsFragmentHandler::createFastChildContext(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rxAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( nElement )
    {
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationship:
        {
            Relation aRelation;
            aRelation.maId     = rxAttribs->getOptionalValue( XML_Id );
            aRelation.maType   = rxAttribs->getOptionalValue( XML_Type );
            aRelation.maTarget = rxAttribs->getOptionalValue( XML_Target );
            if( (aRelation.maId.getLength() > 0) && (aRelation.maType.getLength() > 0) && (aRelation.maTarget.getLength() > 0) )
                (*mxRelations)[ aRelation.maId ] = aRelation;
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

