/*************************************************************************
 *
 *  $RCSfile: atom.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2000-11-14 11:23:12 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <unotools/atom.hxx>

using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
#define NMSP_UTIL ::com::sun::star::util

AtomProvider::AtomProvider()
{
    m_nAtoms = 1;
}

AtomProvider::~AtomProvider()
{
}

int AtomProvider::getAtom( const ::rtl::OUString& rString, sal_Bool bCreate )
{
    ::std::hash_map< ::rtl::OUString, int, OUStringHash >::iterator it = m_aAtomMap.find( rString );
    if( it != m_aAtomMap.end() )
        return it->second;
    if( ! bCreate )
        return INVALID_ATOM;
    m_aAtomMap[ rString ] = m_nAtoms;
    m_aStringMap[ m_nAtoms ] = rString;
    m_nAtoms++;
    return m_nAtoms-1;
}

void AtomProvider::getAll( ::std::list< ::utl::AtomDescription >& atoms )
{
    atoms.clear();
    ::std::hash_map< ::rtl::OUString, int, OUStringHash >::const_iterator it = m_aAtomMap.begin();

    ::utl::AtomDescription aDesc;
    while( it != m_aAtomMap.end() )
    {
        aDesc.atom          = it->second;
        aDesc.description   = it->first;
        atoms.push_back( aDesc );
        ++it;
    }
}

void AtomProvider::getRecent( int atom, ::std::list< ::utl::AtomDescription >& atoms )
{
    atoms.clear();

    ::std::hash_map< ::rtl::OUString, int, OUStringHash >::const_iterator it = m_aAtomMap.begin();

    ::utl::AtomDescription aDesc;
    while( it != m_aAtomMap.end() )
    {
        if( it->second > atom )
        {
            aDesc.atom          = it->second;
            aDesc.description   = it->first;
            atoms.push_back( aDesc );
        }
        ++it;
    }
}

const ::rtl::OUString& AtomProvider::getString( int nAtom ) const
{
    static ::rtl::OUString aEmpty;
    ::std::hash_map< int, ::rtl::OUString, ::std::hash< int > >::const_iterator it = m_aStringMap.find( nAtom );

    return it == m_aStringMap.end() ? aEmpty : it->second;
}

void AtomProvider::overrideAtom( int atom, const ::rtl::OUString& description )
{
    m_aAtomMap[ description ] = atom;
    m_aStringMap[ atom ] = description;
    if( m_nAtoms <= atom )
        m_nAtoms=atom+1;
}

sal_Bool AtomProvider::hasAtom( int atom ) const
{
    return m_aStringMap.find( atom ) != m_aStringMap.end() ? sal_True : sal_False;
}

// -----------------------------------------------------------------------

MultiAtomProvider::MultiAtomProvider()
{
}

MultiAtomProvider::~MultiAtomProvider()
{
    for( ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::iterator it = m_aAtomLists.begin(); it != m_aAtomLists.end(); ++it )
        delete it->second;
}


sal_Bool MultiAtomProvider::insertAtomClass( int atomClass )
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::iterator it =
          m_aAtomLists.find( atomClass );
    if( it != m_aAtomLists.end() )
        return sal_False;
    m_aAtomLists[ atomClass ] = new AtomProvider();
    return sal_True;
}

int MultiAtomProvider::getAtom( int atomClass, const ::rtl::OUString& rString, sal_Bool bCreate )
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::iterator it =
          m_aAtomLists.find( atomClass );
    if( it != m_aAtomLists.end() )
        return it->second->getAtom( rString, bCreate );

    if( bCreate )
    {
        AtomProvider* pNewClass;
        m_aAtomLists[ atomClass ] = pNewClass = new AtomProvider();
        return pNewClass->getAtom( rString, bCreate );
    }
    return INVALID_ATOM;
}

int MultiAtomProvider::getLastAtom( int atomClass ) const
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::const_iterator it =
          m_aAtomLists.find( atomClass );

    return it != m_aAtomLists.end() ? it->second->getLastAtom() : INVALID_ATOM;
}

void MultiAtomProvider::getRecent( int atomClass, int atom, ::std::list< ::utl::AtomDescription >& atoms )
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::const_iterator it =
          m_aAtomLists.find( atomClass );
    if( it != m_aAtomLists.end() )
        it->second->getRecent( atom, atoms );
    else
        atoms.clear();
}

const ::rtl::OUString& MultiAtomProvider::getString( int atomClass, int atom ) const
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::const_iterator it =
          m_aAtomLists.find( atomClass );
    if( it != m_aAtomLists.end() )
        return it->second->getString( atom );

    static ::rtl::OUString aEmpty;
    return aEmpty;
}

sal_Bool MultiAtomProvider::hasAtom( int atomClass, int atom ) const
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::const_iterator it = m_aAtomLists.find( atomClass );
    return it != m_aAtomLists.end() ? it->second->hasAtom( atom ) : sal_False;
}

void MultiAtomProvider::getClass( int atomClass, ::std::list< ::utl::AtomDescription >& atoms) const
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::const_iterator it = m_aAtomLists.find( atomClass );

    if( it != m_aAtomLists.end() )
        it->second->getAll( atoms );
    else
        atoms.clear();
}

void MultiAtomProvider::overrideAtom( int atomClass, int atom, const ::rtl::OUString& description )
{
    ::std::hash_map< int, AtomProvider*, ::std::hash< int > >::const_iterator it = m_aAtomLists.find( atomClass );
    if( it == m_aAtomLists.end() )
        m_aAtomLists[ atomClass ] = new AtomProvider();
    m_aAtomLists[ atomClass ]->overrideAtom( atom, description );
}

// -----------------------------------------------------------------------

AtomServer::AtomServer()
{
}

AtomServer::~AtomServer()
{
}

sal_Int32 AtomServer::getAtom( sal_Int32 atomClass, const ::rtl::OUString& description, sal_Bool create )
{
    ::osl::Guard< ::osl::Mutex > guard( m_aMutex );

    return m_aProvider.getAtom( atomClass, description, create );
}

Sequence< Sequence< NMSP_UTIL::AtomDescription > > AtomServer::getClasses( const Sequence< sal_Int32 >& atomClasses )
{
    ::osl::Guard< ::osl::Mutex > guard( m_aMutex );

    Sequence< Sequence< NMSP_UTIL::AtomDescription > > aRet( atomClasses.getLength() );
    for( int i = 0; i < atomClasses.getLength(); i++ )
    {
        aRet.getArray()[i] = getClass( atomClasses.getConstArray()[i] );
    }
    return aRet;
}

Sequence< NMSP_UTIL::AtomDescription > AtomServer::getClass( sal_Int32 atomClass )
{
    ::osl::Guard< ::osl::Mutex > guard( m_aMutex );

    ::std::list< ::utl::AtomDescription > atoms;
    m_aProvider.getClass( atomClass, atoms );

    Sequence< NMSP_UTIL::AtomDescription > aRet( atoms.size() );
    for( int i = aRet.getLength()-1; i >= 0; i-- )
    {
        aRet.getArray()[i].atom         = atoms.back().atom;
        aRet.getArray()[i].description  = atoms.back().description;
        atoms.pop_back();
    }

    return aRet;
}

Sequence< NMSP_UTIL::AtomDescription > AtomServer::getRecentAtoms( sal_Int32 atomClass, sal_Int32 atom )
{
    ::osl::Guard< ::osl::Mutex > guard( m_aMutex );

    ::std::list< ::utl::AtomDescription > atoms;
    m_aProvider.getRecent( atomClass, atom, atoms );

    Sequence< NMSP_UTIL::AtomDescription > aRet( atoms.size() );
    for( int i = aRet.getLength()-1; i >= 0; i-- )
    {
        aRet.getArray()[i].atom         = atoms.back().atom;
        aRet.getArray()[i].description  = atoms.back().description;
        atoms.pop_back();
    }

    return aRet;
}

Sequence< ::rtl::OUString > AtomServer::getAtomDescriptions( const Sequence< AtomClassRequest >& atoms )
{
    ::osl::Guard< ::osl::Mutex > guard( m_aMutex );

    int nStrings = 0, i;
    for( i = 0; i < atoms.getLength(); i++ )
        nStrings += atoms.getConstArray()[ i ].atoms.getLength();
    Sequence< ::rtl::OUString > aRet( nStrings );
    for( i = 0, nStrings = 0; i < atoms.getLength(); i++ )
    {
        const AtomClassRequest& rRequest = atoms.getConstArray()[i];
        for( int n = 0; n < rRequest.atoms.getLength(); n++ )
            aRet.getArray()[ nStrings++ ] = m_aProvider.getString( rRequest.atomClass, rRequest.atoms.getConstArray()[ n ] );
    }
    return aRet;
}

// -----------------------------------------------------------------------

AtomClient::AtomClient( const Reference< XAtomServer >& xServer ) :
        m_xServer( xServer )
{
}

AtomClient::~AtomClient()
{
}

int AtomClient::getAtom( int atomClass, const ::rtl::OUString& description, sal_Bool bCreate )
{
    int nAtom = m_aProvider.getAtom( atomClass, description, sal_False );
    if( nAtom == INVALID_ATOM && bCreate )
    {
        nAtom = m_xServer->getAtom( atomClass, description, bCreate );
        if( nAtom != INVALID_ATOM )
            m_aProvider.overrideAtom( atomClass, nAtom, description );
    }
    return nAtom;
}

const ::rtl::OUString& AtomClient::getString( int atomClass, int atom )
{
    if( ! m_aProvider.hasAtom( atomClass, atom ) )
    {
        Sequence< NMSP_UTIL::AtomDescription > aSeq;
        aSeq = m_xServer->getRecentAtoms( atomClass, m_aProvider.getLastAtom( atomClass ) );
        const NMSP_UTIL::AtomDescription* pDescriptions = aSeq.getConstArray();
        for( int i = 0; i < aSeq.getLength(); i++ )
            m_aProvider.overrideAtom( atomClass,
                                      pDescriptions[i].atom,
                                      pDescriptions[i].description
                                      );

        if( ! m_aProvider.hasAtom( atomClass, atom ) )
        {
            // holes may occur by the above procedure!
            Sequence< AtomClassRequest > aSeq( 1 );
            aSeq.getArray()[0].atomClass = atomClass;
            aSeq.getArray()[0].atoms.realloc( 1 );
            aSeq.getArray()[0].atoms.getArray()[0] = atom;
            Sequence< ::rtl::OUString > aRet = m_xServer->getAtomDescriptions( aSeq );
            if( aRet.getLength() == 1 )
                m_aProvider.overrideAtom( atomClass, atom, aRet.getConstArray()[0] );
        }
    }
    return m_aProvider.getString( atomClass, atom );
}

void AtomClient::updateAtomClasses( const Sequence< sal_Int32 >& atomClasses )
{
    Sequence< Sequence< NMSP_UTIL::AtomDescription > > aUpdate = m_xServer->getClasses( atomClasses );
    for( int i = 0; i < atomClasses.getLength(); i++ )
    {
        int nClass = atomClasses.getConstArray()[i];
        const Sequence< NMSP_UTIL::AtomDescription >& rClass = aUpdate.getConstArray()[i];
        const NMSP_UTIL::AtomDescription* pDesc = rClass.getConstArray();
        for( int n = 0; n < rClass.getLength(); n++, pDesc++ )
            m_aProvider.overrideAtom( nClass, pDesc->atom, pDesc->description );
    }
}
