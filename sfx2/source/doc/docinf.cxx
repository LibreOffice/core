/*************************************************************************
 *
 *  $RCSfile: docinf.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:36:01 $
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

#ifndef _BIGINT_HXX //autogen wg. BigInt
#include <tools/bigint.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#include <tools/urlobj.hxx>
#include <svtools/saveopt.hxx>
#include <tools/tenccvt.hxx>
#include <svtools/useroptions.hxx>
#include "rtl/tencinfo.h"

#include "docfilt.hxx"
#include "fcontnr.hxx"
#include "openflag.hxx"
#include "app.hxx"
#include "docinf.hxx"
#include "docfile.hxx"
#include "sfxtypes.hxx"

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

IMPL_PTRHINT(SfxDocumentInfoHint, SfxDocumentInfo)

//=========================================================================

class SfxPSProperty_Impl
{
private:
    UINT32 nId;
    UINT32 nType;

public:
    SfxPSProperty_Impl( UINT32 nIdP, UINT32 nTypeP ) : nId( nIdP ), nType( nTypeP ) {}
    virtual ~SfxPSProperty_Impl();

    virtual ULONG   Save( SvStream& rStream ) = 0;
    virtual ULONG   Load( SvStream& rStream );
    virtual ULONG   Len() = 0;

    UINT32          GetId() const { return nId; }
    UINT32          GetType() const { return nType; }
};

//-------------------------------------------------------------------------

SfxPSProperty_Impl::~SfxPSProperty_Impl()
{
}

//-------------------------------------------------------------------------

ULONG SfxPSProperty_Impl::Load( SvStream& )
{
    return 0;
}


//=========================================================================

class SfxPSCodePageProperty_Impl : public SfxPSProperty_Impl
{
    private:
        CharSet nEncoding;

    public:
        SfxPSCodePageProperty_Impl( CharSet nCharSet ) : SfxPSProperty_Impl( 1, VT_I2 ), nEncoding( nCharSet ) {}
        virtual ~SfxPSCodePageProperty_Impl() {}

    virtual ULONG   Save( SvStream& rStream );
    virtual ULONG   Len();
};

ULONG SfxPSCodePageProperty_Impl::Save( SvStream& rStream )
{
    sal_uInt32 nCodePage = rtl_getWindowsCodePageFromTextEncoding(nEncoding);
    if (nCodePage == 0)
        nCodePage = 1252;
    rStream << (UINT16)nCodePage;
    return rStream.GetErrorCode();
}

ULONG SfxPSCodePageProperty_Impl::Len()
{
    return sizeof( UINT16 );
}

//=========================================================================

class SfxPSStringProperty_Impl : public SfxPSProperty_Impl
{
private:
    String aString;
    CharSet nEncoding;
    BOOL bIsUniCode;

public:
    SfxPSStringProperty_Impl( UINT32 nIdP, const String& aStr ) :
        aString(aStr), SfxPSProperty_Impl( nIdP, VT_LPSTR ), bIsUniCode(FALSE)
    {
        nEncoding = RTL_TEXTENCODING_UTF8;
    }

    SfxPSStringProperty_Impl( UINT32 nIdP ) :
        SfxPSProperty_Impl( nIdP, VT_LPSTR ), bIsUniCode(FALSE)
    {
        nEncoding = RTL_TEXTENCODING_UTF8;
    }

    void SetCodePage( UINT16 nCodePage );
    void SetIsUniCode() { bIsUniCode = TRUE; }

    virtual ULONG   Save( SvStream& rStream );
    virtual ULONG   Load( SvStream& rStream );
    virtual ULONG   Len();

    const String&   GetString() { return aString; }
};


void SfxPSStringProperty_Impl::SetCodePage( UINT16 nCodePage )
{
    rtl_TextEncoding nEnc = rtl_getTextEncodingFromWindowsCodePage(nCodePage);
    if (nEnc != RTL_TEXTENCODING_DONTKNOW)
        nEncoding = nEnc;
}

//-------------------------------------------------------------------------

ULONG SfxPSStringProperty_Impl::Save( SvStream& rStream )
{
    // Now we always write property strings with UTF8 encoding, so we
    // can ensure full unicode compatibility. The code page attribute is
    // written with UTF8 set!
    // Force nEncoding set to UTF8!
    ByteString aTemp( aString, RTL_TEXTENCODING_UTF8 );
    nEncoding = RTL_TEXTENCODING_UTF8;
    UINT32 nLen = aTemp.Len();
    rStream << (UINT32)( nLen + 1 );
    rStream.Write( aTemp.GetBuffer(), nLen );
    rStream << '\0';
    return rStream.GetErrorCode();
}

//-------------------------------------------------------------------------

ULONG SfxPSStringProperty_Impl::Load( SvStream& rStream )
{
    UINT32 nLen;
    rStream >> nLen;
    if ( nLen > 0 )
    {
        if ( bIsUniCode )
        {
            sal_Unicode* pString = new sal_Unicode[ nLen ];
            for ( UINT32 i = 0; i < nLen; i++ )
                rStream >> pString[ i ];
            if ( pString[ i - 1 ] == 0 )
            {
                if ( nLen > 1 )
                    aString = String( pString, (USHORT) nLen - 1 );
                else
                    aString = String();
            }

            delete pString;
        }
        else
        {
            ByteString aTemp;
            if ( nLen>1 )
            {
                rStream.Read( aTemp.AllocBuffer( (xub_StrLen)( nLen - 1 ) ), nLen );
                aString = String( aTemp, nEncoding );
            }
            else
                aString = String();
        }
    }
    else
        aString.Erase();

    return rStream.GetErrorCode();
}

//-------------------------------------------------------------------------

ULONG SfxPSStringProperty_Impl::Len()
{
    if ( bIsUniCode )
        return aString.Len() + 5;
    else
    {
        // Non-unicode strings are always stored with UTF8 encoding
        return ByteString( aString, RTL_TEXTENCODING_UTF8 ).Len() + 5;
    }
}

//=========================================================================

class SfxDocumentInfo_Impl
{
public:
    String  aCopiesTo;
    String  aOriginal;
    String  aReferences;
    String  aRecipient;
    String  aReplyTo;
    String  aBlindCopies;
    String  aInReplyTo;
    String  aNewsgroups;
    String  aSpecialMimeType;
    USHORT  nPriority;
    BOOL    bUseUserData;

    SfxDocumentInfo_Impl() : nPriority( 0 ), bUseUserData( 1 ) {}
};

//=========================================================================

class SfxPSUINT32Property_Impl : public SfxPSProperty_Impl
{
private:
    UINT32  aInt;

  public:
    SfxPSUINT32Property_Impl( UINT32 nIdP, UINT32 aIntP ) :
        aInt( aIntP ), SfxPSProperty_Impl( nIdP, VT_I4 ) {}

    virtual ULONG   Save( SvStream& rStream );
    virtual ULONG   Len();
};

//-------------------------------------------------------------------------

ULONG SfxPSUINT32Property_Impl::Save( SvStream& rStream )
{
    rStream << aInt;
    return rStream.GetErrorCode();
}

//-------------------------------------------------------------------------

ULONG SfxPSUINT32Property_Impl::Len()
{
    return 4;
}

//=========================================================================

class SfxPSDateTimeProperty_Impl : public SfxPSProperty_Impl
{
private:
    DateTime aDateTime;

public:
    SfxPSDateTimeProperty_Impl( UINT32 nIdP, const DateTime& rDateTime ) :
        aDateTime( rDateTime ), SfxPSProperty_Impl( nIdP, VT_FILETIME ) {}
    SfxPSDateTimeProperty_Impl( UINT32 nIdP ) :
        SfxPSProperty_Impl( nIdP, VT_FILETIME ) {};

    virtual ULONG Save( SvStream& rStream );
    virtual ULONG Load( SvStream& rStream );
    virtual ULONG Len();

    const DateTime& GetDateTime() { return aDateTime; }
};

//-------------------------------------------------------------------------

ULONG SfxPSDateTimeProperty_Impl::Save(SvStream &rStream)
{
    // Nicht Valid ist das gleiche, wie bei MS, nur nicht konvertiert
    if( aDateTime.IsValid() )
        aDateTime.ConvertToUTC();
    BigInt a100nPerSecond(10000000L);
    BigInt a100nPerDay=a100nPerSecond*BigInt(60L*60*24);
    USHORT nYears=aDateTime.GetYear()-1601;
    long nDays=
        nYears*365+nYears/4-nYears/100+nYears/400+
            aDateTime.GetDayOfYear()-1;
    BigInt aTime=
        a100nPerDay*BigInt(nDays)+a100nPerSecond*
            BigInt((long)( aDateTime.GetSec() +
                   60* aDateTime.GetMin() +
                   60L*60* aDateTime.GetHour() ));

    BigInt aUlongMax((ULONG)ULONG_MAX);
    aUlongMax += 1;

    rStream<<(UINT32)(aTime % aUlongMax) ;
    rStream<<(UINT32)(aTime / aUlongMax);
    return rStream.GetErrorCode();
}

//-------------------------------------------------------------------------

ErrCode SfxPSDateTimeProperty_Impl::Load( SvStream& rStream )
{
    UINT32 nLow, nHigh;
    rStream >> nLow;
    rStream >> nHigh;
    BigInt aUlongMax( (ULONG)ULONG_MAX );
    aUlongMax += 1;
    BigInt aTime = aUlongMax * BigInt( nHigh );
    aTime += nLow;
    BigInt a100nPerSecond(10000000L);
    BigInt a100nPerDay = a100nPerSecond*BigInt( 60L * 60 * 24 );
    ULONG nDays = aTime / a100nPerDay;
    USHORT nYears = (USHORT)
        (( nDays - ( nDays / ( 4 * 365 ) ) + ( nDays / ( 100 * 365 ) ) -
          ( nDays / ( 400 * 365 ) ) ) / 365 );
    nDays -= nYears * 365 + nYears / 4 - nYears / 100 + nYears / 400;
    USHORT nMonths = 0;
    for( long nDaysCount = nDays; nDaysCount >= 0; )
    {
        nDays = nDaysCount;
        nMonths ++;
        nDaysCount-= Date(  1, nMonths, 1601 + nYears ).GetDaysInMonth();
    }
    Date _aDate( (USHORT)( nDays + 1 ), nMonths, nYears + 1601 );
    Time _aTime( ( aTime / ( a100nPerSecond * BigInt( 60 * 60 ) ) ) %
                 BigInt( 24 ),
                 ( aTime / ( a100nPerSecond * BigInt( 60 ) ) ) %
                 BigInt( 60 ),
                 ( aTime / ( a100nPerSecond ) ) %
                 BigInt( 60 ) );
    aDateTime = DateTime( _aDate, _aTime );
    aDateTime.ConvertToLocalTime();
    return rStream.GetErrorCode();
}

//-------------------------------------------------------------------------

ULONG SfxPSDateTimeProperty_Impl::Len()
{
    return 8;
}

//=========================================================================

typedef SfxPSProperty_Impl *SfxPSPropertyPtr_Impl;
SV_DECL_PTRARR_DEL(SfxPSPropertyArr_Impl, SfxPSPropertyPtr_Impl, 10, 10);
SV_IMPL_PTRARR(SfxPSPropertyArr_Impl, SfxPSPropertyPtr_Impl);

struct SfxPSSection_Impl
{
    SvGlobalName aId;
    SfxPSPropertyArr_Impl aProperties;
    ULONG Save(SvStream &rStream);
    ULONG Load( SvStream& rStream );
};

//=========================================================================

class SfxPS_Impl
{
    SfxPSSection_Impl aSection;
    USHORT GetPos( UINT32 nId );

  public:

    void   SetSectionName(const SvGlobalName& aIdP);
    void   AddProperty( SfxPSProperty_Impl* pProp);
    SfxPSProperty_Impl* GetProperty( UINT32 nId );

    ULONG Save(SvStream &rStream);
    ULONG Load( SvStream& rStream );
};


USHORT SfxPS_Impl::GetPos( UINT32 nId )
{
    SfxPSPropertyArr_Impl& rProperties = aSection.aProperties;
    USHORT nCount = rProperties.Count();
    for( USHORT n = 0 ; n < nCount; n++ )
        if( rProperties.GetObject( n )->GetId() == nId )
            return n;
    return USHRT_MAX;
}


void  SfxPS_Impl::AddProperty( SfxPSProperty_Impl* pProp)
{
    USHORT nPos = GetPos( pProp->GetId() );
    if( nPos != USHRT_MAX )
    {
        delete aSection.aProperties[ nPos ];
        aSection.aProperties.Remove( nPos );
    }
    aSection.aProperties.Insert(pProp,0);
}

//-------------------------------------------------------------------------

void SfxPS_Impl::SetSectionName(const SvGlobalName& aIdP)
{
    aSection.aId = aIdP;
}


ULONG SfxPS_Impl::Save(SvStream &rStream)
{
    SvGlobalName aName;
    rStream << (UINT16) 0xfffe // ByteOrder
        << (UINT16) 0          // version
        << (UINT16) 1          // Os MinorVersion
        << (UINT16)            // Os Type
#if defined(MAC)
        1
#elif defined(WNT)
        2
#else
        0
#endif
        << aName
        << (UINT32)1; // Immer eine Section
    return aSection.Save(rStream);
}


ULONG SfxPS_Impl::Load( SvStream& rStream )
{
    SvGlobalName aName;
    UINT16 nByteOrder;
    UINT16 nVersion;
    UINT16 nOsMinor;
    UINT16 nOsType;
    UINT32 nSections;

    rStream >> nByteOrder >> nVersion >> nOsMinor >> nOsType >> aName >> nSections;

    if( nSections != 1 )
    {
        DBG_WARNINGFILE( "DocInfo contains more than one section" );
        return ERRCODE_IO_GENERAL;
    }
    SetSectionName( aName );

/*    if ( nOsMinor == 5 )
        rStream.SetStreamCharSet( RTL_TEXTENCODING_UTF8 );*/

    return aSection.Load( rStream );
}

ULONG SfxPSSection_Impl::Load( SvStream& rStream )
{
//Nur eine Section laden: ( Use of more than 1 section is discouraged
//and will not be supported in future windows apis).

    UINT16 nCodePage = 0;

    UINT32 nPos;
    rStream >> aId;
    rStream >> nPos;

    rStream.Seek( nPos ); // SectionHeader
    UINT32 nLen;
    UINT32 nCount;
    rStream >> nLen;
    rStream >> nCount;
    UINT32 *pKeyIds = new UINT32[ nCount ];
    UINT32 *pPositions = new UINT32[ nCount ];
    USHORT n;
    ULONG nErr = 0;
    for( n = 0; n < nCount && !rStream.GetErrorCode(); n++ )
    {
        rStream >> pKeyIds[ n ];
        rStream >> pPositions[ n ];
    }
    for( n = 0; n < nCount && !nErr; n++ )
    {
        rStream.Seek( nPos + pPositions[ n ] );
        UINT32 nId;
        rStream >> nId;
        SfxPSProperty_Impl* pProp = 0;
        switch( nId )
        {
            case VT_LPSTR:
            {
                pProp = new SfxPSStringProperty_Impl( pKeyIds[ n ] );
                ((SfxPSStringProperty_Impl*)pProp)->SetCodePage( nCodePage );
                break;
            }
            case VT_FILETIME:
            {
                pProp = new SfxPSDateTimeProperty_Impl( pKeyIds[ n ] );
                break;
            }
            case VT_LPWSTR:
            {
                pProp = new SfxPSStringProperty_Impl( pKeyIds[ n ] );
                ((SfxPSStringProperty_Impl*)pProp)->SetIsUniCode();
                break;
            }
            case VT_I2:
            {
                if( pKeyIds[ n ] == 1 )
                {
                    rStream >> nCodePage;
                }
                break;
            }
        }

        if( pProp )
        {
            nErr = pProp->Load( rStream );
            aProperties.Insert( pProp, 0 );
        }
    }
    delete[] pKeyIds;
    delete[] pPositions;
    return nErr;
}

//-------------------------------------------------------------------------

ULONG SfxPSSection_Impl::Save(SvStream &rStream)
{

//Nur eine Section schreiben: ( Use of more than 1 section is discouraged
//and will not be supported in future windows apis).

    ULONG nPos = rStream.Tell();
    nPos += 20;
    rStream << aId << (UINT32)nPos; //Offset


    //SectionHeader Schreiben
    ULONG nLen=8;
    USHORT n;
    for(n=0;n<aProperties.Count();n++)
        nLen+=(((aProperties[n]->Len() + 3) >> 2) << 2)+12;
    rStream << (UINT32) nLen << (UINT32) aProperties.Count();

    //PropertyId/Offsetpaare schreiben

    nLen=8+aProperties.Count()*8;
    for(n=0;n<aProperties.Count();n++)
    {
        rStream << aProperties[n]->GetId() << (UINT32)nLen;
        nLen+=(((aProperties[n]->Len() + 3) >> 2) << 2) + 4;
    }

    //Inhalte schreiben
    for(n=0;n<aProperties.Count();n++)
    {
        rStream << (UINT32) aProperties[n]->GetType();
        aProperties[n]->Save(rStream);
        nLen=aProperties[n]->Len();
        while(nLen++%4) //Auffuellen auf DWORD Grenze
            rStream << (UINT8) 0;
    }

    return rStream.GetErrorCode();
}


SfxPSProperty_Impl* SfxPS_Impl::GetProperty( UINT32 nId )
{
    USHORT nPos = GetPos( nId );
    return nPos == USHRT_MAX ? 0 : aSection.aProperties.GetObject( nPos );
}

//========================================================================


SvStream& PaddWithBlanks_Impl(SvStream &rStream, USHORT nCount)
{
    for ( USHORT n = nCount; n; --n )
        rStream << ' ';
    return rStream;
}

//-------------------------------------------------------------------------

inline SvStream& Skip(SvStream &rStream, USHORT nCount)
{
    rStream.SeekRel(nCount);
    return rStream;
}

//-------------------------------------------------------------------------

struct FileHeader
{
    String      aHeader;
    USHORT      nVersion;
    BOOL        bPasswd;

    FileHeader( const char* pHeader, USHORT nV, BOOL bPass ) :
        nVersion( nV ), bPasswd( bPass ) { aHeader = String::CreateFromAscii( pHeader ); }
    FileHeader( SvStream& rStream );

    void    Save( SvStream& rStream ) const;
};

//-------------------------------------------------------------------------


FileHeader::FileHeader( SvStream& rStream )
{
    BYTE b;
    long nVer = rStream.GetVersion();
    rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
    rStream.ReadByteString( aHeader ); //(dv??)
    rStream >> nVersion >> b;
    rStream.SetVersion( nVer );
    bPasswd = (BOOL)b;
}

//-------------------------------------------------------------------------

void FileHeader::Save( SvStream& rStream ) const
{
    long nVer = rStream.GetVersion();
    rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
    rStream.WriteByteString( aHeader );
    rStream.SetVersion( nVer );
    rStream << nVersion;
    rStream << (BYTE)bPasswd;
}

//-------------------------------------------------------------------------

void SfxDocUserKey::AdjustTitle_Impl()
{
    if ( aTitle.Len() > SFXDOCUSERKEY_LENMAX )
        aTitle.Erase( SFXDOCUSERKEY_LENMAX );
}

//-------------------------------------------------------------------------

BOOL SfxDocUserKey::Load(SvStream &rStream)
{
    rStream.ReadByteString( aTitle );
    Skip(rStream, SFXDOCUSERKEY_LENMAX - aTitle.Len());
    rStream.ReadByteString( aWord );
    Skip(rStream, SFXDOCUSERKEY_LENMAX - aWord.Len());
    return rStream.GetError() == SVSTREAM_OK;
}


//-------------------------------------------------------------------------

BOOL SfxDocUserKey::Save(SvStream &rStream) const
{
    DBG_ASSERT( aTitle.Len() <= SFXDOCUSERKEY_LENMAX, "length of user key title overflow" );
    DBG_ASSERT( aWord.Len() <= SFXDOCUSERKEY_LENMAX, "length of user key value overflow" );

    // save the title of the user key
    String aString = aTitle;
    aString.Erase( SFXDOCUSERKEY_LENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCUSERKEY_LENMAX - aString.Len());

    // save the value of the user key
    aString = aWord;
    aString.Erase( SFXDOCUSERKEY_LENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCUSERKEY_LENMAX - aString.Len());

    return rStream.GetError() == SVSTREAM_OK;
}

//-------------------------------------------------------------------------
SfxDocUserKey::SfxDocUserKey( const String& rTitle, const String& rWord ) :
        aTitle( rTitle ), aWord( rWord )
{
    //!AdjustTitle_Impl();
}
//------------------------------------------------------------------------
const SfxDocUserKey& SfxDocUserKey::operator=(const SfxDocUserKey &rCopy)
{
    aTitle = rCopy.aTitle;
    aWord = rCopy.aWord;
    //!AdjustTitle_Impl();
    return *this;
}
// SfxDocumentInfo -------------------------------------------------------

sal_Bool TestValidity_Impl( const String& rString, sal_Bool bURL )
{
    sal_Bool bRet = sal_True;
    xub_StrLen nLen = rString.Len();
    if ( nLen >= 1024 &&
         ( !bURL || INetURLObject::CompareProtocolScheme( rString ) == INET_PROT_NOT_VALID ) )
        // !bURL == the default target has not so many characters
        // bURL ==  the reload url must have a valid protocol
        bRet = sal_False;

    return bRet;
}

BOOL SfxDocumentInfo::Load( SvStream& rStream )
{
    long d, t;
    USHORT nUS;
    BYTE nByte;
    FileHeader aHeader(rStream);
    if( ! aHeader.aHeader.EqualsAscii( pDocInfoHeader ))
    {
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
        return FALSE;
    }
    Free();
    bPasswd = aHeader.bPasswd;
    rStream >> nUS;
    //eFileCharSet = (CharSet)nUS;
    eFileCharSet = GetSOLoadTextEncoding( nUS );

        // Einstellen an den Streams
    rStream.SetStreamCharSet(eFileCharSet);

    rStream >> nByte;
    bPortableGraphics = nByte? 1: 0;
    rStream >> nByte;
    bQueryTemplate = nByte? 1: 0;

    aCreated.Load(rStream);
    aChanged.Load(rStream);
    aPrinted.Load(rStream);

    rStream.ReadByteString( aTitle );
    Skip(rStream, SFXDOCINFO_TITLELENMAX - aTitle.Len());
    rStream.ReadByteString( aTheme );
    Skip(rStream, SFXDOCINFO_THEMELENMAX - aTheme.Len());
    rStream.ReadByteString( aComment );
    Skip(rStream, SFXDOCINFO_COMMENTLENMAX- aComment.Len());
    rStream.ReadByteString( aKeywords );
    Skip(rStream, SFXDOCINFO_KEYWORDLENMAX - aKeywords.Len());

    USHORT i;
    for(i = 0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i].Load(rStream);

    rStream.ReadByteString( aTemplateName );
    rStream.ReadByteString( aTemplateFileName );
    rStream >> d >> t;
    aTemplateDate = DateTime(Date(d), Time(t));

    // wurde mal fuer MB in Panik eingebaut und dann doch nie benutzt :-)
    if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
    {
        USHORT nMailAddr;
        rStream >> nMailAddr;
        for( i = 0; i < nMailAddr; i++ )
        {
            String aDummyString;
            USHORT nDummyFlags;
            rStream.ReadByteString( aDummyString );
            rStream >> nDummyFlags;
        }
    }

    rStream >> lTime;
    if(aHeader.nVersion > 4)
        rStream >> nDocNo;
    else
        nDocNo = 1;
    rStream >> nUserDataSize;
    if(nUserDataSize) {
        pUserData = new char[nUserDataSize];
        rStream.Read(pUserData,nUserDataSize);
    }

    BOOL bOK = (rStream.GetError() == SVSTREAM_OK);
    nByte = 0;                          // wg.Kompatibilitaet;
    rStream >> nByte;                   // evtl. nicht in DocInfo enthalten
    bTemplateConfig = nByte ? 1 : 0;
    if( aHeader.nVersion > 5 )
    {
        rStream >> bReloadEnabled;
        rStream.ReadByteString( aReloadURL );
        rStream >> nReloadSecs;
        rStream.ReadByteString( aDefaultTarget );

        if ( !TestValidity_Impl( aReloadURL, sal_True ) )
        {
            // the reload url is invalid -> reset all reload attributes
            bReloadEnabled = FALSE;
            aReloadURL.Erase();
            nReloadSecs = 60;
            aDefaultTarget.Erase();
        }
        else if ( !TestValidity_Impl( aDefaultTarget, sal_False ) )
            // the default target is invalid -> reset it
            aDefaultTarget.Erase();
    }
    if ( aHeader.nVersion > 6 )
    {
        rStream >> nByte;
        bSaveGraphicsCompressed = nByte? 1: 0;
    }
    if ( aHeader.nVersion > 7 )
    {
        rStream >> nByte;
        bSaveOriginalGraphics = nByte? 1: 0;
    }
    if ( aHeader.nVersion > 8 )
    {
        rStream >> nByte;
        bSaveVersionOnClose = nByte? 1: 0;

        rStream.ReadByteString( pImp->aCopiesTo );
        rStream.ReadByteString( pImp->aOriginal );
        rStream.ReadByteString( pImp->aReferences );
        rStream.ReadByteString( pImp->aRecipient );
        rStream.ReadByteString( pImp->aReplyTo );
        rStream.ReadByteString( pImp->aBlindCopies );
        rStream.ReadByteString( pImp->aInReplyTo );
        rStream.ReadByteString( pImp->aNewsgroups );
        rStream >> pImp->nPriority;
    }
    if ( aHeader.nVersion > 9 )
    {
        rStream.ReadByteString( pImp->aSpecialMimeType );
    }
    if ( aHeader.nVersion > 10 )
    {
        rStream >> nByte;
        pImp->bUseUserData = nByte ? TRUE : FALSE;
    }
    return bOK;
}

#pragma optimize ( "", off )

ULONG SfxDocumentInfo::LoadPropertySet( SvStorage* pStorage )
{
    SvStorageStreamRef aStrPropSet = pStorage->OpenStream(
        String::CreateFromAscii( pPropSlot ), STREAM_STD_READ );
    if ( !aStrPropSet.Is() )
        return ERRCODE_IO_ACCESSDENIED;
    aStrPropSet->SetBufferSize( STREAM_BUFFER_SIZE );
    SfxPS_Impl* pPS = new SfxPS_Impl;
    ULONG nErr = pPS->Load( *aStrPropSet );

    UINT32 aStrArr[] =  { PID_TITLE, PID_SUBJECT, PID_KEYWORDS, PID_TEMPLATE, PID_COMMENTS, 0 };
    UINT32 aLens[] =    { SFXDOCINFO_TITLELENMAX, SFXDOCINFO_THEMELENMAX,
                          SFXDOCINFO_KEYWORDLENMAX, USHRT_MAX, SFXDOCINFO_COMMENTLENMAX };

#if ( __GNUC__ == 2 ) && ( __GNUC_MINOR__ == 95 )
    void ( SfxDocumentInfo::*pStrFuncs[] )( String& ) =
#else
    void ( SfxDocumentInfo::*pStrFuncs[] )( const String& ) =
#endif
    {
        &SfxDocumentInfo::SetTitle,
        &SfxDocumentInfo::SetTheme,
        &SfxDocumentInfo::SetKeywords,
        &SfxDocumentInfo::SetTemplateName,
        &SfxDocumentInfo::SetComment
    };

    for( USHORT n = 0; aStrArr[ n ]; n++ )
    {
        SfxPSStringProperty_Impl* pStr = ( SfxPSStringProperty_Impl* )pPS->GetProperty( aStrArr[ n ] );
        if( pStr )
        {
            USHORT nLen = (USHORT)aLens[ n ];
            ( this->*pStrFuncs[ n ] )( pStr->GetString().Copy( 0, nLen ) );
        }
    }

    String aName;
    DateTime aTime;
    SfxPSStringProperty_Impl* pStr;
    SfxPSDateTimeProperty_Impl* pDate;

    pStr = (SfxPSStringProperty_Impl*)  pPS->GetProperty( PID_AUTHOR );
    if( pStr ) aName = pStr->GetString();
    else aName.Erase();
    pDate = (SfxPSDateTimeProperty_Impl*) pPS->GetProperty( PID_CREATE_DTM );
    if( pDate ) aTime = pDate->GetDateTime();
    else aTime = DateTime();
    SetCreated( SfxStamp( aName.Copy(0, TIMESTAMP_MAXLENGTH ), aTime ) );

    pStr = (SfxPSStringProperty_Impl*)  pPS->GetProperty( PID_LASTAUTHOR );
    if( pStr ) aName = pStr->GetString();
    else aName.Erase();
    pDate = (SfxPSDateTimeProperty_Impl*) pPS->GetProperty( PID_LASTSAVED_DTM );
    if( pDate ) aTime = pDate->GetDateTime();
    else aTime = DateTime();
    SetChanged( SfxStamp( aName.Copy(0, TIMESTAMP_MAXLENGTH ), aTime ) );

    pDate = (SfxPSDateTimeProperty_Impl*) pPS->GetProperty( PID_LASTPRINTED_DTM );
    if( pDate ) aTime = pDate->GetDateTime();
    else aTime = DateTime();
    DateTime aTmpTime = aTime;
    aTmpTime.ConvertToUTC();
    if( aTmpTime != DateTime( Date( 1, 1, 1601 ), Time( 0, 0, 0 ) ) )
        SetPrinted( SfxStamp( String(), aTime ) );
    else
        SetPrinted( SfxStamp( TIMESTAMP_INVALID_DATETIME ));

    pStr = (SfxPSStringProperty_Impl*) pPS->GetProperty( PID_REVNUMBER );
    if( pStr )
        SetDocumentNumber( (USHORT)pStr->GetString().ToInt32() );

    pDate = (SfxPSDateTimeProperty_Impl*) pPS->GetProperty( PID_EDITTIME );
    if( pDate )
    {
        DateTime aDateTime = pDate->GetDateTime();
        aDateTime.ConvertToUTC();
        SetTime( aDateTime.GetTime() );
    }

    delete pPS;
    return ERRCODE_NONE;
}

#pragma optimize ( "", on )

//-------------------------------------------------------------------------
BOOL SfxDocumentInfo::SavePropertySet( SvStorage *pStorage) const
{
    SfxPS_Impl* pPS = new SfxPS_Impl;
    SvStorageStreamRef aStrPropSet = pStorage->OpenStream(
        String::CreateFromAscii( pPropSlot ), STREAM_TRUNC | STREAM_STD_WRITE );
    if ( !aStrPropSet.Is() )
    {
        DBG_ERRORFILE( "can not open the property set" );
        return FALSE;
    }

    pPS->SetSectionName( SvGlobalName(
        0xf29f85e0, 0x4ff9, 0x1068, 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_TITLE, GetTitle() ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_SUBJECT, GetTheme() ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_KEYWORDS, GetKeywords() ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_TEMPLATE, GetTemplateName() ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_COMMENTS, GetComment() ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_AUTHOR, GetCreated().GetName() ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl( PID_LASTAUTHOR, GetChanged().GetName() ) );
    pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_CREATE_DTM, GetCreated().GetTime() ) );
    pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_LASTSAVED_DTM, GetChanged().GetTime() ) );
    if ( GetPrinted().GetTime() != GetCreated().GetTime() )
        pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_LASTPRINTED_DTM, GetPrinted().GetTime() ) );
    DateTime aEditTime( Date( 1, 1, 1601 ), Time( GetTime() ) );
    aEditTime.ConvertToLocalTime();
    pPS->AddProperty( new SfxPSDateTimeProperty_Impl( PID_EDITTIME, aEditTime ) );
    pPS->AddProperty( new SfxPSStringProperty_Impl(
        PID_REVNUMBER, String::CreateFromInt32( GetDocumentNumber() ) ) );
    pPS->AddProperty( new SfxPSCodePageProperty_Impl( RTL_TEXTENCODING_UTF8 ));
    pPS->Save( *aStrPropSet );
    delete pPS;
    return ( aStrPropSet->GetErrorCode() == 0 );
}

//-------------------------------------------------------------------------

BOOL SfxDocumentInfo::Save( SvStream& rStream ) const
{
    FileHeader aHeader(pDocInfoHeader, VERSION, bPasswd? 1: 0);
    aHeader.Save(rStream);
    CharSet eNewFileCharSet = GetSOStoreTextEncoding( eFileCharSet );
    rStream << (USHORT)eNewFileCharSet;
    rStream.SetStreamCharSet(eNewFileCharSet);
    rStream << (bPortableGraphics? (BYTE)1: (BYTE)0)
            << (bQueryTemplate? (BYTE)1: (BYTE)0);
    aCreated.Save(rStream);
    aChanged.Save(rStream);
    aPrinted.Save(rStream);

    DBG_ASSERT( aTitle.Len() <= SFXDOCINFO_TITLELENMAX , "length of title overflow" );
    DBG_ASSERT( aTheme.Len() <= SFXDOCINFO_THEMELENMAX , "length of theme overflow" );
    DBG_ASSERT( aComment.Len() <= SFXDOCINFO_COMMENTLENMAX , "length of description overflow" );
    DBG_ASSERT( aKeywords.Len() <= SFXDOCINFO_KEYWORDLENMAX , "length of keywords overflow" );

    // save the title
    String aString = aTitle;
    aString.Erase( SFXDOCINFO_TITLELENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_TITLELENMAX - aString.Len());
    // save the theme
    aString = aTheme;
    aString.Erase( SFXDOCINFO_THEMELENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_THEMELENMAX - aString.Len());
    // save the description
    aString = aComment;
    aString.Erase( SFXDOCINFO_COMMENTLENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_COMMENTLENMAX - aString.Len());
    // save the keywords
    aString = aKeywords;
    aString.Erase( SFXDOCINFO_KEYWORDLENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_KEYWORDLENMAX - aString.Len());

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i].Save(rStream);
    rStream.WriteByteString( aTemplateName );
    rStream.WriteByteString( aTemplateFileName );
    rStream << (long)aTemplateDate.GetDate()
            << (long)aTemplateDate.GetTime();

    // wurde mal fuer MB in Panik eingebaut und dann doch nie benutzt :-)
    if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
        rStream << (USHORT) 0;

    rStream << GetTime() << GetDocumentNumber();

    rStream << nUserDataSize;
    if(pUserData)
        rStream.Write(pUserData, nUserDataSize);
    rStream << (bTemplateConfig? (BYTE)1: (BYTE)0);
    if( aHeader.nVersion > 5 )
    {
        rStream << bReloadEnabled;
        rStream.WriteByteString( aReloadURL );
        rStream << nReloadSecs;
        rStream.WriteByteString( aDefaultTarget );
    }
    if ( aHeader.nVersion > 6 )
        rStream << (bSaveGraphicsCompressed? (BYTE)1: (BYTE)0);
    if ( aHeader.nVersion > 7 )
        rStream << (bSaveOriginalGraphics? (BYTE)1: (BYTE)0);
    if ( aHeader.nVersion > 8 )
    {
        rStream << (bSaveVersionOnClose? (BYTE)1: (BYTE)0);
        rStream.WriteByteString( pImp->aCopiesTo );
        rStream.WriteByteString( pImp->aOriginal );
        rStream.WriteByteString( pImp->aReferences );
        rStream.WriteByteString( pImp->aRecipient );
        rStream.WriteByteString( pImp->aReplyTo );
        rStream.WriteByteString( pImp->aBlindCopies );
        rStream.WriteByteString( pImp->aInReplyTo );
        rStream.WriteByteString( pImp->aNewsgroups );
        rStream << pImp->nPriority;
    }
    if ( aHeader.nVersion > 9 )
    {
        rStream.WriteByteString( pImp->aSpecialMimeType );
    }
    if ( aHeader.nVersion > 10 )
    {
        rStream << ( pImp->bUseUserData ? (BYTE)1: (BYTE)0 );
    }

    return rStream.GetError() == SVSTREAM_OK;
}

//-------------------------------------------------------------------------

BOOL SfxDocumentInfo::Load(SvStorage* pStorage)
{
#ifdef DBG_UTIL
    if(!pStorage->IsStream( String::CreateFromAscii( pDocInfoSlot )))
        return FALSE;
#endif
    if ( pStorage->GetVersion() >= SOFFICE_FILEFORMAT_60 )
    {
        DBG_ERROR("This method only supports binary file format, use service StandaloneDocumentInfo!");
        return FALSE;
    }

    SvStorageStreamRef aStr = pStorage->OpenStream( String::CreateFromAscii( pDocInfoSlot ),STREAM_STD_READ);
    if(!aStr.Is())
        return FALSE;
    aStr->SetVersion( pStorage->GetVersion() );
    aStr->SetBufferSize(STREAM_BUFFER_SIZE);
    return Load(*aStr);
}

//-------------------------------------------------------------------------

BOOL SfxDocumentInfo::Save(SvStorage* pStorage) const
{
    SvStorageStreamRef aStr = pStorage->OpenStream( String::CreateFromAscii( pDocInfoSlot ), STREAM_TRUNC | STREAM_STD_READWRITE);
    if(!aStr.Is())
        return FALSE;
    aStr->SetVersion( pStorage->GetVersion() );
    aStr->SetBufferSize(STREAM_BUFFER_SIZE);
    if(!Save(*aStr))
        return FALSE;
    return SavePropertySet( pStorage );
}

//-------------------------------------------------------------------------

const SfxDocUserKey& SfxDocumentInfo::GetUserKey( USHORT n ) const
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    return aUserKeys[n];
}

//-------------------------------------------------------------------------

void SfxDocumentInfo::SetUserKey( const SfxDocUserKey& rKey, USHORT n )
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    aUserKeys[n] = rKey;
}


//-------------------------------------------------------------------------

SfxDocumentInfo::SfxDocumentInfo( const SfxDocumentInfo& rInf ):
    nUserDataSize(0),
    pUserData(0)
{
    pImp = new SfxDocumentInfo_Impl;
    bReadOnly = rInf.bReadOnly;
    bReloadEnabled = FALSE;
    nReloadSecs = 60;
    *this = rInf;
}

BOOL SfxDocumentInfo::IsReloadEnabled() const
{
    return bReloadEnabled;
}

void SfxDocumentInfo::EnableReload( BOOL bEnable )
{
    bReloadEnabled = bEnable;
}

const String& SfxDocumentInfo::GetDefaultTarget() const
{
    return aDefaultTarget;
}

void SfxDocumentInfo::SetDefaultTarget( const String& rString )
{
    aDefaultTarget = rString;
}

const String& SfxDocumentInfo::GetReloadURL() const
{
    return aReloadURL;
}

void SfxDocumentInfo::SetReloadURL( const String& rString )
{
    aReloadURL = rString;
}

ULONG SfxDocumentInfo::GetReloadDelay() const
{
    return nReloadSecs;
}

void SfxDocumentInfo::SetReloadDelay( ULONG nSec )
{
    nReloadSecs = nSec;
}

//-------------------------------------------------------------------------
void SfxDocumentInfo::Clear()
{
    BOOL _bReadOnly           = bReadOnly;
    BOOL _bPortableGraphics   = bPortableGraphics;
    BOOL _bSaveGraphicsCompressed = bSaveGraphicsCompressed;
    BOOL _bSaveOriginalGraphics = bSaveOriginalGraphics;

    (*this) = SfxDocumentInfo();

    bReadOnly           = _bReadOnly;
    bPortableGraphics   = _bPortableGraphics;
    bSaveGraphicsCompressed = _bSaveGraphicsCompressed;
    bSaveOriginalGraphics = _bSaveOriginalGraphics;
}

const SfxDocumentInfo& SfxDocumentInfo::operator=( const SfxDocumentInfo& rInf)
{
    if( this == &rInf ) return *this;

    bReadOnly           = rInf.bReadOnly;
    bPasswd             = rInf.bPasswd;
    bPortableGraphics   = rInf.bPortableGraphics;
    bSaveGraphicsCompressed = rInf.bSaveGraphicsCompressed;
    bSaveOriginalGraphics = rInf.bSaveOriginalGraphics;
    bQueryTemplate      = rInf.bQueryTemplate;
    bTemplateConfig     = rInf.bTemplateConfig;
    eFileCharSet        = rInf.eFileCharSet;

    aCreated  = rInf.aCreated;
    aChanged  = rInf.aChanged;
    aPrinted  = rInf.aPrinted;
    aTitle    = rInf.aTitle;
    aTheme    = rInf.aTheme;
    aComment  = rInf.aComment;
    aKeywords = rInf.aKeywords;

    for(USHORT i=0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i] = rInf.aUserKeys[i];

    aTemplateName = rInf.aTemplateName;
    aTemplateFileName = rInf.aTemplateFileName;
    aTemplateDate = rInf.aTemplateDate;

    aDefaultTarget = rInf.GetDefaultTarget();
    aReloadURL = rInf.GetReloadURL();
    bReloadEnabled = rInf.IsReloadEnabled();
    nReloadSecs = rInf.GetReloadDelay();

    Free();

    nUserDataSize = rInf.nUserDataSize;
    if(nUserDataSize) {
        pUserData = new char[nUserDataSize];
        memcpy(pUserData, rInf.pUserData, nUserDataSize);
    }
    lTime = rInf.lTime;
    nDocNo = rInf.nDocNo;

    bSaveVersionOnClose     = rInf.bSaveVersionOnClose;
    pImp->aCopiesTo         = rInf.pImp->aCopiesTo;
    pImp->aOriginal         = rInf.pImp->aOriginal;
    pImp->aReferences       = rInf.pImp->aReferences;
    pImp->aRecipient        = rInf.pImp->aRecipient;
    pImp->aReplyTo          = rInf.pImp->aReplyTo;
    pImp->aBlindCopies      = rInf.pImp->aBlindCopies;
    pImp->aInReplyTo        = rInf.pImp->aInReplyTo;
    pImp->aNewsgroups       = rInf.pImp->aNewsgroups;
    pImp->aSpecialMimeType  = rInf.pImp->aSpecialMimeType;
    pImp->nPriority         = rInf.pImp->nPriority;
    pImp->bUseUserData      = rInf.pImp->bUseUserData;

    return *this;
}

//-------------------------------------------------------------------------

int SfxDocumentInfo::operator==( const SfxDocumentInfo& rCmp) const
{
    if(eFileCharSet != rCmp.eFileCharSet ||
        bPasswd != rCmp.bPasswd ||
        bPortableGraphics != rCmp.bPortableGraphics ||
        bSaveGraphicsCompressed != rCmp.bSaveGraphicsCompressed ||
        bSaveOriginalGraphics != rCmp.bSaveOriginalGraphics ||
        bQueryTemplate != rCmp.bQueryTemplate  ||
        bTemplateConfig != rCmp.bTemplateConfig  ||
        aCreated != rCmp.aCreated ||
        aChanged != rCmp.aChanged ||
        aPrinted != rCmp.aPrinted ||
        aTitle != rCmp.aTitle ||
        aTheme != rCmp.aTheme ||
        aComment != rCmp.aComment ||
        aKeywords != rCmp.aKeywords ||
        aTemplateName != rCmp.aTemplateName ||
        aTemplateDate != rCmp.aTemplateDate ||
       IsReloadEnabled() != rCmp.IsReloadEnabled() ||
       GetReloadURL() != rCmp.GetReloadURL() ||
       GetReloadDelay() != rCmp.GetReloadDelay() ||
       GetDefaultTarget() != rCmp.GetDefaultTarget())
        return FALSE;

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i) {
        if(aUserKeys[i] != rCmp.aUserKeys[i])
            return FALSE;
    }
    if(nUserDataSize != rCmp.nUserDataSize)
        return FALSE;
    if(nUserDataSize)
        return 0 == memcmp(pUserData, rCmp.pUserData, nUserDataSize);

    if ( pImp->aCopiesTo        != rCmp.pImp->aCopiesTo      ||
         pImp->aOriginal        != rCmp.pImp->aOriginal      ||
         pImp->aReferences      != rCmp.pImp->aReferences    ||
         pImp->aRecipient       != rCmp.pImp->aRecipient     ||
         pImp->aReplyTo         != rCmp.pImp->aReplyTo       ||
         pImp->aBlindCopies     != rCmp.pImp->aBlindCopies   ||
         pImp->aInReplyTo       != rCmp.pImp->aInReplyTo     ||
         pImp->aNewsgroups      != rCmp.pImp->aNewsgroups    ||
         pImp->aSpecialMimeType != rCmp.pImp->aSpecialMimeType ||
         pImp->nPriority        != rCmp.pImp->nPriority      ||
         pImp->bUseUserData     != rCmp.pImp->bUseUserData   ||
         bSaveVersionOnClose    != rCmp.bSaveVersionOnClose )
        return FALSE;

    return TRUE;
}

//-------------------------------------------------------------------------

const SfxDocumentInfo &SfxDocumentInfo::CopyUserData(const SfxDocumentInfo &rSource)
{
    bQueryTemplate = rSource.bQueryTemplate;
    bTemplateConfig = rSource.bTemplateConfig;

    SetReloadDelay( rSource.GetReloadDelay() );
    SetReloadURL( rSource.GetReloadURL() );
    EnableReload( rSource.IsReloadEnabled() );
    SetDefaultTarget( rSource.GetDefaultTarget() );

    aTitle = rSource.aTitle;
    aTheme = rSource.aTheme;
    aComment = rSource.aComment;
    aKeywords = rSource.aKeywords;

    aTemplateName     = rSource.aTemplateName;
    aTemplateFileName = rSource.aTemplateFileName;
    aTemplateDate     = rSource.aTemplateDate;

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i] = rSource.aUserKeys[i];

    if(pUserData)
    {
        delete []pUserData;
        pUserData = 0;
    }
    nUserDataSize = rSource.nUserDataSize;
    if(nUserDataSize) {
        pUserData = new char[nUserDataSize];
        memcpy(pUserData, rSource.pUserData, nUserDataSize);
    }

    pImp->aCopiesTo         = rSource.pImp->aCopiesTo;
    pImp->aOriginal         = rSource.pImp->aOriginal;
    pImp->aReferences       = rSource.pImp->aReferences;
    pImp->aRecipient        = rSource.pImp->aRecipient;
    pImp->aReplyTo          = rSource.pImp->aReplyTo;
    pImp->aBlindCopies      = rSource.pImp->aBlindCopies;
    pImp->aInReplyTo        = rSource.pImp->aInReplyTo;
    pImp->aNewsgroups       = rSource.pImp->aNewsgroups;
    pImp->aSpecialMimeType  = rSource.pImp->aSpecialMimeType;
    pImp->nPriority         = rSource.pImp->nPriority;
    pImp->bUseUserData      = rSource.pImp->bUseUserData;

    return *this;
}

//-------------------------------------------------------------------------

void SfxDocumentInfo::Free()
{
    delete []pUserData;
    pUserData = 0;
    nUserDataSize = 0;
}

//-------------------------------------------------------------------------

String SfxDocumentInfo::AdjustTextLen_Impl( const String& rText, USHORT nMax )
{
    String aRet = rText;
/*! pb: dont cut any longer because the new file format has no length limit
    if ( aRet.Len() > nMax )
        aRet.Erase( nMax ); */
    return aRet;
}

//-------------------------------------------------------------------------

SfxDocumentInfo::SfxDocumentInfo() :
    bPasswd(FALSE),
    bQueryTemplate(FALSE),
    bTemplateConfig(FALSE),
    eFileCharSet(gsl_getSystemTextEncoding()),
    nUserDataSize(0),
    pUserData(0),
    lTime(0),
    nDocNo(1),
    aChanged( TIMESTAMP_INVALID_DATETIME ),
    aPrinted( TIMESTAMP_INVALID_DATETIME ),
    bSaveVersionOnClose( FALSE )
{
    pImp = new SfxDocumentInfo_Impl;

    bReadOnly = FALSE;
    bReloadEnabled = FALSE;
    nReloadSecs = 60;
    SfxApplication *pSfxApp = SFX_APP();
    bPortableGraphics = TRUE;
    SvtSaveOptions aSaveOptions;
    bSaveGraphicsCompressed = aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsCompressed;
    bSaveOriginalGraphics = aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsOriginal;

    const String aInf( DEFINE_CONST_UNICODE( "Info " ) );
    for( USHORT i = 0; i < MAXDOCUSERKEYS; ++i )    {
        aUserKeys[i].aTitle = aInf;
        aUserKeys[i].aTitle += String::CreateFromInt32(i+1);
    }
}

//-------------------------------------------------------------------------

SfxDocumentInfo::~SfxDocumentInfo()
{
    Free();
    delete pImp;
}

String SfxDocumentInfo::GetCopiesTo() const
{
    return pImp->aCopiesTo;
}

void SfxDocumentInfo::SetCopiesTo( const String& rStr )
{
    pImp->aCopiesTo = rStr;
}

String SfxDocumentInfo::GetOriginal() const
{
    return pImp->aOriginal;
}

void SfxDocumentInfo::SetOriginal( const String& rStr )
{
    pImp->aOriginal = rStr;
}

String SfxDocumentInfo::GetReferences() const
{
    return pImp->aReferences;
}

void SfxDocumentInfo::SetReferences( const String& rStr )
{
    pImp->aReferences = rStr;
}

String SfxDocumentInfo::GetRecipient() const
{
    return pImp->aRecipient;
}

void SfxDocumentInfo::SetRecipient( const String& rStr )
{
    pImp->aRecipient = rStr;
}

String SfxDocumentInfo::GetReplyTo() const
{
    return pImp->aReplyTo;
}

void SfxDocumentInfo::SetReplyTo( const String& rStr )
{
    pImp->aReplyTo = rStr;
}

String SfxDocumentInfo::GetBlindCopies() const
{
    return pImp->aBlindCopies;
}

void SfxDocumentInfo::SetBlindCopies( const String& rStr )
{
    pImp->aBlindCopies = rStr;
}

String SfxDocumentInfo::GetInReplyTo() const
{
    return pImp->aInReplyTo;
}

void SfxDocumentInfo::SetInReplyTo( const String& rStr )
{
    pImp->aInReplyTo = rStr;
}

String SfxDocumentInfo::GetNewsgroups() const
{
    return pImp->aNewsgroups;
}

void SfxDocumentInfo::SetNewsgroups( const String& rStr )
{
    pImp->aNewsgroups = rStr;
}

String SfxDocumentInfo::GetSpecialMimeType() const
{
    return pImp->aSpecialMimeType;
}

void SfxDocumentInfo::SetSpecialMimeType( const String& rStr )
{
    pImp->aSpecialMimeType = rStr;
}

USHORT SfxDocumentInfo::GetPriority() const
{
    return pImp->nPriority;
}

void SfxDocumentInfo::SetPriority( USHORT nPrio )
{
    pImp->nPriority = nPrio;
}

BOOL SfxDocumentInfo::IsUseUserData() const
{
    return pImp->bUseUserData;
}

void SfxDocumentInfo::SetUseUserData( BOOL bNew )
{
    pImp->bUseUserData = bNew;
}

//-----------------------------------------------------------------------------

ErrCode SfxDocumentInfo::Load(const String& rName)
{
    SfxMedium aMedium( rName, SFX_STREAM_READONLY_MAKECOPY, TRUE );
    if ( !aMedium.GetStorage() || SVSTREAM_OK != aMedium.GetError() )
        // Datei existiert nicht oder ist kein Storage
        return ERRCODE_IO_CANTREAD;

    // Filter-Detection wegen FileFormat-Version
    const SfxFilter *pFilter = 0;
    if ( 0 != SFX_APP()->GetFilterMatcher().GuessFilter( aMedium, &pFilter ) || !pFilter )
        // unbekanntes Dateiformat
        return ERRCODE_IO_CANTREAD;

    // Storage "offnen
    SvStorageRef xStor = aMedium.GetStorage();
    xStor->SetVersion( pFilter->GetVersion() );
    return Load( xStor ) ? ERRCODE_NONE : ERRCODE_IO_CANTREAD;
}

//------------------------------------------------------------------------

void SfxDocumentInfo::SetTitle( const String& rVal )
{
    aTitle = AdjustTextLen_Impl( rVal, SFXDOCINFO_TITLELENMAX );
}
//------------------------------------------------------------------------

void SfxDocumentInfo::SetTheme( const String& rVal )
{
    aTheme = AdjustTextLen_Impl( rVal, SFXDOCINFO_THEMELENMAX );
}
//------------------------------------------------------------------------

void SfxDocumentInfo::SetComment( const String& rVal )
{
    aComment = AdjustTextLen_Impl( rVal, SFXDOCINFO_COMMENTLENMAX );
}
//------------------------------------------------------------------------

void SfxDocumentInfo::SetKeywords( const String& rVal )
{
    aKeywords = AdjustTextLen_Impl( rVal, SFXDOCINFO_KEYWORDLENMAX );
}

void SfxDocumentInfo::DeleteUserData( BOOL bUseAuthor )
{
    SfxStamp aCreated;
    if ( bUseAuthor  )
        aCreated.SetName( SvtUserOptions().GetFullName() );
    SetCreated( aCreated );
    SfxStamp aInvalid( TIMESTAMP_INVALID_DATETIME );
    SetChanged( aInvalid );
    SetPrinted( aInvalid );
    SetTime( 0L );
    SetDocumentNumber( 1 );
}
