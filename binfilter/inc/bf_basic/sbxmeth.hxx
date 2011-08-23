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


#ifndef __SBX_SBXMETHOD_HXX
#define __SBX_SBXMETHOD_HXX

#include "sbxvar.hxx"

namespace binfilter {

class SbxMethodImpl;

class SbxMethod : public SbxVariable
{
    SbxMethodImpl* mpSbxMethodImpl;	// Impl data

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_METHOD,1);
    TYPEINFO();
    SbxMethod( const String& r, SbxDataType t )
    : SbxVariable( t ) { SetName( r ); }
    SbxMethod( const SbxMethod& r ) : SvRefBase( r ), SbxVariable( r ) {}
    SbxMethod& operator=( const SbxMethod& r )
    { SbxVariable::operator=( r ); return *this; }
    virtual SbxClassType GetClass() const;
};

#ifndef __SBX_SBXMETHODREF_HXX
#define __SBX_SBXMETHODREF_HXX

#ifndef SBX_METHOD_DECL_DEFINED
#define SBX_METHOD_DECL_DEFINED
SV_DECL_REF(SbxMethod)
#endif
SV_IMPL_REF(SbxMethod)

#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
