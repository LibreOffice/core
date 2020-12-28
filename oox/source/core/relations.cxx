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

#include <sal/config.h>

#include <algorithm>
#include <string_view>

#include <oox/core/relations.hxx>

namespace oox::core {

namespace {

OUString lclRemoveFileName( const OUString& rPath )
{
    return rPath.copy( 0, ::std::max< sal_Int32 >( rPath.lastIndexOf( '/' ), 0 ) );
}

OUString lclAppendFileName( std::u16string_view rPath, const OUString& rFileName )
{
    return rPath.empty() ? rFileName :
        rPath + OUStringChar('/') + rFileName;
}

OUString createOfficeDocRelationTypeTransitional(std::u16string_view rType)
{
    return OUString::Concat("http://schemas.openxmlformats.org/officeDocument/2006/relationships/")
        + rType;
}

OUString createOfficeDocRelationTypeStrict(std::u16string_view rType)
{
    return OUString::Concat("http://purl.oclc.org/ooxml/officeDocument/relationships/") + rType;
}

}

Relations::Relations( const OUString& rFragmentPath )
    : maMap()
    , maFragmentPath( rFragmentPath )
{
}

const Relation* Relations::getRelationFromRelId( const OUString& rId ) const
{
    ::std::map< OUString, Relation >::const_iterator aIt = maMap.find( rId );
    return (aIt == maMap.end()) ? nullptr : &aIt->second;
}

const Relation* Relations::getRelationFromFirstType( std::u16string_view rType ) const
{
    for (auto const& elem : maMap)
        if( elem.second.maType.equalsIgnoreAsciiCase( rType ) )
            return &elem.second;
    return nullptr;
}

RelationsRef Relations::getRelationsFromTypeFromOfficeDoc( std::u16string_view rType ) const
{
    RelationsRef xRelations = std::make_shared<Relations>( maFragmentPath );
    for (auto const& elem : maMap)
        if( elem.second.maType.equalsIgnoreAsciiCase( createOfficeDocRelationTypeTransitional(rType) ) ||
                elem.second.maType.equalsIgnoreAsciiCase( createOfficeDocRelationTypeStrict(rType) ))
            xRelations->maMap[ elem.first ] = elem.second;
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

OUString Relations::getFragmentPathFromFirstType( std::u16string_view rType ) const
{
    const Relation* pRelation = getRelationFromFirstType( rType );
    return pRelation ? getFragmentPathFromRelation( *pRelation ) : OUString();
}

OUString Relations::getFragmentPathFromFirstTypeFromOfficeDoc( std::u16string_view rType ) const
{
    OUString aTransitionalType(createOfficeDocRelationTypeTransitional(rType));
    const Relation* pRelation = getRelationFromFirstType( aTransitionalType );
    if(!pRelation)
    {
        OUString aStrictType = createOfficeDocRelationTypeStrict(rType);
        pRelation = getRelationFromFirstType( aStrictType );
    }
    return pRelation ? getFragmentPathFromRelation( *pRelation ) : OUString();
}

} // namespace oox::core

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
