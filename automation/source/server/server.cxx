/*************************************************************************
 *
 *  $RCSfile: server.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:48:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// do not use Application Idle but AutoTimer instead
#define TIMERIDLE

#define NO_JPEG

#ifndef NO_JPEG
#include <svtools/jpeg.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_ //handmade
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif

#include <vos/socket.hxx>

#if 1
#ifndef _SVTOOLS_TTPROPS_HXX // handmade
#include <svtools/ttprops.hxx>
#endif
#ifndef _BASIC_TTSTRHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#include <svtools/stringtransfer.hxx>
#endif
#ifndef _SV_SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#include "testtool.hrc"
#ifndef _SV_BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
// Hat keinen Includeschutz
#include <svtools/svtdata.hxx>
//#ifndef _DTRANS_HXX //autogen
//#include <so2/dtrans.hxx>
//#endif
#endif // 1

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_URI_H_
#include <rtl/uri.h>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif

#ifndef _STATEMNT_HXX
#include "statemnt.hxx"
#endif
#ifndef _SCMDSTRM_HXX
#include "scmdstrm.hxx"
#endif
#include "rcontrol.hxx"

#ifndef _SERVER_HXX
#include "server.hxx"
#endif
#ifndef SVTOOLS_TESTTOOL_HXX
#include "testtool.hxx"
#endif
#ifndef AUTOMATION_HXX
#include "automation.hxx"
#endif

#include "svtools/svtmsg.hrc"

#ifdef DBG_UTIL
void TestToolDebugPrint( const sal_Char *pString )
{
    StatementList::DirectLog( S_AssertError, UniString( pString, RTL_TEXTENCODING_UTF8 ) );
}
void SAL_CALL osl_TestToolDebugPrint( const sal_Char *pString )
{
    TestToolDebugPrint( pString );
}
#endif


ULONG RemoteControlCommunicationManager::nPortIs = TT_PORT_NOT_INITIALIZED;
USHORT RemoteControlCommunicationManager::nComm = 0;
BOOL RemoteControlCommunicationManager::bQuiet = FALSE;

#if OSL_DEBUG_LEVEL > 1
RemoteControlCommunicationManager::RemoteControlCommunicationManager( EditWindow * pDbgWin )
#else
RemoteControlCommunicationManager::RemoteControlCommunicationManager()
#endif
: CommunicationManagerServerViaSocket( GetPort(), 1, TRUE )
#if OSL_DEBUG_LEVEL > 1
, m_pDbgWin( pDbgWin )
#endif
, pTimer( NULL )
{
    bIsPortValid = ( GetPort() != 0 );
    if ( bQuiet )
    {
        SetInfoType( CM_NO_TEXT );
    }
    else
    {
        SetInfoType( CM_SHORT_TEXT | CM_ALL );
        ByteString aByteString;
        InfoMsg( InfoString( aByteString, CM_ALL ) );   // Anzeigen, daß wir da sind
    }
}

RemoteControlCommunicationManager::~RemoteControlCommunicationManager()
{
    if ( pTimer )
        delete pTimer;
}

void RemoteControlCommunicationManager::ConnectionOpened( CommunicationLink* pCL )
{
    StatementFlow::pCommLink = pCL;
    CommunicationManagerServerViaSocket::ConnectionOpened( pCL );
}


void RemoteControlCommunicationManager::ConnectionClosed( CommunicationLink* pCL )
{
    StatementFlow::pCommLink = NULL;
    CommunicationManagerServerViaSocket::ConnectionClosed( pCL );
}


IMPL_LINK( RemoteControlCommunicationManager, SetWinCaption, Timer*, pTimer_ )
{
    if ( pTimer )
    {
        delete pTimer;
        pTimer = NULL;
    }

    if ( StatementList::GetFirstDocFrame() )
    {
        if ( !aOriginalWinCaption.Len() )
            aOriginalWinCaption = StatementList::GetFirstDocFrame()->GetText();
        StatementList::GetFirstDocFrame()->SetText(String(aOriginalWinCaption).AppendAscii(" TT").Append(aAdditionalWinCaption).AppendAscii("[").Append(UniString::CreateFromInt32(nPortToListen)).AppendAscii("]"));
    }
    else
    {   // Dann Probieren wir es eben in 1 Sekunde nochmal
        pTimer = new Timer();   // Wird im Link gelöscht
        pTimer->SetTimeout( 1000 );
        pTimer->SetTimeoutHdl( LINK( this, RemoteControlCommunicationManager, SetWinCaption ) );
        pTimer->Start();
    }
    return 0;
}

void RemoteControlCommunicationManager::InfoMsg( InfoString aMsg )
{
    if ( !bIsPortValid )
        return;
    aAdditionalWinCaption = UniString( aMsg, RTL_TEXTENCODING_ASCII_US );
    SetWinCaption();
#if OSL_DEBUG_LEVEL > 1
    m_pDbgWin->AddText( UniString( (ByteString)aMsg, RTL_TEXTENCODING_ASCII_US ) );
    m_pDbgWin->AddText( "\n" );
#endif
}

ULONG RemoteControlCommunicationManager::GetPort()
{
    if ( TT_PORT_NOT_INITIALIZED == nPortIs )
    {   // Read Config

        USHORT i;
        // are we to be automated at all?
        BOOL bAutomate = FALSE;
        for ( i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
        {
            if ( Application::GetCommandLineParam( i ).EqualsIgnoreCaseAscii("/enableautomation")
                || Application::GetCommandLineParam( i ).EqualsIgnoreCaseAscii("-enableautomation"))
            {
                bAutomate = TRUE;
                break;
            }
        }

// if started within Portal determin location of testtool.ini/rc by analysing the commandline
// /userid:demo1[/export/home/user/demo1]
// -userid:demo1[/export/home/user/demo1]
        String aIniFileDir;
        for ( i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
        {
            if ( Application::GetCommandLineParam( i ).Copy(0,8).EqualsIgnoreCaseAscii("/userid:")
                || Application::GetCommandLineParam( i ).Copy(0,8).EqualsIgnoreCaseAscii("-userid:") )
            {
                rtl::OUString aEncHome
                    = Application::GetCommandLineParam(i).GetBuffer();

                rtl::OUString aDecHome = rtl::Uri::decode(aEncHome,
                        rtl_UriDecodeWithCharset,
                        RTL_TEXTENCODING_UTF8);

                aIniFileDir = aDecHome;
                aIniFileDir.Erase( 0, aIniFileDir.Search('[')+1 );
                aIniFileDir.Erase( aIniFileDir.Search(']') );
            }
        }

        if ( ! aIniFileDir.Len() )
            aIniFileDir = Config::GetDefDirectory();

        Config aConf(Config::GetConfigName( aIniFileDir, CUniString("testtool") ));
        aConf.SetGroup("Communication");

        ByteString aNoTesttoolKey( ByteString("Exclude_").Append( ByteString( Application::GetAppFileName(), RTL_TEXTENCODING_UTF8 ) ) );
// -notesttool
        for ( i = 0 ; i < Application::GetCommandLineParamCount() ; i++ )
        {
            if ( Application::GetCommandLineParam( i ).CompareIgnoreCaseToAscii("-notesttool") == COMPARE_EQUAL )
                aConf.WriteKey( aNoTesttoolKey, "something" );
        }

        nPortIs = aConf.ReadKey("TTPort","0").ToInt32();

        // noch prüfen ob dieses Office getestet werden soll.
        if ( !bAutomate || aConf.ReadKey( aNoTesttoolKey, "" ) != "" )
            nPortIs = 0;

        nComm = (USHORT)aConf.ReadKey("Comm","0").ToInt32();
        if ( nComm )
            aConf.DeleteKey("Comm");

        bQuiet = ( aConf.ReadKey("Quiet","no").CompareIgnoreCaseToAscii("yes") == COMPARE_EQUAL );
    }
    return nPortIs;
}

#if OSL_DEBUG_LEVEL > 1
#define MIN_IDLE 10000      // Ruhe vor dem Sturm min 10 Sekunden
#else
#define MIN_IDLE 60000      // Ruhe vor dem Sturm min 1 Minuten
#endif

class ExtraIdle : public AutoTimer
{
    virtual void    Timeout();

    USHORT nStep;
    ImplRemoteControl *pRemoteControl;
public:
    ExtraIdle( ImplRemoteControl *pRC );
};


ExtraIdle::ExtraIdle( ImplRemoteControl *pRC )
: nStep( 0 )
, pRemoteControl (pRC )
{
    SetTimeout( 120000 );   // 2 Minuten
#if OSL_DEBUG_LEVEL > 1
    SetTimeout( 40000 );    // 40 Sekunden
#endif
    Start();
}

void ExtraIdle::Timeout()
{
    if ( !StatementList::pTTProperties )
        StatementList::pTTProperties = new TTProperties();

    if ( !StatementList::pTTProperties->GetSlots() )
    {
        delete this;
        return;
    }

    // Müssen wir selbst idlen?
#if OSL_DEBUG_LEVEL > 1
    ULONG nLastInputInterval = Application::GetLastInputInterval();
    BOOL bIsInModalMode = Application::IsInModalMode();
    if ( bIsInModalMode || nLastInputInterval < MIN_IDLE )
#else
    if ( Application::IsInModalMode() || Application::GetLastInputInterval() < MIN_IDLE )
#endif
    {
        if ( nStep )    // Schon angefangen? dann abbrechen, sonst später nochmal
        {
            Sound::Beep();
            Sound::Beep();
#if OSL_DEBUG_LEVEL < 2
            delete this;
#endif
        }
#if OSL_DEBUG_LEVEL > 1
        Sound::Beep();
        Sound::Beep();
#endif
        return;
    }

    if ( StatementList::pFirst )    // Verarbeitung neu aufsetzen
    {
        GetpApp()->PostUserEvent( LINK( pRemoteControl, ImplRemoteControl, CommandHdl ) );
        return;
    }


    switch ( nStep++ )      // Probieren ob wir noch was machen können
    {
        case 0:
        {
            SfxPoolItem *pItem = new SfxStringItem((USHORT)StatementList::pTTProperties->nSidNewDocDirect, CUniString("swriter/web") );
            new StatementSlot( StatementList::pTTProperties->nSidNewDocDirect, pItem );
            SetTimeout(30000);
            return;
        }
        case 1:
        {
            new StatementSlot( StatementList::pTTProperties->nSidSourceView );
#if OSL_DEBUG_LEVEL > 1
            SetTimeout(7000);
#else
            SetTimeout(1500);
#endif
            return;
        }
        case 2:
        {
            new StatementSlot( StatementList::pTTProperties->nSidSelectAll );
            return;
        }
        case 3:
        {

#if OSL_DEBUG_LEVEL > 1
//#define TT_NO_DECRYPT
#define TT_CODE
#else
#define TT_CODE
#endif

#ifdef TT_NO_DECRYPT
            String aStr =
                ""
                ;

#else
            ByteString aStr =
                "\n"
                "VRQJ`ob\n"
                "YEZO\n"
                "ob\n"
                "UmRo`\n"
                "5J~O2o5+90~5,6xW$+5:c9o0UXRm`Y UQ~JP~X]`Y\\|%Y`Yo]~O||2[pP0Y1J,|V),,7:,+|JS+U*[/O|K\n"
                "|KaLYNV~]]2W/]*Y9|`*Y,P=[5P|U\n"
                "]}mqbw`zZU\\L\n"
                "LZdYWo9\n"
                "/J\n"
                "U~[QoZ\n"
                "Rqd~V\n"
                ",)1~00\n"
                "\n"
                ")0~*2=\n"
                "++2\\5&K|~5n9r~9/*9<*~051*Q|0~0rY|~./97~Q*7,Z9<|KY0:=K*<=w~qY`IbOKzLwN,`7b,V~]E`]b\\ORE~\n"
                "\n"
                "Vq~bR`W;a+Y\\J=LKJa+W*I/PbR~JLUX[|b~`Z2P/R*[9a~W=9~/9p8=a*P=J0OZ~7L`JbL=P<WbaLQbPO]JYKbD\n"
                "aY`J5J:b~7=2~+9)9W1,50b9X3P0`YbYVJ`Jb  \\`Z]`Vb\n"
                "VRQJ`b"
                ;
#endif

#ifdef TT_CODE
            for ( int i = 0 ; i < aStr.Len() ; i++ )
            {
                if ( aStr.GetChar(i) < 32 || aStr.GetChar(i) > 126 )
                {
                    // do nothing
                }
                else
                {
                    aStr.SetChar( i, aStr.GetChar(i) - 32 );
                    aStr.SetChar( i, 126 - aStr.GetChar(i) );
                }

                if ( i > (aStr.Len() / 2) && (i&1) )
                {
                    sal_Char c = aStr.GetChar(i);
                    aStr.SetChar( i, aStr.GetChar(aStr.Len()-i-1) );
                    aStr.SetChar( aStr.Len()-i-1, c );
                }
            }
#endif

//          ::svt::OStringTransfer::CopyString( UniString( aStr, RTL_TEXTENCODING_ASCII_US ) );

            new StatementSlot( StatementList::pTTProperties->nSidPaste );
            return;
        }
        case 4:
        {
            new StatementSlot( StatementList::pTTProperties->nSidSourceView );
            return;
        }
        case 5:
        {
            new StatementSlot( StatementList::pTTProperties->nSidSelectAll );
            new StatementSlot( StatementList::pTTProperties->nSidCopy );
            new StatementSlot( StatementList::pTTProperties->nSidPaste );
            return;
        }
        case 6:
        {
            ByteString aTr("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-");
            ByteString aData =
"P-S-0U04Fihixh00l0004b0b300-PS0g30428333y243q334j44426a6a65576c8k97aJecf7feccedg2inj3ghlshde5krk+lno"
"PpqpBfjsgusp1unp-po-PS0gm044x465e6b6L6boygeg-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo-ooo"
"-ooo-ooo-oo-1M04020Y30J0o080B040R040M-N0M700l010l000k000000000006000N011I112r222M-N0gJ40D000U001R011"
"0110500vr0001014p148mcg1R4koV18s95cwkAE2V8gImM5kgQY9WcosCw22I556p669I99aoaadrddd6eeeNghhIhhiriik6lll"
"NlmmImoprppp6qqqNsttItturuuw6xxxNxyyHyAA6BBBNBCCHCEE6FFFNFGGHGII6JJJNJKKHKMM6NNNNNOOHOQQ6RRRNRSSCSUU"
"NUVVIVVWpWWYIYYZrZZZ6+++M-N0Q700R000l000l000g00000006000N011I112r222M-N0kJ40C0003110d1110110r00t6000"
"Q041l18cF14gtk1ous48Acw295gAlEIMv28cxkY5FosQE2595dU9sY56q669N9aaCaddNdeeIeghrhhh6iiiNkllIllmrmmo6ppp"
"NpqqIqstrttt6uuuIwwxrxxx6yyyIAABrBBB6CCCIEEFrFFF6GGGIIIJrJJJ6KKKIMMNrNNN6OOOIQQRrRRR6SSSIUUVrVVV6WWW"
"IYYZrZZZ6+++U-S0d3009004Q040Of0TPU5QGjFCdPoji85WiqEopkCag321kP8dW4yO4KRlNi9iwzeTKup+Yk0lrdcicCEeLtVQ"
"z1IFeROmSJBa7VYMYY-0EWGkJWH6LpAVdrUepM7ScEpkTBkenX3YGuoFVU0IGk+dSzPpv0N6U07eTPFgid-YtvOD2ws5C96qDgIL"
"vhsoWmBPAozc+KgPjiVuW0TJnrt6PqF63p2VJEJ6A+l33JqESWh0G4yn1JkcaaEBnw17xmaf0q4BGkVy40Jj+FAyioG3KEukCtP1"
"OAdOe4ASVCPuUrQDFsqBoRWN6jqxOBfH-30WbgyZy+HtyI6xNVvt3M0lnfscjA8rBUeoRXifTPCceY6t46AR9ooG2jVzdmo+PQ6R"
"cAEDd7VE3GvUyDJzn2e0yyzypEdnCzUZorT029pk4LHJYsRQmR5smaW9EuCbt2A2s2Nd9ZKAkcJSWoTGPV5p6d1PZCiYt6kVETBB"
"K7zNWhRK7kMBCag7zELQ2e6HWHM+BwO4nJA-30uF2a2WgcgndWuk6gPbha0D5WFPq902KmjNwyg5xkVQvgd9W9SCfiFd95Ndh9yj"
"Odd7k38da3xWqtwcHPOEb7AvIPqAdRbz3XNNEYFu7bS9Iz-0UVQJc-gtgPCQ7cledmoGTULsGpjeu0TzkJi2tusMDnR4cisDw2rz"
"Vhs36hPC0oSH7V-UMAjVIC3dRFwNoc20a0+Culnm3q9QQJsgt00IeEoRXCh3jUg3eO8yGBOpFwYap5OrpoAfMeR6Q8L0sUIgI7B3"
"Oy9q5WMBAxg5PYnBSxZlywhwDlb45Il6Y+F-NaH62MEoByaq02d2aaEz5Bwx45DqfEC4ACqd4FYjI9IbAgqH7uFopm+JQRSHrSNd"
"ct0dwNo+FAUaD926b3wtUoRIPJ-MTLLiQcC92bTBue9RkDqqYRcXxn06S9Jm6Qhpk9IjH8JLyIinJj3EAF7bTH9jkf170OvzuO2j"
"I2jenHhQvnKoDSHSmWenEhfEHkVgekpfIOhkBhqLVaEvb83EyfD2Awrbk5+lwyvOne6yBA36rdrmna4xFOsvqGxRcgcJy-lXnjCn"
"eeWhGvqAbmSf7LcDwqykK9jqADpRqkXSq7MB7ZOHSgJhNitiw3i6y9LYjRNlq4Lc-00zCNL3CThC65Ajjlw8550bAbqa0d0Jz3BT"
"kH6EDgQhRUhjtyK9y9CjraNEw9ERUq6MmYa989nsRqsPxo+zi2IbOfxy9q3lFL-QSWn5qwp7nTFUwVe-XaDxnGfWOIYXXfIkILs-"
"lWPSm51tjj967w11u-YylxUO++EfuLsmr1c3jLdgcDYmK9roIAmz1t1vAalje3oyXDp335xkQ24rS1JhdokOn5cWpizqiE5bsbg4"
"4gWkfJ2IEVnSoWmj8eNeAFuoT0wzWWm9UgmDKRH2INGJy6OHTwn7zawsiPo796yQd6OsPORlTrUR-bEMLPj8nZdMwyX-Jb8npd2-"
"zV9JMRdNarUy1aF0tiihB0o+kQh5iy9r9BMqhPjf+WckJ9WWqmSQTEqAl+zwgw-+vH5WomSNVjbDLchO9Ae-ggdQGPcb+7Dq0X-d"
"XsFHj76-a0eUqKlN6cgHMKgKSmv8xcMVnCIPAnqR0SsThTWe8GSgo3pTGWTgBrtb1X2OfHMHsi8D3gkpPwKvoxoEuSJcTmD2kiAS"
"Pk3wl5C5NZDe9OrZMdDg6VQpDybXJ7EWLCdwsPoTGqhcGOGvrJ2WgFuuem+wP1ZGhkpee9rU7CTad9q9DxVgNzGWk+lGid6rKswa"
"1+Uc57RmFASpo3qbaGvuMReTLCWXsocM6lvXmSZHAhhaaV7EHH9sJglnrUlniII4I0gVZHFLys8VKKb2yKbAYHeSY3VlmgRywmqd"
"UXugq90wSsh0poya0qEAF9CjjadQumckue1unyK1sdcUwyxQOcARlHjLWYd3lS2ozCTQ48zZXesU66bAUfTdoXoOInm7MpPgwiDp"
"XDqJrEMEChxb747KzIHfxSdi++EwdRNK7RHEmgVhqiZkW1WqBEnjst6Oz08ztIPVknfPjq8NDB4h9g1sD+l1xQNzHNg+Jb1Vmii6"
"1dP-57LPdOhlWSTKYaCmzwAhGqyOlPrY9zXZodpZuoL2kjTBLBxaeGcM+NONZcN7GqIqFcNlhVgMXKHsd-WEBBR957ZZn7hk-mbb"
"FGxWLzaiHE6t48mXupNDlxi6d1w-yaPlmczA0gTsEhqRrsEbj48ProNvyivlaY06bdYSvGN7IOBc1ezBJiFd5OTz+RbzIsqJpCsJ"
"BOTSLjAdwXCzq-XExGbygb3X2oURVXxTB4q0e6euBRnXkIJuTM7SfQfQkdEEjN7J56t3oxP6B0cA4lgSDhURzsDzrkk0ECxfwaU3"
"ovagJuvzx07aksPdxkQ8aqEy618F-4wjCr3hZq8gq3gu7RJ4ovXa86R7ZskSYJC01o2OpfvJh0WqpYiIuE0zBqpI3kTJQZ0Or5ku"
"9RzhbzbV1AU0BzJ5vPTOBRIOIAiJiBiOdI8fR3dcWle3xCder+W6QELyr6NaldJipQCeAMwRr5mpzZESGAhuU3BDdkCh5ENgMUE-"
"sWotoCfnOwT7tJlXLHODk8K7Z4zYCG9Dh2fQazDE0JqBDruomfatotGADn25BCDpk6GI6SSftpUd71Qr1JBrgOr33aWswl983Uk7"
"cq9Em7vGtACekHlvOOVJfbdh76nNHzuQ1Z1oBvuU9l-dAg+-QWWFQ18D8U+zmYn1jypyarIXSrcIb67wLDTFXWm8F9XPmFWRBD3d"
"WukVJwhGNV5ZHVE1wCudY07ZIEAd1kgzgPcRSxFhRhFpXsnESjJhUNCA3DlrARwzz+llg0xpVHrJiddYT36P453qxpOmIE9e6-qJ"
"h4ipfTTt8f2Kq4mdWniErPtI+wrN-edvCQFtPdrL+tpV6EpPRXgmHnjRhV0eWWzqxdRZacX98CME3pvwDYWkO8TOUlcNQSKTU1iF"
"FC9WIBA8PulsCFVNH1qJwZxYYcaX6CGNnR7vHiIBDsTE51J4b4fYucNYFG9V5mCUdrJT57tHk9eghSOfgeHZDxuvQt8619pwKma7"
"3Nl00EFklZOk+APRmKviL+iyiG1sWfA3E0xUPznlQgvsHJRzD9u0TzHsB6tIMKLmOxvVHG9knpHerjAXNqIp7jwZzvYXIyW8kw8g"
"3ycECFaB2Y2U0l00NE7l2Aca2y5uhk+QJygN0857SQMVSEXjy+Q84nQjkTh1GAtFACtdHRhwQ6FhQMLjFu6zyxuFycbQA7qNSsiy"
"90wlAaUBBtFhxMV0TPd8DbVScjJoMSAYMh6GhAHnKOZsbdqvwtHKdZWZ9HQbdmVOt0xnnK5Ju9KfwhuHMZIoPt73BqspII6qBobB"
"5kfcwm183j4fwapcs50EoGgz2UZGuK88agfskePeYt9DOQD3qxxfuJ5lZUFHa8aqFJIT6MG2Kwtwuu0zBqTz8x5DYM7PDh29F9FU"
"1ge-wqqIMqmXlpbO65sila1be1yRGABAbw2njF5txZEAaqEyEo9FUPqnKQ4y1NQqSXkCpsqpO06UUCyBBzaDjawwoHkKOT1-zqpz"
"FU7JNudONE3fuYk83U9thALoAIeG6FKizOLgU4AcDcszCmGZgylUI-Edd9mAKL9nJe+YdiYxl7uX4mATdO30KcuDrRoTxBbiHbuA"
"qlorQn1D0opRuIhzVLm8+z8QRFlNA0683M1QYE+Lhka+kaIDvE8RHQHel4bOsMFp6lmV6D3cNhQvpG1sECm02a5tgF52reEBaYEw"
"OhD+RQiFedTm3OQg5iq2c04kidOoDgaPNGs1VitbrhIvAuzStaWksap3jp9UrAN1O-0nAECIfSP0QHVkGWtduz6XSmJ7MsLPmPJ3"
"hRjY7DtZXWjvtHcj9ooAXcPsI+3YgG951n7urnyB1kbQV+ZdlAbI11Y3orBMB+le8goi66fWyEX9FHpFEL32jNqSghzvyEC1227-"
"p5t8vx19mYHbOghy5K7voWUAXsjX2gwzicmKiNJR9OrHppAbVEVzVjOuYWmwCpGWFW1DlaoOc03PWkgqvVeezQY8IiM9Rptnniwf"
"Xa1XnMPo6ES0MHE5nwC8tT65VVw3C2peCu720i6oVvevcoMGeP3PVgvBkudifs0GNH7AaOGVFhrbE68B8sq6AH8BFvXhZfzdhb1f"
"Y1p-GVyr3qECy393zFEq0wHg2Vls4OiVD-J0d7JFKsuhUPgdykTCWhbqkdvwUUyg7qXPvdeC09AUAszRcVsk5iihIr1+N-0ATkGU"
"i6GPwTlzw-dALNmjbVjHOSAsWaihe303RxAmD4akSPWkjgtot17BTZfaSgaNH+ESoUGJ3GgPJqD8UBsAShIF-X0wwyFpDkTwESHg"
"jNwUF9EpszCwj1myzqZG9hIp76G1ymz7BuZF0T5pdA1GMG8AGuRbXEtJMkHsDJoztG06Jqm-khFPydXg-VB1k+l9AMwzzvtCDacK"
"k22WU1fByYcDpmW0Y9YF-zeZDDcQJVF8tT8cNNjt9GdIF3103ZFP8oulWCfnXETCKz3YQFsm3qOUu6GJ-lb2foo1WJqGpcCbyPmy"
"Ib95rQLJnk56YC1KmN5zMJ831cVsERyvdPOSW8kg-2uk8m3J4zgAWAhvvBOofIjFb5yNf0shVv-JJ9f49ZFcQ+LKDFKX3iNV1E-G"
"MxeEwbi-uGP8BGO4vGgV0IFbgswumfhk14OF3q+1qwRFpq4hr1s6zQEAgoVAW3QE4tsQpYW3JkcqDcnSOjbePZeFrFMor-o3UG2F"
"jmw8667eXk3UiM9vq5EpyrbQxexsJ3tKy7w6lGsumfMWIlcSglkLUzicysuPgqT5Wuzn8MkGvTYve2UyunErUnD-+Qwr0rDo1tOG"
"bbtcNNeFInx5rDK3DHahjTON3d3oTpePxioVK3sRLDh185yKMzTQv812ADCFcwvFHbetPF41f7kot00O2OMUkw4OPvuTRkhdAhgd"
"il2SM9bunNaNHqh9Ov8Qv3SKEl1O-BwzjYF0VWjkxycswQFqQotUPw+Q-6FrCPFWvaF2CP2F319stMfD-8bHsd87KZfQ9ChereG4"
"Z8XP8dNMipn-evkOVVFqfgN16dO8Ya9nqGFIpIW1Ljv7wOAzdZFsm5C1EuQoKzwyXDO0BDjceBsyTt40H0upG8D1N1ZP66OPIeQy"
"oXQwI63e+NnuYA0687-d6N6rDscj+VHn2R0RUXQFZ2+EANqcqvan4y0Erpl01fAfmLaI8pmOgsRUDvuF5e9YnWNhxtSzS4fsjj1J"
"1EIGpcw0WfiaOul1s19ZIECoLBx-#S";


//#if OSL_DEBUG_LEVEL > 1
//          SvFileStream aStream( "d:\\gh_writeback.jpg" , STREAM_STD_READWRITE | STREAM_TRUNC );
//#else
            SvMemoryStream aStream;
//#endif
            xub_StrLen c;
            xub_StrLen cRest;

            xub_StrLen nIndex;
            for ( nIndex = 0 ; nIndex < aData.Len() ; nIndex++ )
            {
                if ( ( nIndex & 3 ) == 0 )
                {
                    cRest = aData.GetChar( nIndex );
                    cRest = aTr.Search( (sal_Char)cRest );
                }
                else
                {
                    c = aData.GetChar( nIndex );
                    c = aTr.Search( (sal_Char)c );

                    c <<= 2;
                    c |= ( ( cRest & 0x30 ) >> 4 );
                    cRest <<= 2;

                    aStream << sal_Char(c);
                }
            }

            aStream.Seek(0);
#ifndef NO_JPEG
            Graphic aGraphic;
            if ( ImportJPEG( aStream, aGraphic, NULL ) )
            {
                Bitmap *pBmp = new Bitmap( aGraphic.GetBitmap() );
                StatementList::pTTProperties->Img( pBmp );
                delete pBmp;
            }
            else
#endif
            {
//              ::svt::OStringTransfer::CopyString( CUniString("\nSorry! no bitmap") );
            }

/***********************************************************************
//          USHORT nBC = pBmp->GetBitCount();
//          pBmp->Scale( 0.02, 0.02 );
//          nBC = pBmp->GetBitCount();
//          SvMemoryStream aStream;
            SvFileStream aStream( "d:\gh_small50.jpg", STREAM_STD_READ );

            aStream.Seek( 0 );
            xub_StrLen c;
            String aOut;
            String aDreierGruppe;
            xub_StrLen cRest=0;
            aStream >> c;
            while ( !aStream.IsEof() )
            {
                cRest <<= 2;        // Im ersten Durchgang egal, da immer 0
                cRest |= ( c & 0x03 );
                c >>= 2;
                aDreierGruppe += aTr.GetChar( c );

                if ( aDreierGruppe.Len() == 3 )
                {
                    aOut += aTr.GetChar( cRest );
                    aOut += aDreierGruppe;
                    cRest = 0;
                    aDreierGruppe = "";
                }
                aStream >> c;
            }
            if ( aDreierGruppe.Len() )
            {
                aOut += cRest;
                aOut += aDreierGruppe;
            }
            ::svt::OStringTransfer::CopyString( aOut );
**********************************************************************************/

            new StatementSlot( StatementList::pTTProperties->nSidPaste );
            return;
        }
        case 7:
        {
            new StatementSlot( 20384 ); // FN_TOOL_ANKER_CHAR aus SW?
            return;
        }
    }

    // Wir sind am Ende

#if OSL_DEBUG_LEVEL < 2
    delete this;
#endif
}

IMPL_LINK( ImplRemoteControl, IdleHdl, Application*, pApp )
{
    if( StatementList::pFirst )
    {
        #if OSL_DEBUG_LEVEL > 1
        m_pDbgWin->AddText( "* " );
        #endif
        GetpApp()->PostUserEvent( LINK( this, ImplRemoteControl, CommandHdl ) );
    }
    return 0;
}



IMPL_LINK( ImplRemoteControl, CommandHdl, Application*, pApp )
{
#if OSL_DEBUG_LEVEL > 1
    m_pDbgWin->AddText( "Entering CommandHdl\n" );
#endif

    if ( StatementList::MaybeResetSafeReschedule() )
    {
        StatementList::bExecuting = FALSE;      // Wird nacher im SafeReschedule wieder zurückgesetzt
#if OSL_DEBUG_LEVEL > 1
        m_pDbgWin->AddText( "SafeReschedule has been reset\n" );
#endif
    }

    if ( ( StatementList::bReadingCommands && !StatementList::bDying ) ||
         ( StatementList::bExecuting ) ||
         ( StatementList::IsInReschedule() ) )
        {
#if OSL_DEBUG_LEVEL > 1
            if ( StatementList::bReadingCommands )
                m_pDbgWin->AddText( "Reading Commands " );
            if ( StatementList::bExecuting )
                m_pDbgWin->AddText( "In Execute " );
            if ( StatementList::IsInReschedule() )
                m_pDbgWin->AddText( "In Reschedule " );
            m_pDbgWin->AddText( "Leaving CommandHdl\n" );
#endif
            return 0;        // Garnicht erst irgendwelchen blödsinn machen
        }

    while( StatementList::pFirst && ( !StatementList::bReadingCommands || StatementList::bDying ) )
        // Schleift hier bis Befehl nicht zurückkommt,
        // Wird dann rekursiv über IdleHdl und PostUserEvent aufgerufen.
    {
        m_bInsideExecutionLoop = TRUE;
#ifdef TIMERIDLE
        m_aIdleTimer.Stop();
        m_aIdleTimer.Start();
#endif
        StatementList *pC = StatementList::pFirst;

//      MessBox MB( pMainWin, WB_DEF_OK|WB_OK, "Pause ...", "... und Weiter" );
//      MB.Execute();

        if ( !StatementList::bCatchGPF )
        {
            if (!pC->CheckWindowWait()  ||  !pC->Execute())
            {
#if OSL_DEBUG_LEVEL > 1
                m_pDbgWin->AddText( "Leaving CommandHdl\n" );
#endif
                return 0;        // So dass die App nochmal ´ne chance bekommt
            }
        }
        else
        {
            try
            {
                if (!pC->CheckWindowWait()  ||  !pC->Execute())
                {
#if OSL_DEBUG_LEVEL > 1
                    m_pDbgWin->AddText( "Leaving CommandHdl\n" );
#endif
                    return 0;        // So dass die App nochmal ´ne chance bekommt
                }
            }
            catch( ... )
            {
                if ( !StatementFlow::bUseIPC )
                    throw;  // aus der Hilfe heraus nicht leise abbrechen

                try
                {
                    ModelessDialog *pDlg = new ModelessDialog(NULL);
                    pDlg->SetOutputSizePixel(Size(150,0));
                    pDlg->SetText( String ( TTProperties::GetSvtResId( TT_GPF ) ) );
                    pDlg->Show();
                    DBG_ERROR("GPF");
                    pC->ReportError( GEN_RES_STR0( S_GPF_ABORT ) );
                    StatementList::bDying = TRUE;
                    while ( StatementList::pFirst )         // Kommandos werden übersprungen
                        StatementList::NormalReschedule();
                    delete pDlg;
                }
                catch ( ... )
                {
                    pApp->Quit();
                }
                pApp->Quit();
            }
        }


        for (int xx = 1;xx < 20;xx++)
            StatementList::NormalReschedule();

        m_bInsideExecutionLoop = FALSE;
    }

    StatementList::nWindowWaitUId = 0;  // Warten rücksetzen, da handler sowieso verlassen wird

/*    if( StatementList::pFirst && !StatementList::bReadingCommands )
         // Abfrage nötig, da andere CommandHdl aktiv sein können oder
         // neue Commands gelesen werden können
    {
        delete StatementList::pFirst;     // Löscht die gesamte Liste !!
        StatementList::pFirst   = NULL;
        StatementList::pCurrent = NULL;   // Nur zur Sicherheit, sollte hier sowieso NULL sein
    }*/

#if OSL_DEBUG_LEVEL > 1
    m_pDbgWin->AddText( "Leaving CommandHdl\n" );
#endif
    return 0;
}

IMPL_LINK( ImplRemoteControl, QueCommandsEvent, CommunicationLink*, pCL )
{
    SvStream *pTemp = pCL->GetServiceData();
    QueCommands( SI_IPCCommandBlock, pTemp );
    delete pTemp;
    return 0;
}

BOOL ImplRemoteControl::QueCommands( ULONG nServiceId, SvStream *pIn )
{
//    return TRUE;
    USHORT nId;

    if( !m_bIdleInserted )
    {
#ifdef TIMERIDLE
        m_aIdleTimer.SetTimeoutHdl( LINK( this, ImplRemoteControl, IdleHdl ) );
        m_aIdleTimer.SetTimeout( 1000 );
        m_aIdleTimer.Start();
#else
        GetpApp()->InsertIdleHdl( LINK( this, ImplRemoteControl, IdleHdl ), 1 );
#endif
        m_bIdleInserted = TRUE;
    }


    StatementList::bReadingCommands = TRUE;


    SCmdStream *pCmdStream = new SCmdStream(pIn);
#if OSL_DEBUG_LEVEL > 1
    if (!m_pDbgWin->bQuiet)
        m_pDbgWin->Show();
    m_pDbgWin->AddText( "Reading " );
    m_pDbgWin->AddText( String::CreateFromInt64( nServiceId ) );
    m_pDbgWin->AddText( " :\n" );
#endif

    if( nServiceId != SI_IPCCommandBlock && nServiceId != SI_DirectCommandBlock )
    {
        DBG_ERROR1( "Ungültiger Request :%i", (int)nServiceId );
        return FALSE;
    }

    pCmdStream->Read( nId );
    while( !pIn->IsEof() )
    {
        switch( nId )
        {
            case SICommand:
            {
                new StatementCommand( pCmdStream );     // Wird im Konstruktor an Liste angehängt
                break;
            }
            case SIControl:
            case SIStringControl:
            {
                new StatementControl( pCmdStream, nId );     // Wird im Konstruktor an Liste angehängt
                break;
            }
            case SISlot:
            {
                new StatementSlot( pCmdStream );    // Wird im Konstruktor an Liste angehängt
                break;
            }
            case SIUnoSlot:
            {
                new StatementUnoSlot( pCmdStream );    // Wird im Konstruktor an Liste angehängt
                break;
            }
            case SIFlow:
            {
                new StatementFlow( nServiceId, pCmdStream, this );              // Wird im Konstruktor an Liste angehängt
                break;
            }
            default:
                DBG_ERROR1( "Unbekannter Request Nr:%il", nId );
                break;
        }
        if( !pIn->IsEof() )
            pCmdStream->Read( nId );
        else
            DBG_ERROR( "truncated input stream" );
    }

    StatementList::bReadingCommands = FALSE;

    delete pCmdStream;
#if OSL_DEBUG_LEVEL > 1
    m_pDbgWin->AddText( "Done Reading " );
    m_pDbgWin->AddText( String::CreateFromInt64( nServiceId ) );
    m_pDbgWin->AddText( " :\n" );
#endif
    if ( !m_bInsideExecutionLoop )
    {
#ifdef DEBUG
        m_pDbgWin->AddText( "Posting Event for CommandHdl.\n" );
#endif

        GetpApp()->PostUserEvent( LINK( this, ImplRemoteControl, CommandHdl ) );
    }
#ifdef DEBUG
    else
        m_bInsideExecutionLoop = TRUE;
#endif
    return TRUE;
} // BOOL ImplRemoteControl::QueCommands( ULONG nServiceId, SvStream *pIn )


SvStream* ImplRemoteControl::GetReturnStream()
{
    SvStream* pTemp = pRetStream;
    pRetStream = NULL;
    return pTemp;
}

void ImplRemoteControl::ExecuteURL( String &aURL )
{   // hier purzeln dann die URLs private:testtool/* rein
}


ImplRemoteControl::ImplRemoteControl()
: m_bIdleInserted( FALSE )
, m_bInsideExecutionLoop( FALSE )
, pRetStream(NULL)
#if OSL_DEBUG_LEVEL > 1
, m_pDbgWin(NULL)
#endif
{
#if OSL_DEBUG_LEVEL > 1
    if ( RemoteControlCommunicationManager::GetPort() != TT_NO_PORT_DEFINED || RemoteControlCommunicationManager::nComm )
    {
        m_pDbgWin = new EditWindow( NULL, CUniString("Debug Window"), WB_VSCROLL );
        m_pDbgWin->bQuiet = TRUE;
        m_pDbgWin->Hide();
        StatementList::m_pDbgWin = m_pDbgWin;
    }
#endif
    if ( RemoteControlCommunicationManager::GetPort() == TT_NO_PORT_DEFINED )
        pServiceMgr = NULL;
    else
    {
#if OSL_DEBUG_LEVEL > 1
        pServiceMgr = new RemoteControlCommunicationManager( m_pDbgWin );
#else
        pServiceMgr = new RemoteControlCommunicationManager();
#endif
        pServiceMgr->SetDataReceivedHdl( LINK( this, ImplRemoteControl, QueCommandsEvent ) );
        pServiceMgr->StartCommunication();

#ifdef DBG_UTIL
        DbgSetPrintTestTool( TestToolDebugPrint );
        // first change it, so we get the original Pointer
        StatementCommand::pOriginal_osl_DebugMessageFunc = osl_setDebugMessageFunc( osl_TestToolDebugPrint );
        if ( DbgGetErrorOut() != DBG_OUT_TESTTOOL )
            osl_setDebugMessageFunc( StatementCommand::pOriginal_osl_DebugMessageFunc );
#endif
    }
    if ( RemoteControlCommunicationManager::nComm )
        new ExtraIdle( this );      // Setzt die Bearbeitung wieder auf
}

ImplRemoteControl::~ImplRemoteControl()
{
    StatementList::bDying = TRUE;
#if OSL_DEBUG_LEVEL > 1
    if ( m_pDbgWin )
        m_pDbgWin->bQuiet = TRUE;   // Keine Ausgabe mehr im Debugwindow
#endif

#ifdef DBG_UTIL
    // Zurücksetzen, so daß nachfolgende Assertions nicht verloren gehen
    DbgSetPrintTestTool( NULL );
    osl_setDebugMessageFunc( StatementCommand::pOriginal_osl_DebugMessageFunc );
#endif

    if ( StatementList::pFirst )
    {   // Es sind noch Kommandos da, also auch eine Möglichkeit zurückzusenden.
        StatementList::pFirst->ReportError( GEN_RES_STR0( S_APP_SHUTDOWN ) );
        while ( StatementList::pFirst )             // Kommandos werden übersprungen
            StatementList::NormalReschedule();      // Fehler zurückgeschickt
    }

    if ( pServiceMgr )
        pServiceMgr->StopCommunication();

#if OSL_DEBUG_LEVEL > 1
    delete m_pDbgWin;
#endif
    if( m_bIdleInserted )
    {
#ifdef TIMERIDLE
        m_aIdleTimer.Stop();
#else
        GetpApp()->RemoveIdleHdl( LINK( this, ImplRemoteControl, IdleHdl ) );
#endif
        m_bIdleInserted = FALSE;
    }
    delete pServiceMgr;
}

RemoteControl::RemoteControl()
{
    pImpl = new ImplRemoteControl;
}

RemoteControl::~RemoteControl()
{
    delete pImpl;
}

static ::osl::Mutex aMutex;
static RemoteControl* pRemoteControl = 0;
extern "C" void CreateRemoteControl()
{
    if ( !pRemoteControl )
    {
        ::osl::MutexGuard aGuard( aMutex );
        if ( !pRemoteControl )
            pRemoteControl = new RemoteControl();
    }
}

extern "C" void DestroyRemoteControl()
{
    ::osl::MutexGuard aGuard( aMutex );
    delete pRemoteControl;
    pRemoteControl = 0;
}

