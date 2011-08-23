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
#ifndef _SFXHINT_HXX
#define _SFXHINT_HXX

#include <tools/rtti.hxx>

namespace binfilter
{

class  SfxHint
{
public:
    TYPEINFO();

    virtual ~SfxHint();
};

//--------------------------------------------------------------------

#define DECL_PTRHINT(Visibility, Name, Type) \
        class Visibility Name: public ::binfilter::SfxHint \
        { \
            Type* pObj; \
            BOOL  bIsOwner; \
        \
        public: \
            TYPEINFO(); \
            Name( Type* Object, BOOL bOwnedByHint = FALSE ); \
            ~Name(); \
        \
            Type* GetObject() const { return pObj; } \
            BOOL  IsOwner() const { return bIsOwner; } \
        }

#define IMPL_PTRHINT_AUTODELETE(Name, Type) \
        TYPEINIT1(Name, SfxHint);	\
        Name::Name( Type* pObject, BOOL bOwnedByHint ) \
            { pObj = pObject; bIsOwner = bOwnedByHint; } \
        Name::~Name() { if ( bIsOwner ) delete pObj; }

#define IMPL_PTRHINT(Name, Type) \
        TYPEINIT1(Name, SfxHint);	\
        Name::Name( Type* pObject, BOOL bOwnedByHint ) \
            { pObj = pObject; bIsOwner = bOwnedByHint; } \
        Name::~Name() {}


}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
