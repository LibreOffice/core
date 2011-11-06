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



#include "oox/core/relationshandler.hxx"

#include <rtl/ustrbuf.hxx>
#include "oox/helper/attributelist.hxx"

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
        case PR_TOKEN( Relationship ):
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
        case PR_TOKEN( Relationships ):
            xRet = getFastContextHandler();
        break;
    }
    return xRet;
}

// ============================================================================

} // namespace core
} // namespace oox
