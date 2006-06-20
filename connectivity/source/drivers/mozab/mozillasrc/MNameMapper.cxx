/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNameMapper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:51:38 $
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

