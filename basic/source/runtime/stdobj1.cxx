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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <svtools/transfer.hxx>
#include "runtime.hxx"
#include <basic/sbstdobj.hxx>

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



void SbStdPicture::PropType( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( SbERR_PROP_READONLY );
        return;
    }

    GraphicType eType = aGraphic.GetType();
    sal_Int16 nType = 0;

    if( eType == GRAPHIC_BITMAP )
        nType = 1;
    else
    if( eType != GRAPHIC_NONE )
        nType = 2;

    pVar->PutInteger( nType );
}


void SbStdPicture::PropWidth( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( SbERR_PROP_READONLY );
        return;
    }

    Size aSize = aGraphic.GetPrefSize();
    aSize = GetpApp()->GetAppWindow()->LogicToPixel( aSize, aGraphic.GetPrefMapMode() );
    aSize = GetpApp()->GetAppWindow()->PixelToLogic( aSize, MapMode( MAP_TWIP ) );

    pVar->PutInteger( (sal_Int16)aSize.Width() );
}

void SbStdPicture::PropHeight( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( SbERR_PROP_READONLY );
        return;
    }

    Size aSize = aGraphic.GetPrefSize();
    aSize = GetpApp()->GetAppWindow()->LogicToPixel( aSize, aGraphic.GetPrefMapMode() );
    aSize = GetpApp()->GetAppWindow()->PixelToLogic( aSize, MapMode( MAP_TWIP ) );

    pVar->PutInteger( (sal_Int16)aSize.Height() );
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
        SbxArray*    pPar_  = pVar->GetParameters();
        sal_uInt16       nWhich = (sal_uInt16)pVar->GetUserData();
        sal_Bool         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Propteries
        switch( nWhich )
        {
            case ATTR_IMP_TYPE:     PropType( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_WIDTH:    PropWidth( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_HEIGHT:   PropHeight( pVar, pPar_, bWrite ); return;
        }

        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

//-----------------------------------------------------------------------------

void SbStdFont::PropBold( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
        SetBold( pVar->GetBool() );
    else
        pVar->PutBool( IsBold() );
}

void SbStdFont::PropItalic( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
        SetItalic( pVar->GetBool() );
    else
        pVar->PutBool( IsItalic() );
}

void SbStdFont::PropStrikeThrough( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
        SetStrikeThrough( pVar->GetBool() );
    else
        pVar->PutBool( IsStrikeThrough() );
}

void SbStdFont::PropUnderline( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
        SetUnderline( pVar->GetBool() );
    else
        pVar->PutBool( IsUnderline() );
}

void SbStdFont::PropSize( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
{
    if( bWrite )
        SetSize( (sal_uInt16)pVar->GetInteger() );
    else
        pVar->PutInteger( (sal_Int16)GetSize() );
}

void SbStdFont::PropName( SbxVariable* pVar, SbxArray*, sal_Bool bWrite )
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
        SbxArray*    pPar_  = pVar->GetParameters();
        sal_uInt16       nWhich = (sal_uInt16)pVar->GetUserData();
        sal_Bool         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Propteries
        switch( nWhich )
        {
            case ATTR_IMP_BOLD:         PropBold( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_ITALIC:       PropItalic( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_STRIKETHROUGH:PropStrikeThrough( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_UNDERLINE:    PropUnderline( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_SIZE:         PropSize( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_NAME:         PropName( pVar, pPar_, bWrite ); return;
        }

        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


//-----------------------------------------------------------------------------


void SbStdClipboard::MethClear( SbxVariable*, SbxArray* pPar_, sal_Bool )
{
    if( pPar_ && (pPar_->Count() > 1) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

}

void SbStdClipboard::MethGetData( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    (void)pVar;

    if( !pPar_ || (pPar_->Count() != 2) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_uInt16 nFormat = pPar_->Get(1)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

}

void SbStdClipboard::MethGetFormat( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    if( !pPar_ || (pPar_->Count() != 2) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_uInt16 nFormat = pPar_->Get(1)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    pVar->PutBool( sal_False );
}

void SbStdClipboard::MethGetText( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    if( pPar_ && (pPar_->Count() > 1) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    pVar->PutString( String() );
}

void SbStdClipboard::MethSetData( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    (void)pVar;

    if( !pPar_ || (pPar_->Count() != 3) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_uInt16 nFormat = pPar_->Get(2)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

}

void SbStdClipboard::MethSetText( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    (void)pVar;

    if( !pPar_ || (pPar_->Count() != 2) )
    {
        StarBASIC::Error( SbERR_BAD_NUMBER_OF_ARGS );
        return;
    }

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
        SbxArray*    pPar_  = pVar->GetParameters();
        sal_uInt16       nWhich = (sal_uInt16)pVar->GetUserData();
        sal_Bool         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Methods
        switch( nWhich )
        {
            case METH_CLEAR:            MethClear( pVar, pPar_, bWrite ); return;
            case METH_GETDATA:          MethGetData( pVar, pPar_, bWrite ); return;
            case METH_GETFORMAT:        MethGetFormat( pVar, pPar_, bWrite ); return;
            case METH_GETTEXT:          MethGetText( pVar, pPar_, bWrite ); return;
            case METH_SETDATA:          MethSetData( pVar, pPar_, bWrite ); return;
            case METH_SETTEXT:          MethSetText( pVar, pPar_, bWrite ); return;
        }

        SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
