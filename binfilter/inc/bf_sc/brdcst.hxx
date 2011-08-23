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
#ifndef _SC_BRDCST_HXX
#define _SC_BRDCST_HXX

#include "global.hxx"

#include <bf_svtools/smplhint.hxx>
namespace binfilter {
class ScBaseCell;

#define SC_HINT_DYING		SFX_HINT_DYING
#define SC_HINT_DATACHANGED	SFX_HINT_DATACHANGED
#define SC_HINT_TABLEOPDIRTY	SFX_HINT_USER00

class ScHint : public SfxSimpleHint
{
private:
    ScAddress	aAddress;
    ScBaseCell* pCell;

public:
    TYPEINFO();
                        ScHint( ULONG n, const ScAddress& a, ScBaseCell* p )
                            : SfxSimpleHint( n ), aAddress( a ), pCell( p ) {}
    ScBaseCell*			GetCell() const { return pCell; }
    void                SetCell( ScBaseCell* p )    { pCell = p; }
    const ScAddress&	GetAddress() const { return aAddress; }
          ScAddress&    GetAddress()       { return aAddress; }
    void                SetAddress( const ScAddress& rAdr ) { aAddress = rAdr; }
};

class ScAreaChangedHint : public SfxHint
{
private:
    ScRange	aNewRange;
public:
                    TYPEINFO();
                    ScAreaChangedHint(const ScRange& rRange) : aNewRange(rRange) {}
    const ScRange&	GetRange() const { return aNewRange; }
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
