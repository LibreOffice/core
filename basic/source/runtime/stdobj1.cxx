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

#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <svtools/transfer.hxx>
#include "runtime.hxx"
#include "sbstdobj.hxx"

#define ATTR_IMP_TYPE           1
#define ATTR_IMP_WIDTH          2
#define ATTR_IMP_HEIGHT         3
#define ATTR_IMP_BOLD           4
#define ATTR_IMP_ITALIC         5
#define ATTR_IMP_STRIKETHROUGH  6
#define ATTR_IMP_UNDERLINE      7
#define ATTR_IMP_SIZE           9
#define ATTR_IMP_NAME           10

#define METH_CLEAR              20
#define METH_GETDATA            21
#define METH_GETFORMAT          22
#define METH_GETTEXT            23
#define METH_SETDATA            24
#define METH_SETTEXT            25


SbStdFactory::SbStdFactory()
{
}

SbxObject* SbStdFactory::CreateObject( const OUString& rClassName )
{
    if( rClassName.equalsIgnoreAsciiCase("Picture") )
        return new SbStdPicture;
    else if( rClassName.equalsIgnoreAsciiCase("Font") )
        return new SbStdFont;
    else
        return nullptr;
}





void SbStdPicture::PropType( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
        return;
    }

    GraphicType eType = aGraphic.GetType();
    sal_Int16 nType = 0;

    if( eType == GRAPHIC_BITMAP )
        nType = 1;
    else if( eType != GRAPHIC_NONE )
        nType = 2;

    pVar->PutInteger( nType );
}


void SbStdPicture::PropWidth( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
        return;
    }

    Size aSize = aGraphic.GetPrefSize();
    aSize = Application::GetAppWindow()->LogicToPixel( aSize, aGraphic.GetPrefMapMode() );
    aSize = Application::GetAppWindow()->PixelToLogic( aSize, MapMode( MAP_TWIP ) );

    pVar->PutInteger( (sal_Int16)aSize.Width() );
}

void SbStdPicture::PropHeight( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
        return;
    }

    Size aSize = aGraphic.GetPrefSize();
    aSize = Application::GetAppWindow()->LogicToPixel( aSize, aGraphic.GetPrefMapMode() );
    aSize = Application::GetAppWindow()->PixelToLogic( aSize, MapMode( MAP_TWIP ) );

    pVar->PutInteger( (sal_Int16)aSize.Height() );
}



SbStdPicture::SbStdPicture() :
    SbxObject( OUString("Picture"))
{
    // Properties
    SbxVariable* p = Make( "Type", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_TYPE );
    p = Make( "Width", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_WIDTH );
    p = Make( "Height", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_HEIGHT );
}

SbStdPicture::~SbStdPicture()
{
}


SbxVariable* SbStdPicture::Find( const OUString& rName, SbxClassType t )
{
    // entered already?
    return SbxObject::Find( rName, t );
}



void SbStdPicture::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )

{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);

    if( pHint )
    {
        if( pHint->GetId() == SBX_HINT_INFOWANTED )
        {
            SbxObject::Notify( rBC, rHint );
            return;
        }

        SbxVariable* pVar   = pHint->GetVar();
        SbxArray*    pPar_  = pVar->GetParameters();
        const sal_uInt32 nWhich = pVar->GetUserData();
        bool         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

        // Propteries
        switch( nWhich )
        {
            case ATTR_IMP_TYPE:     PropType( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_WIDTH:    PropWidth( pVar, pPar_, bWrite ); return;
            case ATTR_IMP_HEIGHT:   PropHeight( pVar, pPar_, bWrite ); return;
        }

        SbxObject::Notify( rBC, rHint );
    }
}



void SbStdFont::PropBold( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
        SetBold( pVar->GetBool() );
    else
        pVar->PutBool( IsBold() );
}

void SbStdFont::PropItalic( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
        SetItalic( pVar->GetBool() );
    else
        pVar->PutBool( IsItalic() );
}

void SbStdFont::PropStrikeThrough( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
        SetStrikeThrough( pVar->GetBool() );
    else
        pVar->PutBool( IsStrikeThrough() );
}

void SbStdFont::PropUnderline( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
        SetUnderline( pVar->GetBool() );
    else
        pVar->PutBool( IsUnderline() );
}

void SbStdFont::PropSize( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
        SetSize( (sal_uInt16)pVar->GetInteger() );
    else
        pVar->PutInteger( (sal_Int16)GetSize() );
}

void SbStdFont::PropName( SbxVariable* pVar, SbxArray*, bool bWrite )
{
    if( bWrite )
    {
        SetFontName( pVar->GetOUString() );
    }
    else
    {
        pVar->PutString( GetFontName() );
    }
}


SbStdFont::SbStdFont()
    : SbxObject( OUString("Font") )
    , bBold(false)
    , bItalic(false)
    , bStrikeThrough(false)
    , bUnderline(false)
    , nSize(0)
{
    // Properties
    SbxVariable* p = Make( "Bold", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_BOLD );
    p = Make( "Italic", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_ITALIC );
    p = Make( "StrikeThrough", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_STRIKETHROUGH );
    p = Make( "Underline", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_UNDERLINE );
    p = Make( "Size", SbxCLASS_PROPERTY, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_SIZE );

    // handle name property yourself
    p = Find( "Name", SbxCLASS_PROPERTY );
    assert(p && "No Name Property");
    p->SetUserData( ATTR_IMP_NAME );
}

SbStdFont::~SbStdFont()
{
}

SbxVariable* SbStdFont::Find( const OUString& rName, SbxClassType t )
{
    return SbxObject::Find( rName, t );
}

void SbStdFont::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);

    if( pHint )
    {
        if( pHint->GetId() == SBX_HINT_INFOWANTED )
        {
            SbxObject::Notify( rBC, rHint );
            return;
        }

        SbxVariable* pVar   = pHint->GetVar();
        SbxArray*    pPar_  = pVar->GetParameters();
        const sal_uInt32 nWhich = pVar->GetUserData();
        bool         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

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

        SbxObject::Notify( rBC, rHint );
    }
}





void SbStdClipboard::MethClear( SbxVariable*, SbxArray* pPar_, bool )
{
    if( pPar_ && (pPar_->Count() > 1) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

}

void SbStdClipboard::MethGetData( SbxVariable* pVar, SbxArray* pPar_, bool )
{
    (void)pVar;

    if( !pPar_ || (pPar_->Count() != 2) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_uInt16 nFormat = pPar_->Get(1)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

}

void SbStdClipboard::MethGetFormat( SbxVariable* pVar, SbxArray* pPar_, bool )
{
    if( !pPar_ || (pPar_->Count() != 2) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_uInt16 nFormat = pPar_->Get(1)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    pVar->PutBool( false );
}

void SbStdClipboard::MethGetText( SbxVariable* pVar, SbxArray* pPar_, bool )
{
    if( pPar_ && (pPar_->Count() > 1) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    pVar->PutString( OUString() );
}

void SbStdClipboard::MethSetData( SbxVariable* pVar, SbxArray* pPar_, bool )
{
    (void)pVar;

    if( !pPar_ || (pPar_->Count() != 3) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_uInt16 nFormat = pPar_->Get(2)->GetInteger();
    if( !nFormat  || nFormat > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

}

void SbStdClipboard::MethSetText( SbxVariable* pVar, SbxArray* pPar_, bool )
{
    (void)pVar;

    if( !pPar_ || (pPar_->Count() != 2) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

}



SbStdClipboard::SbStdClipboard() :
    SbxObject( OUString("Clipboard") )
{
    SbxVariable* p = Find( "Name", SbxCLASS_PROPERTY );
    assert(p && "No Name Property");
    p->SetUserData( ATTR_IMP_NAME );

    // register methods
    p = Make( "Clear", SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_CLEAR );
    p = Make( "GetData", SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_GETDATA );
    p = Make( "GetFormat", SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_GETFORMAT );
    p = Make( "GetText", SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_GETTEXT );
    p = Make( "SetData", SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_SETDATA );
    p = Make( "SetText", SbxCLASS_METHOD, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_SETTEXT );
}

SbStdClipboard::~SbStdClipboard()
{
}


SbxVariable* SbStdClipboard::Find( const OUString& rName, SbxClassType t )
{
    return SbxObject::Find( rName, t );
}



void SbStdClipboard::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);

    if( pHint )
    {
        if( pHint->GetId() == SBX_HINT_INFOWANTED )
        {
            SbxObject::Notify( rBC, rHint );
            return;
        }

        SbxVariable* pVar   = pHint->GetVar();
        SbxArray*    pPar_  = pVar->GetParameters();
        const sal_uInt32 nWhich = pVar->GetUserData();
        bool         bWrite = pHint->GetId() == SBX_HINT_DATACHANGED;

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

        SbxObject::Notify( rBC, rHint );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
