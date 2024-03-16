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

#pragma once

#include <rtl/ustring.hxx>
#include "types.hxx"

class SvTokenStream;
class SvIdlDataBase;
class SvMetaModule;
class SvMetaTypeEnum;
class SvStringHashEntry;
class SvMetaType;
class SvMetaClass;
class SvBOOL;

class SvIdlParser
{
    SvIdlDataBase&  rBase;
    SvTokenStream & rInStm;
public:
    SvIdlParser( SvIdlDataBase& rBase_, SvTokenStream & rInStrm_) : rBase(rBase_), rInStm(rInStrm_) {}
    void         ReadSvIdl( const OUString & rPath );
    void         ReadModuleHeader(SvMetaModule& rModule);
    void         ReadModuleBody(SvMetaModule& rModule);
    void         ReadModuleElement( SvMetaModule& rModule );
    void         ReadInclude( SvMetaModule& rModule );
    void         ReadInterfaceOrShell( SvMetaModule& rModule, MetaTypeType aMetaTypeType );
    void         ReadInterfaceOrShellEntry( SvMetaClass& rClass );
    bool         ReadSlot( SvMetaSlot& rSlot );
    void         ReadInterfaceOrShellMethod( SvMetaAttribute& rAttr );
    void         ReadItem();
    void         ReadStruct();
    void         ReadEnum();
    void         ReadEnumValue( SvMetaTypeEnum& rEnum );
    void         ReadSlotId(SvIdentifier& rSlotId);
    void         ReadSlotAttribute( SvMetaSlot& rSlot );
    SvMetaClass* ReadKnownClass();
    SvMetaType*  ReadKnownType();
    void         Read(char cChar);
    bool         ReadIfBoolAttribute( SvBOOL&, SvStringHashEntry const * pName);
    void         ReadIfIdAttribute( SvIdentifier& rIdentifier, SvStringHashEntry const * pName );
    bool         ReadIf(char cChar);
    void         ReadDelimiter();
    bool         ReadIfDelimiter();
    OString      ReadIdentifier();
    OString      ReadString();
    void         Read(SvStringHashEntry const *);
    bool         ReadIf(SvStringHashEntry const *);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
