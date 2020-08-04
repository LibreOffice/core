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

#include <basic/sbxobj.hxx>
#include <vcl/graph.hxx>
#include "sbxfac.hxx"
#include <basic/basicdllapi.h>

class SbStdFactory final : public SbxFactory
{
public:
    SbStdFactory();

    virtual SbxObject*  CreateObject( const OUString& rClassName ) override;
};

class SbStdPicture final : public SbxObject
{
    Graphic     aGraphic;

    virtual ~SbStdPicture() override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void    PropType( SbxVariable* pVar, bool bWrite );
    void    PropWidth( SbxVariable* pVar, bool bWrite );
    void    PropHeight( SbxVariable* pVar, bool bWrite );

public:

    SbStdPicture();

    const Graphic& GetGraphic() const { return aGraphic; }
    void    SetGraphic( const Graphic& rGrf ) { aGraphic = rGrf; }
};

class SbStdFont final : public SbxObject
{
    bool    bBold;
    bool    bItalic;
    bool    bStrikeThrough;
    bool    bUnderline;
    sal_uInt16  nSize;
    OUString  aName;

    virtual ~SbStdFont() override;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void    PropBold( SbxVariable* pVar, bool bWrite );
    void    PropItalic( SbxVariable* pVar, bool bWrite );
    void    PropStrikeThrough( SbxVariable* pVar, bool bWrite );
    void    PropUnderline( SbxVariable* pVar, bool bWrite );
    void    PropSize( SbxVariable* pVar, bool bWrite );
    void    PropName( SbxVariable* pVar, bool bWrite );

public:

    SbStdFont();

    void     SetBold( bool bB ) { bBold = bB; }
    bool     IsBold() const { return bBold; }
    void     SetItalic( bool bI ) { bItalic = bI; }
    bool     IsItalic() const { return bItalic; }
    void     SetStrikeThrough( bool bS ) { bStrikeThrough = bS; }
    bool     IsStrikeThrough() const { return bStrikeThrough; }
    void     SetUnderline( bool bU ) { bUnderline = bU; }
    bool     IsUnderline() const { return bUnderline; }
    void     SetSize( sal_uInt16 nS ) { nSize = nS; }
    sal_uInt16 GetSize() const { return nSize; }
};

class SbStdClipboard final : public SbxObject
{
    virtual ~SbStdClipboard() override;
    virtual void   Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    static void    MethClear( SbxArray const * pPar_ );
    static void    MethGetData( SbxArray* pPar_ );
    static void    MethGetFormat( SbxVariable* pVar, SbxArray* pPar_ );
    static void    MethGetText( SbxVariable* pVar, SbxArray const * pPar_ );
    static void    MethSetData( SbxArray* pPar_ );
    static void    MethSetText( SbxArray const * pPar_ );

public:

    SbStdClipboard();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
