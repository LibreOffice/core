/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: relations.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:14:58 $
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

#include "oox/core/relations.hxx"
#include "oox/helper/helper.hxx"

using ::rtl::OUString;

namespace oox {
namespace core {

// ============================================================================

Relations::Relations( const OUString& rBasePath ) :
    maBasePath( rBasePath )
{
}

const Relation* Relations::getRelationFromRelId( const OUString& rId ) const
{
    const_iterator aIt = find( rId );
    return (aIt == end()) ? 0 : &aIt->second;
}

const Relation* Relations::getRelationFromType( const OUString& rType ) const
{
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( aIt->second.maType == rType )
            return &aIt->second;
    return 0;
}

RelationsRef Relations::getRelationsFromType( const OUString& rType ) const
{
    RelationsRef xRelations( new Relations( maBasePath ) );
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( aIt->second.maType == rType )
            (*xRelations)[ aIt->first ] = aIt->second;
    return xRelations;
}

OUString Relations::getTargetFromRelId( const OUString& rRelId ) const
{
    if( const Relation* pRelation = getRelationFromRelId( rRelId ) )
        return pRelation->maTarget;
    return OUString();
}

OUString Relations::getTargetFromType( const OUString& rType ) const
{
    if( const Relation* pRelation = getRelationFromType( rType ) )
        return pRelation->maTarget;
    return OUString();
}

OUString Relations::getFragmentPathFromTarget( const OUString& rTarget ) const
{
    const sal_Unicode cDirSep = '/';

    // no target, no fragment path
    if( rTarget.getLength() == 0 )
        return OUString();

    // absolute target, or empty fragment path -> return target
    if( (rTarget[ 0 ] == cDirSep) || (maBasePath.getLength() == 0) )
        return rTarget;

    sal_Int32 nLastSepPos = maBasePath.lastIndexOf( cDirSep );
    OUString aPath = (nLastSepPos < 0) ? maBasePath : maBasePath.copy( 0, nLastSepPos );

    const OUString sBack = CREATE_OUSTRING( "../" );

    // First, count the number of "../"'s found in relative path string.
    sal_Int32 nCount = 0, nPos = 0;
    while ( true )
    {
        nPos = rTarget.indexOf(sBack, nCount*3);
        if ( nPos != nCount*3 )
            break;
        ++nCount;
    }

    // Now, reduce the base path's directory level by the count.
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        sal_Int32 pos = aPath.lastIndexOf(cDirSep);
        if ( pos < 0 )
            // This is unexpected.  Bail out.
            return rTarget;
        aPath = aPath.copy( 0, pos );
    }

    aPath += OUString( cDirSep );
    aPath += rTarget.copy( nCount*3 );
    return aPath;
}

OUString Relations::getFragmentPathFromRelId( const OUString& rRelId ) const
{
    return getFragmentPathFromTarget( getTargetFromRelId( rRelId ) );
}

OUString Relations::getFragmentPathFromType( const OUString& rType ) const
{
    return getFragmentPathFromTarget( getTargetFromType( rType ) );
}

// ============================================================================

} // namespace core
} // namespace oox

