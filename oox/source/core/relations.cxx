/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: relations.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/core/relations.hxx"
#include <rtl/ustrbuf.hxx>
#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace oox {
namespace core {

// ============================================================================

namespace {

OUString lclRemoveFileName( const OUString& rPath )
{
    return rPath.copy( 0, ::std::max< sal_Int32 >( rPath.lastIndexOf( '/' ), 0 ) );
}

OUString lclAppendFileName( const OUString& rPath, const OUString& rFileName )
{
    return (rPath.getLength() == 0) ? rFileName :
        OUStringBuffer( rPath ).append( sal_Unicode( '/' ) ).append( rFileName ).makeStringAndClear();
}

} // namespace

// ============================================================================

Relations::Relations( const OUString& rFragmentPath ) :
    maFragmentPath( rFragmentPath )
{
}

const Relation* Relations::getRelationFromRelId( const OUString& rId ) const
{
    const_iterator aIt = find( rId );
    return (aIt == end()) ? 0 : &aIt->second;
}

const Relation* Relations::getRelationFromFirstType( const OUString& rType ) const
{
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( aIt->second.maType == rType )
            return &aIt->second;
    return 0;
}

RelationsRef Relations::getRelationsFromType( const OUString& rType ) const
{
    RelationsRef xRelations( new Relations( maFragmentPath ) );
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( aIt->second.maType == rType )
            (*xRelations)[ aIt->first ] = aIt->second;
    return xRelations;
}

OUString Relations::getExternalTargetFromRelId( const OUString& rRelId ) const
{
    const Relation* pRelation = getRelationFromRelId( rRelId );
    return (pRelation && pRelation->mbExternal) ? pRelation->maTarget : OUString();
}

OUString Relations::getExternalTargetFromFirstType( const OUString& rType ) const
{
    const Relation* pRelation = getRelationFromFirstType( rType );
    return (pRelation && pRelation->mbExternal) ? pRelation->maTarget : OUString();
}

OUString Relations::getFragmentPathFromRelation( const Relation& rRelation ) const
{
    // no target, no fragment path
    if( rRelation.mbExternal || (rRelation.maTarget.getLength() == 0) )
        return OUString();

    // absolute target: return it without leading slash (#i100978)
    if( rRelation.maTarget[ 0 ] == '/' )
        return rRelation.maTarget.copy( 1 );

    // empty fragment path: return target
    if( maFragmentPath.getLength() == 0 )
        return rRelation.maTarget;

    // resolve relative target path according to base path
    OUString aPath = lclRemoveFileName( maFragmentPath );
    sal_Int32 nStartPos = 0;
    while( nStartPos < rRelation.maTarget.getLength() )
    {
        sal_Int32 nSepPos = rRelation.maTarget.indexOf( '/', nStartPos );
        if( nSepPos < 0 ) nSepPos = rRelation.maTarget.getLength();
        // append next directory name from aTarget to aPath, or remove last directory on '../'
        if( (nStartPos + 2 == nSepPos) && (rRelation.maTarget[ nStartPos ] == '.') && (rRelation.maTarget[ nStartPos + 1 ] == '.') )
            aPath = lclRemoveFileName( aPath );
        else
            aPath = lclAppendFileName( aPath, rRelation.maTarget.copy( nStartPos, nSepPos - nStartPos ) );
        // move nStartPos to next directory name
        nStartPos = nSepPos + 1;
    }

    return aPath;
}

OUString Relations::getFragmentPathFromRelId( const OUString& rRelId ) const
{
    const Relation* pRelation = getRelationFromRelId( rRelId );
    return pRelation ? getFragmentPathFromRelation( *pRelation ) : OUString();
}

OUString Relations::getFragmentPathFromFirstType( const OUString& rType ) const
{
    const Relation* pRelation = getRelationFromFirstType( rType );
    return pRelation ? getFragmentPathFromRelation( *pRelation ) : OUString();
}

// ============================================================================

} // namespace core
} // namespace oox

