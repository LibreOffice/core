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
#ifndef _SC_BRDCST_HXX
#define _SC_BRDCST_HXX

#include "global.hxx"
#include "address.hxx"
#include <tools/rtti.hxx>
#include <svl/hint.hxx>
#include <svl/smplhint.hxx>

class SvtBroadcaster;

#define SC_HINT_DYING       SFX_HINT_DYING
#define SC_HINT_DATACHANGED SFX_HINT_DATACHANGED
#define SC_HINT_TABLEOPDIRTY    SFX_HINT_USER00
#define SC_HINT_CALCALL         SFX_HINT_USER01

class ScHint : public SfxSimpleHint
{
private:
    ScAddress   aAddress;
    SvtBroadcaster* mpBroadcaster;

public:
    TYPEINFO();
    ScHint( sal_uLong n, const ScAddress& a, SvtBroadcaster* p );
    SvtBroadcaster* GetBroadcaster() const;
    void SetBroadcaster( SvtBroadcaster* p );
    const ScAddress&    GetAddress() const { return aAddress; }
          ScAddress&    GetAddress()       { return aAddress; }
    void                SetAddress( const ScAddress& rAdr ) { aAddress = rAdr; }
};

class ScAreaChangedHint : public SfxHint
{
private:
    ScRange aNewRange;
public:
                    TYPEINFO();
                    ScAreaChangedHint(const ScRange& rRange) : aNewRange(rRange) {}
    const ScRange&  GetRange() const { return aNewRange; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
