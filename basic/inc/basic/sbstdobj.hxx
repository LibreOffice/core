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

#ifndef _SBSTDOBJ1_HXX
#define _SBSTDOBJ1_HXX

#include <basic/sbxobj.hxx>
#include <vcl/graph.hxx>
#include <basic/sbxfac.hxx>
class StarBASIC;
class SbStdFactory;

//--------------------
// class SbStdFactory
//--------------------
class SbStdFactory : public SbxFactory
{
public:
    SbStdFactory();

    virtual SbxObject*  CreateObject( const String& rClassName );
};

//--------------------
// class SbStdPicture
//--------------------
class SbStdPicture : public SbxObject
{
protected:
    Graphic     aGraphic;

   ~SbStdPicture();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    void    PropType( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropWidth( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropHeight( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );

public:
    TYPEINFO();

    SbStdPicture();
    virtual SbxVariable* Find( const String&, SbxClassType );

    Graphic GetGraphic() const { return aGraphic; }
    void    SetGraphic( const Graphic& rGrf ) { aGraphic = rGrf; }
};

//-----------------
// class SbStdFont
//-----------------
class SbStdFont : public SbxObject
{
protected:
    BOOL    bBold;
    BOOL    bItalic;
    BOOL    bStrikeThrough;
    BOOL    bUnderline;
    USHORT  nSize;
    String  aName;

   ~SbStdFont();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    void    PropBold( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropItalic( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropStrikeThrough( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropUnderline( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropSize( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );
    void    PropName( SbxVariable* pVar, SbxArray* pPar, BOOL bWrite );

public:
    TYPEINFO();

    SbStdFont();
    virtual SbxVariable* Find( const String&, SbxClassType );

    void    SetBold( BOOL bB ) { bBold = bB; }
    BOOL    IsBold() const { return bBold; }
    void    SetItalic( BOOL bI ) { bItalic = bI; }
    BOOL    IsItalic() const { return bItalic; }
    void    SetStrikeThrough( BOOL bS ) { bStrikeThrough = bS; }
    BOOL    IsStrikeThrough() const { return bStrikeThrough; }
    void    SetUnderline( BOOL bU ) { bUnderline = bU; }
    BOOL    IsUnderline() const { return bUnderline; }
    void    SetSize( USHORT nS ) { nSize = nS; }
    USHORT  GetSize() const { return nSize; }
    void    SetFontName( const String& rName ) { aName = rName; }
    String  GetFontName() const { return aName; }
};

//----------------------
// class SbStdClipboard
//----------------------
class SbStdClipboard : public SbxObject
{
protected:

   ~SbStdClipboard();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    void    MethClear( SbxVariable* pVar, SbxArray* pPar_, BOOL bWrite );
    void    MethGetData( SbxVariable* pVar, SbxArray* pPar_, BOOL bWrite );
    void    MethGetFormat( SbxVariable* pVar, SbxArray* pPar_, BOOL bWrite );
    void    MethGetText( SbxVariable* pVar, SbxArray* pPar_, BOOL bWrite );
    void    MethSetData( SbxVariable* pVar, SbxArray* pPar_, BOOL bWrite );
    void    MethSetText( SbxVariable* pVar, SbxArray* pPar_, BOOL bWrite );

public:
    TYPEINFO();

    SbStdClipboard();
    virtual SbxVariable* Find( const String&, SbxClassType );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
