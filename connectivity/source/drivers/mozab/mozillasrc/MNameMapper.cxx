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

#include <MNameMapper.hxx>

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */


using namespace connectivity::mozab;

bool
MNameMapper::ltstr::operator()( const OUString &s1, const OUString &s2) const
{
    return s1.compareTo(s2) < 0;
}

MNameMapper::MNameMapper()
{
    mDirMap = new MNameMapper::dirMap;
    mUriMap = new MNameMapper::uriMap;
}
MNameMapper::~MNameMapper()
{
    clear();
}

void MNameMapper::reset()
{
    clear();
    mDirMap = new MNameMapper::dirMap;
    mUriMap = new MNameMapper::uriMap;
}
void MNameMapper::clear()
{
    if ( mUriMap != NULL ) {
        delete mUriMap;
    }
    if ( mDirMap != NULL ) {
        MNameMapper::dirMap::iterator   iter;
        for (iter = mDirMap -> begin(); iter != mDirMap -> end(); ++iter) {
            NS_IF_RELEASE(((*iter).second));
        }
        delete mDirMap;
    }
}
const char * getURI(const nsIAbDirectory*  directory)
{
    nsresult retCode;
    nsCOMPtr<nsIRDFResource> rdfResource = do_QueryInterface((nsISupports *)directory, &retCode) ;
    if (NS_FAILED(retCode)) { return NULL; }
    const char * uri;
    retCode=rdfResource->GetValueConst(&uri);
    if (NS_FAILED(retCode)) { return NULL; }
    return uri;
}

// May modify the name passed in so that it's unique
nsresult
MNameMapper::add( OUString& str, nsIAbDirectory* abook )
{
    MNameMapper::dirMap::iterator   iter;

    OSL_TRACE( "IN MNameMapper::add()" );

    if ( abook == NULL ) {
        OSL_TRACE( "\tOUT MNameMapper::add() called with null abook" );
        return NS_ERROR_NULL_POINTER;
    }

    OUString ouUri=OUString::createFromAscii(getURI(abook));
    if ( mUriMap->find (ouUri) != mUriMap->end() ) //There's already an entry with same uri
    {
        return NS_ERROR_FILE_NOT_FOUND;
    }
    mUriMap->insert( MNameMapper::uriMap::value_type( ouUri, abook ) );

    OUString tempStr=str;
    long count =1;
    while ( mDirMap->find( tempStr ) != mDirMap->end() ) {

        tempStr = str + OUString::number(count);
        count ++;
    }
    str = tempStr;
    NS_IF_ADDREF(abook);
    mDirMap->insert( MNameMapper::dirMap::value_type( str, abook ) );
    OSL_TRACE( "\tOUT MNameMapper::add()" );
    return 0;
}

bool
MNameMapper::getDir( const OUString& str, nsIAbDirectory* *abook )
{
    MNameMapper::dirMap::iterator   iter;

    OSL_TRACE( "IN MNameMapper::getDir( %s )", OUtoCStr(str)?OUtoCStr(str):"NULL" );

    if ( (iter = mDirMap->find( str )) != mDirMap->end() ) {
        *abook = (*iter).second;
        NS_IF_ADDREF(*abook);
    } else {
        *abook = NULL;
    }

    OSL_TRACE( "\tOUT MNameMapper::getDir() : %s", (*abook)?"True":"False" );

    return( (*abook) != NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
