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

#include <boost/unordered_map.hpp>
#include <list>
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
        ::boost::unordered_map< int, ::rtl::OUString, ::boost::hash< int > > m_aStringMap;
        ::boost::unordered_map< ::rtl::OUString, int, ::rtl::OUStringHash >    m_aAtomMap;
    public:
        AtomProvider();
        ~AtomProvider();

        int getAtom( const ::rtl::OUString&, sal_Bool bCreate = sal_False );
        int getLastAtom() const { return m_nAtoms-1; }
        const ::rtl::OUString& getString( int ) const;

        void overrideAtom( int atom, const ::rtl::OUString& description );

        sal_Bool hasAtom( int atom ) const;
    };


    class UNOTOOLS_DLLPUBLIC MultiAtomProvider
    {
        ::boost::unordered_map< int, AtomProvider*, ::boost::hash< int > > m_aAtomLists;
    public:
        MultiAtomProvider();
        ~MultiAtomProvider();

        int getLastAtom( int atomClass ) const;

        sal_Bool insertAtomClass( int atomClass );

        int getAtom( int atomClass, const ::rtl::OUString& rString, sal_Bool bCreate = sal_False );

        const ::rtl::OUString& getString( int atomClass, int atom ) const;

        void overrideAtom( int atomClass, int atom, const ::rtl::OUString& description );
        void overrideAtom( int atomClass, const ::com::sun::star::util::AtomDescription& newDescription )
            { overrideAtom( atomClass, newDescription.atom, newDescription.description ); }
        sal_Bool hasAtom( int atomClass, int atom ) const;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
