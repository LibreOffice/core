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

#include <bf_svtools/zforlist.hxx>
#include <tools/urlobj.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif

#define _SVX_FLDITEM_CXX

#include <unotools/localfilehelper.hxx>

#define ITEMID_FIELD	0

#include <bf_svtools/poolitem.hxx>


#include <bf_svx/itemdata.hxx>

#include "flditem.hxx"
#include "adritem.hxx"

// #90477#
#include <tools/tenccvt.hxx>
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

#define FRAME_MARKER	(sal_uInt32)0x21981357
#define CHARSET_MARKER	(FRAME_MARKER+1)

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1( SvxFieldItem, SfxPoolItem );

/*N*/ SV_IMPL_PERSIST1( SvxFieldData, SvPersistBase );

// -----------------------------------------------------------------------

/*N*/ SvxFieldData::SvxFieldData()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldData::~SvxFieldData()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldData* SvxFieldData::Clone() const
/*N*/ {
/*N*/ 	return new SvxFieldData;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxFieldData::operator==( const SvxFieldData& rFld ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( Type() == rFld.Type(), "==: Verschiedene Typen" );
/*N*/ 	return TRUE;	// Basicklasse immer gleich.
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxFieldData::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxFieldData::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ }


/*?*/ MetaAction* SvxFieldData::createBeginComment() const
/*?*/ {
/*?*/	DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	return new MetaCommentAction( "FIELD_SEQ_BEGIN" );
/*?*/ }

/*?*/ MetaAction* SvxFieldData::createEndComment() const
/*?*/ {
/*?*/	DBG_BF_ASSERT(0, "STRIP");return NULL;//STRIP001 	return new MetaCommentAction( "FIELD_SEQ_END" );
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldItem::SvxFieldItem( SvxFieldData* pFld, const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ 	pField = pFld;	// gehoert direkt dem Item
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldItem::SvxFieldItem( const SvxFieldData& rField, const USHORT nId ) :
/*N*/ 	SfxPoolItem( nId )
/*N*/ {
/*N*/ 	pField = rField.Clone();
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldItem::SvxFieldItem( const SvxFieldItem& rItem ) :
/*N*/ 	SfxPoolItem	( rItem )
/*N*/ {
/*N*/ 	pField = rItem.GetField() ? rItem.GetField()->Clone() : 0;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldItem::~SvxFieldItem()
/*N*/ {
/*N*/ 	delete pField;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFieldItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SvxFieldItem(*this);
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxFieldItem::Create( SvStream& rStrm, USHORT ) const
/*N*/ {
/*N*/ 	SvxFieldData* pData = 0;
/*N*/ 	SvPersistStream aPStrm( GetClassManager(), &rStrm );
/*N*/ 	aPStrm >> pData;
/*N*/ 
/*N*/ 	if( aPStrm.IsEof() )
/*?*/ 		aPStrm.SetError( SVSTREAM_GENERALERROR );
/*N*/ 
/*N*/ 	if ( aPStrm.GetError() == ERRCODE_IO_NOFACTORY )
/*?*/ 		aPStrm.ResetError();	// Eigentlich einen Code, dass nicht alle Attr gelesen wurden...
/*N*/ 
/*N*/ 	return new SvxFieldItem( pData, Which() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxFieldItem::Store( SvStream& rStrm, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( pField, "SvxFieldItem::Store: Feld?!" );
/*N*/ 	SvPersistStream aPStrm( GetClassManager(), &rStrm );
/*N*/ 	// Das ResetError in der obigen Create-Methode gab es in 3.1 noch nicht,
/*N*/ 	// deshalb duerfen beim 3.x-Export neuere Items nicht gespeichert werden!
/*N*/ 	if ( ( rStrm.GetVersion() <= SOFFICE_FILEFORMAT_31 ) && pField &&
/*N*/ 			pField->GetClassId() == 50 /* SdrMeasureField */ )
/*N*/ 	{
/*N*/ 		// SvxFieldData reicht nicht, weil auch nicht am ClassMgr angemeldet
/*?*/ 		SvxURLField aDummyData;
/*?*/ 		aPStrm << &aDummyData;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aPStrm << pField;
/*N*/ 
/*N*/ 	return rStrm;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxFieldItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );
/*N*/ 
/*N*/ 	const SvxFieldData* pOtherFld = ((const SvxFieldItem&)rItem).GetField();
/*N*/ 	if ( !pField && !pOtherFld )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	if ( ( !pField && pOtherFld ) || ( pField && !pOtherFld ) )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	return ( ( pField->Type() == pOtherFld->Type() )
/*N*/ 				&& ( *pField == *pOtherFld ) );
/*N*/ }

// =================================================================
// Es folgen die Ableitungen von SvxFieldData...
// =================================================================

/*N*/ SV_IMPL_PERSIST1( SvxDateField, SvxFieldData );

// -----------------------------------------------------------------------

/*N*/ SvxDateField::SvxDateField()
/*N*/ {
/*N*/ 	nFixDate = Date().GetDate();
/*N*/ 	eType = SVXDATETYPE_VAR;
/*N*/ 	eFormat = SVXDATEFORMAT_STDSMALL;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxDateField::SvxDateField( const Date& rDate, SvxDateType eT, SvxDateFormat eF )
/*N*/ {
/*N*/ 	nFixDate = rDate.GetDate();
/*N*/ 	eType = eT;
/*N*/ 	eFormat = eF;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldData* SvxDateField::Clone() const
/*N*/ {
/*N*/ 	return new SvxDateField( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxDateField::operator==( const SvxFieldData& rOther ) const
/*N*/ {
/*N*/ 	if ( rOther.Type() != Type() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	const SvxDateField& rOtherFld = (const SvxDateField&) rOther;
/*N*/ 	return ( ( nFixDate == rOtherFld.nFixDate ) &&
/*N*/ 				( eType == rOtherFld.eType ) &&
/*N*/ 				( eFormat == rOtherFld.eFormat ) );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxDateField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	USHORT nType, nFormat;
/*N*/ 
/*N*/ 	rStm >> nFixDate;
/*N*/ 	rStm >> nType;
/*N*/ 	rStm >> nFormat;
/*N*/ 
/*N*/ 	eType = (SvxDateType)nType;
/*N*/ 	eFormat= (SvxDateFormat)nFormat;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxDateField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	rStm << nFixDate;
/*N*/ 	rStm << (USHORT)eType;
/*N*/ 	rStm << (USHORT)eFormat;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SV_IMPL_PERSIST1( SvxURLField, SvxFieldData );

// -----------------------------------------------------------------------

/*N*/ SvxURLField::SvxURLField()
/*N*/ {
/*N*/ 	eFormat = SVXURLFORMAT_URL;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxURLField::SvxURLField( const XubString& rURL, const XubString& rRepres, SvxURLFormat eFmt )
/*N*/ 	: aURL( rURL ), aRepresentation( rRepres )
/*N*/ {
/*N*/ 	eFormat = eFmt;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxFieldData* SvxURLField::Clone() const
/*N*/ {
/*N*/ 	return new SvxURLField( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxURLField::operator==( const SvxFieldData& rOther ) const
/*N*/ {
/*N*/ 	if ( rOther.Type() != Type() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	const SvxURLField& rOtherFld = (const SvxURLField&) rOther;
/*N*/ 	return ( ( eFormat == rOtherFld.eFormat ) &&
/*N*/ 				( aURL == rOtherFld.aURL ) &&
/*N*/ 				( aRepresentation == rOtherFld.aRepresentation ) &&
/*N*/ 				( aTargetFrame == rOtherFld.aTargetFrame ) );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxURLField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	USHORT nFormat;
/*N*/ 	sal_uInt32 nFrameMarker, nCharSetMarker;
/*N*/ 	long nUlongSize = (long)sizeof(sal_uInt32);
/*N*/ 	String aTmpURL;
/*N*/ 
/*N*/ 	rStm >> nFormat;
/*N*/ 
/*N*/ 	// UNICODE: rStm >> aTmpURL;
/*N*/ 	rStm.ReadByteString(aTmpURL);
/*N*/ 
/*N*/ 	// UNICODE: rStm >> aRepresentation;
/*N*/ 	// read to a temp string first, read text encoding and
/*N*/ 	// convert later to stay compatible to fileformat
/*N*/ 	ByteString aTempString;
/*N*/ 	rtl_TextEncoding aTempEncoding = RTL_TEXTENCODING_MS_1252;  // #101493# Init for old documents
/*N*/ 	rStm.ReadByteString(aTempString);
/*N*/ 
/*N*/ 	rStm >> nFrameMarker;
/*N*/ 	if ( nFrameMarker == FRAME_MARKER )
/*N*/ 	{
/*N*/ 		// UNICODE: rStm >> aTargetFrame;
/*N*/ 		rStm.ReadByteString(aTargetFrame);
/*N*/ 
/*N*/ 		rStm >> nCharSetMarker;
/*N*/ 		if ( nCharSetMarker == CHARSET_MARKER )
/*N*/ 		{
/*N*/ 			USHORT nCharSet;
/*N*/ 			rStm >> nCharSet;
/*N*/ 
/*N*/ 			// remember encoding
/*N*/ 			aTempEncoding = (rtl_TextEncoding)nCharSet;
/*N*/ 		}
/*N*/ 		else
/*?*/ 			rStm.SeekRel( -nUlongSize );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		rStm.SeekRel( -nUlongSize );
/*N*/ 
/*N*/ 	// now build representation string due to known encoding
/*N*/ 	aRepresentation = String(aTempString, aTempEncoding);
/*N*/ 
/*N*/ 	eFormat= (SvxURLFormat)nFormat;
/*N*/ 
/*N*/ 	// Relatives Speichern => Beim laden absolut machen.
/*N*/ 	aURL = ::binfilter::StaticBaseUrl::RelToAbs( aTmpURL );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ void SvxURLField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	// Relatives Speichern der URL
/*N*/ 	String aTmpURL = ::binfilter::StaticBaseUrl::AbsToRel( aURL );
/*N*/ 
/*N*/ 	rStm << (USHORT)eFormat;
/*N*/ 
/*N*/ 	// UNICODE: rStm << aTmpURL;
/*N*/ 	rStm.WriteByteString(aTmpURL);
/*N*/ 
/*N*/ 	// UNICODE: rStm << aRepresentation;
/*N*/ 	rStm.WriteByteString(aRepresentation);
/*N*/ 
/*N*/ 	rStm << FRAME_MARKER;
/*N*/ 
/*N*/ 	// UNICODE: rStm << aTargetFrame;
/*N*/ 	rStm.WriteByteString(aTargetFrame);
/*N*/ 
/*N*/ 	rStm << CHARSET_MARKER;
/*N*/ 
/*N*/ 	// #90477# rStm << (USHORT)GetStoreCharSet(gsl_getSystemTextEncoding(), rStm.GetVersion());
/*N*/ 	rStm << (USHORT)GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rStm.GetVersion());
/*N*/ }


// =================================================================
// Die Felder, die aus Calc ausgebaut wurden:
// =================================================================

/*N*/ SV_IMPL_PERSIST1( SvxPageField, SvxFieldData );

/*N*/ SvxFieldData* __EXPORT SvxPageField::Clone() const
/*N*/ {
/*N*/ 	return new SvxPageField;		// leer
/*N*/ }

/*N*/ int __EXPORT SvxPageField::operator==( const SvxFieldData& rCmp ) const
/*N*/ {
/*N*/ 	return ( rCmp.Type() == TYPE(SvxPageField) );
/*N*/ }

/*N*/ void __EXPORT SvxPageField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ void __EXPORT SvxPageField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ }



/*N*/ SV_IMPL_PERSIST1( SvxPagesField, SvxFieldData );

/*N*/ SvxFieldData* __EXPORT SvxPagesField::Clone() const
/*N*/ {
/*N*/ 	return new SvxPagesField;	// leer
/*N*/ }

/*N*/ int __EXPORT SvxPagesField::operator==( const SvxFieldData& rCmp ) const
/*N*/ {
/*N*/ 	return ( rCmp.Type() == TYPE(SvxPagesField) );
/*N*/ }

/*N*/ void __EXPORT SvxPagesField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ void __EXPORT SvxPagesField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ SV_IMPL_PERSIST1( SvxTimeField, SvxFieldData );

/*N*/ SvxFieldData* __EXPORT SvxTimeField::Clone() const
/*N*/ {
/*N*/ 	return new SvxTimeField;	// leer
/*N*/ }

/*N*/ int __EXPORT SvxTimeField::operator==( const SvxFieldData& rCmp ) const
/*N*/ {
/*N*/ 	return ( rCmp.Type() == TYPE(SvxTimeField) );
/*N*/ }

/*N*/ void __EXPORT SvxTimeField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ void __EXPORT SvxTimeField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ }


/*N*/ SV_IMPL_PERSIST1( SvxFileField, SvxFieldData );

/*N*/ SvxFieldData* __EXPORT SvxFileField::Clone() const
/*N*/ {
/*N*/ 	return new SvxFileField;	// leer
/*N*/ }

/*N*/ int __EXPORT SvxFileField::operator==( const SvxFieldData& rCmp ) const
/*N*/ {
/*N*/ 	return ( rCmp.Type() == TYPE(SvxFileField) );
/*N*/ }

/*N*/ void __EXPORT SvxFileField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ void __EXPORT SvxFileField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ SV_IMPL_PERSIST1( SvxTableField, SvxFieldData );

/*N*/ SvxFieldData* __EXPORT SvxTableField::Clone() const
/*N*/ {
/*N*/ 	return new SvxTableField;	// leer
/*N*/ }

/*N*/ int __EXPORT SvxTableField::operator==( const SvxFieldData& rCmp ) const
/*N*/ {
/*N*/ 	return ( rCmp.Type() == TYPE(SvxTableField) );
/*N*/ }

/*N*/ void __EXPORT SvxTableField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

/*N*/ void __EXPORT SvxTableField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ }

//----------------------------------------------------------------------------
//		SvxExtTimeField
//----------------------------------------------------------------------------

/*N*/ SV_IMPL_PERSIST1( SvxExtTimeField, SvxFieldData );

//----------------------------------------------------------------------------

/*N*/ SvxExtTimeField::SvxExtTimeField()
/*N*/ {
/*N*/ 	nFixTime = Time().GetTime();
/*N*/ 	eType = SVXTIMETYPE_VAR;
/*N*/ 	eFormat = SVXTIMEFORMAT_STANDARD;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SvxExtTimeField::SvxExtTimeField( const Time& rTime, SvxTimeType eT, SvxTimeFormat eF )
/*N*/ {
/*N*/ 	nFixTime = rTime.GetTime();
/*N*/ 	eType = eT;
/*N*/ 	eFormat = eF;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SvxFieldData* SvxExtTimeField::Clone() const
/*N*/ {
/*N*/ 	return new SvxExtTimeField( *this );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ int SvxExtTimeField::operator==( const SvxFieldData& rOther ) const
/*N*/ {
/*N*/ 	if ( rOther.Type() != Type() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	const SvxExtTimeField& rOtherFld = (const SvxExtTimeField&) rOther;
/*N*/ 	return ( ( nFixTime == rOtherFld.nFixTime ) &&
/*N*/ 				( eType == rOtherFld.eType ) &&
/*N*/ 				( eFormat == rOtherFld.eFormat ) );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SvxExtTimeField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	USHORT nType, nFormat;
/*N*/ 
/*N*/ 	rStm >> nFixTime;
/*N*/ 	rStm >> nType;
/*N*/ 	rStm >> nFormat;
/*N*/ 
/*N*/ 	eType = (SvxTimeType) nType;
/*N*/ 	eFormat= (SvxTimeFormat) nFormat;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SvxExtTimeField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	rStm << nFixTime;
/*N*/ 	rStm << (USHORT) eType;
/*N*/ 	rStm << (USHORT) eFormat;
/*N*/ }

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//		SvxExtFileField
//----------------------------------------------------------------------------

/*N*/ SV_IMPL_PERSIST1( SvxExtFileField, SvxFieldData );

//----------------------------------------------------------------------------

/*N*/ SvxExtFileField::SvxExtFileField()
/*N*/ {
/*N*/ 	eType = SVXFILETYPE_VAR;
/*N*/ 	eFormat = SVXFILEFORMAT_FULLPATH;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SvxExtFileField::SvxExtFileField( const XubString& rStr, SvxFileType eT, SvxFileFormat eF )
/*N*/ {
/*N*/ 	aFile = rStr;
/*N*/ 	eType = eT;
/*N*/ 	eFormat = eF;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SvxFieldData* SvxExtFileField::Clone() const
/*N*/ {
/*N*/ 	return new SvxExtFileField( *this );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ int SvxExtFileField::operator==( const SvxFieldData& rOther ) const
/*N*/ {
/*N*/ 	if ( rOther.Type() != Type() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	const SvxExtFileField& rOtherFld = (const SvxExtFileField&) rOther;
/*N*/ 	return ( ( aFile == rOtherFld.aFile ) &&
/*N*/ 				( eType == rOtherFld.eType ) &&
/*N*/ 				( eFormat == rOtherFld.eFormat ) );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SvxExtFileField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	USHORT nType, nFormat;
/*N*/ 
/*N*/ 	// UNICODE: rStm >> aFile;
/*N*/ 	rStm.ReadByteString(aFile);
/*N*/ 
/*N*/ 	rStm >> nType;
/*N*/ 	rStm >> nFormat;
/*N*/ 
/*N*/ 	eType = (SvxFileType) nType;
/*N*/ 	eFormat= (SvxFileFormat) nFormat;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SvxExtFileField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	// UNICODE: rStm << aFile;
/*N*/ 	rStm.WriteByteString(aFile);
/*N*/ 
/*N*/ 	rStm << (USHORT) eType;
/*N*/ 	rStm << (USHORT) eFormat;
/*N*/ }

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//		SvxAuthorField
//----------------------------------------------------------------------------

/*N*/ SV_IMPL_PERSIST1( SvxAuthorField, SvxFieldData );

//----------------------------------------------------------------------------

/*N*/ SvxAuthorField::SvxAuthorField()
/*N*/ {
/*N*/ 	eType = SVXAUTHORTYPE_VAR;
/*N*/ 	eFormat = SVXAUTHORFORMAT_FULLNAME;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SvxAuthorField::SvxAuthorField( const SvxAddressItem& rAdrItem,
/*N*/ 									SvxAuthorType eT, SvxAuthorFormat eF )
/*N*/ {
/*N*/ 	aName 	   = rAdrItem.GetName();
/*N*/ 	aFirstName = rAdrItem.GetFirstName();
/*N*/ 	aShortName = rAdrItem.GetShortName();
/*N*/ 	eType   = eT;
/*N*/ 	eFormat = eF;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ SvxFieldData* SvxAuthorField::Clone() const
/*N*/ {
/*N*/ 	return new SvxAuthorField( *this );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ int SvxAuthorField::operator==( const SvxFieldData& rOther ) const
/*N*/ {
/*N*/ 	if ( rOther.Type() != Type() )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	const SvxAuthorField& rOtherFld = (const SvxAuthorField&) rOther;
/*N*/ 	return ( ( aName == rOtherFld.aName ) &&
/*N*/ 				( aFirstName == rOtherFld.aFirstName ) &&
/*N*/ 				( aShortName == rOtherFld.aShortName ) &&
/*N*/ 				( eType == rOtherFld.eType ) &&
/*N*/ 				( eFormat == rOtherFld.eFormat ) );
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SvxAuthorField::Load( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	USHORT nType, nFormat;
/*N*/ 
/*N*/ 	// UNICODE: rStm >> aName;
/*N*/ 	rStm.ReadByteString(aName);
/*N*/ 
/*N*/ 	// UNICODE: rStm >> aFirstName;
/*N*/ 	rStm.ReadByteString(aFirstName);
/*N*/ 
/*N*/ 	// UNICODE: rStm >> aShortName;
/*N*/ 	rStm.ReadByteString(aShortName);
/*N*/ 
/*N*/ 	rStm >> nType;
/*N*/ 	rStm >> nFormat;
/*N*/ 
/*N*/ 	eType = (SvxAuthorType) nType;
/*N*/ 	eFormat= (SvxAuthorFormat) nFormat;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ void SvxAuthorField::Save( SvPersistStream & rStm )
/*N*/ {
/*N*/ 	// UNICODE: rStm << aName;
/*N*/ 	rStm.WriteByteString(aName);
/*N*/ 
/*N*/ 	// UNICODE: rStm << aFirstName;
/*N*/ 	rStm.WriteByteString(aFirstName);
/*N*/ 
/*N*/ 	// UNICODE: rStm << aShortName;
/*N*/ 	rStm.WriteByteString(aShortName);
/*N*/ 
/*N*/ 	rStm << (USHORT) eType;
/*N*/ 	rStm << (USHORT) eFormat;
/*N*/ }

//----------------------------------------------------------------------------

/*N*/ XubString SvxAuthorField::GetFormatted() const
/*N*/ {
/*N*/ 	XubString aString;
/*N*/ 
/*N*/ 	switch( eFormat )
/*N*/ 	{
/*N*/ 		case SVXAUTHORFORMAT_FULLNAME:
/*N*/ 			aString  = aFirstName;
/*N*/ 			aString += sal_Unicode(' ');
/*N*/ 			aString += aName;
/*N*/ 		break;
/*N*/ 
/*N*/ 		case SVXAUTHORFORMAT_NAME:
/*N*/ 			aString = aName;
/*N*/ 		break;
/*N*/ 
/*N*/ 		case SVXAUTHORFORMAT_FIRSTNAME:
/*N*/ 			aString = aFirstName;
/*N*/ 		break;
/*N*/ 
/*N*/ 		case SVXAUTHORFORMAT_SHORTNAME:
/*N*/ 			aString = aShortName;
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	return( aString );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
