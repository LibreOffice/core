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

#ifndef INCLUDED_BASIC_INC_SBSTDOBJ_HXX
#define INCLUDED_BASIC_INC_SBSTDOBJ_HXX

#include <basic/sbxobj.hxx>
#include <vcl/graph.hxx>
#include <basic/sbxfac.hxx>
#include <basic/basicdllapi.h>

// class SbStdFactory
class BASIC_DLLPUBLIC SbStdFactory : public SbxFactory
{
public:
    SbStdFactory();

    virtual SbxObject*  CreateObject( const OUString& rClassName );
};

// class SbStdPicture
class BASIC_DLLPUBLIC SbStdPicture : public SbxObject
{
protected:
    Graphic     aGraphic;

   ~SbStdPicture();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    void    PropType( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropWidth( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropHeight( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

public:
    TYPEINFO_OVERRIDE();

    SbStdPicture();
    virtual SbxVariable* Find( const OUString&, SbxClassType );

    Graphic GetGraphic() const { return aGraphic; }
    void    SetGraphic( const Graphic& rGrf ) { aGraphic = rGrf; }
};

// class SbStdFont
class BASIC_DLLPUBLIC SbStdFont : public SbxObject
{
protected:
    sal_Bool    bBold;
    sal_Bool    bItalic;
    sal_Bool    bStrikeThrough;
    sal_Bool    bUnderline;
    sal_uInt16  nSize;
    OUString  aName;

   ~SbStdFont();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    void    PropBold( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropItalic( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropStrikeThrough( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropUnderline( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropSize( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropName( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

public:
    TYPEINFO_OVERRIDE();

    SbStdFont();
    virtual SbxVariable* Find( const OUString&, SbxClassType );

    void     SetBold( sal_Bool bB ) { bBold = bB; }
    sal_Bool IsBold() const { return bBold; }
    void     SetItalic( sal_Bool bI ) { bItalic = bI; }
    sal_Bool IsItalic() const { return bItalic; }
    void     SetStrikeThrough( sal_Bool bS ) { bStrikeThrough = bS; }
    sal_Bool IsStrikeThrough() const { return bStrikeThrough; }
    void     SetUnderline( sal_Bool bU ) { bUnderline = bU; }
    sal_Bool IsUnderline() const { return bUnderline; }
    void     SetSize( sal_uInt16 nS ) { nSize = nS; }
    sal_uInt16 GetSize() const { return nSize; }
    void     SetFontName( const OUString& rName ) { aName = rName; }
    OUString GetFontName() const { return aName; }
};

// class SbStdClipboard
class BASIC_DLLPUBLIC SbStdClipboard : public SbxObject
{
protected:

   ~SbStdClipboard();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );

    void    MethClear( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethGetData( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethGetFormat( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethGetText( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethSetData( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethSetText( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );

public:
    TYPEINFO_OVERRIDE();

    SbStdClipboard();
    virtual SbxVariable* Find( const OUString&, SbxClassType );
};

#endif // INCLUDED_BASIC_INC_SBSTDOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
