/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SBSTDOBJ1_HXX
#define _SBSTDOBJ1_HXX

#include <basic/sbxobj.hxx>
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
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

    void    PropType( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropWidth( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );
    void    PropHeight( SbxVariable* pVar, SbxArray* pPar, sal_Bool bWrite );

public:
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
    sal_Bool    bBold;
    sal_Bool    bItalic;
    sal_Bool    bStrikeThrough;
    sal_Bool    bUnderline;
    sal_uInt16  nSize;
    String  aName;

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
    SbStdFont();
    virtual SbxVariable* Find( const String&, SbxClassType );

    void    SetBold( sal_Bool bB ) { bBold = bB; }
    sal_Bool    IsBold() const { return bBold; }
    void    SetItalic( sal_Bool bI ) { bItalic = bI; }
    sal_Bool    IsItalic() const { return bItalic; }
    void    SetStrikeThrough( sal_Bool bS ) { bStrikeThrough = bS; }
    sal_Bool    IsStrikeThrough() const { return bStrikeThrough; }
    void    SetUnderline( sal_Bool bU ) { bUnderline = bU; }
    sal_Bool    IsUnderline() const { return bUnderline; }
    void    SetSize( sal_uInt16 nS ) { nSize = nS; }
    sal_uInt16  GetSize() const { return nSize; }
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

    void    MethClear( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethGetData( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethGetFormat( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethGetText( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethSetData( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );
    void    MethSetText( SbxVariable* pVar, SbxArray* pPar_, sal_Bool bWrite );

public:
    SbStdClipboard();
    virtual SbxVariable* Find( const String&, SbxClassType );
};

#endif
