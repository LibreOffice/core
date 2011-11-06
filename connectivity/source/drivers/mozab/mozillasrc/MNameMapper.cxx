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
#include "precompiled_connectivity.hxx"


#include <MNameMapper.hxx>

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */


using namespace connectivity::mozab;
using namespace rtl;

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

