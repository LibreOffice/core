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

#include <basic/sberrors.hxx>
#include <basic/sbstar.hxx>
#include <vcl/outdev.hxx>
#include <sbstdobj.hxx>

namespace {
    enum class ATTR_IMP
    {
        TYPE           = 1,
        WIDTH          = 2,
        HEIGHT         = 3,
        BOLD           = 4,
        ITALIC         = 5,
        STRIKETHROUGH  = 6,
        UNDERLINE      = 7,
        SIZE           = 9,
        NAME           = 10
    };

    enum class METH
    {
        CLEAR     = 20,
        GETDATA   = 21,
        GETFORMAT = 22,
        GETTEXT   = 23,
        SETDATA   = 24,
        SETTEXT   = 25
    };
}

SbStdFactory::SbStdFactory()
{
}

SbxObjectRef SbStdFactory::CreateObject( const OUString& rClassName )
{
    if( rClassName.equalsIgnoreAsciiCase("Picture") )
        return new SbStdPicture;
    else if( rClassName.equalsIgnoreAsciiCase("Font") )
        return new SbStdFont;
    else
        return nullptr;
}


void SbStdPicture::PropType( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
        return;
    }

    GraphicType eType = aGraphic.GetType();
    sal_Int16 nType = 0;

    if( eType == GraphicType::Bitmap )
        nType = 1;
    else if( eType != GraphicType::NONE )
        nType = 2;

    pVar->PutInteger( nType );
}


void SbStdPicture::PropWidth( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
        return;
    }

    Size aSize = OutputDevice::LogicToLogic(aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), MapMode(MapUnit::MapTwip));
    pVar->PutInteger( static_cast<sal_Int16>(aSize.Width()) );
}

void SbStdPicture::PropHeight( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROP_READONLY );
        return;
    }

    Size aSize = OutputDevice::LogicToLogic(aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), MapMode(MapUnit::MapTwip));
    pVar->PutInteger( static_cast<sal_Int16>(aSize.Height()) );
}


SbStdPicture::SbStdPicture() :
    SbxObject( "Picture" )
{
    // Properties
    SbxVariable* p = Make( "Type", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::TYPE) );
    p = Make( "Width", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::WIDTH) );
    p = Make( "Height", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::HEIGHT) );
}

SbStdPicture::~SbStdPicture()
{
}


void SbStdPicture::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )

{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);

    if( !pHint )
        return;

    if( pHint->GetId() == SfxHintId::BasicInfoWanted )
    {
        SbxObject::Notify( rBC, rHint );
        return;
    }

    SbxVariable* pVar   = pHint->GetVar();
    const sal_uInt32 nWhich = pVar->GetUserData();
    bool         bWrite = pHint->GetId() == SfxHintId::BasicDataChanged;

    // Properties
    switch( nWhich )
    {
        case static_cast<sal_uInt32>(ATTR_IMP::TYPE):     PropType( pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::WIDTH):    PropWidth( pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::HEIGHT):   PropHeight( pVar, bWrite ); return;
    }

    SbxObject::Notify( rBC, rHint );
}


void SbStdFont::PropBold( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
        SetBold( pVar->GetBool() );
    else
        pVar->PutBool( IsBold() );
}

void SbStdFont::PropItalic( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
        SetItalic( pVar->GetBool() );
    else
        pVar->PutBool( IsItalic() );
}

void SbStdFont::PropStrikeThrough( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
        SetStrikeThrough( pVar->GetBool() );
    else
        pVar->PutBool( IsStrikeThrough() );
}

void SbStdFont::PropUnderline( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
        SetUnderline( pVar->GetBool() );
    else
        pVar->PutBool( IsUnderline() );
}

void SbStdFont::PropSize( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
        SetSize( static_cast<sal_uInt16>(pVar->GetInteger()) );
    else
        pVar->PutInteger( static_cast<sal_Int16>(GetSize()) );
}

void SbStdFont::PropName( SbxVariable* pVar, bool bWrite )
{
    if( bWrite )
    {
        aName = pVar->GetOUString();
    }
    else
    {
        pVar->PutString( aName );
    }
}


SbStdFont::SbStdFont()
    : SbxObject( "Font" )
    , bBold(false)
    , bItalic(false)
    , bStrikeThrough(false)
    , bUnderline(false)
    , nSize(0)
{
    // Properties
    SbxVariable* p = Make( "Bold", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::BOLD) );
    p = Make( "Italic", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::ITALIC) );
    p = Make( "StrikeThrough", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::STRIKETHROUGH) );
    p = Make( "Underline", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::UNDERLINE) );
    p = Make( "Size", SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::SIZE) );

    // handle name property yourself
    p = Find( "Name", SbxClassType::Property );
    assert(p && "No Name Property");
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::NAME) );
}

SbStdFont::~SbStdFont()
{
}

void SbStdFont::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);

    if( !pHint )
        return;

    if( pHint->GetId() == SfxHintId::BasicInfoWanted )
    {
        SbxObject::Notify( rBC, rHint );
        return;
    }

    SbxVariable* pVar   = pHint->GetVar();
    const sal_uInt32 nWhich = pVar->GetUserData();
    bool         bWrite = pHint->GetId() == SfxHintId::BasicDataChanged;

    // Properties
    switch( nWhich )
    {
        case static_cast<sal_uInt32>(ATTR_IMP::BOLD):         PropBold( pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::ITALIC):       PropItalic( pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::STRIKETHROUGH):PropStrikeThrough(  pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::UNDERLINE):    PropUnderline( pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::SIZE):         PropSize( pVar, bWrite ); return;
        case static_cast<sal_uInt32>(ATTR_IMP::NAME):         PropName( pVar, bWrite ); return;
    }

    SbxObject::Notify( rBC, rHint );
}


void SbStdClipboard::MethClear( SbxArray const * pPar_ )
{
    if (pPar_ && (pPar_->Count() > 1))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

}

void SbStdClipboard::MethGetData( SbxArray* pPar_ )
{
    if (!pPar_ || (pPar_->Count() != 2))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_Int16 nFormat = pPar_->Get(1)->GetInteger();
    if( nFormat <= 0 || nFormat > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

}

void SbStdClipboard::MethGetFormat( SbxVariable* pVar, SbxArray* pPar_ )
{
    if (!pPar_ || (pPar_->Count() != 2))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_Int16 nFormat = pPar_->Get(1)->GetInteger();
    if( nFormat <= 0 || nFormat > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    pVar->PutBool( false );
}

void SbStdClipboard::MethGetText( SbxVariable* pVar, SbxArray const * pPar_ )
{
    if (pPar_ && (pPar_->Count() > 1))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    pVar->PutString( OUString() );
}

void SbStdClipboard::MethSetData( SbxArray* pPar_ )
{
    if (!pPar_ || (pPar_->Count() != 3))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

    sal_Int16 nFormat = pPar_->Get(2)->GetInteger();
    if( nFormat <= 0 || nFormat > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

}

void SbStdClipboard::MethSetText( SbxArray const * pPar_ )
{
    if (!pPar_ || (pPar_->Count() != 2))
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_NUMBER_OF_ARGS );
        return;
    }

}


SbStdClipboard::SbStdClipboard() :
    SbxObject( "Clipboard" )
{
    SbxVariable* p = Find( "Name", SbxClassType::Property );
    assert(p && "No Name Property");
    p->SetUserData( static_cast<sal_uInt32>(ATTR_IMP::NAME) );

    // register methods
    p = Make( "Clear", SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(METH::CLEAR) );
    p = Make( "GetData", SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(METH::GETDATA) );
    p = Make( "GetFormat", SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(METH::GETFORMAT) );
    p = Make( "GetText", SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(METH::GETTEXT) );
    p = Make( "SetData", SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(METH::SETDATA) );
    p = Make( "SetText", SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( static_cast<sal_uInt32>(METH::SETTEXT) );
}

SbStdClipboard::~SbStdClipboard()
{
}

void SbStdClipboard::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SbxHint* pHint = dynamic_cast<const SbxHint*>(&rHint);

    if( !pHint )
        return;

    if( pHint->GetId() == SfxHintId::BasicInfoWanted )
    {
        SbxObject::Notify( rBC, rHint );
        return;
    }

    SbxVariable* pVar   = pHint->GetVar();
    SbxArray*    pPar_  = pVar->GetParameters();
    const sal_uInt32 nWhich = pVar->GetUserData();

    // Methods
    switch( nWhich )
    {
        case static_cast<sal_uInt32>(METH::CLEAR):            MethClear( pPar_ ); return;
        case static_cast<sal_uInt32>(METH::GETDATA):          MethGetData( pPar_ ); return;
        case static_cast<sal_uInt32>(METH::GETFORMAT):        MethGetFormat( pVar, pPar_ ); return;
        case static_cast<sal_uInt32>(METH::GETTEXT):          MethGetText( pVar, pPar_ ); return;
        case static_cast<sal_uInt32>(METH::SETDATA):          MethSetData( pPar_ ); return;
        case static_cast<sal_uInt32>(METH::SETTEXT):          MethSetText( pPar_ ); return;
    }

    SbxObject::Notify( rBC, rHint );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
