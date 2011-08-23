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

#ifndef _SB_SBPROPERTY_HXX
#define _SB_SBPROPERTY_HXX


#include "sbxprop.hxx"
#include "sbdef.hxx"

namespace binfilter {

class SbModule;

class SbProperty : public SbxProperty
{
    friend class SbiFactory;
    friend class SbModule;
    friend class SbProcedureProperty;
    SbModule* pMod;
    BOOL	 bInvalid;
    SbProperty( const String&, SbxDataType, SbModule* );
    virtual ~SbProperty();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICPROP,1);
    TYPEINFO();
    SbModule* GetModule() { return pMod; }
};

#ifndef __SB_SBPROPERTYREF_HXX
#define __SB_SBPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProperty)
#endif


class SbProcedureProperty : public SbxProperty
{
    bool mbSet;		// Flag for set command

    virtual ~SbProcedureProperty();

public:
    SbProcedureProperty( const String& r, SbxDataType t )
            : SbxProperty( r, t ) // , pMod( p )
            , mbSet( false )
    {}
    TYPEINFO();

    bool isSet( void )
        { return mbSet; }
    void setSet( bool bSet )
        { mbSet = bSet; }
};

#ifndef __SB_SBPROCEDUREPROPERTYREF_HXX
#define __SB_SBPROCEDUREPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProcedureProperty)
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
