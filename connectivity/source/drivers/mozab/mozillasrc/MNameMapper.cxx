/*************************************************************************
 *
 *  $RCSfile: MNameMapper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:38:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Darren Kenny
 *
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
}
MNameMapper::~MNameMapper()
{
    if ( mDirMap != NULL ) {
        MNameMapper::dirMap::iterator   iter;
        for (iter = mDirMap -> begin(); iter != mDirMap -> end(); ++iter) {
            NS_IF_RELEASE(((*iter).second));
        }
        delete mDirMap;
    }
}

// May modify the name passed in so that it's unique
void
MNameMapper::add( ::rtl::OUString& str, nsIAbDirectory* abook )
{
    MNameMapper::dirMap::iterator   iter;

    OSL_TRACE( "IN MNameMapper::add()\n" );

    if ( abook == NULL ) {
        OSL_TRACE( "\tOUT MNameMapper::add() called with null abook\n" );
        return;
    }

    if ( mDirMap->find( str ) != mDirMap->end() ) {
        // TODO - There's already and entry, so make the name unique
    }
    NS_IF_ADDREF(abook);
    mDirMap->insert( MNameMapper::dirMap::value_type( str, abook ) );
    OSL_TRACE( "\tOUT MNameMapper::add()\n" );
}

// Will replace the given dir
void
MNameMapper::replace( const ::rtl::OUString& str, nsIAbDirectory* abook )
{
    // TODO - needs to be implemented...
    OSL_TRACE( "IN/OUT MNameMapper::add()\n" );
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

