/*************************************************************************
 *
 *  $RCSfile: atom.hxx,v $
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

#ifndef _UTL_ATOM_HXX_
#define _UTL_ATOM_HXX_

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif

#ifndef _COM_SUN_STAR_UTIL_XATOMSERVER_HPP_
#include <com/sun/star/util/XAtomServer.hpp>
#endif

#define INVALID_ATOM 0

namespace utl {

    struct AtomDescription
    {
        int                 atom;
        ::rtl::OUString     description;
    };

    class AtomProvider
    {
        int                                     m_nAtoms;
        ::std::hash_map< int, ::rtl::OUString, ::std::hash< int > > m_aStringMap;
        ::std::hash_map< ::rtl::OUString, int, OUStringHash >   m_aAtomMap;
    public:
        AtomProvider();
        ~AtomProvider();

        int getAtom( const ::rtl::OUString&, sal_Bool bCreate = sal_False );
        int getLastAtom() const { return m_nAtoms-1; }
        const ::rtl::OUString& getString( int ) const;

        void getAll( ::std::list< AtomDescription >& atoms );

        void getRecent( int atom, ::std::list< AtomDescription >& atoms );

        void overrideAtom( int atom, const ::rtl::OUString& description );

        sal_Bool hasAtom( int atom ) const;
    };


    class MultiAtomProvider
    {
        ::std::hash_map< int, AtomProvider*, ::std::hash< int > > m_aAtomLists;
    public:
        MultiAtomProvider();
        ~MultiAtomProvider();

        int getLastAtom( int atomClass ) const;

        sal_Bool insertAtomClass( int atomClass );

        int getAtom( int atomClass, const ::rtl::OUString& rString, sal_Bool bCreate = sal_False );

        void getRecent( int atomClass, int atom, ::std::list< AtomDescription >& atoms );

        const ::rtl::OUString& getString( int atomClass, int atom ) const;
        void getClass( int atomClass, ::std::list< AtomDescription >& atoms ) const;

        void overrideAtom( int atomClass, int atom, const ::rtl::OUString& description );
        void overrideAtom( int atomClass, const ::com::sun::star::util::AtomDescription& newDescription )
            { overrideAtom( atomClass, newDescription.atom, newDescription.description ); }
        sal_Bool hasAtom( int atomClass, int atom ) const;
    };

    class AtomServer : public ::cppu::WeakAggImplHelper1< ::com::sun::star::util::XAtomServer >
    {
    private:
        MultiAtomProvider   m_aProvider;
        ::osl::Mutex        m_aMutex;
    public:
        AtomServer();
        virtual ~AtomServer();

        const ::rtl::OUString& getString( int atomClass, int atom ) const
            { return m_aProvider.getString( atomClass, atom ); }

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomDescription > SAL_CALL getClass( sal_Int32 atomClass );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomDescription > > SAL_CALL getClasses( const ::com::sun::star::uno::Sequence< sal_Int32 >& atomClasses );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAtomDescriptions( const ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomClassRequest >& atoms );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomDescription > SAL_CALL getRecentAtoms( sal_Int32 atomClass, sal_Int32 atom );
        virtual sal_Int32 SAL_CALL getAtom( sal_Int32 atomClass, const ::rtl::OUString& description, sal_Bool create );
    };

    class AtomClient
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XAtomServer >     m_xServer;
        MultiAtomProvider m_aProvider;
    public:
        AtomClient( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XAtomServer >& );
        ~AtomClient();

        void updateAtomClasses( const ::com::sun::star::uno::Sequence< sal_Int32 >& atomClasses );
        int getAtom( int atomClass, const ::rtl::OUString& description, sal_Bool bCreate );
        const ::rtl::OUString& getString( int atomClass, int atom );
    };
}

#endif
