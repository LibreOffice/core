/*************************************************************************
 *
 *  $RCSfile: stdobj1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#include "runtime.hxx"
#pragma hdrstop
#include "stdobj1.hxx"

#include "segmentc.hxx"
//#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

#define ATTR_IMP_TYPE           1
#define ATTR_IMP_WIDTH          2
#define ATTR_IMP_HEIGHT         3
#define ATTR_IMP_BOLD           4
#define ATTR_IMP_ITALIC         5
#define ATTR_IMP_STRIKETHROUGH  6
#define ATTR_IMP_UNDERLINE      7
#define ATTR_IMP_WEIGHT         8
#define ATTR_IMP_SIZE           9
#define ATTR_IMP_NAME           10

#define METH_CLEAR              20
#define METH_GETDATA            21
#define METH_GETFORMAT          22
#define METH_GETTEXT            23
#define METH_SETDATA            24
#define METH_SETTEXT            25

//------------------------------------------------------------------------------
SbStdFactory::SbStdFactory()
{
}

SbxObject* SbStdFactory::CreateObject( const String& rClassName )
{
    if( rClassName.EqualsIgnoreCaseAscii( String( RTL_CONSTASCII_USTRINGPARAM("Picture") ) ) )
        return new SbStdPicture;
    else
        if( rClassName.EqualsIgnoreCaseAscii( String( RTL_CONSTASCII_USTRINGPARAM("Font") ) ) )
        return new SbStdFont;
    else
        return NULL;
}

//------------------------------------------------------------------------------



void SbStdPicture::PropType( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( SbERR_PROP_READONLY );
        return;
    }

    GraphicType eType = aGraphic.GetType();
    INT16 nType = 0;

    if( eType == GRAPHIC_BITMAP )
        nType = 1;
    else
    if( eType != GRAPHIC_NONE )
        nType = 2;

    pVar->PutInteger( nType );
}


void SbStdPicture::PropWidth( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( SbERR_PROP_READONLY );
        return;
    }

    Size aSize = aGraphic.GetPrefSize();
    aSize = GetpApp()->GetAppWindow()->LogicToPixel( aSize, aGraphic.GetPrefMapMode() );
    aSize = GetpApp()->GetAppWindow()->PixelToLogic( aSize, MapMode( MAP_TWIP ) );

    pVar->PutInteger( (INT16)aSize.Width() );
}

void SbStdPicture::PropHeight( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( SbERR_PROP_READONLY );
        return;
    }

    Size aSize = aGraphic.GetPrefSize();
    aSize = GetpApp()->GetAppWindow()->LogicToPixel( aSize, aGraphic.GetPrefMapMode() );
    aSize = GetpApp()->GetAppWindow()->PixelToLogic( aSize, MapMode( MAP_TWIP ) );

    pVar->PutInteger( (INT16)aSize.Height() );
}


TYPEINIT1( SbStdPicture, SbxObject );

SbStdPicture::SbStdPicture() :
    SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("Picture") ) )
{
    // Properties
    SbxVariable* p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Type") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READ | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_TYPE );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Width") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READ | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_WIDTH );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Height") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READ | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_HEIGHT );
}

SbStdPicture::~SbStdPicture()
{
}


SbxVariable* SbStdPicture::Find( const String& rName, SbxClassType t )
{
    // Bereits eingetragen?
    return SbxObject::Find( rName, t );
}



void SbStdPicture::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                               const SfxHint& rHint, const TypeId& rHintType )

{
    const SbxHint* pHint = PTR_CAST( SbxHint, &rHint );

    if( pHint )
    {
        if( pHint->GetId() == SBX_HINT_INFOWANTED )
        {
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
            return;
        }

        SbxVariable* pVar   = pHint->GetVar();
        SbxArray*    pPar   = pVar->GetParameters();
        USHORT       nWhich = (USHORT)pVar->GetUserData();
        BOOL         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Propteries
        switch( nWhich )
        {
            case ATTR_IMP_TYPE:     PropType( pVar, pPar, bWrite ); return;
            case ATTR_IMP_WIDTH:    PropWidth( pVar, pPar, bWrite ); return;
            case ATTR_IMP_HEIGHT:   PropHeight( pVar, pPar, bWrite ); return;
        }

        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

//-----------------------------------------------------------------------------

void SbStdFont::PropBold( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
        SetBold( pVar->GetBool() );
    else
        pVar->PutBool( IsBold() );
}

void SbStdFont::PropItalic( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
        SetItalic( pVar->GetBool() );
    else
        pVar->PutBool( IsItalic() );
}

void SbStdFont::PropStrikeThrough( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
        SetStrikeThrough( pVar->GetBool() );
    else
        pVar->PutBool( IsStrikeThrough() );
}

void SbStdFont::PropUnderline( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
        SetUnderline( pVar->GetBool() );
    else
        pVar->PutBool( IsUnderline() );
}

void SbStdFont::PropSize( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
        SetSize( (USHORT)pVar->GetInteger() );
    else
        pVar->PutInteger( (INT16)GetSize() );
}

void SbStdFont::PropName( SbxVariable* pVar, SbxArray*, BOOL bWrite )
{
    if( bWrite )
        SetFontName( pVar->GetString() );
    else
        pVar->PutString( GetFontName() );
}


TYPEINIT1( SbStdFont, SbxObject );

SbStdFont::SbStdFont() :
    SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("Font") ) )
{
    // Properties
    SbxVariable* p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Bold") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READWRITE | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_BOLD );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Italic") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READWRITE | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_ITALIC );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("StrikeThrough") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READWRITE | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_STRIKETHROUGH );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Underline") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READWRITE | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_UNDERLINE );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Size") ), SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SBX_READWRITE | SBX_DONTSTORE );
    p->SetUserData( ATTR_IMP_SIZE );

    // Name Property selbst verarbeiten
    p = Find( String( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_PROPERTY );
    DBG_ASSERT( p, "Keine Name Property" );
    p->SetUserData( ATTR_IMP_NAME );
}

SbStdFont::~SbStdFont()
{
}


SbxVariable* SbStdFont::Find( const String& rName, SbxClassType t )
{
    // Bereits eingetragen?
    return SbxObject::Find( rName, t );
}



void SbStdFont::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST( SbxHint, &rHint );

    if( pHint )
    {
        if( pHint->GetId() == SBX_HINT_INFOWANTED )
        {
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
            return;
        }

        SbxVariable* pVar   = pHint->GetVar();
        SbxArray*    pPar   = pVar->GetParameters();
        USHORT       nWhich = (USHORT)pVar->GetUserData();
        BOOL         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Propteries
        switch( nWhich )
        {
            case ATTR_IMP_BOLD:         PropBold( pVar, pPar, bWrite ); return;
            case ATTR_IMP_ITALIC:       PropItalic( pVar, pPar, bWrite ); return;
            case ATTR_IMP_STRIKETHROUGH:PropStrikeThrough( pVar, pPar, bWrite ); return;
            case ATTR_IMP_UNDERLINE:    PropUnderline( pVar, pPar, bWrite ); return;
            case ATTR_IMP_SIZE:         PropSize( pVar, pPar, bWrite ); return;
            case ATTR_IMP_NAME:         PropName( pVar, pPar, bWrite ); return;
        }

        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


//-----------------------------------------------------------------------------
void SbStdClipboard::MethClear( SbxVariable*, SbxArray* pPar, BOOL )
{
    if( pPar && (pPar->Count() > 1) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    Clipboard::Clear();
}

void SbStdClipboard::MethGetData( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    if( !pPar || (pPar->Count() != 2) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    USHORT nFormat = pPar->Get(1)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    if( nFormat == FORMAT_STRING )
        pVar->PutString( Clipboard::PasteString() );
    else
    if( (nFormat == FORMAT_BITMAP) ||
        (nFormat == FORMAT_GDIMETAFILE ) )
    {
        SbxObjectRef xPic = new SbStdPicture;
        Graphic aGraph;
        aGraph.Paste();
        ((SbStdPicture*)(SbxObject*)xPic)->SetGraphic( aGraph );
        pVar->PutObject( xPic );
    }
}

void SbStdClipboard::MethGetFormat( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    if( !pPar || (pPar->Count() != 2) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    USHORT nFormat = pPar->Get(1)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    pVar->PutBool( Clipboard::HasFormat( nFormat ) );
}

void SbStdClipboard::MethGetText( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    if( pPar && (pPar->Count() > 1) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    pVar->PutString( Clipboard::PasteString() );
}

void SbStdClipboard::MethSetData( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    if( !pPar || (pPar->Count() != 3) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    USHORT nFormat = pPar->Get(2)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    if( nFormat == FORMAT_STRING )
    {
        Clipboard::CopyString( pPar->Get(1)->GetString() );
    }
    else
    if( (nFormat == FORMAT_BITMAP) ||
        (nFormat == FORMAT_GDIMETAFILE) )
    {
        SbxObject* pObj = (SbxObject*)pPar->Get(1)->GetObject();

        if( pObj && pObj->IsA( TYPE( SbStdPicture ) ) )
            ((SbStdPicture*)(SbxObject*)pObj)->GetGraphic().Copy();
    }
}

void SbStdClipboard::MethSetText( SbxVariable* pVar, SbxArray* pPar, BOOL )
{
    if( !pPar || (pPar->Count() != 2) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    Clipboard::CopyString( pPar->Get(1)->GetString() );
}


TYPEINIT1( SbStdClipboard, SbxObject );

SbStdClipboard::SbStdClipboard() :
    SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("Clipboard") ) )
{
    // Name Property selbst verarbeiten
    SbxVariable* p = Find( String( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_PROPERTY );
    DBG_ASSERT( p, "Keine Name Property" );
    p->SetUserData( ATTR_IMP_NAME );

    //Methoden registrieren
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("Clear") ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p->SetUserData( METH_CLEAR );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("GetData") ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p->SetUserData( METH_GETDATA );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("GetFormat") ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p->SetUserData( METH_GETFORMAT );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("GetText") ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p->SetUserData( METH_GETTEXT );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("SetData") ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p->SetUserData( METH_SETDATA );
    p = Make( String( RTL_CONSTASCII_USTRINGPARAM("SetText") ), SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SBX_DONTSTORE );
    p->SetUserData( METH_SETTEXT );
}

SbStdClipboard::~SbStdClipboard()
{
}


SbxVariable* SbStdClipboard::Find( const String& rName, SbxClassType t )
{
    // Bereits eingetragen?
    return SbxObject::Find( rName, t );
}



void SbStdClipboard::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                 const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST( SbxHint, &rHint );

    if( pHint )
    {
        if( pHint->GetId() == SBX_HINT_INFOWANTED )
        {
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
            return;
        }

        SbxVariable* pVar   = pHint->GetVar();
        SbxArray*    pPar   = pVar->GetParameters();
        USHORT       nWhich = (USHORT)pVar->GetUserData();
        BOOL         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Methods
        switch( nWhich )
        {
            case METH_CLEAR:            MethClear( pVar, pPar, bWrite ); return;
            case METH_GETDATA:          MethGetData( pVar, pPar, bWrite ); return;
            case METH_GETFORMAT:        MethGetFormat( pVar, pPar, bWrite ); return;
            case METH_GETTEXT:          MethGetText( pVar, pPar, bWrite ); return;
            case METH_SETDATA:          MethSetData( pVar, pPar, bWrite ); return;
            case METH_SETTEXT:          MethSetText( pVar, pPar, bWrite ); return;
        }

        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


