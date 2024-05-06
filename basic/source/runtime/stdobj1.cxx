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
    enum UserData
    {
        ATTR_IMP_TYPE           = 1,
        ATTR_IMP_WIDTH          = 2,
        ATTR_IMP_HEIGHT         = 3,
        ATTR_IMP_BOLD           = 4,
        ATTR_IMP_ITALIC         = 5,
        ATTR_IMP_STRIKETHROUGH  = 6,
        ATTR_IMP_UNDERLINE      = 7,
        ATTR_IMP_SIZE           = 9,
        ATTR_IMP_NAME           = 10,
        METH_CLEAR              = 20,
        METH_GETDATA            = 21,
        METH_GETFORMAT          = 22,
        METH_GETTEXT            = 23,
        METH_SETDATA            = 24,
        METH_SETTEXT            = 25
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
    SbxObject( u"Picture"_ustr )
{
    // Properties
    SbxVariable* p = Make( u"Type"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_TYPE );
    p = Make( u"Width"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_WIDTH );
    p = Make( u"Height"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::Read | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_HEIGHT );
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
        case ATTR_IMP_TYPE:     PropType( pVar, bWrite ); return;
        case ATTR_IMP_WIDTH:    PropWidth( pVar, bWrite ); return;
        case ATTR_IMP_HEIGHT:   PropHeight( pVar, bWrite ); return;
        default: break;
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
    : SbxObject( u"Font"_ustr )
    , bBold(false)
    , bItalic(false)
    , bStrikeThrough(false)
    , bUnderline(false)
    , nSize(0)
{
    // Properties
    SbxVariable* p = Make( u"Bold"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_BOLD );
    p = Make( u"Italic"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_ITALIC );
    p = Make( u"StrikeThrough"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_STRIKETHROUGH );
    p = Make( u"Underline"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_UNDERLINE );
    p = Make( u"Size"_ustr, SbxClassType::Property, SbxVARIANT );
    p->SetFlags( SbxFlagBits::ReadWrite | SbxFlagBits::DontStore );
    p->SetUserData( ATTR_IMP_SIZE );

    // handle name property yourself
    p = Find( u"Name"_ustr, SbxClassType::Property );
    assert(p && "No Name Property");
    p->SetUserData( ATTR_IMP_NAME );
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
        case ATTR_IMP_BOLD:         PropBold( pVar, bWrite ); return;
        case ATTR_IMP_ITALIC:       PropItalic( pVar, bWrite ); return;
        case ATTR_IMP_STRIKETHROUGH:PropStrikeThrough(  pVar, bWrite ); return;
        case ATTR_IMP_UNDERLINE:    PropUnderline( pVar, bWrite ); return;
        case ATTR_IMP_SIZE:         PropSize( pVar, bWrite ); return;
        case ATTR_IMP_NAME:         PropName( pVar, bWrite ); return;
        default: break;
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
    SbxObject( u"Clipboard"_ustr )
{
    SbxVariable* p = Find( u"Name"_ustr, SbxClassType::Property );
    assert(p && "No Name Property");
    p->SetUserData( ATTR_IMP_NAME );

    // register methods
    p = Make( u"Clear"_ustr, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_CLEAR );
    p = Make( u"GetData"_ustr, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_GETDATA );
    p = Make( u"GetFormat"_ustr, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_GETFORMAT );
    p = Make( u"GetText"_ustr, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_GETTEXT );
    p = Make( u"SetData"_ustr, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_SETDATA );
    p = Make( u"SetText"_ustr, SbxClassType::Method, SbxEMPTY );
    p->SetFlag( SbxFlagBits::DontStore );
    p->SetUserData( METH_SETTEXT );
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
        case METH_CLEAR:            MethClear( pPar_ ); return;
        case METH_GETDATA:          MethGetData( pPar_ ); return;
        case METH_GETFORMAT:        MethGetFormat( pVar, pPar_ ); return;
        case METH_GETTEXT:          MethGetText( pVar, pPar_ ); return;
        case METH_SETDATA:          MethSetData( pPar_ ); return;
        case METH_SETTEXT:          MethSetText( pPar_ ); return;
        default: break;
    }

    SbxObject::Notify( rBC, rHint );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
