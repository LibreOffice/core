/*************************************************************************
 *
 *  $RCSfile: appdde.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-16 15:30:58 $
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

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVDDE_HXX //autogen
#include <svtools/svdde.hxx>
#endif
#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif
#ifndef _LINKMGR_HXX //autogen
#include <so3/linkmgr.hxx>
#endif
#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
#pragma hdrstop

#include "app.hxx"
#include "appdata.hxx"
#include "objsh.hxx"
#include "viewfrm.hxx"
#include "topfrm.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "sfxsids.hrc"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "helper.hxx"

//========================================================================

String SfxDdeServiceName_Impl( const String& sIn )
{
    ByteString sTemp = U2S( sIn );
    ByteString sReturn;

    for ( sal_uInt16 n = sTemp.Len(); n; --n )
        if ( sTemp.Copy( n-1, 1 ).IsAlphaNumericAscii() )
            sReturn += sTemp.GetChar(n-1);

    return S2U( sReturn );
}


class ImplDdeService : public DdeService
{
public:
    ImplDdeService( const String& rNm )
        : DdeService( rNm )
    {}
    virtual BOOL MakeTopic( const String& );

    virtual String  Topics();
//  virtual String  Formats();
//  virtual String  SysItems();
//  virtual String  Status();

    virtual BOOL SysTopicExecute( const String* pStr );
};

class SfxDdeTriggerTopic_Impl : public DdeTopic
{
public:
    SfxDdeTriggerTopic_Impl()
    : DdeTopic( DEFINE_CONST_UNICODE("TRIGGER") )
    {}

    virtual BOOL Execute( const String* );
};

class SfxDdeDocTopic_Impl : public DdeTopic
{
public:
    SfxObjectShell* pSh;
    DdeData aData;
    SvData aSvData;

    SfxDdeDocTopic_Impl( SfxObjectShell* pShell )
        : DdeTopic( pShell->GetTitle(SFX_TITLE_FULLNAME) ), pSh( pShell )
    {}

    virtual DdeData* Get( ULONG );
    virtual BOOL Put( const DdeData* );
    virtual BOOL Execute( const String* );
    virtual BOOL StartAdviseLoop();
    virtual BOOL MakeItem( const String& rItem );

// wird benoetigt?
//  virtual void Connect( long n );
//  virtual void Disconnect( long n );
//  virtual void StopAdviseLoop();

};


SV_DECL_PTRARR( SfxDdeDocTopics_Impl, SfxDdeDocTopic_Impl * const, 4, 4 )
SV_IMPL_PTRARR( SfxDdeDocTopics_Impl, SfxDdeDocTopic_Impl * const )

//========================================================================

BOOL SfxAppEvent_Impl( ApplicationEvent &rAppEvent,
                       const String &rCmd, const String &rEvent )

/*  [Beschreibung]

    Pr"uft, ob 'rCmd' das Event 'rEvent' ist (ohne '(') und baut
    aus diesem dann ein <ApplicationEvent> zusammen, das per
    <Application::AppEvent()> ausgef"uhrt werden kann. Ist 'rCmd' das
    angegegeben Event 'rEvent', dann wird TRUE zur"uckgegeben, sonst FALSE.


    [Beispiel]

    rCmd = "Open(\"d:\doc\doc.sdw\")"
    rEvent = "Open"
*/

{
    String aEvent( rEvent );
    aEvent += 0x0028 ;// "(" = 28h
    if ( rCmd.CompareIgnoreCaseToAscii( aEvent, aEvent.Len() ) == COMPARE_EQUAL )
    {
        String aData( rCmd );
        aData.Erase( 0, aEvent.Len() );
        if ( aData.Len() > 2 )
        {
            // in das ApplicationEvent-Format wandeln
            aData.Erase( aData.Len()-1, 1 );
            for ( USHORT n = 0; n < aData.Len(); ++n )
            {
                if ( aData.GetChar(n) == 0x0022 ) // " = 22h
                    for ( ; aData.GetChar(++n) != 0x0022 ; )
                        /* empty loop */ ;
                else if ( aData.GetChar(n) == 0x0020 ) // SPACE = 20h
                    aData.SetChar(n, '\n');
            }
            aData.EraseAllChars( 0x0022 );
            ApplicationAddress aAddr;
            rAppEvent = ApplicationEvent( String(), aAddr, U2S(rEvent), aData );
            return TRUE;
        }
    }

    return FALSE;
}

//-------------------------------------------------------------------------

long SfxApplication::DdeExecute
(
    const String&   rCmd        // in unserer BASIC-Syntax formuliert
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxApplication-Subklasse gerichtete DDE-Kommandos
    zu empfangen.

    Die Basisimplementierung versteht die API-Funktionalit"at der
    betreffenden SfxApplication-Subklasse in BASIC-Syntax. R"uckgabewerte
    k"onnen dabei leider nicht "ubertragen werden.
*/

{
    // Print oder Open-Event?
    ApplicationEvent aAppEvent;
    if ( SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Print") ) ||
         SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Open") ) )
        GetpApp()->AppEvent( aAppEvent );
    else
    {
        // alle anderen per BASIC
        EnterBasicCall();
        StarBASIC* pBasic = GetBasic();
        DBG_ASSERT( pBasic, "Wo ist mein Basic???" );
        SbxVariable* pRet = pBasic->Execute( rCmd );
        LeaveBasicCall();
        if( !pRet )
        {
            SbxBase::ResetError();
            return 0;
        }
    }
    return 1;
}

//--------------------------------------------------------------------

long SfxApplication::DdeGetData
(
    const String&   rItem,      // das anzusprechende Item
    SvData&         rData       // in: Format, out: angeforderte Daten
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxApplication-Subklasse gerichtete DDE-Daten-Anforderungen
    zu empfangen.

    Die Basisimplementierung liefert keine Daten und gibt 0 zur"uck.
*/

{
    return 0;
}

//--------------------------------------------------------------------

long SfxApplication::DdeSetData
(
    const String&   rItem,      // das anzusprechende Item
    const SvData&   rData       // Daten-Format und Daten selbst
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxApplication-Subklasse gerichtete DDE-Daten
    zu empfangen.

    Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/

{
    return 0;
}

//--------------------------------------------------------------------

SvPseudoObject* SfxApplication::DdeCreateHotLink
(
    const String&   rItem      // das zu erzeugende Item
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seiner SfxApplication-Subklasse einen DDE-Hotlink einzurichten

    Die Basisimplementierung erzeugt keinen und liefert 0 zur"uck.
*/

{
    return 0;
}

//========================================================================

long SfxObjectShell::DdeExecute
(
    const String&   rCmd        // in unserer BASIC-Syntax formuliert
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxObjectShell-Subklasse gerichtete DDE-Kommandos
    zu empfangen.

    Die Basisimplementierung f"uhrt nichts aus und liefert 0 zur"uck.
*/

{
    StarBASIC* pBasic = GetBasic();
    DBG_ASSERT( pBasic, "Wo ist mein Basic???" ) ;
    SbxVariable* pRet = pBasic->Execute( rCmd );
    if( !pRet )
    {
        SbxBase::ResetError();
        return 0;
    }

    return 1;
}

//--------------------------------------------------------------------

long SfxObjectShell::DdeGetData
(
    const String&   rItem,      // das anzusprechende Item
    SvData&         rData       // in: Format, out: angeforderte Daten
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxObjectShell-Subklasse gerichtete DDE-Daten-Anforderungen
    zu empfangen.

    Die Basisimplementierung liefert keine Daten und gibt 0 zur"uck.
*/

{
    return 0;
}

//--------------------------------------------------------------------

long SfxObjectShell::DdeSetData
(
    const String&   rItem,      // das anzusprechende Item
    const SvData&   rData       // Daten-Format und Daten selbst
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxObjectShell-Subklasse gerichtete DDE-Daten
    zu empfangen.

    Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/

{
    return 0;
}

//--------------------------------------------------------------------

SvPseudoObject* SfxObjectShell::DdeCreateHotLink
(
    const String&   rItem      // das zu erzeugende Item
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seiner SfxObjectShell-Subklasse einen DDE-Hotlink einzurichten

    Die Basisimplementierung erzeugt keinen und liefert 0 zur"uck.
*/

{
    return 0;
}

//========================================================================

long SfxViewFrame::DdeExecute
(
    const String&   rCmd        // in unserer BASIC-Syntax formuliert
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxViewFrame-Subklasse gerichtete DDE-Kommandos
    zu empfangen.

    Die Basisimplementierung versteht die API-Funktionalit"at des
    betreffenden SfxViewFrame, der darin dargestellten SfxViewShell und
    der betreffenden SfxObjectShell-Subklasse in BASIC-Syntax.
    R"uckgabewerte k"onnen dabei leider nicht "ubertragen werden.
*/

{
    if ( GetObjectShell() )
        return GetObjectShell()->DdeExecute( rCmd );

    return 0;
}

//--------------------------------------------------------------------

long SfxViewFrame::DdeGetData
(
    const String&   rItem,      // das anzusprechende Item
    SvData&         rData       // in: Format, out: angeforderte Daten
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxViewFrame-Subklasse gerichtete DDE-Daten-Anforderungen
    zu empfangen.

    Die Basisimplementierung liefert keine Daten und gibt 0 zur"uck.
*/

{
    return 0;
}

//--------------------------------------------------------------------

long SfxViewFrame::DdeSetData
(
    const String&   rItem,      // das anzusprechende Item
    const SvData&   rData       // Daten-Format und Daten selbst
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seine SfxViewFrame-Subklasse gerichtete DDE-Daten
    zu empfangen.

    Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/

{
    return 0;
}

//--------------------------------------------------------------------

SvPseudoObject* SfxViewFrame::DdeCreateHotLink
(
    const String&   rItem      // das zu erzeugende Item
)

/*  [Beschreibung]

    Diese Methode kann vom Applikationsentwickler "uberladen werden,
    um an seiner SfxViewFrame-Subklasse einen DDE-Hotlink einzurichten

    Die Basisimplementierung erzeugt keinen und liefert 0 zur"uck.
*/

{
    return 0;
}


//========================================================================

BOOL SfxApplication::InitializeDde()
{
    DBG_ASSERT( !pAppData_Impl->pDdeService,
                "Dde kann nicht mehrfach initialisiert werden" )

    pAppData_Impl->pDdeService = new ImplDdeService( Application::GetAppName() );
    int nError = pAppData_Impl->pDdeService->GetError();
    if( !nError )
    {
        pAppData_Impl->pDocTopics = new SfxDdeDocTopics_Impl;

        // wir wollen auf jedenfall RTF unterstuetzen!
        pAppData_Impl->pDdeService->AddFormat( FORMAT_RTF );

        // Config-Pfad als Topic wegen Mehrfachstart
        INetURLObject aOfficeLockFile( SvtPathOptions().GetUserConfigPath() );
        aOfficeLockFile.insertName( DEFINE_CONST_UNICODE( "soffice.lck" ) );
        String aService( SfxDdeServiceName_Impl( aOfficeLockFile.GetMainURL() ) );
        aService.ToUpperAscii();
        pAppData_Impl->pDdeService2 = new ImplDdeService( aService );
        pAppData_Impl->pTriggerTopic = new SfxDdeTriggerTopic_Impl;
        pAppData_Impl->pDdeService2->AddTopic( *pAppData_Impl->pTriggerTopic );
    }
    return !nError;
}

//--------------------------------------------------------------------

void SfxApplication::AddDdeTopic( SfxObjectShell* pSh )
{
    DBG_ASSERT( pAppData_Impl->pDocTopics, "es gibt gar keinen Dde-Service" )
    //OV: Im Serverbetrieb ist DDE abgeklemmt!
    if( !pAppData_Impl->pDocTopics )
        return;

    // doppeltes Eintragen verhindern
    String sShellNm;
    BOOL bFnd = FALSE;
    for( USHORT n = pAppData_Impl->pDocTopics->Count(); n; )
        if( (*pAppData_Impl->pDocTopics)[ --n ]->pSh == pSh )
        {
            // JP 18.03.96 - Bug 26470
            //  falls das Document unbenannt wurde, ist trotzdem ein
            //  neues Topics anzulegen!
            if( !bFnd )
            {
                bFnd = TRUE;
                (sShellNm = pSh->GetTitle(SFX_TITLE_FULLNAME)).ToLowerAscii();
            }
            String sNm( (*pAppData_Impl->pDocTopics)[ n ]->GetName() );
            if( sShellNm == sNm.ToLowerAscii() )
                return ;
        }

    const SfxDdeDocTopic_Impl* pTopic = new SfxDdeDocTopic_Impl( pSh );
    pAppData_Impl->pDocTopics->Insert( pTopic,
                                       pAppData_Impl->pDocTopics->Count() );
    pAppData_Impl->pDdeService->AddTopic( *pTopic );
}

void SfxApplication::RemoveDdeTopic( SfxObjectShell* pSh )
{
    DBG_ASSERT( pAppData_Impl->pDocTopics, "es gibt gar keinen Dde-Service" )
    //OV: Im Serverbetrieb ist DDE abgeklemmt!
    if( !pAppData_Impl->pDocTopics )
        return;

    SfxDdeDocTopic_Impl* pTopic;
    for( USHORT n = pAppData_Impl->pDocTopics->Count(); n; )
        if( ( pTopic = (*pAppData_Impl->pDocTopics)[ --n ])->pSh == pSh )
        {
            pAppData_Impl->pDdeService->RemoveTopic( *pTopic );
            pAppData_Impl->pDocTopics->DeleteAndDestroy( n );
        }
}

const DdeService* SfxApplication::GetDdeService() const
{
    return pAppData_Impl->pDdeService;
}

DdeService* SfxApplication::GetDdeService()
{
    return pAppData_Impl->pDdeService;
}

//--------------------------------------------------------------------

BOOL ImplDdeService::MakeTopic( const String& rNm )
{
    // Workaround gegen Event nach unserem Main() unter OS/2
    // passierte wenn man beim Beenden aus dem OffMgr die App neu startet
    if ( !Application::IsInExecute() )
        return FALSE;

    // das Topic rNm wird gesucht, haben wir es ?
    // erstmal nur ueber die ObjectShells laufen und die mit dem
    // Namen heraussuchen:
    BOOL bRet = FALSE;
    String sNm( rNm );
    sNm.ToLowerAscii();
    TypeId aType( TYPE(SfxObjectShell) );
    SfxObjectShell* pShell = SfxObjectShell::GetFirst( &aType );
    while( pShell )
    {
        String sTmp( pShell->GetTitle(SFX_TITLE_FULLNAME) );
        sTmp.ToLowerAscii();
        if( sTmp == sNm )       // die wollen wir haben
        {
            SFX_APP()->AddDdeTopic( pShell );
            bRet = TRUE;
            break;
        }
        pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }

    if( !bRet )
    {
        INetURLObject aWorkPath( SvtPathOptions().GetWorkPath() );
        INetURLObject aFile;
        if ( aWorkPath.GetNewAbsURL( rNm, &aFile ) &&
             SfxContentHelper::IsDocument( aFile.GetMainURL() ) )
        {
            // File vorhanden

            // dann versuche die Datei zu laden:
            SfxStringItem aName( SID_FILE_NAME, aFile.GetMainURL() );
            SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, TRUE);
//          SfxBoolItem aHidden(SID_HIDDEN, TRUE);
            // minimiert!
            SfxUInt16Item aViewStat( SID_VIEW_ZOOM_MODE, 0 );
            SfxRectangleItem aRectItem( SID_VIEW_POS_SIZE, Rectangle() );

            SfxBoolItem aSilent(SID_SILENT, TRUE);
            SfxDispatcher* pDispatcher = SFX_APP()->GetDispatcher_Impl();
            const SfxPoolItem* pRet = pDispatcher->Execute( SID_OPENDOC,
                    SFX_CALLMODE_SYNCHRON,
                    &aName, &aNewView,
                    &aViewStat,&aRectItem/*aHidden*/,
                    &aSilent, 0L );

            if( pRet && pRet->ISA( SfxViewFrameItem ) &&
                ((SfxViewFrameItem*)pRet)->GetFrame() &&
                0 != ( pShell = ((SfxViewFrameItem*)pRet)
                    ->GetFrame()->GetObjectShell() ) )
            {
                SFX_APP()->AddDdeTopic( pShell );
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

String ImplDdeService::Topics()
{
    String sRet;
    if( GetSysTopic() )
        sRet += GetSysTopic()->GetName();

    TypeId aType( TYPE(SfxObjectShell) );
    SfxObjectShell* pShell = SfxObjectShell::GetFirst( &aType );
    while( pShell )
    {
        if( SfxViewFrame::GetFirst( pShell, TYPE(SfxTopViewFrame) ))
        {
            if( sRet.Len() )
                sRet += '\t';
            sRet += pShell->GetTitle(SFX_TITLE_FULLNAME);
        }
        pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }
    if( sRet.Len() )
        sRet += DEFINE_CONST_UNICODE("\r\n");
    return sRet;
}

BOOL ImplDdeService::SysTopicExecute( const String* pStr )
{
    return (BOOL)SFX_APP()->DdeExecute( *pStr );
}

//--------------------------------------------------------------------

BOOL SfxDdeTriggerTopic_Impl::Execute( const String* pStr )
{
//    Application::GetAppWindow()->ToTop(TOTOP_RESTOREWHENMIN);
    return TRUE;
}

//--------------------------------------------------------------------
DdeData* SfxDdeDocTopic_Impl::Get( ULONG nFormat )
{
    aSvData = SvData( nFormat );
    long nRet = pSh->DdeGetData( GetCurItem(), aSvData );
    if( nRet && aSvData.HasData() )
    {
        const char * pGetData;
        long nLen = aSvData.GetMemorySize();
        aSvData.GetData( (void**)&pGetData, TRANSFER_REFERENCE );
        aData = DdeData( pGetData, nLen, nFormat );
        return &aData;
    }
    return 0;
}

BOOL SfxDdeDocTopic_Impl::Put( const DdeData* pData )
{
    aSvData = SvData( pData->GetFormat() );
    aSvData.SetData( (void*)(const void*)*pData,
                    (long)*pData, TRANSFER_REFERENCE );

    long nRet = aSvData.HasData() ? pSh->DdeSetData( GetCurItem(), aSvData ) : 0;

    return 0 != nRet;
}

BOOL SfxDdeDocTopic_Impl::Execute( const String* pStr )
{
    long nRet = pStr ? pSh->DdeExecute( *pStr ) : 0;
    return 0 != nRet;
}

BOOL SfxDdeDocTopic_Impl::MakeItem( const String& rItem )
{
    AddItem( DdeItem( rItem ) );
    return TRUE;
}

BOOL SfxDdeDocTopic_Impl::StartAdviseLoop()
{
    SvPseudoObject* pNewObj = pSh->DdeCreateHotLink( GetCurItem() );
    if( !pNewObj )
        return FALSE;

    // dann richten wir auch einen entsprechenden SvBaseLink ein
    String sNm, sTmp( Application::GetAppName() );
    ::MakeLnkName( sNm, &sTmp, pSh->GetTitle(SFX_TITLE_FULLNAME), GetCurItem() );
    new SvBaseLink( sNm, OBJECT_DDE_EXTERN, pNewObj );
    return TRUE;
}


