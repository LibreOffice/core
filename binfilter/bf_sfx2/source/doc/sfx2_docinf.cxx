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

#ifndef _BIGINT_HXX //autogen wg. BigInt
#include <tools/bigint.hxx>
#endif
#include <tools/urlobj.hxx>
#include <bf_svtools/saveopt.hxx>
#include <tools/tenccvt.hxx>
#include <bf_svtools/useroptions.hxx>
#include <sot/exchange.hxx>
#include "rtl/tencinfo.h"

#include "fcontnr.hxx"
#include "openflag.hxx"
#include "app.hxx"
#include "docfile.hxx"
namespace binfilter {

/*N*/ TYPEINIT1_AUTOFACTORY(SfxDocumentInfoItem, SfxStringItem);

//------------------------------------------------------------------------

/*?*/ SfxDocumentInfoItem::SfxDocumentInfoItem() :
/*?*/ 
/*?*/   SfxStringItem()
/*?*/ 
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

//========================================================================

static const char __FAR_DATA pDocInfoSlot[] = "SfxDocumentInfo";
static const char __FAR_DATA pPropSlot[] = "\005SummaryInformation";
static const char __FAR_DATA pDocInfoHeader[] = "SfxDocumentInfo";
#define VERSION 11
#define STREAM_BUFFER_SIZE 2048

#define VT_I2         2
#define VT_I4         3
#define VT_LPSTR      30
#define VT_LPWSTR     31
#define VT_FILETIME   64

#define PID_TITLE              0x02
#define PID_SUBJECT            0x03
#define PID_AUTHOR             0x04
#define PID_KEYWORDS           0x05
#define PID_COMMENTS           0x06
#define PID_TEMPLATE           0x07
#define PID_LASTAUTHOR         0x08
#define PID_REVNUMBER          0x09
#define PID_EDITTIME           0x0a
#define PID_LASTPRINTED_DTM    0x0b
#define PID_CREATE_DTM         0x0c
#define PID_LASTSAVED_DTM      0x0d

//=========================================================================

/*N*/ IMPL_PTRHINT(SfxDocumentInfoHint, SfxDocumentInfo)

//=========================================================================

/*N*/ class SfxPSProperty_Impl
/*N*/ {
/*N*/ private:
/*N*/ 	UINT32 nId;
/*N*/ 	UINT32 nType;
/*N*/
/*N*/ public:
/*N*/ 	SfxPSProperty_Impl( UINT32 nIdP, UINT32 nTypeP ) : nId( nIdP ), nType( nTypeP ) {}
/*N*/ 	virtual ~SfxPSProperty_Impl();
/*N*/
/*N*/ 	virtual ULONG	Save( SvStream& rStream ) = 0;
/*N*/ 	virtual ULONG	Load( SvStream& rStream );
/*N*/ 	virtual ULONG	Len() = 0;
/*N*/
/*N*/ 	UINT32			GetId() const { return nId; }
/*N*/ 	UINT32			GetType() const { return nType; }
/*N*/ };

//-------------------------------------------------------------------------

/*N*/ SfxPSProperty_Impl::~SfxPSProperty_Impl()
/*N*/ {
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSProperty_Impl::Load( SvStream& )
/*N*/ {
/*N*/ 	return 0;
/*N*/ }


//=========================================================================

/*N*/ class SfxPSCodePageProperty_Impl : public SfxPSProperty_Impl
/*N*/ {
/*N*/ 	private:
/*N*/ 		CharSet nEncoding;
/*N*/
/*N*/ 	public:
/*N*/ 		SfxPSCodePageProperty_Impl( CharSet nCharSet ) : SfxPSProperty_Impl( 1, VT_I2 ), nEncoding( nCharSet ) {}
/*N*/ 		virtual ~SfxPSCodePageProperty_Impl() {}
/*N*/
/*N*/ 	virtual ULONG	Save( SvStream& rStream );
/*N*/ 	virtual ULONG	Len();
/*N*/ };

/*N*/ ULONG SfxPSCodePageProperty_Impl::Save( SvStream& rStream )
/*N*/ {
/*N*/     sal_uInt32 nCodePage = rtl_getWindowsCodePageFromTextEncoding(nEncoding);
/*N*/     if (nCodePage == 0)
/*N*/         nCodePage = 1252;
/*N*/ 	rStream << (UINT16)nCodePage;
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

/*N*/ ULONG SfxPSCodePageProperty_Impl::Len()
/*N*/ {
/*N*/ 	return sizeof( UINT16 );
/*N*/ }

//=========================================================================

/*N*/ class SfxPSStringProperty_Impl : public SfxPSProperty_Impl
/*N*/ {
/*N*/ private:
/*N*/ 	String aString;
/*N*/     CharSet nEncoding;
/*N*/     BOOL bIsUniCode;
/*N*/
/*N*/ public:
/*N*/     SfxPSStringProperty_Impl( UINT32 nIdP, const String& aStr ) :
/*N*/         aString(aStr), SfxPSProperty_Impl( nIdP, VT_LPSTR ), bIsUniCode(FALSE)
/*N*/     {
/*N*/         nEncoding = RTL_TEXTENCODING_UTF8;
/*N*/     }
/*N*/
/*N*/     SfxPSStringProperty_Impl( UINT32 nIdP ) :
/*N*/         SfxPSProperty_Impl( nIdP, VT_LPSTR ), bIsUniCode(FALSE)
/*N*/     {
/*N*/         nEncoding = RTL_TEXTENCODING_UTF8;
/*N*/     }
/*N*/
/*N*/     void SetIsUniCode() { bIsUniCode = TRUE; }
/*N*/
/*N*/ 	virtual ULONG	Save( SvStream& rStream );
/*N*/ 	virtual ULONG	Load( SvStream& rStream );
/*N*/ 	virtual ULONG 	Len();
/*N*/
/*N*/ 	const String&	GetString() { return aString; }
/*N*/ };

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSStringProperty_Impl::Save( SvStream& rStream )
/*N*/ {
/*N*/ 	// Now we always write property strings with UTF8 encoding, so we
/*N*/ 	// can ensure full unicode compatibility. The code page attribute is
/*N*/ 	// written with UTF8 set!
/*N*/ 	// Force nEncoding set to UTF8!
/*N*/     ByteString aTemp( aString, RTL_TEXTENCODING_UTF8 );
/*N*/ 	nEncoding = RTL_TEXTENCODING_UTF8;
/*N*/ 	UINT32 nLen = aTemp.Len();
/*N*/ 	rStream << (UINT32)( nLen + 1 );
/*N*/ 	rStream.Write( aTemp.GetBuffer(), nLen );
/*N*/ 	rStream << '\0';
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSStringProperty_Impl::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	UINT32 nLen;
/*N*/ 	rStream >> nLen;
/*N*/ 	if ( nLen > 0 )
/*N*/ 	{
/*N*/         if ( bIsUniCode )
/*N*/         {
/*N*/             sal_Unicode* pString = new sal_Unicode[ nLen ];
/*N*/             UINT32 i; for ( i = 0; i < nLen; i++ )
/*N*/                 rStream >> pString[ i ];
/*N*/             if ( pString[ i - 1 ] == 0 )
/*N*/             {
/*N*/                 if ( nLen > 1 )
/*N*/                     aString = String( pString, (USHORT) nLen - 1 );
/*N*/                 else
/*N*/                     aString = String();
/*N*/             }
/*N*/
/*N*/             delete[] pString;
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             ByteString aTemp;
/*N*/             if ( nLen>1 )
/*N*/             {
/*N*/                 rStream.Read( aTemp.AllocBuffer( (xub_StrLen)( nLen - 1 ) ), nLen );
/*N*/                 aString = String( aTemp, nEncoding );
/*N*/             }
/*N*/             else
/*N*/                 aString = String();
/*N*/         }
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aString.Erase();
/*N*/
/*N*/ 	USHORT nPos=0;
/*N*/ 	if ( (nPos = aString.Search( (sal_Unicode)'\0') ) != STRING_NOTFOUND )
/*N*/ 		aString.Erase( nPos );
/*N*/
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSStringProperty_Impl::Len()
/*N*/ {
/*N*/ 	if ( bIsUniCode )
/*N*/ 		return aString.Len() + 5;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Non-unicode strings are always stored with UTF8 encoding
/*N*/ 		return ByteString( aString, RTL_TEXTENCODING_UTF8 ).Len() + 5;
/*N*/ 	}
/*N*/ }

//=========================================================================

/*N*/ class SfxDocumentInfo_Impl
/*N*/ {
/*N*/ public:
/*N*/ 	String	aCopiesTo;
/*N*/ 	String	aOriginal;
/*N*/ 	String	aReferences;
/*N*/ 	String	aRecipient;
/*N*/ 	String	aReplyTo;
/*N*/ 	String	aBlindCopies;
/*N*/ 	String	aInReplyTo;
/*N*/ 	String	aNewsgroups;
/*N*/ 	String	aSpecialMimeType;
/*N*/ 	USHORT	nPriority;
/*N*/ 	BOOL	bUseUserData;
/*N*/
/*N*/ 	SfxDocumentInfo_Impl() : nPriority( 0 ), bUseUserData( 1 ) {}
/*N*/ };

//=========================================================================

/*N*/ class SfxPSUINT32Property_Impl : public SfxPSProperty_Impl
/*N*/ {
/*N*/ private:
/*N*/ 	UINT32	aInt;
/*N*/
/*N*/   public:
/*N*/ 	SfxPSUINT32Property_Impl( UINT32 nIdP, UINT32 aIntP ) :
/*N*/ 		aInt( aIntP ), SfxPSProperty_Impl( nIdP, VT_I4 ) {}
/*N*/
/*N*/ 	virtual ULONG	Save( SvStream& rStream );
/*N*/ 	virtual ULONG	Len();
/*N*/ };

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSUINT32Property_Impl::Save( SvStream& rStream )
/*N*/ {
/*N*/ 	rStream << aInt;
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSUINT32Property_Impl::Len()
/*N*/ {
/*N*/ 	return 4;
/*N*/ }

//=========================================================================

/*N*/ class SfxPSDateTimeProperty_Impl : public SfxPSProperty_Impl
/*N*/ {
/*N*/ private:
/*N*/ 	DateTime aDateTime;
/*N*/
/*N*/ public:
/*N*/ 	SfxPSDateTimeProperty_Impl( UINT32 nIdP, const DateTime& rDateTime ) :
/*N*/ 		aDateTime( rDateTime ), SfxPSProperty_Impl( nIdP, VT_FILETIME ) {}
/*N*/ 	SfxPSDateTimeProperty_Impl( UINT32 nIdP ) :
/*N*/ 		SfxPSProperty_Impl( nIdP, VT_FILETIME ) {};
/*N*/
/*N*/ 	virtual ULONG Save( SvStream& rStream );
/*N*/ 	virtual ULONG Load( SvStream& rStream );
/*N*/ 	virtual ULONG Len();
/*N*/
/*N*/ 	const DateTime& GetDateTime() { return aDateTime; }
/*N*/ };

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSDateTimeProperty_Impl::Save(SvStream &rStream)
/*N*/ {
/*N*/ 	// Nicht Valid ist das gleiche, wie bei MS, nur nicht konvertiert
/*N*/ 	if( aDateTime.IsValid() )
/*N*/ 		aDateTime.ConvertToUTC();
/*N*/ 	BigInt a100nPerSecond(10000000L);
/*N*/ 	BigInt a100nPerDay=a100nPerSecond*BigInt(60L*60*24);
/*N*/ 	USHORT nYears=aDateTime.GetYear()-1601;
/*N*/ 	long nDays=
/*N*/ 		nYears*365+nYears/4-nYears/100+nYears/400+
/*N*/ 			aDateTime.GetDayOfYear()-1;
/*N*/ 	BigInt aTime=
/*N*/ 		a100nPerDay*BigInt(nDays)+a100nPerSecond*
/*N*/ 			BigInt((long)( aDateTime.GetSec() +
/*N*/ 				   60* aDateTime.GetMin() +
/*N*/ 				   60L*60* aDateTime.GetHour() ));
/*N*/
/*N*/ 	BigInt aUlongMax(SAL_MAX_UINT32);
/*N*/ 	aUlongMax += 1;
/*N*/
/*N*/ 	rStream<<rStream<<static_cast<sal_uInt32>(static_cast<ULONG>(aTime % aUlongMax));
/*N*/ 	rStream<<rStream<<static_cast<sal_uInt32>(static_cast<ULONG>(aTime / aUlongMax));
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ErrCode SfxPSDateTimeProperty_Impl::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	UINT32 nLow, nHigh;
/*N*/ 	rStream >> nLow;
/*N*/ 	rStream >> nHigh;
/*N*/ 	BigInt aUlongMax( SAL_MAX_UINT32 );
/*N*/ 	aUlongMax += 1;
/*N*/ 	BigInt aTime = aUlongMax * BigInt( nHigh );
/*N*/ 	aTime += nLow;
/*N*/ 	BigInt a100nPerSecond(10000000L);
/*N*/ 	BigInt a100nPerDay = a100nPerSecond*BigInt( 60L * 60 * 24 );
/*N*/ 	ULONG nDays = aTime / a100nPerDay;
/*N*/ 	USHORT nYears = (USHORT)
/*N*/ 		(( nDays - ( nDays / ( 4 * 365 ) ) + ( nDays / ( 100 * 365 ) ) -
/*N*/ 		  ( nDays / ( 400 * 365 ) ) ) / 365 );
/*N*/ 	nDays -= nYears * 365 + nYears / 4 - nYears / 100 + nYears / 400;
/*N*/ 	USHORT nMonths = 0;
/*N*/ 	for( long nDaysCount = nDays; nDaysCount >= 0; )
/*N*/ 	{
/*N*/ 		nDays = nDaysCount;
/*N*/ 		nMonths ++;
/*N*/ 		nDaysCount-= Date(  1, nMonths, 1601 + nYears ).GetDaysInMonth();
/*N*/ 	}
/*N*/ 	Date _aDate( (USHORT)( nDays + 1 ), nMonths, nYears + 1601 );
/*N*/ 	Time _aTime( ( aTime / ( a100nPerSecond * BigInt( 60 * 60 ) ) ) %
/*N*/ 				 BigInt( 24 ),
/*N*/ 				 ( aTime / ( a100nPerSecond * BigInt( 60 ) ) ) %
/*N*/ 				 BigInt( 60 ),
/*N*/ 				 ( aTime / ( a100nPerSecond ) ) %
/*N*/ 				 BigInt( 60 ) );
/*N*/ 	aDateTime = DateTime( _aDate, _aTime );
/*N*/ 	aDateTime.ConvertToLocalTime();
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSDateTimeProperty_Impl::Len()
/*N*/ {
/*N*/ 	return 8;
/*N*/ }

//=========================================================================

/*N*/ typedef SfxPSProperty_Impl *SfxPSPropertyPtr_Impl;
/*N*/ SV_DECL_PTRARR_DEL(SfxPSPropertyArr_Impl, SfxPSPropertyPtr_Impl, 10, 10)//STRIP008 ;
/*N*/ SV_IMPL_PTRARR(SfxPSPropertyArr_Impl, SfxPSPropertyPtr_Impl);

/*N*/ struct SfxPSSection_Impl
/*N*/ {
/*N*/ 	SvGlobalName aId;
/*N*/ 	SfxPSPropertyArr_Impl aProperties;
/*N*/ 	ULONG Save(SvStream &rStream);
/*N*/ };

//=========================================================================

/*N*/ class SfxPS_Impl
/*N*/ {
/*N*/ 	SfxPSSection_Impl aSection;
/*N*/ 	USHORT GetPos( UINT32 nId );
/*N*/
/*N*/   public:
/*N*/
/*N*/ 	void   SetSectionName(const SvGlobalName& aIdP);
/*N*/ 	void   AddProperty( SfxPSProperty_Impl* pProp);
/*N*/
/*N*/ 	ULONG Save(SvStream &rStream);
/*N*/ };


/*N*/ USHORT SfxPS_Impl::GetPos( UINT32 nId )
/*N*/ {
/*N*/ 	SfxPSPropertyArr_Impl& rProperties = aSection.aProperties;
/*N*/ 	USHORT nCount = rProperties.Count();
/*N*/ 	for( USHORT n = 0 ; n < nCount; n++ )
/*N*/ 		if( rProperties.GetObject( n )->GetId() == nId )
/*N*/ 			return n;
/*N*/ 	return USHRT_MAX;
/*N*/ }


/*N*/ void  SfxPS_Impl::AddProperty( SfxPSProperty_Impl* pProp)
/*N*/ {
/*N*/ 	USHORT nPos = GetPos( pProp->GetId() );
/*N*/ 	if( nPos != USHRT_MAX )
/*N*/ 	{
/*?*/ 		delete aSection.aProperties[ nPos ];
/*?*/ 		aSection.aProperties.Remove( nPos );
/*N*/ 	}
/*N*/ 	aSection.aProperties.Insert(pProp,0);
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ void SfxPS_Impl::SetSectionName(const SvGlobalName& aIdP)
/*N*/ {
/*N*/ 	aSection.aId = aIdP;
/*N*/ }


/*N*/ ULONG SfxPS_Impl::Save(SvStream &rStream)
/*N*/ {
/*N*/ 	SvGlobalName aName;
/*N*/ 	rStream << (UINT16) 0xfffe // ByteOrder
/*N*/ 		<< (UINT16) 0          // version
/*N*/ 		<< (UINT16) 1          // Os MinorVersion
/*N*/ 		<< (UINT16)            // Os Type
/*N*/ #if defined(MAC)
/*N*/ 		1
/*N*/ #elif defined(WNT)
/*N*/ 		2
/*N*/ #else
/*N*/ 		0
/*N*/ #endif
/*N*/ 		<< aName
/*N*/ 		<< (UINT32)1; // Immer eine Section
/*N*/ 	return aSection.Save(rStream);
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ ULONG SfxPSSection_Impl::Save(SvStream &rStream)
/*N*/ {
/*N*/
/*N*/ //Nur eine Section schreiben: ( Use of more than 1 section is discouraged
/*N*/ //and will not be supported in future windows apis).
/*N*/
/*N*/ 	ULONG nPos = rStream.Tell();
/*N*/ 	nPos += 20;
/*N*/ 	rStream << aId << (UINT32)nPos; //Offset
/*N*/
/*N*/
/*N*/ 	//SectionHeader Schreiben
/*N*/ 	ULONG nLen=8;
/*N*/ 	USHORT n;
/*N*/ 	for(n=0;n<aProperties.Count();n++)
/*N*/ 		nLen+=(((aProperties[n]->Len() + 3) >> 2) << 2)+12;
/*N*/ 	rStream << (UINT32) nLen << (UINT32) aProperties.Count();
/*N*/
/*N*/ 	//PropertyId/Offsetpaare schreiben
/*N*/
/*N*/ 	nLen=8+aProperties.Count()*8;
/*N*/ 	for(n=0;n<aProperties.Count();n++)
/*N*/ 	{
/*N*/ 		rStream << aProperties[n]->GetId() << (UINT32)nLen;
/*N*/ 		nLen+=(((aProperties[n]->Len() + 3) >> 2) << 2) + 4;
/*N*/ 	}
/*N*/
/*N*/ 	//Inhalte schreiben
/*N*/ 	for(n=0;n<aProperties.Count();n++)
/*N*/ 	{
/*N*/ 		rStream << (UINT32) aProperties[n]->GetType();
/*N*/ 		aProperties[n]->Save(rStream);
/*N*/ 		nLen=aProperties[n]->Len();
/*N*/ 		while(nLen++%4) //Auffuellen auf DWORD Grenze
/*N*/ 			rStream << (UINT8) 0;
/*N*/ 	}
/*N*/
/*N*/ 	return rStream.GetErrorCode();
/*N*/ }

//========================================================================

/*N*/ SvStream& PaddWithBlanks_Impl(SvStream &rStream, USHORT nCount)
/*N*/ {
/*N*/ 	for ( USHORT n = nCount; n; --n )
/*N*/ 		rStream << ' ';
/*N*/ 	return rStream;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ inline SvStream& Skip(SvStream &rStream, USHORT nCount)
/*N*/ {
/*N*/ 	rStream.SeekRel(nCount);
/*N*/ 	return rStream;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ struct FileHeader
/*N*/ {
/*N*/ 	String		aHeader;
/*N*/ 	USHORT		nVersion;
/*N*/ 	BOOL		bPasswd;
/*N*/
/*N*/ 	FileHeader( const char* pHeader, USHORT nV, BOOL bPass ) :
/*N*/ 		nVersion( nV ), bPasswd( bPass ) { aHeader = String::CreateFromAscii( pHeader ); }
/*N*/ 	FileHeader( SvStream& rStream );
/*N*/
/*N*/ 	void	Save( SvStream& rStream ) const;
/*N*/ };

//-------------------------------------------------------------------------


/*N*/ FileHeader::FileHeader( SvStream& rStream )
/*N*/ {
/*N*/ 	BYTE b;
/*N*/ 	long nVer = rStream.GetVersion();
/*N*/ 	rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
/*N*/     rStream.ReadByteString( aHeader ); //(dv??)
/*N*/ 	rStream >> nVersion >> b;
/*N*/ 	rStream.SetVersion( nVer );
/*N*/ 	bPasswd = (BOOL)b;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ void FileHeader::Save( SvStream& rStream ) const
/*N*/ {
/*N*/ 	long nVer = rStream.GetVersion();
/*N*/ 	rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
/*N*/     rStream.WriteByteString( aHeader );
/*N*/ 	rStream.SetVersion( nVer );
/*N*/ 	rStream << nVersion;
/*N*/ 	rStream << (BYTE)bPasswd;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ BOOL SfxDocUserKey::Load(SvStream &rStream)
/*N*/ {
/*N*/     rStream.ReadByteString( aTitle );
/*N*/ 	Skip(rStream, SFXDOCUSERKEY_LENMAX - aTitle.Len());
/*N*/     rStream.ReadByteString( aWord );
/*N*/ 	Skip(rStream, SFXDOCUSERKEY_LENMAX - aWord.Len());
/*N*/ 	return rStream.GetError() == SVSTREAM_OK;
/*N*/ }


//-------------------------------------------------------------------------

/*N*/ BOOL SfxDocUserKey::Save(SvStream &rStream) const
/*N*/ {
/*N*/ 	DBG_ASSERT( aTitle.Len() <= SFXDOCUSERKEY_LENMAX, "length of user key title overflow" );
/*N*/ 	DBG_ASSERT( aWord.Len() <= SFXDOCUSERKEY_LENMAX, "length of user key value overflow" );
/*N*/
/*N*/ 	// save the title of the user key
/*N*/ 	String aString = aTitle;
/*N*/ 	aString.Erase( SFXDOCUSERKEY_LENMAX );
/*N*/ 	rStream.WriteByteString( aString );
/*N*/     PaddWithBlanks_Impl(rStream, SFXDOCUSERKEY_LENMAX - aString.Len());
/*N*/
/*N*/ 	// save the value of the user key
/*N*/ 	aString = aWord;
/*N*/ 	aString.Erase( SFXDOCUSERKEY_LENMAX );
/*N*/ 	rStream.WriteByteString( aString );
/*N*/     PaddWithBlanks_Impl(rStream, SFXDOCUSERKEY_LENMAX - aString.Len());
/*N*/
/*N*/ 	return rStream.GetError() == SVSTREAM_OK;
/*N*/ }

//-------------------------------------------------------------------------
/*N*/ SfxDocUserKey::SfxDocUserKey( const String& rTitle, const String& rWord ) :
/*N*/ 		aTitle( rTitle ), aWord( rWord )
/*N*/ {
/*N*/ }
//------------------------------------------------------------------------
/*N*/ const SfxDocUserKey& SfxDocUserKey::operator=(const SfxDocUserKey &rCopy)
/*N*/ {
/*N*/ 	aTitle = rCopy.aTitle;
/*N*/ 	aWord = rCopy.aWord;
/*N*/ 	return *this;
/*N*/ }
// SfxDocumentInfo -------------------------------------------------------

/*N*/ sal_Bool TestValidity_Impl( const String& rString, sal_Bool bURL )
/*N*/ {
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	xub_StrLen nLen = rString.Len();
/*N*/ 	if ( nLen >= 1024 &&
/*N*/ 		 ( !bURL || INetURLObject::CompareProtocolScheme( rString ) == INET_PROT_NOT_VALID ) )
/*N*/ 		// !bURL == the default target has not so many characters
/*N*/ 		// bURL ==  the reload url must have a valid protocol
/*N*/ 		bRet = sal_False;
/*N*/
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL SfxDocumentInfo::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	long d, t;
/*N*/ 	USHORT nUS;
/*N*/ 	BYTE nByte;
/*N*/ 	FileHeader aHeader(rStream);
/*N*/ 	if( ! aHeader.aHeader.EqualsAscii( pDocInfoHeader ))
/*N*/ 	{
/*N*/ 		rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ 	Free();
/*N*/ 	bPasswd = aHeader.bPasswd;
/*N*/ 	rStream >> nUS;
/*N*/ 	//eFileCharSet = (CharSet)nUS;
/*N*/     eFileCharSet = GetSOLoadTextEncoding( nUS );
/*N*/
/*N*/ 		// Einstellen an den Streams
/*N*/ 	rStream.SetStreamCharSet(eFileCharSet);
/*N*/
/*N*/ 	rStream >> nByte;
/*N*/ 	bPortableGraphics = nByte? 1: 0;
/*N*/ 	rStream >> nByte;
/*N*/ 	bQueryTemplate = nByte? 1: 0;
/*N*/
/*N*/ 	aCreated.Load(rStream);
/*N*/ 	aChanged.Load(rStream);
/*N*/ 	aPrinted.Load(rStream);
/*N*/
/*N*/     rStream.ReadByteString( aTitle );
/*N*/ 	Skip(rStream, SFXDOCINFO_TITLELENMAX - aTitle.Len());
/*N*/     rStream.ReadByteString( aTheme );
/*N*/ 	Skip(rStream, SFXDOCINFO_THEMELENMAX - aTheme.Len());
/*N*/     rStream.ReadByteString( aComment );
/*N*/ 	Skip(rStream, SFXDOCINFO_COMMENTLENMAX- aComment.Len());
/*N*/     rStream.ReadByteString( aKeywords );
/*N*/ 	Skip(rStream, SFXDOCINFO_KEYWORDLENMAX - aKeywords.Len());
/*N*/
/*N*/ 	USHORT i;
/*N*/ 	for(i = 0; i < MAXDOCUSERKEYS; ++i)
/*N*/ 		aUserKeys[i].Load(rStream);
/*N*/
/*N*/     rStream.ReadByteString( aTemplateName );
/*N*/     rStream.ReadByteString( aTemplateFileName );
/*N*/ 	rStream >> d >> t;
/*N*/ 	aTemplateDate = DateTime(Date(d), Time(t));
/*N*/
/*N*/ 	// wurde mal fuer MB in Panik eingebaut und dann doch nie benutzt :-)
/*N*/ 	if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
/*N*/ 	{
/*N*/ 		USHORT nMailAddr;
/*N*/ 		rStream >> nMailAddr;
/*N*/ 		for( i = 0; i < nMailAddr; i++ )
/*N*/ 		{
/*N*/ 			String aDummyString;
/*N*/ 			USHORT nDummyFlags;
/*N*/             rStream.ReadByteString( aDummyString );
/*N*/ 			rStream >> nDummyFlags;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	rStream >> lTime;
/*N*/ 	if(aHeader.nVersion > 4)
/*N*/ 		rStream >> nDocNo;
/*N*/ 	else
/*N*/ 		nDocNo = 1;
/*N*/ 	rStream >> nUserDataSize;
/*N*/ 	if(nUserDataSize) {
/*N*/ 		pUserData = new char[nUserDataSize];
/*N*/ 		rStream.Read(pUserData,nUserDataSize);
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bOK = (rStream.GetError() == SVSTREAM_OK);
/*N*/ 	nByte = 0;                          // wg.Kompatibilitaet;
/*N*/ 	rStream >> nByte;                   // evtl. nicht in DocInfo enthalten
/*N*/ 	bTemplateConfig = nByte ? 1 : 0;
/*N*/ 	if( aHeader.nVersion > 5 )
/*N*/ 	{
/*N*/ 		rStream >> bReloadEnabled;
/*N*/         rStream.ReadByteString( aReloadURL );
/*N*/ 		rStream >> nReloadSecs;
/*N*/         rStream.ReadByteString( aDefaultTarget );
/*N*/
/*N*/ 		if ( !TestValidity_Impl( aReloadURL, sal_True ) )
/*N*/ 		{
/*N*/ 			// the reload url is invalid -> reset all reload attributes
/*N*/ 			bReloadEnabled = FALSE;
/*N*/ 			aReloadURL.Erase();
/*N*/ 			nReloadSecs = 60;
/*N*/         	aDefaultTarget.Erase();
/*N*/ 		}
/*N*/ 		else if ( !TestValidity_Impl( aDefaultTarget, sal_False ) )
/*N*/ 			// the default target is invalid -> reset it
/*N*/         	aDefaultTarget.Erase();
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 6 )
/*N*/ 	{
/*N*/ 		rStream >> nByte;
/*N*/ 		bSaveGraphicsCompressed = nByte? 1: 0;
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 7 )
/*N*/ 	{
/*N*/ 		rStream >> nByte;
/*N*/ 		bSaveOriginalGraphics = nByte? 1: 0;
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 8 )
/*N*/ 	{
/*N*/ 		rStream >> nByte;
/*N*/ 		bSaveVersionOnClose = nByte? 1: 0;
/*N*/
/*N*/         rStream.ReadByteString( pImp->aCopiesTo );
/*N*/         rStream.ReadByteString( pImp->aOriginal );
/*N*/         rStream.ReadByteString( pImp->aReferences );
/*N*/         rStream.ReadByteString( pImp->aRecipient );
/*N*/         rStream.ReadByteString( pImp->aReplyTo );
/*N*/         rStream.ReadByteString( pImp->aBlindCopies );
/*N*/         rStream.ReadByteString( pImp->aInReplyTo );
/*N*/         rStream.ReadByteString( pImp->aNewsgroups );
/*N*/ 		rStream	>> pImp->nPriority;
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 9 )
/*N*/ 	{
/*N*/         rStream.ReadByteString( pImp->aSpecialMimeType );
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 10 )
/*N*/ 	{
/*N*/ 		rStream >> nByte;
/*N*/ 		pImp->bUseUserData = nByte ? TRUE : FALSE;
/*N*/ 	}
/*N*/ 	return bOK;
/*N*/ }

//-------------------------------------------------------------------------
/*N*/ BOOL SfxDocumentInfo::SavePropertySet( SvStorage *pStorage) const
/*N*/ {
/*N*/ 	SfxPS_Impl* pPS = new SfxPS_Impl;
/*N*/ 	SvStorageStreamRef aStrPropSet = pStorage->OpenStream(
/*N*/ 		String::CreateFromAscii( pPropSlot ), STREAM_TRUNC | STREAM_STD_WRITE );
/*N*/ 	if ( !aStrPropSet.Is() )
/*N*/ 	{
/*N*/ 		DBG_ERRORFILE( "can not open the property set" );
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	pPS->SetSectionName( SvGlobalName(
/*N*/ 		0xf29f85e0, 0x4ff9, 0x1068, 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_TITLE, GetTitle() ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_SUBJECT, GetTheme() ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_KEYWORDS, GetKeywords() ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_TEMPLATE, GetTemplateName() ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_COMMENTS, GetComment() ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_AUTHOR, GetCreated().GetName() ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl( PID_LASTAUTHOR, GetChanged().GetName() ) );
/*N*/ 	pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_CREATE_DTM, GetCreated().GetTime() ) );
/*N*/ 	pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_LASTSAVED_DTM, GetChanged().GetTime() ) );
/*N*/ 	if ( GetPrinted().GetTime() != GetCreated().GetTime() )
/*N*/ 		pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_LASTPRINTED_DTM, GetPrinted().GetTime() ) );
/*N*/ 	DateTime aEditTime( Date( 1, 1, 1601 ), Time( GetTime() ) );
/*N*/ 	aEditTime.ConvertToLocalTime();
/*N*/ 	pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_EDITTIME, aEditTime ) );
/*N*/ 	pPS->AddProperty( new SfxPSStringProperty_Impl(
/*N*/ 		PID_REVNUMBER, String::CreateFromInt32( GetDocumentNumber() ) ) );
/*N*/ 	pPS->AddProperty( new SfxPSCodePageProperty_Impl( RTL_TEXTENCODING_UTF8 ));
/*N*/ 	pPS->Save( *aStrPropSet );
/*N*/ 	delete pPS;
/*N*/ 	return ( aStrPropSet->GetErrorCode() == 0 );
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ BOOL SfxDocumentInfo::Save( SvStream& rStream ) const
/*N*/ {
/*N*/ 	FileHeader aHeader(pDocInfoHeader, VERSION, bPasswd? 1: 0);
/*N*/ 	aHeader.Save(rStream);
/*N*/ 	CharSet eNewFileCharSet = GetSOStoreTextEncoding( eFileCharSet );
/*N*/ 	rStream << (USHORT)eNewFileCharSet;
/*N*/ 	rStream.SetStreamCharSet(eNewFileCharSet);
/*N*/ 	rStream << (bPortableGraphics? (BYTE)1: (BYTE)0)
/*N*/ 			<< (bQueryTemplate? (BYTE)1: (BYTE)0);
/*N*/ 	aCreated.Save(rStream);
/*N*/ 	aChanged.Save(rStream);
/*N*/ 	aPrinted.Save(rStream);
/*N*/
/*N*/ 	DBG_ASSERT( aTitle.Len() <= SFXDOCINFO_TITLELENMAX , "length of title overflow" );
/*N*/ 	DBG_ASSERT( aTheme.Len() <= SFXDOCINFO_THEMELENMAX , "length of theme overflow" );
/*N*/ 	DBG_ASSERT( aComment.Len() <= SFXDOCINFO_COMMENTLENMAX , "length of description overflow" );
/*N*/ 	DBG_ASSERT( aKeywords.Len() <= SFXDOCINFO_KEYWORDLENMAX , "length of keywords overflow" );
/*N*/
/*N*/ 	// save the title
/*N*/ 	String aString = aTitle;
/*N*/ 	aString.Erase( SFXDOCINFO_TITLELENMAX );
/*N*/ 	rStream.WriteByteString( aString );
/*N*/     PaddWithBlanks_Impl(rStream, SFXDOCINFO_TITLELENMAX - aString.Len());
/*N*/ 	// save the theme
/*N*/ 	aString = aTheme;
/*N*/ 	aString.Erase( SFXDOCINFO_THEMELENMAX );
/*N*/ 	rStream.WriteByteString( aString );
/*N*/     PaddWithBlanks_Impl(rStream, SFXDOCINFO_THEMELENMAX - aString.Len());
/*N*/ 	// save the description
/*N*/ 	aString = aComment;
/*N*/ 	aString.Erase( SFXDOCINFO_COMMENTLENMAX );
/*N*/ 	rStream.WriteByteString( aString );
/*N*/     PaddWithBlanks_Impl(rStream, SFXDOCINFO_COMMENTLENMAX - aString.Len());
/*N*/ 	// save the keywords
/*N*/ 	aString = aKeywords;
/*N*/ 	aString.Erase( SFXDOCINFO_KEYWORDLENMAX );
/*N*/ 	rStream.WriteByteString( aString );
/*N*/     PaddWithBlanks_Impl(rStream, SFXDOCINFO_KEYWORDLENMAX - aString.Len());
/*N*/
/*N*/ 	for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i)
/*N*/ 		aUserKeys[i].Save(rStream);
/*N*/     rStream.WriteByteString( aTemplateName );
/*N*/     rStream.WriteByteString( aTemplateFileName );
/*N*/     rStream << (long)aTemplateDate.GetDate()
/*N*/ 			<< (long)aTemplateDate.GetTime();
/*N*/
/*N*/ 	// wurde mal fuer MB in Panik eingebaut und dann doch nie benutzt :-)
/*N*/ 	if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
/*N*/ 		rStream << (USHORT) 0;
/*N*/
/*N*/     rStream << GetTime() << GetDocumentNumber();
/*N*/
/*N*/ 	rStream << nUserDataSize;
/*N*/ 	if(pUserData)
/*N*/ 		rStream.Write(pUserData, nUserDataSize);
/*N*/ 	rStream << (bTemplateConfig? (BYTE)1: (BYTE)0);
/*N*/ 	if( aHeader.nVersion > 5 )
/*N*/ 	{
/*N*/ 		rStream << bReloadEnabled;
/*N*/         rStream.WriteByteString( aReloadURL );
/*N*/ 		rStream << nReloadSecs;
/*N*/         rStream.WriteByteString( aDefaultTarget );
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 6 )
/*N*/ 		rStream << (bSaveGraphicsCompressed? (BYTE)1: (BYTE)0);
/*N*/ 	if ( aHeader.nVersion > 7 )
/*N*/ 		rStream << (bSaveOriginalGraphics? (BYTE)1: (BYTE)0);
/*N*/ 	if ( aHeader.nVersion > 8 )
/*N*/ 	{
/*N*/ 		rStream << (bSaveVersionOnClose? (BYTE)1: (BYTE)0);
/*N*/         rStream.WriteByteString( pImp->aCopiesTo );
/*N*/         rStream.WriteByteString( pImp->aOriginal );
/*N*/         rStream.WriteByteString( pImp->aReferences );
/*N*/         rStream.WriteByteString( pImp->aRecipient );
/*N*/         rStream.WriteByteString( pImp->aReplyTo );
/*N*/         rStream.WriteByteString( pImp->aBlindCopies );
/*N*/         rStream.WriteByteString( pImp->aInReplyTo );
/*N*/         rStream.WriteByteString( pImp->aNewsgroups );
/*N*/         rStream << pImp->nPriority;
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 9 )
/*N*/ 	{
/*N*/         rStream.WriteByteString( pImp->aSpecialMimeType );
/*N*/ 	}
/*N*/ 	if ( aHeader.nVersion > 10 )
/*N*/ 	{
/*N*/ 		rStream << ( pImp->bUseUserData ? (BYTE)1: (BYTE)0 );
/*N*/ 	}
/*N*/
/*N*/ 	return rStream.GetError() == SVSTREAM_OK;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ BOOL SfxDocumentInfo::Load(SvStorage* pStorage)
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if(!pStorage->IsStream( String::CreateFromAscii( pDocInfoSlot )))
/*N*/ 		return FALSE;
/*N*/ #endif
/*N*/     if ( pStorage->GetVersion() >= SOFFICE_FILEFORMAT_60 )
/*N*/     {
/*N*/         DBG_ERROR("This method only supports binary file format, use service StandaloneDocumentInfo!");
/*N*/         return FALSE;
/*N*/     }
/*N*/
/*N*/ 	SvStorageStreamRef aStr = pStorage->OpenStream( String::CreateFromAscii( pDocInfoSlot ),STREAM_STD_READ);
/*N*/ 	if(!aStr.Is())
/*N*/ 		return FALSE;
/*N*/ 	aStr->SetVersion( pStorage->GetVersion() );
/*N*/ 	aStr->SetBufferSize(STREAM_BUFFER_SIZE);
/*N*/ 	BOOL bRet = Load(*aStr);
/*N*/ 	if ( bRet )
/*N*/ 	{
/*N*/ 		String aStr = SotExchange::GetFormatMimeType( pStorage->GetFormat() );
/*N*/ 		USHORT nPos = aStr.Search(';');
/*N*/ 		if ( nPos != STRING_NOTFOUND )
/*N*/ 			pImp->aSpecialMimeType = aStr.Copy( 0, nPos );
/*N*/ 		else
/*N*/ 			pImp->aSpecialMimeType = aStr;
/*N*/ 	}
/*N*/
/*N*/ 	return bRet;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ BOOL SfxDocumentInfo::Save(SvStorage* pStorage) const
/*N*/ {
/*N*/ 	SvStorageStreamRef aStr = pStorage->OpenStream( String::CreateFromAscii( pDocInfoSlot ), STREAM_TRUNC | STREAM_STD_READWRITE);
/*N*/ 	if(!aStr.Is())
/*N*/ 		return FALSE;
/*N*/ 	aStr->SetVersion( pStorage->GetVersion() );
/*N*/ 	aStr->SetBufferSize(STREAM_BUFFER_SIZE);
/*N*/ 	if(!Save(*aStr))
/*N*/ 		return FALSE;
/*N*/ 	return SavePropertySet( pStorage );
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ const SfxDocUserKey& SfxDocumentInfo::GetUserKey( USHORT n ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
/*N*/ 	return aUserKeys[n];
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ void SfxDocumentInfo::SetUserKey( const SfxDocUserKey& rKey, USHORT n )
/*N*/ {
/*N*/ 	DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
/*N*/ 	aUserKeys[n] = rKey;
/*N*/ }


//-------------------------------------------------------------------------

/*N*/ SfxDocumentInfo::SfxDocumentInfo( const SfxDocumentInfo& rInf ):
/*N*/ 	nUserDataSize(0),
/*N*/ 	pUserData(0)
/*N*/ {
/*N*/ 	pImp = new SfxDocumentInfo_Impl;
/*N*/ 	bReadOnly = rInf.bReadOnly;
/*N*/ 	bReloadEnabled = FALSE;
/*N*/ 	nReloadSecs = 60;
/*N*/ 	*this = rInf;
/*N*/ }

/*N*/ BOOL SfxDocumentInfo::IsReloadEnabled() const
/*N*/ {
/*N*/ 	return bReloadEnabled;
/*N*/ }

/*?*/ void SfxDocumentInfo::EnableReload( BOOL bEnable )
/*?*/ {
/*?*/ 	bReloadEnabled = bEnable;
/*?*/ }

/*N*/ const String& SfxDocumentInfo::GetDefaultTarget() const
/*N*/ {
/*N*/ 	return aDefaultTarget;
/*N*/ }

/*?*/ void SfxDocumentInfo::SetDefaultTarget( const String& rString )
/*?*/ {
/*?*/ 	aDefaultTarget = rString;
/*?*/ }

/*N*/ const String& SfxDocumentInfo::GetReloadURL() const
/*N*/ {
/*N*/ 	return aReloadURL;
/*N*/ }

/*?*/ void SfxDocumentInfo::SetReloadURL( const String& rString )
/*?*/ {
/*?*/ 	aReloadURL = rString;
/*?*/ }

/*N*/ ULONG SfxDocumentInfo::GetReloadDelay() const
/*N*/ {
/*N*/ 	return nReloadSecs;
/*N*/ }

/*N*/ void SfxDocumentInfo::SetReloadDelay( ULONG nSec )
/*N*/ {
/*N*/ 	nReloadSecs = nSec;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ const SfxDocumentInfo& SfxDocumentInfo::operator=( const SfxDocumentInfo& rInf)
/*N*/ {
/*N*/ 	if( this == &rInf ) return *this;
/*N*/
/*N*/ 	bReadOnly           = rInf.bReadOnly;
/*N*/ 	bPasswd             = rInf.bPasswd;
/*N*/ 	bPortableGraphics   = rInf.bPortableGraphics;
/*N*/ 	bSaveGraphicsCompressed = rInf.bSaveGraphicsCompressed;
/*N*/ 	bSaveOriginalGraphics = rInf.bSaveOriginalGraphics;
/*N*/ 	bQueryTemplate      = rInf.bQueryTemplate;
/*N*/ 	bTemplateConfig     = rInf.bTemplateConfig;
/*N*/ 	eFileCharSet        = rInf.eFileCharSet;
/*N*/
/*N*/ 	aCreated  = rInf.aCreated;
/*N*/ 	aChanged  = rInf.aChanged;
/*N*/ 	aPrinted  = rInf.aPrinted;
/*N*/ 	aTitle    = rInf.aTitle;
/*N*/ 	aTheme    = rInf.aTheme;
/*N*/ 	aComment  = rInf.aComment;
/*N*/ 	aKeywords = rInf.aKeywords;
/*N*/
/*N*/ 	for(USHORT i=0; i < MAXDOCUSERKEYS; ++i)
/*N*/ 		aUserKeys[i] = rInf.aUserKeys[i];
/*N*/
/*N*/ 	aTemplateName = rInf.aTemplateName;
/*N*/ 	aTemplateFileName = rInf.aTemplateFileName;
/*N*/ 	aTemplateDate = rInf.aTemplateDate;
/*N*/
/*N*/ 	aDefaultTarget = rInf.GetDefaultTarget();
/*N*/ 	aReloadURL = rInf.GetReloadURL();
/*N*/ 	bReloadEnabled = rInf.IsReloadEnabled();
/*N*/ 	nReloadSecs = rInf.GetReloadDelay();
/*N*/
/*N*/ 	Free();
/*N*/
/*N*/ 	nUserDataSize = rInf.nUserDataSize;
/*N*/ 	if(nUserDataSize) {
/*N*/ 		pUserData = new char[nUserDataSize];
/*N*/ 		memcpy(pUserData, rInf.pUserData, nUserDataSize);
/*N*/ 	}
/*N*/ 	lTime = rInf.lTime;
/*N*/ 	nDocNo = rInf.nDocNo;
/*N*/
/*N*/ 	bSaveVersionOnClose 	= rInf.bSaveVersionOnClose;
/*N*/ 	pImp->aCopiesTo    		= rInf.pImp->aCopiesTo;
/*N*/ 	pImp->aOriginal    		= rInf.pImp->aOriginal;
/*N*/ 	pImp->aReferences  		= rInf.pImp->aReferences;
/*N*/ 	pImp->aRecipient   		= rInf.pImp->aRecipient;
/*N*/ 	pImp->aReplyTo     		= rInf.pImp->aReplyTo;
/*N*/ 	pImp->aBlindCopies 		= rInf.pImp->aBlindCopies;
/*N*/ 	pImp->aInReplyTo   		= rInf.pImp->aInReplyTo;
/*N*/ 	pImp->aNewsgroups  		= rInf.pImp->aNewsgroups;
/*N*/ 	pImp->aSpecialMimeType 	= rInf.pImp->aSpecialMimeType;
/*N*/ 	pImp->nPriority    		= rInf.pImp->nPriority;
/*N*/ 	pImp->bUseUserData		= rInf.pImp->bUseUserData;
/*N*/
/*N*/ 	return *this;
/*N*/ }

//-------------------------------------------------------------------------

/*?*/ int SfxDocumentInfo::operator==( const SfxDocumentInfo& rCmp) const
/*?*/ {
/*?*/ 	if(eFileCharSet != rCmp.eFileCharSet ||
/*?*/ 		bPasswd != rCmp.bPasswd ||
/*?*/ 		bPortableGraphics != rCmp.bPortableGraphics ||
/*?*/ 		bSaveGraphicsCompressed != rCmp.bSaveGraphicsCompressed ||
/*?*/ 		bSaveOriginalGraphics != rCmp.bSaveOriginalGraphics ||
/*?*/ 		bQueryTemplate != rCmp.bQueryTemplate  ||
/*?*/ 		bTemplateConfig != rCmp.bTemplateConfig  ||
/*?*/ 		aCreated != rCmp.aCreated ||
/*?*/ 		aChanged != rCmp.aChanged ||
/*?*/ 		aPrinted != rCmp.aPrinted ||
/*?*/ 		aTitle != rCmp.aTitle ||
/*?*/ 		aTheme != rCmp.aTheme ||
/*?*/ 		aComment != rCmp.aComment ||
/*?*/ 		aKeywords != rCmp.aKeywords ||
/*?*/ 		aTemplateName != rCmp.aTemplateName ||
/*?*/ 		aTemplateDate != rCmp.aTemplateDate ||
/*?*/ 	   IsReloadEnabled() != rCmp.IsReloadEnabled() ||
/*?*/ 	   GetReloadURL() != rCmp.GetReloadURL() ||
/*?*/ 	   GetReloadDelay() != rCmp.GetReloadDelay() ||
/*?*/ 	   GetDefaultTarget() != rCmp.GetDefaultTarget())
/*?*/ 		return FALSE;
/*?*/
/*?*/ 	for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i) {
/*?*/ 		if(aUserKeys[i] != rCmp.aUserKeys[i])
/*?*/ 			return FALSE;
/*?*/ 	}
/*?*/ 	if(nUserDataSize != rCmp.nUserDataSize)
/*?*/ 		return FALSE;
/*?*/ 	if(nUserDataSize)
/*?*/ 		return 0 == memcmp(pUserData, rCmp.pUserData, nUserDataSize);
/*?*/
/*?*/ 	if ( pImp->aCopiesTo		!= rCmp.pImp->aCopiesTo 	 ||
/*?*/ 		 pImp->aOriginal		!= rCmp.pImp->aOriginal      ||
/*?*/ 		 pImp->aReferences		!= rCmp.pImp->aReferences    ||
/*?*/ 		 pImp->aRecipient		!= rCmp.pImp->aRecipient     ||
/*?*/ 		 pImp->aReplyTo			!= rCmp.pImp->aReplyTo       ||
/*?*/ 		 pImp->aBlindCopies		!= rCmp.pImp->aBlindCopies   ||
/*?*/ 		 pImp->aInReplyTo		!= rCmp.pImp->aInReplyTo     ||
/*?*/ 		 pImp->aNewsgroups		!= rCmp.pImp->aNewsgroups    ||
/*?*/ 		 pImp->aSpecialMimeType	!= rCmp.pImp->aSpecialMimeType ||
/*?*/ 		 pImp->nPriority		!= rCmp.pImp->nPriority 	 ||
/*?*/ 		 pImp->bUseUserData		!= rCmp.pImp->bUseUserData	 ||
/*?*/ 		 bSaveVersionOnClose	!= rCmp.bSaveVersionOnClose )
/*?*/ 		return FALSE;
/*?*/
/*?*/ 	return TRUE;
/*?*/ }

//-------------------------------------------------------------------------

/*N*/ void SfxDocumentInfo::Free()
/*N*/ {
/*N*/ 	delete []pUserData;
/*N*/ 	pUserData = 0;
/*N*/ 	nUserDataSize = 0;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ String SfxDocumentInfo::AdjustTextLen_Impl( const String& rText, USHORT nMax )
/*N*/ {
/*N*/ 	String aRet = rText;
/*! pb: dont cut any longer because the new file format has no length limit
    if ( aRet.Len() > nMax )
        aRet.Erase( nMax ); */
/*N*/ 	return aRet;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ SfxDocumentInfo::SfxDocumentInfo() :
/*N*/ 	bPasswd(FALSE),
/*N*/ 	bQueryTemplate(FALSE),
/*N*/ 	bTemplateConfig(FALSE),
/*N*/ 	eFileCharSet(gsl_getSystemTextEncoding()),
/*N*/ 	nUserDataSize(0),
/*N*/ 	pUserData(0),
/*N*/ 	lTime(0),
/*N*/ 	nDocNo(1),
/*N*/ 	aChanged( TIMESTAMP_INVALID_DATETIME ),
/*N*/ 	aPrinted( TIMESTAMP_INVALID_DATETIME ),
/*N*/ 	bSaveVersionOnClose( FALSE )
/*N*/ {
/*N*/ 	pImp = new SfxDocumentInfo_Impl;
/*N*/
/*N*/ 	bReadOnly = FALSE;
/*N*/ 	bReloadEnabled = FALSE;
/*N*/ 	nReloadSecs = 60;
/*N*/ 	SfxApplication *pSfxApp = SFX_APP();
/*N*/     bPortableGraphics = TRUE;
/*N*/     SvtSaveOptions aSaveOptions;
/*N*/     bSaveGraphicsCompressed = FALSE;
/*N*/     bSaveOriginalGraphics = FALSE;
/*N*/
/*N*/ 	const String aInf( DEFINE_CONST_UNICODE( "Info " ) );
/*N*/ 	for( USHORT i = 0; i < MAXDOCUSERKEYS; ++i )    {
/*N*/ 		aUserKeys[i].aTitle = aInf;
/*N*/         aUserKeys[i].aTitle += String::CreateFromInt32(i+1);
/*N*/ 	}
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ SfxDocumentInfo::~SfxDocumentInfo()
/*N*/ {
/*N*/ 	Free();
/*N*/ 	delete pImp;
/*N*/ }

/*?*/ String SfxDocumentInfo::GetCopiesTo() const
/*?*/ {
/*?*/ 	return pImp->aCopiesTo;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetCopiesTo( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aCopiesTo = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetOriginal() const
/*?*/ {
/*?*/ 	return pImp->aOriginal;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetOriginal( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aOriginal = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetReferences() const
/*?*/ {
/*?*/ 	return pImp->aReferences;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetReferences( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aReferences = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetRecipient() const
/*?*/ {
/*?*/ 	return pImp->aRecipient;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetRecipient( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aRecipient = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetReplyTo() const
/*?*/ {
/*?*/ 	return pImp->aReplyTo;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetReplyTo( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aReplyTo = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetBlindCopies() const
/*?*/ {
/*?*/ 	return pImp->aBlindCopies;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetBlindCopies( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aBlindCopies = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetInReplyTo() const
/*?*/ {
/*?*/ 	return pImp->aInReplyTo;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetInReplyTo( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aInReplyTo = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetNewsgroups() const
/*?*/ {
/*?*/ 	return pImp->aNewsgroups;
/*?*/ }

/*?*/ void SfxDocumentInfo::SetNewsgroups( const String& rStr )
/*?*/ {
/*?*/ 	pImp->aNewsgroups = rStr;
/*?*/ }

/*?*/ String SfxDocumentInfo::GetSpecialMimeType() const
/*?*/ {
/*?*/ 	return pImp->aSpecialMimeType;
/*?*/ }

/*N*/ USHORT SfxDocumentInfo::GetPriority() const
/*N*/ {
/*N*/ 	return pImp->nPriority;
/*N*/ }

/*N*/ void SfxDocumentInfo::SetPriority( USHORT nPrio )
/*N*/ {
/*N*/ 	pImp->nPriority = nPrio;
/*N*/ }

/*N*/ BOOL SfxDocumentInfo::IsUseUserData() const
/*N*/ {
/*N*/ 	return pImp->bUseUserData;
/*N*/ }

/*N*/ void SfxDocumentInfo::SetUseUserData( BOOL bNew )
/*N*/ {
/*N*/ 	pImp->bUseUserData = bNew;
/*N*/ }

//-----------------------------------------------------------------------------

/*N*/ void SfxDocumentInfo::SetTitle( const String& rVal )
/*N*/ {
/*N*/ 	aTitle = AdjustTextLen_Impl( rVal, SFXDOCINFO_TITLELENMAX );
/*N*/ }
//------------------------------------------------------------------------

/*N*/ void SfxDocumentInfo::SetTheme( const String& rVal )
/*N*/ {
/*N*/ 	aTheme = AdjustTextLen_Impl( rVal, SFXDOCINFO_THEMELENMAX );
/*N*/ }
//------------------------------------------------------------------------

/*N*/ void SfxDocumentInfo::SetComment( const String& rVal )
/*N*/ {
/*N*/ 	aComment = AdjustTextLen_Impl( rVal, SFXDOCINFO_COMMENTLENMAX );
/*N*/ }
//------------------------------------------------------------------------

/*N*/ void SfxDocumentInfo::SetKeywords( const String& rVal )
/*N*/ {
/*N*/ 	aKeywords = AdjustTextLen_Impl( rVal, SFXDOCINFO_KEYWORDLENMAX );
/*N*/ }
}
