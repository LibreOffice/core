/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcemanager.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 15:41:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

#include "resourcemanager.hxx"

#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/solar.hrc>
#include <svtools/syslocale.hxx>


namespace XmlSec
{
    static ResMgr*          pResMgr = 0;
    static SvtSysLocale*    pSysLocale = 0;

    ResMgr* GetResMgr( void )
    {
        if( !pResMgr )
        {
            ByteString  aName( "xmlsec" );
//          pResMgr = ResMgr::CreateResMgr( aName.GetBuffer(), Application::GetSettings().GetUILanguage() );
//          LanguageType    aLang( LANGUAGE_ENGLISH_US );
//          pResMgr = ResMgr::CreateResMgr( aName.GetBuffer(), aLang );
// MT: Change to Locale
            pResMgr = ResMgr::CreateResMgr( aName.GetBuffer() );
        }

    return pResMgr;
    }

    const LocaleDataWrapper&    GetLocaleData( void )
    {
        if (!pSysLocale)
            pSysLocale = new SvtSysLocale;
        return pSysLocale->GetLocaleData();
    }

    DateTime GetDateTime( const ::com::sun::star::util::DateTime& _rDT )
    {
        return DateTime(
            Date( _rDT.Day, _rDT.Month, _rDT.Year ),
            Time( _rDT.Hours, _rDT.Minutes, _rDT.Seconds, _rDT.HundredthSeconds ) );
    }

    String GetDateTimeString( const ::com::sun::star::util::DateTime& _rDT )
    {
        // --> PB 2004-10-12 #i20172# String with date and time information
        DateTime aDT( GetDateTime( _rDT ) );
        const LocaleDataWrapper& rLoDa = GetLocaleData();
        String sRet( rLoDa.getDate( aDT ) );
        sRet += ' ';
        sRet += rLoDa.getTime( aDT );
        return sRet;
    }

    String GetDateTimeString( const rtl::OUString& _rDate, const rtl::OUString& _rTime )
    {
        String sDay( _rDate, 6, 2 );
        String sMonth( _rDate, 4, 2 );
        String sYear( _rDate, 0, 4 );

        String sHour( _rTime, 0, 2 );
        String sMin( _rTime, 4, 2 );
        String sSec( _rTime, 6, 2 );


        Date aDate( (USHORT)sDay.ToInt32(), (USHORT) sMonth.ToInt32(), (USHORT)sYear.ToInt32() );
        Time aTime( sHour.ToInt32(), sMin.ToInt32(), sSec.ToInt32(), 0 );
        const LocaleDataWrapper& rLoDa = GetLocaleData();
        String aStr( rLoDa.getDate( aDate ) );
        aStr.AppendAscii( " " );
        aStr += rLoDa.getTime( aTime );
        return aStr;
    }

    String GetDateString( const ::com::sun::star::util::DateTime& _rDT )
    {
        return GetLocaleData().getDate( GetDateTime( _rDT ) );
    }

    String GetPureContent( const String& _rRawString, const char* _pCommaReplacement, bool _bPreserveId )
    {
        enum STATE { PRE_ID, ID, EQUALSIGN, PRE_CONT, CONT };
        String      s;
        STATE       e = _bPreserveId? PRE_ID : ID;

        const sal_Unicode*  p = _rRawString.GetBuffer();
        sal_Unicode         c;
        const sal_Unicode   cComma = ',';
        const sal_Unicode   cEqualSign = '=';
        const sal_Unicode   cSpace = ' ';
        String              aCommaReplacement;
        if( _pCommaReplacement )
            aCommaReplacement = String::CreateFromAscii( _pCommaReplacement );

        while( *p )
        {
            c = *p;
            switch( e )
            {
                case PRE_ID:
                    if( c != cSpace )
                    {
                        s += c;
                        e = ID;
                    }
                    break;
                case ID:
                    if( _bPreserveId )
                        s += c;

                    if( c == cEqualSign )
                        e = _bPreserveId? PRE_CONT : CONT;
                    break;
                case EQUALSIGN:
                    break;
                case PRE_CONT:
                    if( c != cSpace )
                    {
                        s += c;
                        e = CONT;
                    }
                    break;
                case CONT:
                    if( c == cComma )
                    {
                        s += aCommaReplacement;
                        e = _bPreserveId? PRE_ID : ID;
                    }
                    else
                        s += c;
                    break;
            }

            ++p;
        }

//      xub_StrLen  nEquPos = _rRawString.SearchAscii( "=" );
//      if( nEquPos == STRING_NOTFOUND )
//          s = _rRawString;
//      else
//      {
//          ++nEquPos;
//          s = String( _rRawString, nEquPos, STRING_MAXLEN );
//          s.EraseLeadingAndTrailingChars();
//      }

        return s;
    }

    String GetContentPart( const String& _rRawString, const String& _rPartId )
    {
        String s;

        xub_StrLen nContStart = _rRawString.Search( _rPartId );
        if( nContStart != STRING_NOTFOUND )
        {
            nContStart = nContStart + _rPartId.Len();
            ++nContStart;                   // now it's start of content, directly after Id

            xub_StrLen  nContEnd = _rRawString.Search( sal_Unicode( ',' ), nContStart );

            s = String( _rRawString, nContStart, nContEnd - nContStart );
        }

        return s;
    }

    /**
     * This Method should consider some string like "C=CN-XXX , O=SUN-XXX , CN=Jack" ,
     * here the first CN represent china , and the second CN represent the common name ,
     * so I changed the method to handle this .
     * By CP , mailto : chandler.peng@sun.com
     **/
    String GetContentPart( const String& _rRawString )
    {
        // search over some parts to find a string
        //static char* aIDs[] = { "CN", "OU", "O", "E", NULL };
        static char const * aIDs[] = { "CN=", "OU=", "O=", "E=", NULL };// By CP
        String sPart;
        int i = 0;
        while ( aIDs[i] )
        {
            String sPartId = String::CreateFromAscii( aIDs[i++] );
            xub_StrLen nContStart = _rRawString.Search( sPartId );
            if ( nContStart != STRING_NOTFOUND )
            {
                nContStart = nContStart + sPartId.Len();
                //++nContStart;                   // now it's start of content, directly after Id // delete By CP
                xub_StrLen nContEnd = _rRawString.Search( sal_Unicode( ',' ), nContStart );
                sPart = String( _rRawString, nContStart, nContEnd - nContStart );
                break;
            }
        }

        return sPart;
    }

    String GetHexString( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rSeq, const char* _pSep, UINT16 _nLineBreak )
    {
        const sal_Int8*         pSerNumSeq = _rSeq.getConstArray();
        int                     nCnt = _rSeq.getLength();
        String                  aStr;
        const char              pHexDigs[ 17 ] = "0123456789ABCDEF";
        char                    pBuffer[ 3 ] = "  ";
        UINT8                   nNum;
        UINT16                  nBreakStart = _nLineBreak? _nLineBreak : 1;
        UINT16                  nBreak = nBreakStart;
        for( int i = 0 ; i < nCnt ; ++i )
        {
            nNum = UINT8( pSerNumSeq[ i ] );

            //MM : exchange the buffer[0] and buffer[1], which make it consistent with Mozilla and Windows
            pBuffer[ 1 ] = pHexDigs[ nNum & 0x0F ];
            nNum >>= 4;
            pBuffer[ 0 ] = pHexDigs[ nNum ];
            aStr.AppendAscii( pBuffer );

            --nBreak;
            if( nBreak )
                aStr.AppendAscii( _pSep );
            else
            {
                nBreak = nBreakStart;
                aStr.AppendAscii( "\n" );
            }
        }

        return aStr;
    }

    long ShrinkToFitWidth( Control& _rCtrl, long _nOffs )
    {
        long    nWidth = _rCtrl.GetTextWidth( _rCtrl.GetText() );
        Size    aSize( _rCtrl.GetSizePixel() );
        nWidth += _nOffs;
        aSize.Width() = nWidth;
        _rCtrl.SetSizePixel( aSize );
        return nWidth;
    }

    void AlignAfterImage( const FixedImage& _rImage, Control& _rCtrl, long _nXOffset )
    {
        Point   aPos( _rImage.GetPosPixel() );
        Size    aSize( _rImage.GetSizePixel() );
        long    n = aPos.X();
        n += aSize.Width();
        n += _nXOffset;
        aPos.X() = n;
        n = aPos.Y();
        n += aSize.Height() / 2;                    // y-position is in the middle of the image
        n -= _rCtrl.GetSizePixel().Height() / 2;    // center Control
        aPos.Y() = n;
        _rCtrl.SetPosPixel( aPos );
    }

    void AlignAfterImage( const FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset )
    {
        AlignAfterImage( _rImage, static_cast< Control& >( _rFI ), _nXOffset );
        ShrinkToFitWidth( _rFI );
    }

    void AlignAndFitImageAndControl( FixedImage& _rImage, FixedInfo& _rFI, long _nXOffset )
    {
        _rImage.SetSizePixel( _rImage.GetImage().GetSizePixel() );
        AlignAfterImage( _rImage, _rFI, _nXOffset );
    }
}


