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

#ifndef INCLUDED_BASIC_INC_SBPROP_HXX
#define INCLUDED_BASIC_INC_SBPROP_HXX

#include <basic/sbxprop.hxx>
#include <basic/sbdef.hxx>
#include <basic/basicdllapi.h>

class SbModule;

class BASIC_DLLPUBLIC SbProperty : public SbxProperty
{
    friend class SbiFactory;
    friend class SbModule;
    friend class SbProcedureProperty;
    SbModule* pMod;
    bool     bInvalid;
    BASIC_DLLPRIVATE SbProperty( const OUString&, SbxDataType, SbModule* );
    virtual ~SbProperty();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICPROP,1);
    SbModule* GetModule() { return pMod; }
};

typedef tools::SvRef<SbProperty> SbPropertyRef;

class BASIC_DLLPUBLIC SbProcedureProperty : public SbxProperty
{
    bool mbSet;     // Flag for set command

    virtual ~SbProcedureProperty();

public:
    SbProcedureProperty( const OUString& r, SbxDataType t )
            : SbxProperty( r, t ) // , pMod( p )
            , mbSet( false )
    {}

    bool isSet()
        { return mbSet; }
    void setSet( bool bSet )
        { mbSet = bSet; }
};

#endif // INCLUDED_BASIC_INC_SBPROP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
