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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
