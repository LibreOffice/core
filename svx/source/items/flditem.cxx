/*************************************************************************
 *
 *  $RCSfile: flditem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:20 $
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

#ifndef _INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#pragma hdrstop

#define _SVX_FLDITEM_CXX

#define ITEMID_FIELD    0
#include "flditem.hxx"
#include "adritem.hxx"

#define FRAME_MARKER    (ULONG)0x21981357
#define CHARSET_MARKER  (FRAME_MARKER+1)

// -----------------------------------------------------------------------

TYPEINIT1( SvxFieldItem, SfxPoolItem );

SV_IMPL_PERSIST1( SvxFieldData, SvPersistBase );

// -----------------------------------------------------------------------

SvxFieldData::SvxFieldData()
{
}

// -----------------------------------------------------------------------

SvxFieldData::~SvxFieldData()
{
}

// -----------------------------------------------------------------------

SvxFieldData* SvxFieldData::Clone() const
{
    return new SvxFieldData;
}

// -----------------------------------------------------------------------

int SvxFieldData::operator==( const SvxFieldData& rFld ) const
{
    DBG_ASSERT( Type() == rFld.Type(), "==: Verschiedene Typen" );
    return TRUE;    // Basicklasse immer gleich.
}

// -----------------------------------------------------------------------

void SvxFieldData::Load( SvPersistStream & rStm )
{
}

// -----------------------------------------------------------------------

void SvxFieldData::Save( SvPersistStream & rStm )
{
}

// -----------------------------------------------------------------------

SvxFieldItem::SvxFieldItem( SvxFieldData* pFld, const USHORT nId ) :
    SfxPoolItem( nId )
{
    pField = pFld;  // gehoert direkt dem Item
}

// -----------------------------------------------------------------------

SvxFieldItem::SvxFieldItem( const SvxFieldData& rField, const USHORT nId ) :
    SfxPoolItem( nId )
{
    pField = rField.Clone();
}

// -----------------------------------------------------------------------

SvxFieldItem::SvxFieldItem( const SvxFieldItem& rItem ) :
    SfxPoolItem ( rItem )
{
    pField = rItem.GetField() ? rItem.GetField()->Clone() : 0;
}

// -----------------------------------------------------------------------

SvxFieldItem::~SvxFieldItem()
{
    delete pField;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFieldItem::Clone( SfxItemPool* ) const
{
    return new SvxFieldItem(*this);
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxFieldItem::Create( SvStream& rStrm, USHORT ) const
{
    SvxFieldData* pData = 0;
    SvPersistStream aPStrm( GetClassManager(), &rStrm );
    aPStrm >> pData;

    if( aPStrm.IsEof() )
        aPStrm.SetError( SVSTREAM_GENERALERROR );

    if ( aPStrm.GetError() == ERRCODE_IO_NOFACTORY )
        aPStrm.ResetError();    // Eigentlich einen Code, dass nicht alle Attr gelesen wurden...

    return new SvxFieldItem( pData, Which() );
}

// -----------------------------------------------------------------------

SvStream& SvxFieldItem::Store( SvStream& rStrm, USHORT nItemVersion ) const
{
    DBG_ASSERT( pField, "SvxFieldItem::Store: Feld?!" );
    SvPersistStream aPStrm( GetClassManager(), &rStrm );
    // Das ResetError in der obigen Create-Methode gab es in 3.1 noch nicht,
    // deshalb duerfen beim 3.x-Export neuere Items nicht gespeichert werden!
    if ( ( rStrm.GetVersion() <= SOFFICE_FILEFORMAT_31 ) && pField &&
            pField->GetClassId() == 50 /* SdrMeasureField */ )
    {
        // SvxFieldData reicht nicht, weil auch nicht am ClassMgr angemeldet
        SvxURLField aDummyData;
        aPStrm << &aDummyData;
    }
    else
        aPStrm << pField;

    return rStrm;
}

// -----------------------------------------------------------------------

int SvxFieldItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );

    const SvxFieldData* pOtherFld = ((const SvxFieldItem&)rItem).GetField();
    if ( !pField && !pOtherFld )
        return TRUE;

    if ( ( !pField && pOtherFld ) || ( pField && !pOtherFld ) )
        return FALSE;

    return ( ( pField->Type() == pOtherFld->Type() )
                && ( *pField == *pOtherFld ) );
}

// =================================================================
// Es folgen die Ableitungen von SvxFieldData...
// =================================================================

SV_IMPL_PERSIST1( SvxDateField, SvxFieldData );

// -----------------------------------------------------------------------

SvxDateField::SvxDateField()
{
    nFixDate = Date().GetDate();
    eType = SVXDATETYPE_VAR;
    eFormat = SVXDATEFORMAT_STDSMALL;
}

// -----------------------------------------------------------------------

SvxDateField::SvxDateField( const Date& rDate, SvxDateType eT, SvxDateFormat eF )
{
    nFixDate = rDate.GetDate();
    eType = eT;
    eFormat = eF;
}

// -----------------------------------------------------------------------

SvxFieldData* SvxDateField::Clone() const
{
    return new SvxDateField( *this );
}

// -----------------------------------------------------------------------

int SvxDateField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return FALSE;

    const SvxDateField& rOtherFld = (const SvxDateField&) rOther;
    return ( ( nFixDate == rOtherFld.nFixDate ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

// -----------------------------------------------------------------------

void SvxDateField::Load( SvPersistStream & rStm )
{
    USHORT nType, nFormat;

    rStm >> nFixDate;
    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxDateType)nType;
    eFormat= (SvxDateFormat)nFormat;
}

// -----------------------------------------------------------------------

void SvxDateField::Save( SvPersistStream & rStm )
{
    rStm << nFixDate;
    rStm << (USHORT)eType;
    rStm << (USHORT)eFormat;
}

// -----------------------------------------------------------------------

XubString SvxDateField::GetFormatted( LanguageType eLanguage, LanguageType eFmt ) const
{
    International aInter( eLanguage, eFmt );
    int bLongDate = FALSE;

    Date aDate; // aktuelles
    if ( eType == SVXDATETYPE_FIX )
        aDate.SetDate( nFixDate );

    SvxDateFormat eTmpFormat = eFormat;

    if ( eTmpFormat == SVXDATEFORMAT_SYSTEM )
    {
        DBG_ERROR( "SVXDATEFORMAT_SYSTEM nicht implementiert!" );
        eTmpFormat = SVXDATEFORMAT_STDSMALL;
    }
    else if ( eTmpFormat == SVXDATEFORMAT_APPDEFAULT )
    {
        DBG_ERROR( "SVXDATEFORMAT_APPDEFAULT: Woher nehmen?" );
        eTmpFormat = SVXDATEFORMAT_STDSMALL;
    }

    switch( eTmpFormat )
    {
        // kurze Formate standard
        case SVXDATEFORMAT_STDSMALL:
        break;
        case SVXDATEFORMAT_STDBIG:
        {
            bLongDate = TRUE;
        }
        break;
        case SVXDATEFORMAT_A:
        {
            // 13.02.96
            aInter.SetDateCentury( FALSE );
        }
        break;
        case SVXDATEFORMAT_B:
        {
            // 13.02.1996
            aInter.SetDateCentury( TRUE );
        }
        break;
        case SVXDATEFORMAT_C:
        {
            // 13.Feb 1996
            aInter.SetLongDateDayOfWeekFormat( DAYOFWEEK_NONE );
            aInter.SetLongDateMonthFormat( MONTH_SHORT );
            aInter.SetDateCentury( TRUE );
            bLongDate = TRUE;
        }
        break;
        case SVXDATEFORMAT_D:
        {
            // 13.Februar 1996
            aInter.SetLongDateDayOfWeekFormat(DAYOFWEEK_NONE);
            aInter.SetLongDateMonthFormat(MONTH_LONG);
            aInter.SetDateCentury(TRUE);
            bLongDate = TRUE;
        }
        break;
        case SVXDATEFORMAT_E:
        {
            // Die, 13.Februar 1996
            aInter.SetLongDateDayOfWeekFormat(DAYOFWEEK_SHORT);
            aInter.SetLongDateMonthFormat(MONTH_LONG);
            aInter.SetDateCentury(TRUE);
            bLongDate = TRUE;
        }
        break;
        case SVXDATEFORMAT_F:
        {
            // Dienstag, 13.Februar 1996
            aInter.SetLongDateDayOfWeekFormat(DAYOFWEEK_LONG);
            aInter.SetLongDateMonthFormat(MONTH_LONG);
            aInter.SetDateCentury(TRUE);
            bLongDate = TRUE;
        }
        break;
    }

    if( bLongDate )
        return aInter.GetLongDate( aDate );
    return aInter.GetDate( aDate );
}


SV_IMPL_PERSIST1( SvxURLField, SvxFieldData );

// -----------------------------------------------------------------------

SvxURLField::SvxURLField()
{
    eFormat = SVXURLFORMAT_URL;
}

// -----------------------------------------------------------------------

SvxURLField::SvxURLField( const XubString& rURL, const XubString& rRepres, SvxURLFormat eFmt )
    : aURL( rURL ), aRepresentation( rRepres )
{
    eFormat = eFmt;
}

// -----------------------------------------------------------------------

SvxFieldData* SvxURLField::Clone() const
{
    return new SvxURLField( *this );
}

// -----------------------------------------------------------------------

int SvxURLField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return FALSE;

    const SvxURLField& rOtherFld = (const SvxURLField&) rOther;
    return ( ( eFormat == rOtherFld.eFormat ) &&
                ( aURL == rOtherFld.aURL ) &&
                ( aRepresentation == rOtherFld.aRepresentation ) &&
                ( aTargetFrame == rOtherFld.aTargetFrame ) );
}

// -----------------------------------------------------------------------

void SvxURLField::Load( SvPersistStream & rStm )
{
    USHORT nFormat;
    ULONG nFrameMarker, nCharSetMarker;
    long nUlongSize = (long)sizeof(ULONG);
    String aTmpURL;

    rStm >> nFormat;

    // UNICODE: rStm >> aTmpURL;
    rStm.ReadByteString(aTmpURL);

    // UNICODE: rStm >> aRepresentation;
    // read to a temp string first, read text encoding and
    // convert later to stay compatible to fileformat
    ByteString aTempString;
    rtl_TextEncoding aTempEncoding;
    rStm.ReadByteString(aTempString);

    rStm >> nFrameMarker;
    if ( nFrameMarker == FRAME_MARKER )
    {
        // UNICODE: rStm >> aTargetFrame;
        rStm.ReadByteString(aTargetFrame);

        rStm >> nCharSetMarker;
        if ( nCharSetMarker == CHARSET_MARKER )
        {
            USHORT nCharSet;
            rStm >> nCharSet;

            // remember encoding
            aTempEncoding = (rtl_TextEncoding)nCharSet;
        }
        else
            rStm.SeekRel( -nUlongSize );
    }
    else
        rStm.SeekRel( -nUlongSize );

    // now build representation string due to known encoding
    aRepresentation = String(aTempString, aTempEncoding);

    eFormat= (SvxURLFormat)nFormat;

    // Relatives Speichern => Beim laden absolut machen.
    aURL = INetURLObject::RelToAbs( aTmpURL );
}

// -----------------------------------------------------------------------

void SvxURLField::Save( SvPersistStream & rStm )
{
    // Relatives Speichern der URL
    String aTmpURL = INetURLObject::AbsToRel( aURL );

    rStm << (USHORT)eFormat;

    // UNICODE: rStm << aTmpURL;
    rStm.WriteByteString(aTmpURL);

    // UNICODE: rStm << aRepresentation;
    rStm.WriteByteString(aRepresentation);

    rStm << FRAME_MARKER;

    // UNICODE: rStm << aTargetFrame;
    rStm.WriteByteString(aTargetFrame);

    rStm << CHARSET_MARKER;
    rStm << (USHORT)GetStoreCharSet( gsl_getSystemTextEncoding(),
                                       rStm.GetVersion() );
}

// =================================================================
// Die Felder, die aus Calc ausgebaut wurden:
// =================================================================

SV_IMPL_PERSIST1( SvxPageField, SvxFieldData );

SvxFieldData* __EXPORT SvxPageField::Clone() const
{
    return new SvxPageField;        // leer
}

int __EXPORT SvxPageField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxPageField) );
}

void __EXPORT SvxPageField::Load( SvPersistStream & rStm )
{
}

void __EXPORT SvxPageField::Save( SvPersistStream & rStm )
{
}

SV_IMPL_PERSIST1( SvxPagesField, SvxFieldData );

SvxFieldData* __EXPORT SvxPagesField::Clone() const
{
    return new SvxPagesField;   // leer
}

int __EXPORT SvxPagesField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxPagesField) );
}

void __EXPORT SvxPagesField::Load( SvPersistStream & rStm )
{
}

void __EXPORT SvxPagesField::Save( SvPersistStream & rStm )
{
}

SV_IMPL_PERSIST1( SvxTimeField, SvxFieldData );

SvxFieldData* __EXPORT SvxTimeField::Clone() const
{
    return new SvxTimeField;    // leer
}

int __EXPORT SvxTimeField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxTimeField) );
}

void __EXPORT SvxTimeField::Load( SvPersistStream & rStm )
{
}

void __EXPORT SvxTimeField::Save( SvPersistStream & rStm )
{
}

SV_IMPL_PERSIST1( SvxFileField, SvxFieldData );

SvxFieldData* __EXPORT SvxFileField::Clone() const
{
    return new SvxFileField;    // leer
}

int __EXPORT SvxFileField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxFileField) );
}

void __EXPORT SvxFileField::Load( SvPersistStream & rStm )
{
}

void __EXPORT SvxFileField::Save( SvPersistStream & rStm )
{
}

SV_IMPL_PERSIST1( SvxTableField, SvxFieldData );

SvxFieldData* __EXPORT SvxTableField::Clone() const
{
    return new SvxTableField;   // leer
}

int __EXPORT SvxTableField::operator==( const SvxFieldData& rCmp ) const
{
    return ( rCmp.Type() == TYPE(SvxTableField) );
}

void __EXPORT SvxTableField::Load( SvPersistStream & rStm )
{
}

void __EXPORT SvxTableField::Save( SvPersistStream & rStm )
{
}

//----------------------------------------------------------------------------
//      SvxExtTimeField
//----------------------------------------------------------------------------

SV_IMPL_PERSIST1( SvxExtTimeField, SvxFieldData );

//----------------------------------------------------------------------------

SvxExtTimeField::SvxExtTimeField()
{
    nFixTime = Time().GetTime();
    eType = SVXTIMETYPE_VAR;
    eFormat = SVXTIMEFORMAT_STANDARD;
}

//----------------------------------------------------------------------------

SvxExtTimeField::SvxExtTimeField( const Time& rTime, SvxTimeType eT, SvxTimeFormat eF )
{
    nFixTime = rTime.GetTime();
    eType = eT;
    eFormat = eF;
}

//----------------------------------------------------------------------------

SvxFieldData* SvxExtTimeField::Clone() const
{
    return new SvxExtTimeField( *this );
}

//----------------------------------------------------------------------------

int SvxExtTimeField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return FALSE;

    const SvxExtTimeField& rOtherFld = (const SvxExtTimeField&) rOther;
    return ( ( nFixTime == rOtherFld.nFixTime ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

//----------------------------------------------------------------------------

void SvxExtTimeField::Load( SvPersistStream & rStm )
{
    USHORT nType, nFormat;

    rStm >> nFixTime;
    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxTimeType) nType;
    eFormat= (SvxTimeFormat) nFormat;
}

//----------------------------------------------------------------------------

void SvxExtTimeField::Save( SvPersistStream & rStm )
{
    rStm << nFixTime;
    rStm << (USHORT) eType;
    rStm << (USHORT) eFormat;
}

//----------------------------------------------------------------------------

XubString SvxExtTimeField::GetFormatted( LanguageType eLanguage, LanguageType eFmt ) const
{
    International aInter( eLanguage, eFmt );
    XubString aStrTime;

    Time aTime; // aktuelle Zeit
    if ( eType == SVXTIMETYPE_FIX )
        aTime.SetTime( nFixTime );

    SvxTimeFormat eTmpFormat = eFormat;

    if ( eTmpFormat == SVXTIMEFORMAT_SYSTEM )
    {
        DBG_ERROR( "SVXTIMEFORMAT_SYSTEM nicht implementiert!" );
        eTmpFormat = SVXTIMEFORMAT_STANDARD;
    }
    else if ( eTmpFormat == SVXTIMEFORMAT_APPDEFAULT )
    {
        DBG_ERROR( "SVXTIMEFORMAT_APPDEFAULT: Woher nehmen?" );
        eTmpFormat = SVXTIMEFORMAT_STANDARD;
    }

    // 12 oder 24 Stunden
    switch( eTmpFormat )
    {
        case SVXTIMEFORMAT_24_HM:
        case SVXTIMEFORMAT_24_HMS:
        case SVXTIMEFORMAT_24_HMSH:
            aInter.SetTimeFormat( HOUR_24 );
        break;

        default:
            aInter.SetTimeFormat( HOUR_12 );
        break;
    }
    switch( eTmpFormat )
    {
        case SVXTIMEFORMAT_12_HM:
        case SVXTIMEFORMAT_24_HM:
             aStrTime = aInter.GetTime( aTime, FALSE );
        break;

        case SVXTIMEFORMAT_12_HMSH:
        case SVXTIMEFORMAT_24_HMSH:
             aStrTime = aInter.GetTime( aTime, TRUE, TRUE );
        break;

        case SVXTIMEFORMAT_STANDARD:
        case SVXTIMEFORMAT_12_HMS:
        case SVXTIMEFORMAT_24_HMS:
        default:
             aStrTime = aInter.GetTime( aTime, TRUE );
        break;

    }

    return( aStrTime );
}

//----------------------------------------------------------------------------
//      SvxExtFileField
//----------------------------------------------------------------------------

SV_IMPL_PERSIST1( SvxExtFileField, SvxFieldData );

//----------------------------------------------------------------------------

SvxExtFileField::SvxExtFileField()
{
    eType = SVXFILETYPE_VAR;
    eFormat = SVXFILEFORMAT_FULLPATH;
}

//----------------------------------------------------------------------------

SvxExtFileField::SvxExtFileField( const XubString& rStr, SvxFileType eT, SvxFileFormat eF )
{
    aFile = rStr;
    eType = eT;
    eFormat = eF;
}

//----------------------------------------------------------------------------

SvxFieldData* SvxExtFileField::Clone() const
{
    return new SvxExtFileField( *this );
}

//----------------------------------------------------------------------------

int SvxExtFileField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return FALSE;

    const SvxExtFileField& rOtherFld = (const SvxExtFileField&) rOther;
    return ( ( aFile == rOtherFld.aFile ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

//----------------------------------------------------------------------------

void SvxExtFileField::Load( SvPersistStream & rStm )
{
    USHORT nType, nFormat;

    // UNICODE: rStm >> aFile;
    rStm.ReadByteString(aFile);

    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxFileType) nType;
    eFormat= (SvxFileFormat) nFormat;
}

//----------------------------------------------------------------------------

void SvxExtFileField::Save( SvPersistStream & rStm )
{
    // UNICODE: rStm << aFile;
    rStm.WriteByteString(aFile);

    rStm << (USHORT) eType;
    rStm << (USHORT) eFormat;
}

//----------------------------------------------------------------------------

XubString SvxExtFileField::GetFormatted() const
{
    XubString aString;

    INetURLObject aURLObj( aFile );

    if( INET_PROT_FILE == aURLObj.GetProtocol() )
    {
        switch( eFormat )
        {
            case SVXFILEFORMAT_FULLPATH:
                aString = aURLObj.GetFull();
            break;

            case SVXFILEFORMAT_PATH:
                aString = aURLObj.GetPath();
            break;

            case SVXFILEFORMAT_NAME:
                aString = aURLObj.GetBase();
            break;

            case SVXFILEFORMAT_NAME_EXT:
                aString = aURLObj.GetName();
            break;
        }
    }
    else
    {
        switch( eFormat )
        {
            case SVXFILEFORMAT_FULLPATH:
                aString = aURLObj.GetMainURL();
            break;

            case SVXFILEFORMAT_PATH:
                aString = aURLObj.GetMainURL();
                aString.Erase( aString.Search( aURLObj.GetLastName() ) );
            break;

            case SVXFILEFORMAT_NAME:
                aString = aURLObj.GetBase();
            break;

            case SVXFILEFORMAT_NAME_EXT:
                aString = aURLObj.GetLastName();
            break;
        }
    }

    return( aString );
}

//----------------------------------------------------------------------------
//      SvxAuthorField
//----------------------------------------------------------------------------

SV_IMPL_PERSIST1( SvxAuthorField, SvxFieldData );

//----------------------------------------------------------------------------

SvxAuthorField::SvxAuthorField()
{
    eType = SVXAUTHORTYPE_VAR;
    eFormat = SVXAUTHORFORMAT_FULLNAME;
}

//----------------------------------------------------------------------------

SvxAuthorField::SvxAuthorField( const SvxAddressItem& rAdrItem,
                                    SvxAuthorType eT, SvxAuthorFormat eF )
{
    aName      = rAdrItem.GetName();
    aFirstName = rAdrItem.GetFirstName();
    aShortName = rAdrItem.GetShortName();
    eType   = eT;
    eFormat = eF;
}

//----------------------------------------------------------------------------

SvxFieldData* SvxAuthorField::Clone() const
{
    return new SvxAuthorField( *this );
}

//----------------------------------------------------------------------------

int SvxAuthorField::operator==( const SvxFieldData& rOther ) const
{
    if ( rOther.Type() != Type() )
        return FALSE;

    const SvxAuthorField& rOtherFld = (const SvxAuthorField&) rOther;
    return ( ( aName == rOtherFld.aName ) &&
                ( aFirstName == rOtherFld.aFirstName ) &&
                ( aShortName == rOtherFld.aShortName ) &&
                ( eType == rOtherFld.eType ) &&
                ( eFormat == rOtherFld.eFormat ) );
}

//----------------------------------------------------------------------------

void SvxAuthorField::Load( SvPersistStream & rStm )
{
    USHORT nType, nFormat;

    // UNICODE: rStm >> aName;
    rStm.ReadByteString(aName);

    // UNICODE: rStm >> aFirstName;
    rStm.ReadByteString(aFirstName);

    // UNICODE: rStm >> aShortName;
    rStm.ReadByteString(aShortName);

    rStm >> nType;
    rStm >> nFormat;

    eType = (SvxAuthorType) nType;
    eFormat= (SvxAuthorFormat) nFormat;
}

//----------------------------------------------------------------------------

void SvxAuthorField::Save( SvPersistStream & rStm )
{
    // UNICODE: rStm << aName;
    rStm.WriteByteString(aName);

    // UNICODE: rStm << aFirstName;
    rStm.WriteByteString(aFirstName);

    // UNICODE: rStm << aShortName;
    rStm.WriteByteString(aShortName);

    rStm << (USHORT) eType;
    rStm << (USHORT) eFormat;
}

//----------------------------------------------------------------------------

XubString SvxAuthorField::GetFormatted() const
{
    XubString aString;

    switch( eFormat )
    {
        case SVXAUTHORFORMAT_FULLNAME:
            aString  = aFirstName;
            aString += sal_Unicode(' ');
            aString += aName;
        break;

        case SVXAUTHORFORMAT_NAME:
            aString = aName;
        break;

        case SVXAUTHORFORMAT_FIRSTNAME:
            aString = aFirstName;
        break;

        case SVXAUTHORFORMAT_SHORTNAME:
            aString = aShortName;
        break;
    }

    return( aString );
}


