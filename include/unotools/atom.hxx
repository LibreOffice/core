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
#ifndef INCLUDED_UNOTOOLS_ATOM_HXX
#define INCLUDED_UNOTOOLS_ATOM_HXX

#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/util/XAtomServer.hpp>
#include <list>
#include <unordered_map>

#define INVALID_ATOM 0

namespace utl {

    struct AtomDescription
    {
        int                 atom;
        OUString            description;
    };

    class AtomProvider
    {
        int                               m_nAtoms;
        std::unordered_map<int, OUString> m_aStringMap;
        std::unordered_map<OUString, int, OUStringHash>           m_aAtomMap;
    public:
        AtomProvider();
        ~AtomProvider();

        int getAtom( const OUString&, bool bCreate = false );
        const OUString& getString( int ) const;
    };

    class UNOTOOLS_DLLPUBLIC MultiAtomProvider
    {
        std::unordered_map<int, AtomProvider*> m_aAtomLists;
    public:
        MultiAtomProvider();
        ~MultiAtomProvider();

        int getAtom( int atomClass, const OUString& rString, bool bCreate = false );

        const OUString& getString( int atomClass, int atom ) const;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
