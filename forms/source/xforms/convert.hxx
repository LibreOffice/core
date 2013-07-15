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

#ifndef _CONVERT_HXX
#define _CONVERT_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <map>

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
    class Type;
} } } }

namespace xforms
{

struct TypeLess
{
    bool operator()( const com::sun::star::uno::Type& rType1,
                     const com::sun::star::uno::Type& rType2 ) const
    { return rType1.getTypeName() < rType2.getTypeName(); }
};

class Convert
{
    typedef com::sun::star::uno::Type Type_t;
    typedef com::sun::star::uno::Sequence<com::sun::star::uno::Type> Types_t;
    typedef com::sun::star::uno::Any Any_t;

    // hold conversion objects
    typedef OUString (*fn_toXSD)( const Any_t& );
    typedef Any_t (*fn_toAny)( const OUString& );
    typedef std::pair<fn_toXSD,fn_toAny> Convert_t;
    typedef std::map<Type_t,Convert_t,TypeLess> Map_t;
    Map_t maMap;

    Convert();

    void init();

public:
    /** get/create Singleton class */
    static Convert& get();

    /// can we convert this type?
    bool hasType( const Type_t& );

    /// get list of convertible types
    Types_t getTypes();

    /// convert any to XML representation
    OUString toXSD( const Any_t& rAny );

    /// convert XML representation to Any of given type
    Any_t toAny( const OUString&, const Type_t& );

    /** replace all sequences of 0x08, 0x0A, 0x0D, 0x20 with a single 0x20.
        also strip leading/trailing whitespace.
    */
    static OUString collapseWhitespace( const OUString& _rString );
};

} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
