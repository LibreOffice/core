/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "oox/core/relations.hxx"

#include <rtl/ustrbuf.hxx>
#include "oox/helper/helper.hxx"

namespace oox {
namespace core {






namespace {

OUString lclRemoveFileName( const OUString& rPath )
{
    return rPath.copy( 0, ::std::max< sal_Int32 >( rPath.lastIndexOf( '/' ), 0 ) );
}

OUString lclAppendFileName( const OUString& rPath, const OUString& rFileName )
{
    return rPath.isEmpty() ? rFileName :
        OUStringBuffer( rPath ).append( '/' ).append( rFileName ).makeStringAndClear();
}

} // namespace



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
        if( aIt->second.maType.equalsIgnoreAsciiCase( rType ) )
            return &aIt->second;
    return 0;
}

RelationsRef Relations::getRelationsFromType( const OUString& rType ) const
{
    RelationsRef xRelations( new Relations( maFragmentPath ) );
    for( const_iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( aIt->second.maType.equalsIgnoreAsciiCase( rType ) )
            (*xRelations)[ aIt->first ] = aIt->second;
    return xRelations;
}

OUString Relations::getExternalTargetFromRelId( const OUString& rRelId ) const
{
    const Relation* pRelation = getRelationFromRelId( rRelId );
    return (pRelation && pRelation->mbExternal) ? pRelation->maTarget : OUString();
}

OUString Relations::getInternalTargetFromRelId( const OUString& rRelId ) const
{
    const Relation* pRelation = getRelationFromRelId( rRelId );
    return (pRelation && !pRelation->mbExternal) ? pRelation->maTarget : OUString();
}

OUString Relations::getFragmentPathFromRelation( const Relation& rRelation ) const
{
    // no target, no fragment path
    if( rRelation.mbExternal || rRelation.maTarget.isEmpty() )
        return OUString();

    // absolute target: return it without leading slash (#i100978)
    if( rRelation.maTarget[ 0 ] == '/' )
        return rRelation.maTarget.copy( 1 );

    // empty fragment path: return target
    if( maFragmentPath.isEmpty() )
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



} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
