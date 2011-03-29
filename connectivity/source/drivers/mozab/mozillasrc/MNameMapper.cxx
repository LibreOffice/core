/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"


#include <MNameMapper.hxx>

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */


using namespace connectivity::mozab;

bool
MNameMapper::ltstr::operator()( const ::rtl::OUString &s1, const ::rtl::OUString &s2) const
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
MNameMapper::add( ::rtl::OUString& str, nsIAbDirectory* abook )
{
    MNameMapper::dirMap::iterator   iter;

    OSL_TRACE( "IN MNameMapper::add()\n" );

    if ( abook == NULL ) {
        OSL_TRACE( "\tOUT MNameMapper::add() called with null abook\n" );
        return NS_ERROR_NULL_POINTER;
    }

    ::rtl::OUString ouUri=::rtl::OUString::createFromAscii(getURI(abook));
    if ( mUriMap->find (ouUri) != mUriMap->end() ) //There's already an entry with same uri
    {
        return NS_ERROR_FILE_NOT_FOUND;
    }
    mUriMap->insert( MNameMapper::uriMap::value_type( ouUri, abook ) );

    ::rtl::OUString tempStr=str;
    long count =1;
    while ( mDirMap->find( tempStr ) != mDirMap->end() ) {

        tempStr = str + ::rtl::OUString::valueOf(count);;
        count ++;
    }
    str = tempStr;
    NS_IF_ADDREF(abook);
    mDirMap->insert( MNameMapper::dirMap::value_type( str, abook ) );
    OSL_TRACE( "\tOUT MNameMapper::add()\n" );
    return 0;
}

bool
MNameMapper::getDir( const ::rtl::OUString& str, nsIAbDirectory* *abook )
{
    MNameMapper::dirMap::iterator   iter;

    OSL_TRACE( "IN MNameMapper::getDir( %s )\n", OUtoCStr(str)?OUtoCStr(str):"NULL" );

    if ( (iter = mDirMap->find( str )) != mDirMap->end() ) {
        *abook = (*iter).second;
        NS_IF_ADDREF(*abook);
    } else {
        *abook = NULL;
    }

    OSL_TRACE( "\tOUT MNameMapper::getDir() : %s\n", (*abook)?"True":"False" );

    return( (*abook) != NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
