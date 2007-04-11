/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbstdobj.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:52:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SBSTDOBJ1_HXX
#define _SBSTDOBJ1_HXX

#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <basic/sbxobj.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef __SBX_SBX_FACTORY_HXX //autogen
#include <basic/sbxfac.hxx>
#endif
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
