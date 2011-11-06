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


#ifndef _UTL_ATOM_HXX_
#define _UTL_ATOM_HXX_

#include "unotools/unotoolsdllapi.h"
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>

#include <hash_map>
#ifndef __SGI_STL_LIST
#include <list>
#endif
#include <com/sun/star/util/XAtomServer.hpp>

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
        ::std::hash_map< ::rtl::OUString, int, ::rtl::OUStringHash >    m_aAtomMap;
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


    class UNOTOOLS_DLLPUBLIC MultiAtomProvider
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

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomDescription > SAL_CALL getClass( sal_Int32 atomClass ) throw();
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomDescription > > SAL_CALL getClasses( const ::com::sun::star::uno::Sequence< sal_Int32 >& atomClasses ) throw();
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAtomDescriptions( const ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomClassRequest >& atoms ) throw();
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::util::AtomDescription > SAL_CALL getRecentAtoms( sal_Int32 atomClass, sal_Int32 atom ) throw();
        virtual sal_Int32 SAL_CALL getAtom( sal_Int32 atomClass, const ::rtl::OUString& description, sal_Bool create ) throw();
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
