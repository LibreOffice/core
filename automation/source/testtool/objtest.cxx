/*************************************************************************
 *
 *  $RCSfile: objtest.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:49:26 $
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

/*#include <com/sun/star/devtools/XIServerProxy.hpp>
#include <com/sun/star/devtools/XInformationClient.hpp>
using namespace com::sun::star::devtools;
*/

#include "sysdir_win.hxx"
#include "registry_win.hxx"

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#include <svtools/stringtransfer.hxx>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
//#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
//#endif
#include <com/sun/star/uno/Any.hxx>
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HXX_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XNamingService.hpp>

#include <cppuhelper/servicefactory.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::bridge;
using namespace com::sun::star::connection;
using namespace rtl;


#include <svtools/svmedit.hxx>
#ifdef OS2
#define INCL_DOSPROCESS
//#include <vcl/sysdep.hxx>
#ifndef _SVPM_H
#include <tools/svpm.h>
#endif
#endif

#ifdef UNX
#include <unistd.h> // readlink
#include <errno.h>
#endif

#include <basic/sbuno.hxx>

//#include <basic/basrid.hxx>

#ifndef _BASICRT_HXX
#include <basic/basicrt.hxx>
#endif
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include "tcommuni.hxx"

#ifndef _CRETSTRM_HXX
#include <cretstrm.hxx>
#endif

#include "objtest.hxx"
#include "rcontrol.hxx"

#include <basic/testtool.hrc>
#include <basic/ttmsg.hrc>

#include <basic/mybasic.hxx>
#include <basic/testtool.hxx>

#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif

#include <algorithm>

#ifndef SBX_VALUE_DECL_DEFINED
#define SBX_VALUE_DECL_DEFINED
SV_DECL_REF(SbxValue)
#endif
SV_IMPL_REF(SbxValue)


#define cMyDelim ' '
#define P_FEHLERLISTE pFehlerListe
#define KEEP_SEQUENCES      100         // Keep Names of last 100 Calls


ControlDefLoad __READONLY_DATA Controls::arClasses [] =
#include "classes.hxx"
CNames *Controls::pClasses = NULL;

ControlDefLoad __READONLY_DATA TestToolObj::arR_Cmds [] =
#include "r_cmds.hxx"
CNames *TestToolObj::pRCommands = NULL;

DBG_NAME( ControlItem )
DBG_NAME( ControlDef )

ControlItem::ControlItem( const sal_Char *Name, SmartId aUIdP )
{
DBG_CTOR(ControlItem,0);
    InitData();
    pData->Kurzname.AssignAscii( Name );
    pData->aUId = aUIdP;
}

ControlItem::ControlItem( const String &Name, SmartId aUIdP )
{
DBG_CTOR(ControlItem,0);
    InitData();
    pData->Kurzname = Name;
    pData->aUId = aUIdP;
}

/*ControlItem::ControlItem( const String &Name, const String &URL, const URLType aType )
{
DBG_CTOR(ControlItem,0);
    InitData();
    pData->Kurzname = Name;
    pData->aURL = URL;
    pData->nUId = aType;
}

ControlItem::ControlItem( const String &Name, const String &URL, const ULONG nUId )
{
DBG_CTOR(ControlItem,0);
    InitData();
    pData->Kurzname = Name;
    pData->aURL = URL;
    pData->nUId = nUId;
}

ControlItem::ControlItem( const char *Name, const String &URL, const ULONG nUId )
{
DBG_CTOR(ControlItem,0);
    InitData();
    pData->Kurzname.AssignAscii( Name );
    pData->aURL = URL;
    pData->nUId = nUId;
} */


ControlItem::ControlItem( ControlData *pDataP )
{
DBG_CTOR(ControlItem,0);
    pData = pDataP;
}

ControlSon::~ControlSon()
{
    if (pSons)
    {
        delete pSons;
        pSons = NULL;
    }
}

ControlItemSon::ControlItemSon(const String &Name, SmartId aUIdP )
: ControlItem( Name, aUIdP )
{}

/*ControlItemSon::ControlItemSon(const String &Name, const String &URL, const URLType aType )
: ControlItem( Name, URL, aType )
{}

ControlItemSon::ControlItemSon(const String &Name, const String &URL, const ULONG nUId )
: ControlItem( Name, URL, nUId )
{}

ControlItemSon::ControlItemSon(const char *Name, const String &URL, const ULONG nUId )
: ControlItem( Name, URL, nUId )
{}*/



BOOL ControlDef::operator < (const ControlItem &rPar)
{
    return pData->Kurzname.CompareIgnoreCaseToAscii(rPar.pData->Kurzname) == COMPARE_LESS;
}

BOOL ControlDef::operator == (const ControlItem &rPar)
{
    return pData->Kurzname.CompareIgnoreCaseToAscii(rPar.pData->Kurzname) == COMPARE_EQUAL;
}

void ControlDef::Write( SvStream &aStream )
{
    if ( pSons )
        aStream.WriteByteString( String('*').Append( pData->Kurzname ), RTL_TEXTENCODING_UTF8 );
    else
        aStream.WriteByteString( pData->Kurzname, RTL_TEXTENCODING_UTF8 );
    aStream << ((USHORT)pData->aUId.IsNumeric());
    if ( !pData->aUId.IsNumeric() )
        aStream.WriteByteString( pData->aUId.GetStr(), RTL_TEXTENCODING_UTF8 );
    else
        aStream << pData->aUId.GetNum();
    if ( pSons )
        for ( int i = 0 ; pSons->Count() > i ; i++ )
            ((ControlDef*)(*pSons)[i])->Write(aStream);
}

ControlDef::ControlDef(const String &Name, SmartId aUIdP )
: ControlItemSon( Name, aUIdP)
{
    DBG_CTOR(ControlDef,0);
}

/*ControlDef::ControlDef(const String &Name, const String &URL, const URLType aType )
: ControlItemSon( Name, URL, aType )
{
    DBG_CTOR(ControlDef,0);
} */

ControlDef::ControlDef(const String &aOldName, const String &aNewName, ControlDef *pOriginal, BOOL bWithSons )
: ControlItemSon("", pOriginal->pData->aUId)
{
    DBG_CTOR(ControlDef,0);
    if ( pOriginal->pData->Kurzname == aOldName )
        pData->Kurzname = aNewName;
    else
        pData->Kurzname = pOriginal->pData->Kurzname;

    if ( bWithSons && pOriginal->pSons )
    {
        pSons = new CNames();
        for ( USHORT i = 0; i < pOriginal->pSons->Count() ; i++)
        {
            ControlDef *pNewDef;
            pNewDef = new ControlDef( aOldName, aNewName, pOriginal->SonGetObject(i) ,TRUE );
            if (! SonInsert(pNewDef))
            {
                DBG_ERROR("Name Doppelt im CopyConstructor. Neuer Name = Controlname!!");
                delete pNewDef;
            }
        }

    }
    else
        pSons = NULL;
}

BOOL ControlItemUId::operator < (const ControlItem &rPar)
{
    return pData->aUId < rPar.pData->aUId;
}

BOOL ControlItemUId::operator == (const ControlItem &rPar)
{
    return pData->aUId == rPar.pData->aUId;
}

SV_IMPL_OP_PTRARR_SORT( CNames, ControlItem* )

void CRevNames::Insert( String aName, SmartId aUId, ULONG nSeq )
{
    ControlItem *pRN = new ReverseName(aName,aUId,nSeq);
    USHORT nPos;
    if ( Seek_Entry(pRN,&nPos) )
        DeleteAndDestroy(nPos);

    if ( !CNames::C40_PTR_INSERT( ControlItem, pRN) )
    {
        DBG_ERROR("Interner Fehler beim Speichern der Lokalen KurzNamen");
        delete pRN;
    }

}

String CRevNames::GetName( SmartId aUId )
{
    ReverseName *pRN = new ReverseName(UniString(),aUId,0);
    USHORT nPos;
    BOOL bSeekOK = Seek_Entry(pRN,&nPos);

    delete pRN;
    if ( bSeekOK )
        return GetObject(nPos)->pData->Kurzname;
    else
    {
        if ( aUId.Equals( UID_ACTIVE ) )
            return CUniString("Active");
        else
            return GEN_RES_STR1( S_NAME_NOT_THERE, aUId.GetText() );
    }
}

void CRevNames::Invalidate ( ULONG nSeq )
{
    USHORT i;
    for (i = 0; i < Count() ;)
    {
        if (((ReverseName*)GetObject(i))->LastSequence < nSeq)
            DeleteAndDestroy(i);
        else
            i++;
    }
}


SV_IMPL_PTRARR(CErrors, ErrorEntry*)


SbxTransportMethod::SbxTransportMethod( SbxDataType DT )
: SbxMethod(CUniString("Dummy"),DT)
{
    nValue = 0;
}


TestToolObj::TestToolObj( String aName, String aFilePath )              // Interner Aufruf
: SbxObject( aName )
, pControls(NULL)
, pReverseSlots(NULL)
, pReverseControls(NULL)
, pReverseControlsSon(NULL)
, pReverseUIds(NULL)
, pSIds(NULL)
, pCommunicationManager(NULL)
, bUseIPC(FALSE)
, IsBlock(FALSE)
, SingleCommandBlock(TRUE)
//, bQuietErrors(TRUE)
, bQuietErrors(FALSE)
, bReturnOK(TRUE)
, nSequence(KEEP_SEQUENCES)
, ProgPath()
, nIdleCount(0)
, nWindowHandlerCallLevel(0)
, aDialogHandlerName()
, pFehlerListe(NULL)
{
    pImpl = new ImplTestToolObj;
    pImpl->ProgParam = String();
    pImpl->aFileBase = DirEntry(aFilePath);
//  pImpl->aLogFileBase = DirEntry();
    pImpl->aHIDDir = DirEntry(aFilePath);
    pImpl->bIsStart = FALSE;
    pImpl->pMyBasic = NULL;

    pImpl->aServerTimeout = Time(0,1,00);           // 1:00 Minuten fest
    InitTestToolObj();
}

TestToolObj::TestToolObj( String aName, MyBasic* pBas )                // Aufruf im Testtool
: SbxObject( aName )
, pControls(NULL)
, pReverseSlots(NULL)
, pReverseControls(NULL)
, pReverseControlsSon(NULL)
, pReverseUIds(NULL)
, pSIds(NULL)
, pCommunicationManager(NULL)
, bUseIPC(TRUE)
, IsBlock(FALSE)
, SingleCommandBlock(TRUE)
//, bQuietErrors(TRUE)
, bQuietErrors(FALSE)
, bReturnOK(TRUE)
, nSequence(KEEP_SEQUENCES)
, ProgPath()
, nIdleCount(0)
, nWindowHandlerCallLevel(0)
, aDialogHandlerName()
, pFehlerListe(NULL)
{
    pImpl = new ImplTestToolObj;
    pImpl->ProgParam = String();
    pImpl->bIsStart = FALSE;
    pImpl->pMyBasic = pBas;

    LoadIniFile();
    InitTestToolObj();

    pCommunicationManager = new CommunicationManagerClientViaSocketTT();
    pCommunicationManager->SetDataReceivedHdl( LINK( this, TestToolObj, ReturnResultsLink ));
}

void TestToolObj::LoadIniFile()             // Laden der IniEinstellungen, die durch den ConfigDialog geändert werden können
{
#define GETSET(aVar, KeyName, Dafault)                          \
    {                                                           \
        ByteString __##aVar##__;                                \
        __##aVar##__ = aConf.ReadKey(KeyName);      \
        if ( !__##aVar##__.Len() )                  \
        {                                                       \
            __##aVar##__ = Dafault;                             \
            aConf.WriteKey(KeyName, __##aVar##__);              \
        }                                                       \
        aVar = UniString( __##aVar##__, RTL_TEXTENCODING_UTF8 );\
    }

#define NEWOLD( NewKey, OldKey )                                                                \
    {                                                                                           \
        ByteString aValue;                                                                      \
        if ( ( (aValue = aConf.ReadKey( OldKey )).Len() ) && !aConf.ReadKey( NewKey ).Len() )   \
            aConf.WriteKey( NewKey, aValue );                                                   \
    }


    Config aConf(Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ));
    aConf.SetGroup("Misc");
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );

    NEWOLD( "BaseDir", "Basisverzeichnis" )
    String aFB;
    GETSET( aFB, "BaseDir", "" );
    pImpl->aFileBase = DirEntry(aFB);

    // remove old keys
    if ( aConf.ReadKey("KeyCodes + Classes").Len() != 0 ||
         aConf.ReadKey("KeyCodes + Classes + Res_Type").Len() != 0 )
    {
        aConf.DeleteKey("KeyCodes + Classes + Res_Type");
        aConf.DeleteKey("KeyCodes + Classes");
    }

    NEWOLD( "LogBaseDir", "LogBasisverzeichnis" )
    String aLFB;
    GETSET( aLFB, "LogBaseDir", ByteString( aFB, RTL_TEXTENCODING_UTF8 ) );
    pImpl->aLogFileBase = DirEntry(aLFB);

    NEWOLD( "HIDDir", "HIDVerzeichnis" )
    String aHID;
    GETSET( aHID, "HIDDir", "" );
    pImpl->aHIDDir = DirEntry(aHID);


    aConf.SetGroup("Misc");

    String aST;
    GETSET( aST, "ServerTimeout", ByteString::CreateFromInt64(Time(0,0,45).GetTime()) );     // 45 Sekunden Initial
    pImpl->aServerTimeout = Time(ULONG(aST.ToInt64()));

    String aSOSE;
    aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Misc" );
    aConf.SetGroup( aCurrentProfile );
    GETSET( aSOSE, "StopOnSyntaxError", "0" );
    pImpl->bStopOnSyntaxError = aSOSE.EqualsAscii("1");


    aConf.SetGroup("GUI Platform");

    String aGP;
    ByteString abGP;
#ifdef WNT
    abGP.Append( "501" );  // WinXP
#elif defined SOLARIS && defined SPARC
    abGP.Append( "01" );  // Solaris SPARC
#elif defined SOLARIS && defined INTEL
    abGP.Append( "05" );  // Solaris x86
#elif defined LINUX
    abGP.Append( "03" );  // Linux
#endif
    GETSET( aGP, "Current", abGP );
}

#define MAKE_TT_KEYWORD( cName, aType, aResultType, nID ) \
{ \
    SbxVariableRef pMeth; \
    pMeth = Make( CUniString(cName), aType, aResultType ); \
    pMeth->SetUserData( nID ); \
}

#define MAKE_REMOTE_COMMAND(aName, aId)                                                                         \
    {\
        SbxTransportMethod *pMeth = new SbxTransportMethod( SbxVARIANT );       \
        pMeth->SetName( aName );                                                                                        \
        pMeth->SetUserData( ID_RemoteCommand );                                                         \
        pMeth->nValue = aId.GetNum();                                                                                            \
        Insert( pMeth );                                                                                                        \
        StartListening( pMeth->GetBroadcaster(), TRUE );                                        \
    }

// SetUserData muß irgendwas sein, sonst wird es im Find rausgefiltert!!!
#define MAKE_USHORT_CONSTANT(cName, nValue)                                                                         \
    {\
        SbxProperty *pVal = new SbxProperty( CUniString( cName) , SbxINTEGER );       \
        pVal->PutInteger( nValue ) ;                                                                                            \
        pVal->SetUserData( 32000 );                                                         \
        Insert( pVal );                                                                                                        \
    }

#define RTLNAME "@SBRTL"    // copied from basic/source/classes/sb.cxx

void TestToolObj::InitTestToolObj()
{
    pImpl->nNumBorders = 0;                 // Für Profiling mit kästchen

    pImpl->nMinRemoteCommandDelay = 0;
    pImpl->nMaxRemoteCommandDelay = 0;
    pImpl->bDoRemoteCommandDelay = FALSE;

    pImpl->bLnaguageExtensionLoaded= FALSE;
    pImpl->pTTSfxBroadcaster = NULL;

    pImpl->nErrorCount = 0;
    pImpl->nWarningCount = 0;
    pImpl->nQAErrorCount = 0;
    pImpl->nIncludeFileWarningCount = 0;

    pImpl->xErrorList = new SbxDimArray( SbxSTRING );
    pImpl->xWarningList = new SbxDimArray( SbxSTRING );
    pImpl->xQAErrorList = new SbxDimArray( SbxSTRING );
    pImpl->xIncludeFileWarningList = new SbxDimArray( SbxSTRING );

    pImpl->nTestCaseLineNr = 0;

    pImpl->bEnableQaErrors = TRUE;
    pImpl->bDebugFindNoErrors = FALSE;

    pImpl->pChildEnv = new Environment;

    pFehlerListe = new CErrors;             // Vor allem anderen. Wer weiss, wer alles einen Fehler auslöst.

    In = new CmdStream();

    pShortNames = new CRevNames;


// overwrite standard "wait" method, cause we can do better than that!!
// Insert Object into SbiStdObject but change listening.
    SbxVariable* pRTL = pImpl->pMyBasic->Find( CUniString(RTLNAME), SbxCLASS_DONTCARE );
    SbxObject* pRTLObject = PTR_CAST( SbxObject, pRTL );
    if ( pRTLObject )
    {
        SbxVariableRef pWait;
        pWait = pRTLObject->Make( CUniString("Wait"), SbxCLASS_METHOD, SbxNULL );
        pWait->SetUserData( ID_Wait );
    // change listener here
        pRTLObject->EndListening( pWait->GetBroadcaster(), TRUE );
        StartListening( pWait->GetBroadcaster(), TRUE );
    }
    else
    {
        DBG_ERROR("Testtool: Could not replace Wait method")
    }

    MAKE_TT_KEYWORD( "Kontext", SbxCLASS_METHOD, SbxNULL, ID_Kontext );
    MAKE_TT_KEYWORD( "GetNextError", SbxCLASS_VARIABLE, SbxVARIANT, ID_GetError );
    MAKE_TT_KEYWORD( "Start", SbxCLASS_METHOD, SbxSTRING, ID_Start );
//      pMeth = Make( "Kill", SbxCLASS_METHOD, SbxNULL );
//      pMeth->SetUserData( ID_Kill );
    /*      pMeth = Make( "TestReset", SbxCLASS_METHOD, SbxNULL );
        pMeth->SetUserData( ID_Reset );*/
    MAKE_TT_KEYWORD( "Use", SbxCLASS_METHOD, SbxNULL, ID_Use );
    MAKE_TT_KEYWORD( "StartUse", SbxCLASS_METHOD, SbxNULL, ID_StartUse );
    MAKE_TT_KEYWORD( "FinishUse", SbxCLASS_METHOD, SbxNULL, ID_FinishUse );

    MAKE_TT_KEYWORD( "CaseLog", SbxCLASS_METHOD, SbxNULL, ID_CaseLog );
    MAKE_TT_KEYWORD( "ExceptLog", SbxCLASS_METHOD, SbxNULL, ID_ExceptLog );
    MAKE_TT_KEYWORD( "PrintLog", SbxCLASS_METHOD, SbxNULL, ID_PrintLog );
    MAKE_TT_KEYWORD( "WarnLog", SbxCLASS_METHOD, SbxNULL, ID_WarnLog );
    MAKE_TT_KEYWORD( "ErrorLog", SbxCLASS_METHOD, SbxNULL, ID_ErrorLog );
    MAKE_TT_KEYWORD( "QAErrorLog", SbxCLASS_METHOD, SbxNULL, ID_QAErrorLog );
    MAKE_TT_KEYWORD( "EnableQaErrors", SbxCLASS_PROPERTY, SbxBOOL, ID_EnableQaErrors );
    MAKE_TT_KEYWORD( "MaybeAddErr", SbxCLASS_METHOD, SbxNULL, ID_MaybeAddErr );
    MAKE_TT_KEYWORD( "ClearError", SbxCLASS_METHOD, SbxNULL, ID_ClearError );
    MAKE_TT_KEYWORD( "SaveIDs", SbxCLASS_METHOD, SbxBOOL, ID_SaveIDs );
    MAKE_TT_KEYWORD( "AutoExecute", SbxCLASS_PROPERTY, SbxBOOL, ID_AutoExecute );   // Achtung! PROPERTY Also eine Variable
    MAKE_TT_KEYWORD( "Execute", SbxCLASS_METHOD, SbxNULL, ID_Execute );
    MAKE_TT_KEYWORD( "StopOnSyntaxError", SbxCLASS_PROPERTY, SbxBOOL, ID_StopOnSyntaxError );

/*  Dialog Handler werden gebraucht, wenn im internen Testtool ein Dialog
    hochgerissen wird. Nach versenden der Remote-Kommandos wird IdleHandler aktiviert.
    Er testet, ob das Reschedule zum WaitForAnswer zurückkehrt. Bleibt das aus, so
    wird erst der RemoteHandler zurückgesetzt und dann die Handler-Sub im Basic
    gerufen.(Entkoppelt über PostUserEvent.)

    In returndaten_verarbeiten wird flag für ausführung des nächsten remote-befehls
    rückgesetzt. Der Handler wird damit auch entwertet. Er gilt also nur für den
    nächsten Remotebefehl.
*/
    MAKE_TT_KEYWORD( "DialogHandler", SbxCLASS_METHOD, SbxNULL, ID_DialogHandler );

    MAKE_TT_KEYWORD( "GetUnoApp", SbxCLASS_METHOD, SbxOBJECT, ID_GetUnoApp );
    MAKE_TT_KEYWORD( "GetIServer", SbxCLASS_METHOD, SbxOBJECT, ID_GetIServer );

    MAKE_TT_KEYWORD( "RemoteCommandDelay", SbxCLASS_METHOD, SbxNULL, ID_RemoteCommandDelay );

       MAKE_TT_KEYWORD( "GetApplicationPath", SbxCLASS_METHOD, SbxSTRING, ID_GetApplicationPath );
       MAKE_TT_KEYWORD( "GetCommonApplicationPath", SbxCLASS_METHOD, SbxSTRING, ID_GetCommonApplicationPath );
       MAKE_TT_KEYWORD( "MakeIniFileName", SbxCLASS_METHOD, SbxSTRING, ID_MakeIniFileName );

/// active constants returning error and warning count
    MAKE_TT_KEYWORD( "GetErrorCount", SbxCLASS_METHOD, SbxULONG, ID_GetErrorCount );
    MAKE_TT_KEYWORD( "GetWarningCount", SbxCLASS_METHOD, SbxULONG, ID_GetWarningCount );
    MAKE_TT_KEYWORD( "GetQAErrorCount", SbxCLASS_METHOD, SbxULONG, ID_GetQAErrorCount );
    MAKE_TT_KEYWORD( "GetUseFileWarningCount", SbxCLASS_METHOD, SbxULONG, ID_GetUseFileWarningCount );

    MAKE_TT_KEYWORD( "GetErrorList", SbxCLASS_METHOD, SbxOBJECT, ID_GetErrorList );
    MAKE_TT_KEYWORD( "GetWarningList", SbxCLASS_METHOD, SbxOBJECT, ID_GetWarningList );
    MAKE_TT_KEYWORD( "GetQAErrorList", SbxCLASS_METHOD, SbxOBJECT, ID_GetQAErrorList );
    MAKE_TT_KEYWORD( "GetUseFileWarningList", SbxCLASS_METHOD, SbxOBJECT, ID_GetUseFileWarningList );

    MAKE_TT_KEYWORD( "GetTestCaseName", SbxCLASS_METHOD, SbxSTRING, ID_GetTestCaseName );
    MAKE_TT_KEYWORD( "GetTestCaseFileName", SbxCLASS_METHOD, SbxSTRING, ID_GetTestCaseFileName );
    MAKE_TT_KEYWORD( "GetTestCaseLineNr", SbxCLASS_METHOD, SbxUSHORT, ID_GetTestCaseLineNr );

    MAKE_TT_KEYWORD( "SetChildEnv", SbxCLASS_METHOD, SbxNULL, ID_SetChildEnv );
    MAKE_TT_KEYWORD( "GetChildEnv", SbxCLASS_METHOD, SbxSTRING, ID_GetChildEnv );

    MAKE_TT_KEYWORD( "GetLinkDestination", SbxCLASS_METHOD, SbxSTRING, ID_GetLinkDestination );
    MAKE_TT_KEYWORD( "GetRegistryValue", SbxCLASS_METHOD, SbxSTRING, ID_GetRegistryValue );

    // Load the Remote Commands from list
    if ( !pRCommands )                 // Ist static, wird also nur einmal geladen
        ReadFlatArray( arR_Cmds, pRCommands );
    USHORT i;
    for ( i = 0 ; i < pRCommands->Count() ; i++ )
        MAKE_REMOTE_COMMAND( pRCommands->GetObject( i )->pData->Kurzname, pRCommands->GetObject( i )->pData->aUId );


// Konstanten für SetControlType
    MAKE_USHORT_CONSTANT("CTBrowseBox",CONST_CTBrowseBox);
    MAKE_USHORT_CONSTANT("CTValueSet",CONST_CTValueSet);

// Konstanten für das Alignment des gesuchten Splitters
    MAKE_USHORT_CONSTANT("AlignLeft",CONST_ALIGN_LEFT);
    MAKE_USHORT_CONSTANT("AlignTop",CONST_ALIGN_TOP);
    MAKE_USHORT_CONSTANT("AlignRight",CONST_ALIGN_RIGHT);
    MAKE_USHORT_CONSTANT("AlignBottom",CONST_ALIGN_BOTTOM);

/// What dialog to use in RC_CloseSysDialog or RC_ExistsSysDialog
    MAKE_USHORT_CONSTANT("FilePicker",CONST_FilePicker);
    MAKE_USHORT_CONSTANT("FolderPicker",CONST_FolderPicker);

/// NodeTypes of the SAX Parser
    MAKE_USHORT_CONSTANT("NodeTypeCharacter",CONST_NodeTypeCharacter);
    MAKE_USHORT_CONSTANT("NodeTypeElement",CONST_NodeTypeElement);
    MAKE_USHORT_CONSTANT("NodeTypeComment",CONST_NodeTypeComment);


    pImpl->pControlsObj = new Controls( CUniString("GetNextCloseWindow") );
    pImpl->pControlsObj -> SetType( SbxVARIANT );
    pImpl->pControlsObj->SetUserData( ID_GetNextCloseWindow );
    Insert( pImpl->pControlsObj );                         // Hier so umständlich wegen Compilerfehlers
    pImpl->pControlsObj->ChangeListener();

    for ( i=0;i<VAR_POOL_SIZE;i++)
    {
/*              pMyVar = new SbxObject( "Dummy" );
        pMyVar -> SetType( SbxVARIANT );*/

//           pMyVar = new SbxMethod( "Dummy", SbxVARIANT );

        pImpl->pMyVars[i] = new SbxTransportMethod( SbxVARIANT );
        pImpl->pMyVars[i] -> SetName( CUniString("VarDummy").Append(String::CreateFromInt32(i) ) );

        Insert( pImpl->pMyVars[i] );
//              StartListening( pMyVars[i]->GetBroadcaster(), TRUE );

        pImpl->pControlsObj = new Controls( CUniString("ControlDummy").Append(String::CreateFromInt32(i) ) );
        pImpl->pControlsObj -> SetType( SbxVARIANT );
        Insert( pImpl->pControlsObj );                         // Hier so umständlich wegen Compilerfehlers
        pImpl->pControlsObj->ChangeListener();
        pImpl->pControlsObjs[i] = pImpl->pControlsObj;
    }

    pControls = new CNames();
    pSIds = new CNames();
    pNameKontext = pControls;

    nMyVar = 0;
    nControlsObj = 0;


// Das ist zum testen des IPC

/*      int sent = 0;

    ModelessDialog *pDlg = new ModelessDialog(NULL);
    pDlg->SetOutputSizePixel(Size(100,30));

    Edit *pMyEd = new Edit(pDlg,WB_CENTER | WB_BORDER);
    pMyEd->SetSizePixel(Size(100,30));
    pDlg->Show();
    pMyEd->Show();
    Time aTime;

    String VollePackung;
    VollePackung.Fill(32760,'*');

    BeginBlock();   // zum warm werden
    EndBlock();
    ResetError();

    while ( pDlg->IsVisible() && !IsError() )
    {
        BeginBlock();
        In->GenCmdFlow (124,VollePackung);
        EndBlock();
        pMyEd->SetText(String("Test Nr. ") + String(++sent));
        while ( aTime.Get100Sec() / 10 == Time().Get100Sec() / 10 );
        aTime = Time();
    }

    delete pMyEd;
    delete pDlg;
*/
// Test ende


}

TestToolObj::~TestToolObj()
{
    EndListening( ((StarBASIC*)GetParent())->GetBroadcaster() );
    pImpl->pNextReturn.Clear();

    pImpl->pControlsObj.Clear();

    for ( int i = 0 ; i < VAR_POOL_SIZE ; i++ )
    {
        pImpl->pMyVars[i].Clear();
        pImpl->pControlsObjs[i].Clear();
    }

    if (pControls)
        delete pControls;
    if (pReverseSlots)
        delete pReverseSlots;
    if (pReverseControls)
        delete pReverseControls;
    if (pReverseControlsSon)
        delete pReverseControlsSon;
    if (pReverseUIds)
        delete pReverseUIds;
    if (pSIds)
        delete pSIds;
    if (pFehlerListe)
    {
        delete pFehlerListe;
        pFehlerListe = NULL;    // da pFehlerListe static ist!!
    }
    if ( pCommunicationManager )
    {
        pCommunicationManager->StopCommunication();
        delete pCommunicationManager;
    }
    delete In;
    if ( pImpl->pTTSfxBroadcaster )
        delete pImpl->pTTSfxBroadcaster;
    delete pShortNames;
    delete pImpl;

    delete pImpl->pChildEnv;
}

SfxBroadcaster& TestToolObj::GetTTBroadcaster()
{
    if ( !pImpl->pTTSfxBroadcaster )
        pImpl->pTTSfxBroadcaster = new SfxBroadcaster;
    return *pImpl->pTTSfxBroadcaster;
}

void TestToolObj::ReadNames( String Filename, CNames *&pNames, CNames *&pUIds, BOOL bIsFlat )
{
/*******************************************************************************
**
**               Folgende Dateiendungen sind vorhanden
**
**               hid.lst                Langname UId
**               *.sid                  Slot Ids Kurzname Langname Datei ist flach
**               *.win                  Controlname Langname Datei mit *name und +name Notation
**
**
*******************************************************************************/


    SvFileStream Stream;
    String       aLine,aName,aLongname;
    SmartId      aUId;
    xub_StrLen   nLineNr;
    USHORT       nElement;
    ControlDef   *pNewDef, *pNewDef2;
    ControlDef   *pFatherDef = NULL;

    nLineNr = 0;    // Wir sind ja noch vor der Datei

    if (! pUIds)
    {
        String aName = (pImpl->aHIDDir + DirEntry(CUniString("hid.lst"))).GetFull();
        {
            TTExecutionStatusHint aHint( TT_EXECUTION_SHOW_ACTION, String(ResId(S_READING_LONGNAMES)), aName );
            GetTTBroadcaster().Broadcast( aHint );
        }
        ReadFlat( aName ,pUIds, TRUE );
        if ( !pUIds )
            return;
        pNewDef = new ControlDef("Active",SmartId(0));
        if (! pUIds->C40_PTR_INSERT(ControlItem, (ControlItem*&)pNewDef))
        {
            ADD_WARNING_LOG2( GEN_RES_STR1c( S_DOUBLE_NAME, "Active" ), Filename, nLineNr );
            delete pNewDef;
        }

    }

    ADD_MESSAGE_LOG( Filename );

    Stream.Open(Filename, STREAM_STD_READ);
    if (!Stream.IsOpen())
    {
        ADD_ERROR(ERR_NO_FILE,GEN_RES_STR1(S_CANNOT_OPEN_FILE, Filename));
        return;
    }

    if ( bIsFlat && !pNames )
    {
        TTExecutionStatusHint aHint( TT_EXECUTION_SHOW_ACTION, String(ResId(S_READING_SLOT_IDS)), Filename );
        GetTTBroadcaster().Broadcast( aHint );
    }
    else
    {
        TTExecutionStatusHint aHint( TT_EXECUTION_SHOW_ACTION, String(ResId(S_READING_CONTROLS)), Filename );
        GetTTBroadcaster().Broadcast( aHint );
    }

    if ( !pNames )
        pNames = new CNames();

    {
        TTExecutionStatusHint aHint( TT_EXECUTION_ENTERWAIT );
        GetTTBroadcaster().Broadcast( aHint );
    }
    while (!Stream.IsEof())
    {
        nLineNr++;

        Stream.ReadByteStringLine(aLine, RTL_TEXTENCODING_IBM_850);
        aLine.EraseLeadingChars();
        aLine.EraseTrailingChars();
        while ( aLine.SearchAscii("  ") != STRING_NOTFOUND )
            aLine.SearchAndReplaceAllAscii("  ",UniString(' '));
        if (aLine.Len() == 0) continue;
        if (aLine.Copy(0,4).CompareIgnoreCaseToAscii("Rem ") == COMPARE_EQUAL) continue;
        if (aLine.Copy(0,1).CompareToAscii("'") == COMPARE_EQUAL) continue;

        if ( (aLine.GetTokenCount(cMyDelim) < 2 || aLine.GetTokenCount(cMyDelim) > 3) && aLine.CompareIgnoreCaseToAscii("*Active") != COMPARE_EQUAL )
        {
            ADD_WARNING_LOG2( GEN_RES_STR1( S_INVALID_LINE, aLine ), Filename, nLineNr );
            continue;
        }

        aName = aLine.GetToken(0,cMyDelim);
        aLongname = aLine.GetToken(1,cMyDelim);

        String aFirstAllowedExtra, aAllowed;
        aFirstAllowedExtra.AssignAscii("+*");
        aAllowed.AssignAscii("_");
        xub_StrLen nIndex = 0;
        BOOL bOK = TRUE;

        while ( bOK && nIndex < aName.Len() )
        {
            sal_Unicode aChar = aName.GetChar( nIndex );
            BOOL bOKThis = FALSE;
            bOKThis |= ( aAllowed.Search( aChar ) != STRING_NOTFOUND );
            if ( !nIndex )
                bOKThis |= ( aFirstAllowedExtra.Search( aChar ) != STRING_NOTFOUND );
            bOKThis |= ( aChar >= 'A' && aChar <= 'Z' );
            bOKThis |= ( aChar >= 'a' && aChar <= 'z' );
            bOKThis |= ( aChar >= '0' && aChar <= '9' );

            bOK &= bOKThis;
            nIndex++;
        }
        if ( !bOK )
        {
            ADD_WARNING_LOG2( CUniString("Zeile \"").Append(aLine).AppendAscii("\" enthält ungültige Zeichen."), Filename, nLineNr );
            continue;
        }

        BOOL bUnoName = ( aLongname.Copy( 0, 5 ).EqualsIgnoreCaseAscii( ".uno:" )
            || aLongname.Copy( 0, 4 ).EqualsIgnoreCaseAscii( "http" )
            || aLongname.Copy( 0, 15 ).EqualsIgnoreCaseAscii( "private:factory" )
            || aLongname.Copy( 0, 8 ).EqualsIgnoreCaseAscii( "service:" )
            || aLongname.Copy( 0, 6 ).EqualsIgnoreCaseAscii( "macro:" )
            || aLongname.Copy( 0, 8 ).EqualsIgnoreCaseAscii( ".HelpId:" ) );
        BOOL bMozillaName = ( !bIsFlat && aLongname.Copy( 0, 4 ).EqualsIgnoreCaseAscii( ".moz" ) );

        if ( aName.GetChar(0) == '+' )          // Kompletten Eintrag kopieren
        {
            aName.Erase(0,1);
            ControlDef WhatName(aLongname,SmartId());
            ControlDef *OldTree;
            if (pNames->Seek_Entry(&WhatName,&nElement))
            {
                OldTree = (ControlDef*)pNames->GetObject(nElement);
                pNewDef = new ControlDef(aLongname,aName,OldTree,TRUE);

                if (! pNames->C40_PTR_INSERT(ControlItem, (ControlItem*&)pNewDef))
                {
                    ADD_WARNING_LOG2( GEN_RES_STR1( S_DOUBLE_NAME, aLine ), Filename, nLineNr );
                    delete pNewDef;
                    pFatherDef = NULL;
                }
                else
                {
                    pFatherDef = pNewDef;
                }
            }
            else
            {
                ADD_WARNING_LOG2( GEN_RES_STR1( S_SHORTNAME_UNKNOWN, aLine ), Filename, nLineNr );
                continue;
            }

        }
        else
        {

            if (aName.CompareIgnoreCaseToAscii("*Active") == COMPARE_EQUAL)
                aUId = SmartId( UID_ACTIVE );
            else if ( !bUnoName && !bMozillaName )
            {   // Bestimmen der ID aus der Hid.Lst
                ControlDef WhatName(aLongname,SmartId());
                if (pUIds->Seek_Entry(&WhatName,&nElement))
                    aUId = pUIds->GetObject(nElement)->pData->aUId;
                else
                {
                    ADD_WARNING_LOG2( GEN_RES_STR1( S_LONGNAME_UNKNOWN, aLine ), Filename, nLineNr );
                    continue;
                }
            }
            else
            {
                if ( bUnoName )
                    aUId = SmartId( aLongname );
                else if ( bMozillaName )
                    aUId = SmartId( aLongname );
                else
                {
                    DBG_ERROR("Unknown URL schema")
                }
            }



            if (aName.GetChar(0) == '*' || bIsFlat)     // Globaler Kurzname (Dialogname oder SId)
            {
                if (!bIsFlat)
                    aName.Erase(0,1);

                   pNewDef = new ControlDef(aName,aUId);

                if (!bIsFlat)
                {
                    pNewDef->Sons( new CNames() );

                       pNewDef2 = new ControlDef(aName,aUId);
                    if (!pNewDef->SonInsert( pNewDef2 ))         // Dialog in eigenen Namespace eintragen
                    {
                        delete pNewDef2;
                        DBG_ERROR(" !!!! ACHTUNG !!!!  Fehler beim einfügen in leere Liste!");
                    }
                }

                if (! pNames->C40_PTR_INSERT(ControlItem, (ControlItem*&)pNewDef))
                {
                    ADD_WARNING_LOG2( GEN_RES_STR1( S_DOUBLE_NAME, aLine ), Filename, nLineNr );
                    delete pNewDef;
                    pFatherDef = NULL;
                }
                else
                {
                    pFatherDef = pNewDef;
                }
            }
            else
            {
                if (!pFatherDef)
                {
                    ADD_WARNING_LOG2( GEN_RES_STR0( S_FIRST_SHORTNAME_REQ_ASTRX ), Filename, nLineNr );
                }
                else
                {
                    pNewDef = new ControlDef(aName,aUId);
                    if (! pFatherDef->SonInsert(pNewDef))
                    {
                        ADD_WARNING_LOG2( GEN_RES_STR1( S_DOUBLE_NAME, aLine ), Filename, nLineNr );
                        delete pNewDef;
                    }
                }
            }
        }
        GetpApp()->Reschedule();
    }
    {
        TTExecutionStatusHint aHint( TT_EXECUTION_LEAVEWAIT );
        GetTTBroadcaster().Broadcast( aHint );
    }
    {
        TTExecutionStatusHint aHint( TT_EXECUTION_HIDE_ACTION );
        GetTTBroadcaster().Broadcast( aHint );
    }

    Stream.Close();
}


void TestToolObj::AddName(String &aBisher, String &aNeu )
{
    String aSl( '/' );
    if ( UniString(aSl).Append(aBisher).Append(aSl).ToUpperAscii().Search( UniString(aSl).Append(aNeu).Append(aSl).ToUpperAscii() ) == STRING_NOTFOUND )
    {
        aBisher += aSl;
        aBisher += aNeu;
    }
}


void TestToolObj::ReadFlat( String Filename, CNames *&pNames, BOOL bSortByName )
//  Wenn bSortByName == FALSE, dann nach UId Sortieren (ControlItemUId statt ControlDef)
{
    SvFileStream Stream;
    String       aLine,aName;
    SmartId      aUId;
    xub_StrLen   nLineNr;
    ControlItem  *pNewItem;
    USHORT       nDoubleCount = 0;

    Stream.Open(Filename, STREAM_STD_READ);

    if (!Stream.IsOpen())
    {
        ADD_ERROR(ERR_NO_FILE,GEN_RES_STR1(S_CANNOT_OPEN_FILE, Filename));
        return;
    }

    nLineNr = 0;    // Wir sind ja noch vor der Datei

    if ( !pNames )
        pNames = new CNames();

    {
        TTExecutionStatusHint aHint( TT_EXECUTION_ENTERWAIT );
        GetTTBroadcaster().Broadcast( aHint );
    }
    ADD_MESSAGE_LOG( Filename );
    while (!Stream.IsEof())
    {
        nLineNr++;

        Stream.ReadByteStringLine(aLine, RTL_TEXTENCODING_IBM_850);
        aLine.EraseLeadingChars();
        aLine.EraseTrailingChars();
        while ( aLine.SearchAscii("  ") != STRING_NOTFOUND )
            aLine.SearchAndReplaceAllAscii("  ",UniString(' '));
        if (aLine.Len() == 0) continue;

        if ( (aLine.GetTokenCount(cMyDelim) < 2 || aLine.GetTokenCount(cMyDelim) > 3) && aLine.CompareIgnoreCaseToAscii("*Active") != COMPARE_EQUAL )
        {
            ADD_WARNING_LOG2( GEN_RES_STR1( S_INVALID_LINE, aLine ), Filename, nLineNr );
            continue;
        }

        aName = aLine.GetToken(0,cMyDelim);
        aUId = SmartId( (ULONG)aLine.GetToken(1,cMyDelim).ToInt64() );

        if ( bSortByName )
            pNewItem = new ControlDef( aName, aUId );
        else
            pNewItem = new ControlItemUId( aName, aUId );
        if ( !pNames->C40_PTR_INSERT( ControlItem, pNewItem ) )
        {
            if ( bSortByName )
            {
                if ( nDoubleCount++ < 10 )
                {
                    ADD_WARNING_LOG2( GEN_RES_STR1( S_DOUBLE_NAME, aLine ), Filename, nLineNr );
                }
            }
            else
            {
                USHORT nNr;
                pNames->Seek_Entry( pNewItem, &nNr );
                AddName( pNames->GetObject(nNr)->pData->Kurzname, pNewItem->pData->Kurzname );
            }
            delete pNewItem;
        }
        GetpApp()->Reschedule();
    }
    {
        TTExecutionStatusHint aHint( TT_EXECUTION_LEAVEWAIT );
        GetTTBroadcaster().Broadcast( aHint );
    }

    Stream.Close();
#ifdef DBG_UTIL
//      int i;
//      for ( i = 0 ; i < pNames->Count() ; i++ )
//      {
//              DBG_ERROR( pNames->GetObject(i)->pData->Kurzname );
//      }
#endif
}

void ReadFlatArray( const ControlDefLoad arWas [], CNames *&pNames )
{
    USHORT nIndex = 0;

    if ( !pNames )
        pNames = new CNames();

    while ( String::CreateFromAscii(arWas[nIndex].Kurzname).Len() > 0 )
    {
        SmartId aUId (arWas[nIndex].nUId);
        const ControlItem *pX = new ControlDef( arWas[nIndex].Kurzname, aUId);
        pNames->C40_PTR_INSERT(ControlItem, pX);
        nIndex++;
    }
}

void TestToolObj::WaitForAnswer ()
{
    if ( bUseIPC )
    {
    #ifdef DBG_UTILx
        USHORT nSysWinModeMemo = GetpApp()->GetSystemWindowMode();
        GetpApp()->SetSystemWindowMode( 0 );
        ModelessDialog aDlg(NULL);
        aDlg.SetOutputSizePixel(Size(200,0));
        aDlg.SetText(CUniString("Waiting for Answer"));
        aDlg.Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
        GetpApp()->SetSystemWindowMode( nSysWinModeMemo );
    #endif
        BOOL bWasRealWait = !bReturnOK;
        BasicRuntime aRun( NULL );
        if ( BasicRuntimeAccess::HasRuntime() )
            aRun = BasicRuntimeAccess::GetRuntime();

        // this timer to terminate Yield below
        Timer aTimer;
        aTimer.SetTimeout( pImpl->aServerTimeout.GetMSFromTime() );
        aTimer.Start();
        while ( !bReturnOK && aTimer.IsActive() && pCommunicationManager->IsCommunicationRunning()
                && aRun.IsValid() && aRun.IsRun() )
        {
            #ifdef OS2
            DosSleep(100);
            #endif
            GetpApp()->Yield();
            if ( BasicRuntimeAccess::HasRuntime() )
                aRun = BasicRuntimeAccess::GetRuntime();
            else
                aRun = BasicRuntime( NULL );
        }
        if ( bWasRealWait && aDialogHandlerName.Len() > 0 )     // Damit das ganze auch im Testtool läuft
            CallDialogHandler(GetpApp());
    }
    else
    {
        Time Ende;

        Ende += pImpl->aServerTimeout;
        SvStream *pTemp = NULL;

        while ( !bReturnOK && Ende > Time() )
        {
//          pTemp = PlugInApplication::GetPlugInApp()->GetReturnFromExecute();
            if ( pTemp )
            {
                ReturnResults( pTemp );
                bReturnOK = TRUE;
            }
            else
            {
                GetpApp()->Reschedule();
            }
            nIdleCount = 0;
        }
    }


    if ( !bReturnOK )
    {
        ADD_ERROR(ERR_EXEC_TIMEOUT,GEN_RES_STR1(S_TIMOUT_WAITING, String::CreateFromInt64(nSequence)));
        bReturnOK = TRUE;
        nSequence++;
    }
}


IMPL_LINK( TestToolObj, IdleHdl, Application*, pApp )
{
    if ( !bReturnOK )
        nIdleCount++;
    if ( nIdleCount > 10 )  // d.h. Schon 10 mal hier gewesen und noch keinmal im WaitForAnswer
    {
        GetpApp()->RemoveIdleHdl( LINK( this, TestToolObj, IdleHdl ) );
        GetpApp()->PostUserEvent( LINK( this, TestToolObj, CallDialogHandler ) );
    }
    return 0;
}

IMPL_LINK( TestToolObj, CallDialogHandler, Application*, pApp )
{
    nWindowHandlerCallLevel++;
    String aName(aDialogHandlerName);
    aDialogHandlerName.Erase();

    ULONG nRememberSequence = nSequence; // Da sich die Sequence im DialogHandler ändert
    ((StarBASIC*)GetParent())->Call( aName );
    nSequence = nRememberSequence;
    // Die Sequenznummern werden dann zwar doppelt vergeben, aber wen kümmerts.

    nWindowHandlerCallLevel--;
    return 0;
}


void TestToolObj::BeginBlock()
{
    WaitForAnswer();
    if ( IsError() )
        return;

    DBG_ASSERT(!IsBlock,"BeginBlock innerhalb eines Blockes")
    In->Reset(nSequence);
    IsBlock = TRUE;
}


void TestToolObj::SendViaSocket()
{
    if ( !pCommunicationManager )
    {
        DBG_ERROR("Kein CommunicationManager vorhanden!!")
        return;
    }

    if ( !pCommunicationManager->IsCommunicationRunning() )
        if ( !pCommunicationManager->StartCommunication( ProgPath, pImpl->ProgParam, pImpl->pChildEnv ) )
        {
            ADD_ERROR(ERR_RESTART_FAIL,GEN_RES_STR1(S_APPLICATION_START_FAILED, ProgPath));
        }
        else
        {
            if ( !pImpl->bIsStart )
            {
                ADD_ERROR(ERR_RESTART,GEN_RES_STR0(S_APPLICATION_RESTARTED));
            }
        }


    bReturnOK = FALSE;
    if ( pCommunicationManager->GetLastNewLink() )
    {
        if ( !pCommunicationManager->GetLastNewLink()->TransferDataStream( In->GetStream() ) )
        {
            ADD_ERROR(ERR_SEND_TIMEOUT,GEN_RES_STR1(S_TIMOUT_SENDING, String::CreateFromInt64(nSequence)));
            nSequence++;
            bReturnOK = TRUE;               // Kein Return zu erwarten
        }
    }
    else
    {
        ADD_ERROR(ERR_SEND_TIMEOUT,GEN_RES_STR1(S_NO_CONNECTION, String::CreateFromInt64(nSequence)));
        nSequence++;
        bReturnOK = TRUE;               // Kein Return zu erwarten
    }

}

void TestToolObj::EndBlock()
{
    if (IsBlock)
    {
        pImpl->LocalStarttime = Time::GetSystemTicks(); // Setzen der Anfangszeit für Performancemessung

        In->GenCmdFlow (F_EndCommandBlock);

        if ( pImpl->bDoRemoteCommandDelay )
        {
            ULONG nTimeWait = pImpl->nMinRemoteCommandDelay;
            if ( pImpl->nMaxRemoteCommandDelay != pImpl->nMinRemoteCommandDelay )
                nTimeWait += Time::GetSystemTicks() % ( pImpl->nMaxRemoteCommandDelay - pImpl->nMinRemoteCommandDelay );
            Timer aTimer;
            aTimer.SetTimeout( nTimeWait );
            aTimer.Start();
            while ( aTimer.IsActive() && pCommunicationManager->IsCommunicationRunning() )
            {
                #ifdef OS2
                DosSleep(100);
                #endif
                GetpApp()->Yield();
            }
        }

        if ( bUseIPC )
            SendViaSocket();
        else
        {
//          PlugInApplication::GetPlugInApp()->ExecuteRemoteStatements( In->GetStream() );
            bReturnOK = FALSE;
            if ( aDialogHandlerName.Len() > 0 )
                GetpApp()->InsertIdleHdl( LINK( this, TestToolObj, IdleHdl ), 1 );
        }
        IsBlock = FALSE;
    }
    else
    {
        DBG_ERROR("EndBlock außerhalb eines Blockes")
    }
}


BOOL TestToolObj::Load( String aName, SbModule *pMod )
{
    BOOL bOk = TRUE;
    SvFileStream aStrm( aName, STREAM_STD_READ );
    if( aStrm.IsOpen() )
    {
        String aText, aLine;
        BOOL bIsFirstLine = TRUE;
        rtl_TextEncoding aFileEncoding = RTL_TEXTENCODING_IBM_850;
        while( !aStrm.IsEof() && bOk )
        {
            aStrm.ReadByteStringLine( aLine, aFileEncoding );
            if ( bIsFirstLine && IsTTSignatureForUnicodeTextfile( aLine ) )
                aFileEncoding = RTL_TEXTENCODING_UTF8;
            else
            {
                if ( !bIsFirstLine )
                    aText += '\n';
                aText += aLine;
                bIsFirstLine = FALSE;
            }
            if( aStrm.GetError() != SVSTREAM_OK )
                bOk = FALSE;
        }
        aText.ConvertLineEnd();
        pMod->SetName(CUniString("--").Append(aName));

        pMod->SetComment( GetRevision( aText ) );

        SbModule* pOldModule = MyBasic::GetCompileModule();
        MyBasic::SetCompileModule( pMod );

        pMod->SetSource( PreCompile( aText ) );

        MyBasic::SetCompileModule( pOldModule );
        if ( WasPrecompilerError() )
            bOk = FALSE;

    }
    else
        bOk = FALSE;
    return bOk;
}


BOOL TestToolObj::ReadNamesBin( String Filename, CNames *&pSIds, CNames *&pControls )
{
    SvFileStream aStream;
    String       aName,aURL;
    SmartId      aUId;
    ControlDef   *pNewDef, *pNewDef2;
    ControlDef   *pFatherDef = NULL;


    aStream.Open(Filename, STREAM_STD_READ);
    if (!aStream.IsOpen())
    {
        ADD_ERROR(ERR_NO_FILE,GEN_RES_STR1(S_CANNOT_OPEN_FILE, Filename));
        return FALSE;
    }

    if ( !pSIds )
        pSIds = new CNames();
    if ( !pControls )
        pControls = new CNames();

    {
        TTExecutionStatusHint aHint( TT_EXECUTION_ENTERWAIT );
        GetTTBroadcaster().Broadcast( aHint );
    }

    USHORT nAnz;
    aStream >> nAnz;
    CNames *pNames = pSIds; // first read all the slots
    BOOL bIsFlat = TRUE;    // Slots do not have children

    while ( nAnz && !aStream.IsEof() )
    {

        aStream.ReadByteString( aName, RTL_TEXTENCODING_UTF8 );

        USHORT nType;
         aStream >> nType;
        if ( !nType /* IsNumeric() */)
        {
            String aStrId;
            aStream.ReadByteString( aStrId, RTL_TEXTENCODING_UTF8 );
            aUId = SmartId( aStrId );
        }
        else
        {
            ULONG nUId;
            aStream >> nUId;
            aUId = SmartId( nUId );
        }

        if (aName.GetChar(0) == '*' || bIsFlat )     // Globaler Kurzname (Dialogname oder SId)
        {
            if (!bIsFlat)
                aName.Erase(0,1);
              pNewDef = new ControlDef(aName,aUId);

            if (!bIsFlat)
            {
                pNewDef->Sons(new CNames());

                pNewDef2 = new ControlDef(aName,aUId);      // Noch einen machen
                if (!pNewDef->SonInsert(pNewDef2))                              // Dialog in eigenen Namespace eintragen
                {
                    delete pNewDef2;
                    DBG_ERROR(" !!!! ACHTUNG !!!!  Fehler beim einfügen in leere Liste!");
                }
            }

            if (! pNames->C40_PTR_INSERT(ControlItem, (ControlItem*&)pNewDef))
            {
                DBG_ERROR(" !!!! ACHTUNG !!!!  Fehler beim einfügen eines namens!");
                delete pNewDef;
                pFatherDef = NULL;
            }
            else
            {
                pFatherDef = pNewDef;
            }
        }
        else
        {
            if (!pFatherDef)
            {
                DBG_ERROR( "Internal Error: Erster Kurzname muß mit * beginnen. Überspringe." );
            }
            else
            {
                   pNewDef = new ControlDef(aName,aUId);
                if (! pFatherDef->SonInsert(pNewDef))
                {
                    delete pNewDef;
                    DBG_ERROR(" !!!! ACHTUNG !!!!  Fehler beim einfügen eines namens!");
                }
            }
        }


        nAnz--;
        if ( !nAnz && bIsFlat )     // We have read all slots
        {
            aStream >> nAnz;
            pNames = pControls; // Now read the controls
            bIsFlat = FALSE;    // Controls *do* have children
        }


        GetpApp()->Reschedule();
    }
    {
        TTExecutionStatusHint aHint( TT_EXECUTION_LEAVEWAIT );
        GetTTBroadcaster().Broadcast( aHint );
    }

    aStream.Close();
    return TRUE;
return FALSE;
}


BOOL TestToolObj::WriteNamesBin( String Filename, CNames *pSIds, CNames *pControls )
{
    BOOL bOk = TRUE;
    SvFileStream aStrm( String(Filename).AppendAscii(".bin"), STREAM_STD_WRITE );
    if( aStrm.IsOpen() )
    {
        int i;
        if ( pSIds )
        {
            aStrm << pSIds->Count();
            for ( i = 0 ; pSIds->Count() > i && bOk ; i++ )
            {
                ((ControlDef*)(*pSIds)[i])->Write(aStrm);
                if( aStrm.GetError() != SVSTREAM_OK )
                    bOk = FALSE;
            }
        }
        else
            aStrm << USHORT( 0 );

        if ( pControls )
        {
            aStrm << pControls->Count();
            for ( i = 0 ; pControls->Count() > i && bOk ; i++ )
            {
                ((ControlDef*)(*pControls)[i])->Write(aStrm);
                if( aStrm.GetError() != SVSTREAM_OK )
                    bOk = FALSE;
            }
        }
        else
            aStrm << USHORT( 0 );
    }
    else
        bOk = FALSE;
    return bOk;
}


void TestToolObj::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                            const SfxHint& rHint, const TypeId& )
{
    static CNames *pUIds = NULL;    // Halten der hid.lst

    const SbxHint* p = PTR_CAST(SbxHint,&rHint);
    if( p )
    {
        SbxVariable* pVar = p->GetVar();
        SbxArray* rPar = pVar->GetParameters();

        ULONG nHintId = p->GetId();
        ULONG nUserData = pVar->GetUserData();
        if( nHintId == SBX_HINT_DATAWANTED )
        {
            nMyVar = 0;
            nControlsObj = 0;
            switch( nUserData )
            {
                case ID_Kontext:
                    if ( !rPar )
                    {
                        pNameKontext = pControls;

                        // So daß nicht immer mal wieder was aus einem alten Kontext dazwischenhaut
                        for (USHORT i=0;i<VAR_POOL_SIZE;i++)
                        {
                            pImpl->pMyVars[i] -> SetName( CUniString("VarDummy").Append(UniString::CreateFromInt32(i)) );
                            pImpl->pControlsObjs[i] -> SetName( CUniString("ControlDummy").Append(UniString::CreateFromInt32(i)) );
                        }
                    }
                    else if ( rPar && rPar->Count() == 2 )
                    {
                        USHORT nElement;
                        SbxVariableRef pArg = rPar->Get( 1 );
                        String aKontext = pArg->GetString();
                        ControlDef WhatName(aKontext,SmartId());
                        if (pControls->Seek_Entry(&WhatName,&nElement))
                        {
                            pNameKontext = ((ControlDef*)pControls->GetObject(nElement))->GetSons();

                            // So daß nicht immer mal wieder was aus einem alten Kontext dazwischenhaut
                            for (USHORT i=0;i<VAR_POOL_SIZE;i++)
                            {
                                pImpl->pMyVars[i] -> SetName( CUniString("VarDummy").Append(UniString::CreateFromInt32(i)) );
                                pImpl->pControlsObjs[i] -> SetName( CUniString("ControlDummy").Append(UniString::CreateFromInt32(i)) );
                            }
                        }
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_Start:
                    if ( rPar && rPar->Count() >= 2 )
                    {
                        SbxVariableRef pArg = rPar->Get( 1 );
                        ProgPath = pArg->GetString();
                        if ( rPar && rPar->Count() >= 3 )
                        {
                            pArg = rPar->Get( 2 );
                            pImpl->ProgParam = pArg->GetString();
                        }
                        else
                            pImpl->ProgParam.Erase();

                        String aTmpStr(ProgPath);
                        aTmpStr += ' ';
                        aTmpStr += pImpl->ProgParam;
                        TTExecutionStatusHint aHint( TT_EXECUTION_SHOW_ACTION, String(ResId(S_STARTING_APPLICATION)), aTmpStr );
                        GetTTBroadcaster().Broadcast( aHint );

                        pImpl->bIsStart = TRUE;
                        BeginBlock();
                        EndBlock();
                        pImpl->bIsStart = FALSE;
                        {
                            TTExecutionStatusHint aHint( TT_EXECUTION_HIDE_ACTION );
                            GetTTBroadcaster().Broadcast( aHint );
                        }
                    }
                    break;
//              case ID_Kill:
//                  if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
//                  {
//                  }
//                  else
//                      SetError( SbxERR_WRONG_ARGS );
//                  break;
                case ID_SaveIDs:
                    if ( rPar && rPar->Count() >= 2 )  // Genau ein Parameter
                    {
                        SbxVariableRef pArg = rPar->Get( 1 );
                        DirEntry FilePath = pImpl->aFileBase + DirEntry(pArg->GetString(),FSYS_STYLE_VFAT);
                        WriteNamesBin( FilePath.GetFull(), pSIds, pControls );
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_AutoExecute:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        pVar->PutBool(SingleCommandBlock);
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_Execute:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        EndBlock();
                        BeginBlock();
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_DialogHandler:
                    if ( rPar && rPar->Count() >= 2 )  // Genau ein Parameter
                    {
                        SbxVariableRef pArg = rPar->Get( 1 );
                        aDialogHandlerName = pArg->GetString();
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_GetError:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        WaitForAnswer();
                        if ( IS_ERROR() )
                        {
//                                                      pVar->PutULong( GET_ERROR()->nError );
                            pVar->PutString( GET_ERROR()->aText );
                            POP_ERROR();
                        }
                        else
                        {
//                                                      pVar->PutULong( 0 );
                            pVar->PutString( String() );
                        }
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_StartUse:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        BasicRuntime aRun = BasicRuntimeAccess::GetRuntime();
                        aLogFileName = DirEntry(aRun.GetModuleName(SbxNAME_NONE)).GetBase().AppendAscii(".res");

                        ADD_RUN_LOG();
                        ADD_CASE_LOG(GEN_RES_STR0(S_READING_FILE));

                        pCommunicationManager->StopCommunication();
                        // Wait for asynchronous events to be processed, so communication will be restarted properly
                        while ( pCommunicationManager->IsCommunicationRunning() )
                            Application::Reschedule();

                        SingleCommandBlock = TRUE;      // Bug 57188
                        IsBlock = FALSE;

//                      pCommunicationManager->StartCommunication();

                        for (USHORT i=0;i<VAR_POOL_SIZE;i++)
                        {
                            pImpl->pMyVars[i] -> SetName( CUniString("VarDummy").Append(UniString::CreateFromInt32(i)) );
                            pImpl->pControlsObjs[i] -> SetName( CUniString("ControlDummy").Append(UniString::CreateFromInt32(i)) );
                        }
                        nMyVar = 0;
                        nControlsObj = 0;

                        if (pControls)
                        {
                            delete pControls;
                            pControls = NULL;
                        }
                        if (pReverseSlots)
                        {
                            delete pReverseSlots;
                            pReverseSlots = NULL;
                        }
                        if (pReverseControls)
                        {
                            delete pReverseControls;
                            pReverseControls = NULL;
                        }
                        if (pReverseControlsSon)
                        {
                            delete pReverseControlsSon;
                            pReverseControlsSon = NULL;
                        }
                        if (pSIds)
                        {
                            delete pSIds;
                            pSIds = NULL;
                        }
                        if (pUIds)
                        {
                            delete pUIds;
                            pUIds = NULL;
                        }
                        if (pReverseUIds)
                        {
                            delete pReverseUIds;
                            pReverseUIds = NULL;
                        }
                        pNameKontext = pControls;
                        pImpl->bLnaguageExtensionLoaded = FALSE;
                        SfxSimpleHint aHint( SBX_HINT_LANGUAGE_EXTENSION_LOADED );
                        GetTTBroadcaster().Broadcast( aHint );

                        pImpl->nMinRemoteCommandDelay = 0;
                        pImpl->nMaxRemoteCommandDelay = 0;
                        pImpl->bDoRemoteCommandDelay = FALSE;
                        pImpl->aTestCaseName.Erase();
                        pImpl->aTestCaseFileName.Erase();
                        pImpl->nTestCaseLineNr = 0;

                        pImpl->bEnableQaErrors = TRUE;
                        pImpl->bDebugFindNoErrors = FALSE;

                        pImpl->pChildEnv->clear();

                        String aName( CUniString( "StopOnSyntaxError" ) );
                        SbxVariableRef xStopOnSyntaxError = SbxObject::Find( aName, SbxCLASS_PROPERTY );
                        if ( xStopOnSyntaxError.Is() )
                            xStopOnSyntaxError->PutBool( pImpl->bStopOnSyntaxError );
                        else
                            SetError( SbxERR_BAD_ACTION );
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_Use:
                    if ( rPar && rPar->Count() >= 2 )
                    {
                        SbxVariableRef pArg = rPar->Get( 1 );
                        DirEntry FilePath(pArg->GetString(),FSYS_STYLE_VFAT);
                        if ( !FilePath.IsAbs() )
                            FilePath = pImpl->aFileBase + FilePath;
                        String Ext = FilePath.GetExtension();
                        if ( Ext.CompareIgnoreCaseToAscii("Win") == COMPARE_EQUAL )
                        {
                            ReadNames( FilePath.GetFull(),pControls,pUIds);
                            pImpl->bLnaguageExtensionLoaded = TRUE;
                            SfxSimpleHint aHint( SBX_HINT_LANGUAGE_EXTENSION_LOADED );
                            GetTTBroadcaster().Broadcast( aHint );
                        }
                        else if ( Ext.CompareIgnoreCaseToAscii("Sid") == COMPARE_EQUAL )
                        {
                            ReadNames( FilePath.GetFull(),pSIds,pUIds,FLAT);
                            pImpl->bLnaguageExtensionLoaded = TRUE;
                            SfxSimpleHint aHint( SBX_HINT_LANGUAGE_EXTENSION_LOADED );
                            GetTTBroadcaster().Broadcast( aHint );
                        }
                        else if ( Ext.CompareIgnoreCaseToAscii("Bin") == COMPARE_EQUAL )
                        {
                            ReadNamesBin( FilePath.GetFull(), pSIds, pControls );
                            pImpl->bLnaguageExtensionLoaded = TRUE;
                            SfxSimpleHint aHint( SBX_HINT_LANGUAGE_EXTENSION_LOADED );
                            GetTTBroadcaster().Broadcast( aHint );
                        }
                        else if ( Ext.CompareIgnoreCaseToAscii("Inc") == COMPARE_EQUAL )
                        {
                            {
                                TTExecutionStatusHint aHint( TT_EXECUTION_SHOW_ACTION, String(ResId(S_READING_BASIC_MODULE)), FilePath.GetFull() );
                                GetTTBroadcaster().Broadcast( aHint );
                            }
                            String aFullPathname = FilePath.GetFull();
                            StarBASIC *pBasic = (StarBASIC*)GetParent();
                            if ( !aModuleWinExistsHdl.Call( &aFullPathname ) &&
                                 !pBasic->FindModule( CUniString( "--" ).Append(aFullPathname) ) )
                            {
                                SbModule *pMod;
                                pMod = pBasic->MakeModule( CUniString("--"), String() );
                                pMod->Clear();
                                if ( Load( aFullPathname, pMod ) )
                                {
                                    if ( !IS_ERROR() )
                                    {
                                        pBasic->Compile( pMod );
                                        pMod->RunInit();
                                    }
                                }
                                else
                                {
                                    ADD_ERROR( SbxERR_CANNOT_LOAD, FilePath.GetFull() );
                                }
                            }
                            {
                                TTExecutionStatusHint aHint( TT_EXECUTION_HIDE_ACTION );
                                GetTTBroadcaster().Broadcast( aHint );
                            }
                        }
                        else
                        {
                            ADD_ERROR(SbxERR_CANNOT_LOAD,FilePath.GetFull());
                        }
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_FinishUse:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        ADD_CASE_LOG( String() );       // Case abschliessen
                        if (!pControls)
                            pControls = new CNames();

                        if (!pSIds)
                            pSIds = new CNames();

                        if (pUIds)
                        {   // save some memory
                            delete pUIds;
                            pUIds = NULL;
                        }

                        pNameKontext = pControls;

                        if ( pImpl->bLnaguageExtensionLoaded )
                        {
                            SfxSimpleHint aHint( SBX_HINT_LANGUAGE_EXTENSION_LOADED );
                            GetTTBroadcaster().Broadcast( aHint );
                        }

                        pImpl->nIncludeFileWarningCount = pImpl->nWarningCount;
                        pImpl->nWarningCount = 0;

                        *pImpl->xIncludeFileWarningList = *pImpl->xWarningList;
                        pImpl->xWarningList->SbxArray::Clear();
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_CaseLog:
                    if ( rPar )  // rPar != NULL  <=>  Es gibt Parameter
                    {
                        USHORT n;
                        String aX;
                        for ( n = 1; n < rPar->Count(); n++ )
                        {
                            SbxVariableRef pArg = rPar->Get( n );
                            aX += pArg->GetString();
                        }
                        pImpl->aTestCaseName = aX;
                        if ( pImpl->aTestCaseName.Len() && BasicRuntimeAccess::HasRuntime() )
                        {
                            BasicRuntime aRun = BasicRuntimeAccess::GetRuntime();
                            pImpl->aTestCaseFileName = aRun.GetModuleName(SbxNAME_SHORT_TYPES);
                            if ( pImpl->aTestCaseFileName.Copy(0,2).CompareToAscii( "--" ) == COMPARE_EQUAL )
                                pImpl->aTestCaseFileName.Erase(0,2);
                            pImpl->nTestCaseLineNr = aRun.GetLine();
                        }
                        else
                        {
                            pImpl->aTestCaseFileName.Erase();
                            pImpl->nTestCaseLineNr = 0;
                        }
                        ADD_CASE_LOG( aX );
                    }
                    break;
                case ID_ExceptLog:
                    if ( IS_ERROR() )
                    {
                        BasicRuntime aRun = BasicRuntimeAccess::GetRuntime();
                        BOOL bWasNewError = FALSE;

                        if ( BasicRuntimeAccess::HasStack() )
                        {
                            for ( USHORT i = 0 ; i < BasicRuntimeAccess::GetStackEntryCount() -1 ; i++ )
                            {
                                BasicErrorStackEntry aThisEntry = BasicRuntimeAccess::GetStackEntry(i);
                                if ( !bWasNewError )
                                {
                                    bWasNewError = TRUE;
                                    ADD_ERROR_LOG( GET_ERROR()->aText, aThisEntry.GetModuleName(SbxNAME_SHORT_TYPES),
                                        aThisEntry.GetLine(), aThisEntry.GetCol1(), aThisEntry.GetCol2(), aThisEntry.GetSourceRevision() );
                                }
                                ADD_CALL_STACK_LOG( String(aThisEntry.GetModuleName(SbxNAME_SHORT_TYPES))
                                    .AppendAscii(": ").Append(aThisEntry.GetMethodName(SbxNAME_SHORT_TYPES)),
                                    aThisEntry.GetModuleName(SbxNAME_SHORT_TYPES),
                                    aThisEntry.GetLine(), aThisEntry.GetCol1(), aThisEntry.GetCol2() );

                            }
                            BasicRuntimeAccess::DeleteStack();
                        }

                        BOOL bIsFirst = TRUE;
                        while ( aRun.IsValid() )
                        {
                            xub_StrLen nErrLn;
                            xub_StrLen nCol1;
                            xub_StrLen nCol2;
                            if ( bIsFirst )
                            {
                                bIsFirst = FALSE;
                                nErrLn = GET_ERROR()->nLine;
                                nCol1 = GET_ERROR()->nCol1;
                                nCol2 = GET_ERROR()->nCol2;
                            }
                            else
                            {
                                nErrLn = aRun.GetLine();
                                nCol1 = aRun.GetCol1();
                                nCol2 = aRun.GetCol2();
                            }

                            if ( !bWasNewError )
                            {
                                bWasNewError = TRUE;
                                ADD_ERROR_LOG( GET_ERROR()->aText, aRun.GetModuleName(SbxNAME_SHORT_TYPES),
                                    nErrLn, nCol1, nCol2, aRun.GetSourceRevision() );
                            }
                            ADD_CALL_STACK_LOG( String(aRun.GetModuleName(SbxNAME_SHORT_TYPES))
                                .AppendAscii(": ").Append(aRun.GetMethodName(SbxNAME_SHORT_TYPES)),
                                aRun.GetModuleName(SbxNAME_SHORT_TYPES),
                                nErrLn, nCol1, nCol2 );
                            aRun = aRun.GetNextRuntime();
                        }
                    }
                    break;
                case ID_ErrorLog:
                    if ( IS_ERROR() )
                    {
                        BasicRuntime aRun = BasicRuntimeAccess::GetRuntime();
                        if ( BasicRuntimeAccess::HasStack() )
                        {
                            BasicErrorStackEntry aThisEntry = BasicRuntimeAccess::GetStackEntry( 0 );
                            ADD_ERROR_LOG( GET_ERROR()->aText, aThisEntry.GetModuleName(SbxNAME_SHORT_TYPES),
                                aThisEntry.GetLine(), aThisEntry.GetCol1(), aThisEntry.GetCol2(), aThisEntry.GetSourceRevision() );
                            BasicRuntimeAccess::DeleteStack();
                        }
                        else
                        {
                            ADD_ERROR_LOG( GET_ERROR()->aText, aRun.GetModuleName(SbxNAME_SHORT_TYPES),
                                StarBASIC::GetErl(), aRun.GetCol1(), aRun.GetCol2(), aRun.GetSourceRevision() );
                        }
                    }
                    break;
                case ID_QAErrorLog:
                    if ( rPar )  // rPar != NULL  <=>  Es gibt Parameter
                    {
                        USHORT n;
                        String aSammel;
                        for ( n = 1; n < rPar->Count(); n++ )
                        {
                            SbxVariableRef pArg = rPar->Get( n );
                            aSammel += pArg->GetString();
                        }
                        ADD_QA_ERROR_LOG( aSammel );
                    }
                    break;
                case ID_PrintLog:
                    if ( rPar )  // rPar != NULL  <=>  Es gibt Parameter
                    {
                        USHORT n;
                        String aSammel;
                        for ( n = 1; n < rPar->Count(); n++ )
                        {
                            SbxVariableRef pArg = rPar->Get( n );
                            aSammel += pArg->GetString();
                        }
                        ADD_MESSAGE_LOG( aSammel );
                    }
                    break;
                case ID_WarnLog:
                    if ( rPar )  // rPar != NULL  <=>  Es gibt Parameter
                    {
                        USHORT n;
                        String aSammel;
                        for ( n = 1; n < rPar->Count(); n++ )
                        {
                            SbxVariableRef pArg = rPar->Get( n );
                            aSammel += pArg->GetString();
                        }
                        ADD_WARNING_LOG( aSammel );

                    }
                    break;
                case ID_ClearError:
                    while ( IS_ERROR() )
                    {
                        POP_ERROR();
                    }
                    break;
                case ID_MaybeAddErr:
                    if ( ((StarBASIC*)GetParent())->GetErr() && ( !IS_ERROR() ||
                         pFehlerListe->GetObject(pFehlerListe->Count()-1)->nError != ((StarBASIC*)GetParent())->GetErr() ) )
                    {
                        ((StarBASIC*)GetParent())->MakeErrorText(((StarBASIC*)GetParent())->GetErr(),String());
                        ADD_ERROR_QUIET(((StarBASIC*)GetParent())->GetErr() , ((StarBASIC*)GetParent())->GetErrorText())
                    }
                    break;
                case ID_GetNextCloseWindow:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        SetError( SbxERR_NOTIMP );
                        break;
//                                              Das ist total rotten und muß wohl komplett neu!!

                        BOOL bWasBlock = IsBlock;
                        if ( !IsBlock )                 // Impliziter call bei Aufruf mit Methode
                            if ( SingleCommandBlock )
                                BeginBlock();
//                                              if ( !IsError() )
//                                                      In->GenCmdSlot (128,rPar);
//                                              ((Controls*)pVar)->pMethodVar->nValue = 128;

                        ULONG nOldValue = ((Controls*)pVar)->GetULong();
                        // Setzen, so daß beim Return der Wert stimmt
                        ((Controls*)pVar)->PutULong( 128 );
                        pImpl->pNextReturn = ((Controls*)pVar)->pMethodVar;
                        if ( SingleCommandBlock )
                            EndBlock();
                        WaitForAnswer();
                        if ( bWasBlock )
                            if ( SingleCommandBlock )
                                BeginBlock();
                        ((Controls*)pVar)->PutULong( nOldValue );

                        // Rücksetzen, so daß beim nächsten Aufruf alles klappt
//                                              ((Controls*)pVar)->SetUserData( 128 );


//                                              ((Controls*)pVar)->SetName("xxx");
                        // Setzen und rücksetzen der ID, so dass der Notify ohne Wirkung bleibt.
                        ((Controls*)pVar)->pMethodVar->SetUserData(ID_ErrorDummy);
                        ((Controls*)pVar)->PutULong( ((Controls*)pVar)->pMethodVar->GetULong() );
                        ((Controls*)pVar)->pMethodVar->SetUserData(ID_Control);

                        pShortNames->Insert( CUniString("xxx"), SmartId( ((Controls*)pVar)->pMethodVar->nValue ), nSequence );

                        nOldValue = ((Controls*)pVar)->GetULong();

                        SbxVariable *pMember;
                        if ( ! (pMember = ((Controls*)pVar)->Find(CUniString("ID"),SbxCLASS_DONTCARE)) )
                        {
                            pMember = new SbxProperty(CUniString("ID"),SbxULONG);
                            ((Controls*)pVar)->Insert(pMember);
                        }
                        pMember->PutULong(((Controls*)pVar)->pMethodVar->nValue);

                        if ( ! (pMember = ((Controls*)pVar)->Find(CUniString("name"),SbxCLASS_DONTCARE)) )
                        {
                            pMember = NULL;
                        }
                        else
                            pMember->PutString(CUniString("xxx"));



                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_RemoteCommand:
                    {
                        if ( SingleCommandBlock )
                            BeginBlock();
                        else
                            if ( ((SbxTransportMethod*)pVar)->nValue & M_WITH_RETURN )
                            {
                                SetError( SbxERR_NOTIMP );
                            }
                        if ( !IsError() )
                            In->GenCmdCommand ((USHORT)(((SbxTransportMethod*)pVar)->nValue),rPar);
                        if ( !IsError() && ((SbxTransportMethod*)pVar)->nValue & M_WITH_RETURN )
                        {
                            pImpl->pNextReturn = ((SbxTransportMethod*)pVar);
                            aNextReturnId = SmartId( ((SbxTransportMethod*)pVar)->nValue );
                        }
                        if ( SingleCommandBlock )
                            EndBlock();
                        if ( !IsError() && (USHORT)((SbxTransportMethod*)pVar)->nValue & M_WITH_RETURN )
                        {
                            WaitForAnswer();
                        }
                        // für einige noch etwas Nachbehandlung
                        switch ( ((SbxTransportMethod*)pVar)->nValue )
                        {
                            case RC_WinTree:
//                              ::svt::OStringTransfer::CopyString(pVar->GetString(), pSomeWindowIDontHave );
                                break;
                        }

                    }
                    break;
                case ID_Dispatch:
                    if ( !rPar || (rPar->Count() % 2) == 1 )  // rPar = NULL  <=>  Kein Parameter ansonsten Gerade Anzahl(Ungerade, da immer Anzahl+1
                    {
                        if ( SingleCommandBlock )
                            BeginBlock();
                        if ( !IsError() )
                            In->GenCmdSlot ( (USHORT)((SbxTransportMethod*)pVar)->nValue, rPar );
                        pVar->PutInteger( (USHORT)((SbxTransportMethod*)pVar)->nValue );
                        if ( SingleCommandBlock )
                            EndBlock();
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_UNODispatch:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter ansonsten Gerade Anzahl(Ungerade, da immer Anzahl+1
                    {
                        if ( SingleCommandBlock )
                            BeginBlock();
                        if ( !IsError() )
                            In->GenCmdUNOSlot ( ((SbxTransportMethod*)pVar)->aUnoSlot );
                        pVar->PutString( ((SbxTransportMethod*)pVar)->aUnoSlot );
                        if ( SingleCommandBlock )
                            EndBlock();
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_Control:
                case ID_StringControl:
                    if ( SingleCommandBlock )
                        BeginBlock();
                    else
                        if ( ((SbxTransportMethod*)pVar)->nValue & M_WITH_RETURN )
                        {
                            SetError( SbxERR_NOTIMP );
                        }
                    if ( !IsError() )
                    {
                        if ( nUserData == ID_Control )
                        {
                            In->GenCmdControl (pVar->GetParent()->GetULong(),
                                (USHORT)((SbxTransportMethod*)pVar)->nValue, rPar);
                            aNextReturnId = SmartId( pVar->GetParent()->GetULong() );
                        }
                        else
                        {
                            In->GenCmdControl (pVar->GetParent()->GetString(),
                                (USHORT)((SbxTransportMethod*)pVar)->nValue, rPar);
                            aNextReturnId = SmartId( pVar->GetParent()->GetString() );
                        }


                        if ( !IsError() && ((SbxTransportMethod*)pVar)->nValue & M_WITH_RETURN )
                        {
                            pImpl->pNextReturn = ((SbxTransportMethod*)pVar);
                        }
                        else
                        {
                            pImpl->pNextReturn = NULL;
                            aNextReturnId = SmartId();
                        }

                    }
                    if ( SingleCommandBlock )
                        EndBlock();
                    if ( !IsError() && (USHORT)((SbxTransportMethod*)pVar)->nValue & M_WITH_RETURN )
                    {
                        WaitForAnswer();
                    }

                    break;
                case ID_GetUnoApp:
                    {
                        // Hier wird der Remote UNO Kram gestartet
                        // Eintrag in die Konfiguration unter
                        // org.openoffice.Office.Common/Start/Connection
                        //  socket,host=0,port=12345;iiop;XBla
                        // oder
                        //  socket,host=0,port=12345;urp;;XBla

                        String aString;
                        aString.AppendAscii( "socket,host=" );
                        aString += GetHostConfig();
                        aString.AppendAscii( ",port=" );
                        aString += String::CreateFromInt32( GetUnoPortConfig() );

                           Reference< XMultiServiceFactory > smgr_xMultiserviceFactory;
                        try
                        {
                            Reference< XMultiServiceFactory > xSMgr = comphelper::getProcessServiceFactory();
// is allways there
/*                          if ( ! xSMgr.is() )
                            {
                                xSMgr = ::cppu::createRegistryServiceFactory(OUString(RTL_CONSTASCII_USTRINGPARAM("applicat.rdb")), sal_True );
                                if ( xSMgr.is() )
                                    comphelper::setProcessServiceFactory( xSMgr );
                            }*/

                            OUString aURL( aString );
                            Reference< XConnector > xConnector( xSMgr->createInstance(
                                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector") ) ), UNO_QUERY );
                            Reference< XConnection > xConnection( xConnector->connect( aURL ) );

                            Reference< XBridgeFactory > xBridgeFactory( xSMgr->createInstance(
                                OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory") ) ), UNO_QUERY );
                            Reference< XBridge > xBridge( xBridgeFactory->createBridge(
                                OUString(), OUString( RTL_CONSTASCII_USTRINGPARAM("urp") ),
                                xConnection, Reference< XInstanceProvider >() ) );

    //                      Reference< XInterface > xRet( xBridge->getInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ) );
                            Reference< XInterface > xRet( xBridge->getInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("StarOffice.ServiceManager")) ) );

    //                      Reference< XNamingService > xNamingService(xRet, UNO_QUERY);

    //                      Reference< XInterface > smgr = xNamingService->getRegisteredObject( OUString( RTL_CONSTASCII_USTRINGPARAM("StarOffice.ServiceManager" ) ) );

                            smgr_xMultiserviceFactory = Reference< XMultiServiceFactory >(xRet, UNO_QUERY);
    //MBA fragen!!
                        }
                        catch( class Exception & rEx)
                        {
                            ADD_ERROR(SbxERR_BAD_ACTION, String( rEx.Message ) );
                        }
                        catch( ... )
                        {
                            ADD_ERROR(SbxERR_BAD_ACTION, CUniString( "Unknown Error" ) );
                        }

                        if( smgr_xMultiserviceFactory.is() )
                        {
                            Any aAny;
//                          aAny <<= xBridge;
                            aAny <<= smgr_xMultiserviceFactory;

                            SbxObjectRef xMySbxObj = GetSbUnoObject( CUniString("RemoteUnoAppFuerTesttool"), aAny );
                            if ( xMySbxObj.Is() )
                                pVar->PutObject( xMySbxObj );
                        }
                    }
                    break;
                case ID_GetIServer:
                    {
                        // Hier wird der Remote UNO Kram gestartet

                        String aString;
//                      aString += GetHostConfig();
//                      aString.AppendAscii( ":" );
//                      aString += String::CreateFromInt32( GetUnoPortConfig() );

                        Reference< XMultiServiceFactory > xSMgr /* = comphelper::getProcessServiceFactory()*/;
//                      if ( ! xSMgr.is() )
                        {
                            xSMgr = ::cppu::createRegistryServiceFactory(OUString(RTL_CONSTASCII_USTRINGPARAM("g:\\iserverproxy.rdb")), sal_True);
//                          comphelper::setProcessServiceFactory( xSMgr );
                        }

                        OUString aURL( aString );
                        Reference< XConnector > xConnector( xSMgr->createInstance(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector") ) ), UNO_QUERY );
                        Reference< XConnection > xConnection( xConnector->connect( OUString( RTL_CONSTASCII_USTRINGPARAM("socket,host=grande,port=7453")) ) );

                        Reference< XBridgeFactory > xBridgeFactory( xSMgr->createInstance(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory") ) ), UNO_QUERY );
                        Reference< XBridge > xBridge( xBridgeFactory->createBridge(
                            OUString(), OUString( RTL_CONSTASCII_USTRINGPARAM("urp") ),
                            xConnection, Reference< XInstanceProvider >() ) );

                        Reference< XInterface > xRet( xBridge->getInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("XIServerProxy")) ) );


/*                      Reference< XIServerProxy > xIS( xRet, UNO_QUERY );
                        if ( xIS.is() )
                        {
                            String aHost( xIS->getIServerHost() );

//                          Reference < XInformationClient > xIC = xIS->createIServerClient( "XInformationClient" );
                            Reference < XInformationClient > xIC = xIS->createInformationClient();
                            xIC->getTree(OUString::createFromAscii("r:\\b_server\\config\\stand.lst"), OUString() );


                            Reference< XTypeProvider > xTP( xRet, UNO_QUERY );
                            Sequence < com::sun::star::uno::Type > s = xTP->getTypes();
                        }
  */

                        if( xRet.is() )
                        {
                            Any aAny;
                            aAny <<= xRet;

                            SbxObjectRef xMySbxObj = GetSbUnoObject( CUniString("IServerProxy"), aAny );
                            if ( xMySbxObj.Is() )
                                pVar->PutObject( xMySbxObj );
                        }
                        // In Basic:
                        // msgbox dbg_SupportedInterfaces
                        // msgbox dbg_Properties
                        // msgbox dbg_Methods
                    }
                    break;
                case ID_RemoteCommandDelay:
                    if ( rPar && rPar->Count() >= 2 && rPar->Count() <=3 )
                    {
                        switch (rPar->Get( 1 )->GetType())
                        {
                            case SbxLONG:       // alles immer als Short übertragen
                            case SbxULONG:
                            case SbxLONG64:
                            case SbxULONG64:
                            case SbxDOUBLE:
                            case SbxINTEGER:
                            case SbxBYTE:
                            case SbxUSHORT:
                            case SbxINT:
                            case SbxUINT:
                            case SbxSINGLE:
                                pImpl->nMinRemoteCommandDelay = rPar->Get( 1 )->GetULong();
                                if ( rPar->Count() == 3 )
                                    pImpl->nMaxRemoteCommandDelay = rPar->Get( 2 )->GetULong();
                                else
                                    pImpl->nMaxRemoteCommandDelay = pImpl->nMinRemoteCommandDelay;
                                break;
                            case SbxBOOL:
                                pImpl->bDoRemoteCommandDelay = rPar->Get( 1 )->GetBool();
                                break;
                            default:
                                SbxBase::SetError( SbxERR_WRONG_ARGS );
                                break;
                        }
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_GetApplicationPath:
                    if ( !rPar )
                    {
                        OUString aUrl = Config::GetDefDirectory();
                        OUString aPath;
                        osl::FileBase::getSystemPathFromFileURL( aUrl, aPath );
                        pVar->PutString( String( aPath ) );
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_GetCommonApplicationPath:
                    if ( !rPar )
                    {
#ifdef WNT
                        ////////  adapted this from setup2\win\source\system\winos.cxx
                        String aSysPath;
                        aSysPath = _SHGetSpecialFolder_COMMON_APPDATA();
                        if ( aSysPath.Len() )
                        {
                            pVar->PutString( aSysPath );
                        }
                        else    // default to ID_GetApplicationPath (same as in setup)
                        {
                            OUString aUrl = Config::GetDefDirectory();
                            OUString aPath;
                            osl::FileBase::getSystemPathFromFileURL( aUrl, aPath );
                            pVar->PutString( String( aPath ) );
                        }
#else
#if UNX
                        pVar->PutString( CUniString( "/etc" ) );
#else
#error not implemented
#endif
#endif
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_MakeIniFileName:
                    if ( rPar && rPar->Count() == 2 )
                    {
                        OUString aUrl = Config::GetConfigName( String(), rPar->Get( 1 )->GetString() );
                        OUString aPath;
                        osl::FileBase::getSystemPathFromFileURL( aUrl, aPath );
                        pVar->PutString( String( aPath ) );
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_Wait:
                    {
                        if( rPar && rPar->Count() == 2 )
                        {
                            long nWait = rPar->Get(1)->GetLong();
                            if( nWait >= 0 )
                            {
#ifdef DEBUG
                                Time aStart;
#endif
                                Timer aTimer;
                                aTimer.SetTimeout( nWait );
                                aTimer.Start();
                                while ( aTimer.IsActive() )
                                    Application::Yield();
#ifdef DEBUG
                                Time aEnd;
                                Time aDiff = aEnd - aStart;
                                ULONG aMS = aDiff.GetMSFromTime();
                                if ( Abs( aDiff.GetMSFromTime() - nWait ) > 100 )
                                {
                                    DBG_ERROR1("Wait was off limit by %i", aDiff.GetMSFromTime() - nWait )
                                }
#endif
                            }
                        }
                        else
                            SetError( SbERR_BAD_ARGUMENT );
                    }
                    break;
                case ID_GetErrorCount:
                    {
                        pVar->PutULong( pImpl->nErrorCount );
                    }
                    break;
                case ID_GetWarningCount:
                    {
                        pVar->PutULong( pImpl->nWarningCount );
                    }
                    break;
                case ID_GetQAErrorCount:
                    {
                        pVar->PutULong( pImpl->nQAErrorCount );
                    }
                    break;
                case ID_GetUseFileWarningCount:
                    {
                        pVar->PutULong( pImpl->nIncludeFileWarningCount );
                    }
                    break;
                case ID_GetErrorList:
                    {
                        if ( ! pImpl->xErrorList->GetDims() )
                            pImpl->xErrorList->AddDim( 1, 32000 );
                        pVar->PutObject( pImpl->xErrorList );
                    }
                    break;
                case ID_GetWarningList:
                    {
                        if ( ! pImpl->xWarningList->GetDims() )
                            pImpl->xWarningList->AddDim( 1, 32000 );
                        pVar->PutObject( pImpl->xWarningList );
                    }
                    break;
                case ID_GetQAErrorList:
                    {
                        if ( ! pImpl->xQAErrorList->GetDims() )
                            pImpl->xQAErrorList->AddDim( 1, 32000 );
                        pVar->PutObject( pImpl->xQAErrorList );
                    }
                    break;
                case ID_GetUseFileWarningList:
                    {
                        if ( ! pImpl->xIncludeFileWarningList->GetDims() )
                            pImpl->xIncludeFileWarningList->AddDim( 1, 32000 );
                        pVar->PutObject( pImpl->xIncludeFileWarningList );
                    }
                    break;
                case ID_GetTestCaseName:
                    {
                        pVar->PutString( pImpl->aTestCaseName );
                    }
                    break;
                case ID_GetTestCaseFileName:
                    {
                        pVar->PutString( pImpl->aTestCaseFileName );
                    }
                    break;
                case ID_GetTestCaseLineNr:
                    {
                        pVar->PutUShort( pImpl->nTestCaseLineNr );
                    }
                    break;
                case ID_SetChildEnv:
                    {
                        if( rPar && rPar->Count() == 3 )
                        {
                            pImpl->pChildEnv->erase( rPar->Get(1)->GetString() );
                            pImpl->pChildEnv->insert( EnvironmentVariable( rPar->Get(1)->GetString(), rPar->Get(2)->GetString() ) );
                        }
                        else
                            SetError( SbERR_BAD_ARGUMENT );
                    }
                    break;
                case ID_GetChildEnv:
                    {
                        if( rPar && rPar->Count() == 2 )
                        {
                            Environment::const_iterator aIter = pImpl->pChildEnv->find( rPar->Get(1)->GetString() );
                            if ( aIter != pImpl->pChildEnv->end() )
                                pVar->PutString( (*aIter).second );
                            else
                                pVar->PutString( String() );
                        }
                        else
                            SetError( SbERR_BAD_ARGUMENT );
                    }
                    break;
                case ID_GetLinkDestination:
                    {
                        if( rPar && rPar->Count() == 2 )
                        {
                            String aSource,aDest;
                            aSource = rPar->Get(1)->GetString();
#ifdef UNX
                            ByteString aByteSource( aSource, osl_getThreadTextEncoding() );
                            char cDest[1024];
                            int nLen = 0;
                            if ( ( nLen = readlink( aByteSource.GetBuffer(), cDest, sizeof(cDest) ) ) >= 0 )
                            {
                                aDest = String( cDest, nLen, osl_getThreadTextEncoding() );
                            }
                            else
                            {
                                int nErr = errno;
                                switch ( nErr )
                                {
                                    case EINVAL: aDest = aSource;
                                        break;
                                    default:
                                        SetError( SbERR_ACCESS_ERROR );
                                }
                            }
#else
                            aDest = aSource;
#endif
                            pVar->PutString( aDest );
                        }
                        else
                            SetError( SbERR_BAD_ARGUMENT );
                    }
                    break;
                case ID_GetRegistryValue:
                    {
                        if( rPar && rPar->Count() == 3 )
                        {
                            String aValue;
#ifdef WNT
                            aValue = ReadRegistry( rPar->Get(1)->GetString(), rPar->Get(2)->GetString() );
#endif
                            pVar->PutString( aValue );
                        }
                        else
                            SetError( SbERR_BAD_ARGUMENT );
                    }
                    break;
            }  //  switch( nUserData )
        }  // if( nHintId == SBX_HINT_DATAWANTED )
        else if( nHintId == SBX_HINT_DATACHANGED )
        {
            switch( nUserData )
            {
                case ID_AutoExecute:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                    {
                        SingleCommandBlock = pVar->GetBool();
                        if ( SingleCommandBlock )
                            EndBlock();
                        else
                            BeginBlock();
                    }
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
                case ID_EnableQaErrors:
                    if ( !rPar )  // rPar = NULL  <=>  Kein Parameter
                        pImpl->bEnableQaErrors = pVar->GetBool();
                    else
                        SetError( SbxERR_WRONG_ARGS );
                    break;
            }
        }  // if( nHintId == SBX_HINT_DATACHANGED )
        else if( nHintId == SBX_HINT_BASICSTART )
        {
            pImpl->nErrorCount = 0;
            pImpl->nWarningCount = 0;
            pImpl->nQAErrorCount = 0;
            pImpl->nIncludeFileWarningCount = 0;

            pImpl->xErrorList->SbxArray::Clear();   // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
            pImpl->xWarningList->SbxArray::Clear(); // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
            pImpl->xQAErrorList->SbxArray::Clear();   // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
            pImpl->xIncludeFileWarningList->SbxArray::Clear();  // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content

            if (pFehlerListe)
                delete pFehlerListe;
            pFehlerListe = new CErrors;
        }  // if( nHintId == SBX_HINT_BASICSTART )
        else if( nHintId == SBX_HINT_BASICSTOP )
        {
            // Log summary to journal
            ADD_CASE_LOG( String() );       // Case abschliessen
            ADD_MESSAGE_LOG( CUniString("***************************************************") );
            if ( pImpl->nErrorCount )
            {
                ADD_WARNING_LOG( GEN_RES_STR1( S_ERRORS_DETECTED, String::CreateFromInt32( pImpl->nErrorCount ) ) );
                pImpl->nWarningCount--;     // Anpassen, da diese Warnung nicht in die Statistik soll
            }
            else
                ADD_MESSAGE_LOG( GEN_RES_STR0( S_NO_ERRORS_DETECTED ) );

            if ( pImpl->nWarningCount )
                ADD_WARNING_LOG( GEN_RES_STR1( S_WARNINGS_DETECTED, String::CreateFromInt32( pImpl->nWarningCount ) ) )
            else
                ADD_MESSAGE_LOG( GEN_RES_STR0( S_NO_WARNINGS_DETECTED ) );

            if ( pImpl->nIncludeFileWarningCount )
                ADD_WARNING_LOG( GEN_RES_STR1( S_INCLUDE_FILE_WARNINGS_DETECTED, String::CreateFromInt32( pImpl->nIncludeFileWarningCount ) ) )
            else
                ADD_MESSAGE_LOG( GEN_RES_STR0( S_NO_INCLUDE_FILE_WARNINGS_DETECTED ) );
            ADD_MESSAGE_LOG( CUniString("***************************************************") );

            pImpl->nErrorCount = 0;
            pImpl->nWarningCount = 0;
            pImpl->nQAErrorCount = 0;
            pImpl->nIncludeFileWarningCount = 0;

            pImpl->xErrorList->SbxArray::Clear();   // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
            pImpl->xWarningList->SbxArray::Clear(); // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
            pImpl->xQAErrorList->SbxArray::Clear();   // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
            pImpl->xIncludeFileWarningList->SbxArray::Clear();  // call SbxArray::Clear because SbxVarArray::Clear only clears dimensions but no content
        }  // if( nHintId == SBX_HINT_BASICSTOP )
        WaitForAnswer();
        if ( IsError() && ( !IS_ERROR() || GET_ERROR()->nError != GetError() ) )
        {
            ((StarBASIC*)GetParent())->MakeErrorText(GetError(),String());
            ADD_ERROR_QUIET(GetError(),String(pVar->GetName()).AppendAscii(": ").
                Append(((StarBASIC*)GetParent())->GetErrorText()));
            if ( bQuietErrors )
                ResetError();
        }
    }
}

void TestToolObj::DebugFindNoErrors( BOOL bDebugFindNoErrors )
{
    pImpl->bDebugFindNoErrors = bDebugFindNoErrors;
}

SbxVariable* TestToolObj::Find( const String& Str, SbxClassType Type)
{
    if ( BasicRuntimeAccess::IsRunInit() )            // wegen Find im "Global" Befehl des Basic
        return NULL;

    SbxVariableRef Old = SbxObject::Find(Str, Type );
    if (Old && Old->GetUserData() != ID_Dispatch && Old->GetUserData() != ID_UNODispatch && Old->GetUserData() != 0 )
        return Old;
    else
    {
        USHORT nElement;
        ControlDef *pWhatName = new ControlDef(Str,SmartId());

        /// nach Controls suchen
        if (pNameKontext && pNameKontext->Seek_Entry(pWhatName,&nElement))
        {
            delete pWhatName;
            pWhatName = ((ControlDef*)pNameKontext->GetObject(nElement));
            pImpl->pControlsObj = pImpl->pControlsObjs[nControlsObj++];
            pImpl->pControlsObj->SetName(pWhatName->pData->Kurzname);

            if ( pWhatName->pData->aUId.IsNumeric() )
                pImpl->pControlsObj->PutULong(pWhatName->pData->aUId.GetNum());
            else
                pImpl->pControlsObj->PutString(pWhatName->pData->aUId.GetStr());

            pShortNames->Insert(pWhatName->pData->Kurzname,pWhatName->pData->aUId,nSequence);

            SbxVariable *pMember;
            if ( ! (pMember = pImpl->pControlsObj->Find(CUniString("ID"),SbxCLASS_DONTCARE)) )
            {
                pMember = new SbxProperty(CUniString("ID"),SbxULONG);
                pImpl->pControlsObj->Insert(pMember);
            }
            if ( pWhatName->pData->aUId.IsNumeric() )
                pMember->PutULong(pWhatName->pData->aUId.GetNum());
            else
                pMember->PutString(pWhatName->pData->aUId.GetStr());

            if ( ! (pMember = pImpl->pControlsObj->Find(CUniString("name"),SbxCLASS_DONTCARE)) )
            {
                pMember = NULL;
            }
            else
                pMember->PutString(pWhatName->pData->Kurzname);

            return pImpl->pControlsObj;
        }

        /// Nach slots suchen
        if (pSIds && pSIds->Seek_Entry(pWhatName,&nElement))
        {
            SbxTransportMethodRef pMyVar;
            pMyVar = pImpl->pMyVars[nMyVar++];
            if ( nMyVar >= VAR_POOL_SIZE )
                nMyVar = 0;
            delete pWhatName;
            pWhatName = ( (ControlDef*)pSIds->GetObject( nElement ) );
            pMyVar->SetName( pWhatName->pData->Kurzname );

            if ( pWhatName->pData->aUId.IsNumeric() )
            {
                pMyVar->SetUserData( ID_Dispatch );
                pMyVar->nValue = pWhatName->pData->aUId.GetNum();
                pShortNames->Insert( Str, pWhatName->pData->aUId, nSequence );
            }
            else
            {
                pMyVar->SetUserData( ID_UNODispatch );
                pMyVar->aUnoSlot = pWhatName->pData->aUId.GetStr();
            }
            return pMyVar;
        }

        /// es kann sich noch um eine SlotID handeln, die numerisch abgefragt wird, statt ausgeführt zu werden
        if ( Str.Copy( Str.Len()-3, 3 ).CompareIgnoreCaseToAscii("_ID") == COMPARE_EQUAL && pSIds )
        {
            delete pWhatName;
            pWhatName = new ControlDef( Str.Copy( 0, Str.Len()-3 ), SmartId() );
            if ( pSIds->Seek_Entry( pWhatName, &nElement ) )
            {   // Nach slots suchen
                SbxVariable *pReturn = new SbxVariable;
                delete pWhatName;
                pWhatName = ( (ControlDef*)pSIds->GetObject( nElement ) );
                pReturn->SetName( pWhatName->pData->Kurzname );

                if ( pWhatName->pData->aUId.IsNumeric() )
                    pReturn->PutULong(pWhatName->pData->aUId.GetNum());
                else
                    pReturn->PutString(pWhatName->pData->aUId.GetStr());
                return pReturn;
            }
        }
        if ( !pImpl->bDebugFindNoErrors )
        {
            ADD_ERROR(SbxERR_PROC_UNDEFINED,GEN_RES_STR1(S_UNKNOWN_SLOT_CONTROL, Str) );
            if ( bQuietErrors )
            {       // Vorsichtshalber Control, falls noch ´ne Methode Folgt.
                pImpl->pControlsObj->SetName(Str);
                pImpl->pControlsObj->SetUserData( ID_ErrorDummy );
                return pImpl->pControlsObj;
            }
        }
    }
    return NULL;
}

String TestToolObj::GetRevision( String const &aSourceIn )
{
    // search $Revision: 1.12 $
    xub_StrLen nPos;
    if ( ( nPos = aSourceIn.SearchAscii( "$Revision:" ) ) != STRING_NOTFOUND )
        return aSourceIn.Copy( nPos+ 10, aSourceIn.SearchAscii( "$", nPos+10 ) -nPos-10);
    else
        return String::CreateFromAscii("No Revision found");
}

BOOL TestToolObj::CError( ULONG code, const String& rMsg, xub_StrLen l, xub_StrLen c1, xub_StrLen c2 )
{
    bWasPrecompilerError = TRUE;
    if ( aCErrorHdl.IsSet() )
    {
        ErrorEntry aErrorEntry( code, rMsg, l, c1, c2 );
        return (BOOL)aCErrorHdl.Call( &aErrorEntry );
    }
    else
    {
        ADD_ERROR( code, rMsg )
        return TRUE;
    }
}

void TestToolObj::CalcPosition( String const &aSource, xub_StrLen nPos, xub_StrLen &l, xub_StrLen &c )
{
    l = 1;
    xub_StrLen nAkt = 0;
    xub_StrLen nNext;
    while ( (nNext = aSource.Search( '\n', nAkt )) != STRING_NOTFOUND && nNext < nPos )
    {
        l++;
        nAkt = nNext+1;
    }
    c = nPos - nAkt;
}


#define CATCH_LABEL         CUniString( "ctch" )
#define CATCHRES_LABEL      CUniString( "ctchres" )
#define ENDCATCH_LABEL      CUniString( "endctch" )

BOOL IsAlphaChar( sal_Unicode cChar )
{
    return  ( cChar >= 'a' && cChar <= 'z' ) ||
            ( cChar >= 'A' && cChar <= 'Z' );
}

BOOL IsInsideString( const String& aSource, const xub_StrLen nStart )
{
    BOOL bInside = FALSE;
    xub_StrLen nPos = nStart-1;

    while ( nPos && aSource.GetChar(nPos) != _CR && aSource.GetChar(nPos) != _LF )
    {
        if ( aSource.GetChar(nPos) == '"' )
            bInside = !bInside;
        nPos--;
    }
    return bInside;
}

BOOL IsValidHit( const String& aSource, const xub_StrLen nStart, const xub_StrLen nEnd )
{
    return !IsAlphaChar( aSource.GetChar(nStart-1) ) && !IsAlphaChar( aSource.GetChar(nEnd+1))
        && !IsInsideString( aSource, nStart );
}


xub_StrLen TestToolObj::ImplSearch( const String &aSource, const xub_StrLen nStart, const xub_StrLen nEnd, const String &aSearch, const xub_StrLen nSearchStart )
{
    xub_StrLen nPos = aSource.Search( aSearch, std::max( nSearchStart, nStart ) );
    if ( nPos > nEnd - aSearch.Len() || nPos == STRING_NOTFOUND )
        return STRING_NOTFOUND;
    else
    {
        if ( IsValidHit( aSource, nPos, nPos+aSearch.Len()-1 ) )
            return nPos;
        else
            return ImplSearch( aSource, nStart, nEnd, aSearch, nPos+aSearch.Len() );
    }
}

xub_StrLen TestToolObj::PreCompilePart( String &aSource, xub_StrLen nStart, xub_StrLen nEnd, String aFinalErrorLabel, USHORT &nLabelCount )
{
    xub_StrLen nTry,nCatch,nEndcatch;
    xub_StrLen nTry2;
    if( (nTry = ImplSearch( aSource, nStart, nEnd, CUniString("try"), nStart )) == STRING_NOTFOUND )
        return nEnd;
    if ( (nCatch = ImplSearch( aSource, nStart, nEnd, CUniString("catch"), nTry )) == STRING_NOTFOUND )
    {
        xub_StrLen l,c;
        CalcPosition( aSource, nTry, l, c );
        CError( SbERR_BAD_BLOCK, CUniString("catch"), l, c, c+2 );
        return nEnd;
    }
    if ( (nEndcatch = ImplSearch( aSource, nStart, nEnd, CUniString("endcatch"), nCatch )) == STRING_NOTFOUND )
    {
        xub_StrLen l,c;
        CalcPosition( aSource, nCatch, l, c );
        CError( SbERR_BAD_BLOCK, CUniString("endcatch"), l, c, c+4 );
        return nEnd;
    }

    nLabelCount++;
    String aStr = String::CreateFromInt32( nLabelCount );
    String aCatchLabel(CATCH_LABEL);
    aCatchLabel += aStr;
    String aCatchresLabel(CATCHRES_LABEL);
    aCatchresLabel += aStr;
    String aEndcatchLabel( ENDCATCH_LABEL);
    aEndcatchLabel += aStr;

    while ( !WasPrecompilerError() && (nTry2 = ImplSearch( aSource, nStart, nEnd, CUniString("try"), nTry+1 )) != STRING_NOTFOUND )
    {   // Wir rekursieren erstmal mit dem 2. Try
        if ( nTry2 < nCatch )
            nEnd += PreCompilePart( aSource, nTry2, nEndcatch+8, aCatchLabel, nLabelCount ) - nEndcatch-8;
        else
            nEnd = PreCompilePart( aSource, nTry2, nEnd, aFinalErrorLabel, nLabelCount );

        if ( (nCatch = ImplSearch( aSource, nStart, nEnd, CUniString("catch"), nTry )) == STRING_NOTFOUND )
        {
            xub_StrLen l,c;
            CalcPosition( aSource, nTry, l, c );
            CError( SbERR_BAD_BLOCK, CUniString("catch"), l, c, c+2 );
            return nEnd;
        }
        if ( (nEndcatch = ImplSearch( aSource, nStart, nEnd, CUniString("endcatch"), nCatch )) == STRING_NOTFOUND )
        {
            xub_StrLen l,c;
            CalcPosition( aSource, nCatch, l, c );
            CError( SbERR_BAD_BLOCK, CUniString("endcatch"), l, c, c+4 );
            return nEnd;
        }
    }

    String aReplacement;
    int nTotalLength = -3 -5 -8;    // try, catch und endcatch fallen raus

    aReplacement.AppendAscii( "on error goto " );
    aReplacement += aCatchLabel;
    aSource.SearchAndReplaceAscii( "try", aReplacement, nTry );
    nTotalLength += aReplacement.Len();


//          on error goto endcse
//          goto endctchXX
//          ctchXX:
//          if err = 35 or err = 18 then : resume : endif :
//          MaybeAddErr
//          on error goto endcse
//          resume ctchresXX
//          ctchresXX:
    aReplacement.Erase();
    aReplacement.AppendAscii( "on error goto " );
    aReplacement += aFinalErrorLabel;
    aReplacement.AppendAscii( " : goto " );
    aReplacement += aEndcatchLabel;
    aReplacement.AppendAscii( " : " );
    aReplacement += aCatchLabel;
    aReplacement.AppendAscii( ": if err = 35 or err = 18 then : on error goto 0 : resume : endif" );
    aReplacement.AppendAscii( " : MaybeAddErr : on error goto " );
    aReplacement += aFinalErrorLabel;
    aReplacement.AppendAscii( " : resume " );
    aReplacement += aCatchresLabel;
    aReplacement.AppendAscii( " : " );
    aReplacement += aCatchresLabel;
    aReplacement.AppendAscii( ": " );
    aSource.SearchAndReplaceAscii( "catch", aReplacement, nCatch );
    nTotalLength += aReplacement.Len();


    aReplacement.Erase();
    aReplacement.AppendAscii("ClearError : ");
    aReplacement += aEndcatchLabel;
    aReplacement.AppendAscii(": ");
    aSource.SearchAndReplaceAscii( "endcatch", aReplacement, nEndcatch );
    nTotalLength += aReplacement.Len();

    if ( aSource.Len() >= STRING_MAXLEN )
    {
        xub_StrLen l,c;
        CalcPosition( aSource, nEndcatch, l, c );
        CError( SbERR_PROG_TOO_LARGE, CUniString("endcatch"), l, c, c+2 );
    }

    return nEnd + nTotalLength;
}


void TestToolObj::PreCompileDispatchParts( String &aSource, String aStart, String aEnd, String aFinalLable )
{
    USHORT nLabelCount = 0;
    xub_StrLen nPartPos = 0;

    while ( !WasPrecompilerError() && (nPartPos = ImplSearch( aSource, nPartPos, aSource.Len(), aStart )) != STRING_NOTFOUND )
    {
        xub_StrLen nEndPart = ImplSearch( aSource, nPartPos, aSource.Len(), aEnd );
        if ( nEndPart == STRING_NOTFOUND )
            return;
        nPartPos = PreCompilePart( aSource, nPartPos, nEndPart, aFinalLable, nLabelCount );
        nPartPos += aEnd.Len();
    }
}


BOOL TestToolObj::WasPrecompilerError()
{
    return bWasPrecompilerError;
}

String TestToolObj::PreCompile( String const &aSourceIn )
{
    // Im CTOR zu früh, und hier grade nicg rechtzeitig. Start und Stop von Programmausführung
    StartListening( ((StarBASIC*)GetParent())->GetBroadcaster(), TRUE );

    xub_StrLen nTestCase;
    xub_StrLen nEndCase;
    xub_StrLen nStartPos = 0;
    String aSource(aSourceIn);
    bWasPrecompilerError = FALSE;

HACK("Ich gestehe alles: Ich war zu faul das richtig zu machen.")
    aSource = String(' ').Append( aSource );        // Da Schlüsselworte an Position 0 sonst nicht gefunden werden


//      Erstmal alle "'" Kommentare raus

    xub_StrLen nComment;
    while ( (nComment = aSource.SearchAscii("'",nStartPos)) != STRING_NOTFOUND )
    {
        USHORT nStringEndCount = 0;
        xub_StrLen nIndex = nComment;
        while ( nIndex && aSource.GetChar(nIndex) != '\n' )
        {
            if ( aSource.GetChar(nIndex) == '"' )
                nStringEndCount++;
            nIndex--;
        }
        if ( (nStringEndCount & 1) == 0 )       // Wir waren also nicht innerhalb eines Strings
        {
            xub_StrLen nComEnd = aSource.SearchAscii("\n",nComment);

            while ( aSource.GetChar(nComEnd) == _CR || aSource.GetChar(nComEnd) == _LF )
                nComEnd--;

            nComEnd++;

            aSource.Erase(nComment,nComEnd-nComment);
        }
        else
            nComment++;
        nStartPos = nComment;
    }


    PreCompileDispatchParts( aSource, CUniString("sub"), CUniString("end sub"), CUniString("0") );
    PreCompileDispatchParts( aSource, CUniString("function"), CUniString("end function"), CUniString("0") );
    PreCompileDispatchParts( aSource, CUniString("testcase"), CUniString("endcase"), CUniString("endcse") );


    xub_StrLen nMainPos = ImplSearch( aSource, 0, aSource.Len(), CUniString("sub main") );
    aSource.SearchAndReplaceAscii("sub main",CUniString("Sub Main StartUse : LoadIncludeFiles : FinishUse "), nMainPos );
    if ( aSource.Len() >= STRING_MAXLEN )
    {
        xub_StrLen l,c;
        CalcPosition( aSource, nMainPos, l, c );
        CError( SbERR_PROG_TOO_LARGE, CUniString("endcatch"), l, c, c+2 );
    }

    while ( (nTestCase = ImplSearch( aSource, 0, aSource.Len(), CUniString("testcase") ) ) != STRING_NOTFOUND )
    {
        xub_StrLen nTcEnd = aSource.SearchAscii("\n",nTestCase);

        while ( aSource.GetChar(nTcEnd) == _CR || aSource.GetChar(nTcEnd) == _LF )
            nTcEnd--;

        nTcEnd++;

        if ( aSource.SearchAscii(":",nTestCase) < nTcEnd )
            nTcEnd = aSource.SearchAscii(":",nTestCase) -1;
        String aSuffix = aSource.Copy(nTestCase+8,nTcEnd-nTestCase-8);
        USHORT nOldLen;
        do
        {
            nOldLen = aSuffix.Len();
            aSuffix.EraseLeadingAndTrailingChars( ' ' );
            aSuffix.EraseLeadingAndTrailingChars( 0x09 );
        } while ( nOldLen != aSuffix.Len() );
        aSource.Erase(nTestCase,nTcEnd-nTestCase);
        aSource.Insert(CUniString("Sub ").Append(aSuffix).AppendAscii(" CaseLog \"").Append(aSuffix).AppendAscii("\" : on error goto endcse : TestEnter "),nTestCase);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Attention!!! The lable endsub is officially used to exit a sub instead of using 'exit sub' or 'return'
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    while ( (nEndCase = ImplSearch( aSource, 0, aSource.Len(), CUniString("endcase") ) ) != STRING_NOTFOUND )
        aSource.SearchAndReplaceAscii("endcase",CUniString("goto endsub : endcse: if ( err = 35 and StopOnSyntaxError ) or err = 18 then : on error goto 0 : resume : endif : MaybeAddErr : ExceptLog : resume endcse_res : endcse_res: on error goto 0 : endsub: TestExit : ClearError : CaseLog \"\" : end sub "), nEndCase );

    if ( aSource.Len() >= STRING_MAXLEN )
    {
        xub_StrLen l,c;
        CalcPosition( aSource, 0, l, c );
        CError( SbERR_PROG_TOO_LARGE, CUniString("endcatch"), l, c, c+2 );
    }
    return aSource;
}

void TestToolObj::AddToListByNr( CNames *&pControls, ControlItemUId *&pNewItem )
{
    USHORT nNr;
    if ( pControls->Seek_Entry( pNewItem, &nNr ) )
    {
        AddName( pControls->GetObject(nNr)->pData->Kurzname, pNewItem->pData->Kurzname );
        delete pNewItem;
        pNewItem = (ControlItemUId*)pControls->GetObject(nNr);  // für einfügen der Söhne
    }
    else
    {
        ControlItem* pNI = pNewItem;
        pControls->C40_PTR_INSERT(ControlItem,pNI);
    }
}

IMPL_LINK( TestToolObj, ReturnResultsLink, CommunicationLink*, pCommLink )
{
    return ReturnResults( pCommLink->GetServiceData() );
}

void TestToolObj::ReadHidLstByNumber()
{
    // Die Hid.Lst nach Nummern sortiert einlesen
    if ( !pReverseUIds )
    {
        String aName = (pImpl->aHIDDir + DirEntry(CUniString("hid.lst"))).GetFull();

        TTExecutionStatusHint aHint( TT_EXECUTION_SHOW_ACTION, String(ResId(S_READING_LONGNAMES)), aName );
        GetTTBroadcaster().Broadcast( aHint );

        ReadFlat( aName, pReverseUIds, FALSE );
        {
            TTExecutionStatusHint aHint( TT_EXECUTION_HIDE_ACTION );
            GetTTBroadcaster().Broadcast( aHint );
        }
    }
}

void TestToolObj::SortControlsByNumber( BOOL bIncludeActive )
{
    // Die Controls einmal hirarchisch und einmal alle flach nach nummer sortiert
    if ( !pReverseControls && !pReverseControlsSon && pControls )
    {
        pReverseControls = new CNames;
        pReverseControlsSon = new CNames;
        USHORT nWin,nCont;
        const String aSl('/');
        for ( nWin = 0 ; nWin < pControls->Count() ; nWin++ )
        {
            String aFatherName( pControls->GetObject(nWin)->pData->Kurzname );
            ControlItemUId *pNewFather = new ControlItemUIdSon(aFatherName,pControls->GetObject(nWin)->pData->aUId);
            AddToListByNr( pReverseControlsSon, pNewFather );
            if (! ((ControlItemUIdSon*)pNewFather)->GetSons() )
                ((ControlItemUIdSon*)pNewFather)->Sons( new CNames );

            // Existieren Söhne, diese in beide Listen eintragen
            CNames *pControlList = ((ControlItemSon*)pControls->GetObject(nWin))->GetSons();
            if ( pControlList )
                for ( nCont = 0 ; nCont < pControlList->Count() ; nCont++ )
                {
                    ControlItemUId *pNewItem;

                    String aCombinedName( aFatherName );
                    aCombinedName.AppendAscii( ":" );
                    aCombinedName.Append( pControlList->GetObject(nCont)->pData->Kurzname );
                    pNewItem = new ControlItemUId( aCombinedName, pControlList->GetObject(nCont)->pData->aUId );
                    AddToListByNr( pReverseControls, pNewItem );

                    pNewItem = new ControlItemUId( pControlList->GetObject(nCont)->pData->Kurzname, pControlList->GetObject(nCont)->pData->aUId );
                    AddToListByNr( ((ControlItemUIdSon*)pNewFather)->GetSons(), pNewItem );
                }
        }
        if ( !bIncludeActive )
        {
            ControlItem *pZeroItem = new ControlItemUId( UniString(), SmartId(0) );
            USHORT nNr;
            if ( pReverseControls->Seek_Entry( pZeroItem, &nNr ) )
            {
                pReverseControls->DeleteAndDestroy( nNr );
// um VorlagenLaden/UntergeordneteIniDatei/SpeichernDlg/OrdnerDlg/OeffnenDlg/MessageBox/LetzteVersion/GrafikEinfuegenDlg/FarbeDlg/ExportierenDlg/DruckerEinrichten/DruckenDlg/DateiEinfuegenDlg/Active zu verhindern
            }
/*          if ( pReverseControlsSon->Seek_Entry( pZeroItem, &nNr ) )
            {
                pReverseControlsSon->DeleteAndDestroy( nNr );
// um VorlagenLaden/UntergeordneteIniDatei/SpeichernDlg/OrdnerDlg/OeffnenDlg/MessageBox/LetzteVersion/GrafikEinfuegenDlg/FarbeDlg/ExportierenDlg/DruckerEinrichten/DruckenDlg/DateiEinfuegenDlg/Active zu verhindern
            }*/
            delete pZeroItem;
        }
    }
}


BOOL TestToolObj::ReturnResults( SvStream *pIn )
{

    USHORT nId;
    ULONG nClearSequence = 0;
    BOOL bSequenceOK = TRUE;
    CNames *pReverseControlsKontext = NULL;

    CRetStream *pRetStream = new CRetStream(pIn);

    pRetStream->Read( nId );
    while( !pIn->IsEof() )
    {
    switch( nId )
    {
        case SIReturn:
        {
            USHORT nRet,nParams;
            SmartId aUId;
            pRetStream->Read(nRet);
            if ( pRetStream->GetNextType() == BinString )
            {
                String aUStrId;     // UniqueStringID Used for Mozilla Integration
                pRetStream->Read( aUStrId );
                aUId = SmartId( aUStrId );
            }
            else
            {
                ULONG nUId;
                pRetStream->Read( nUId );         // bei Sequence einfach die Sequence
                aUId = SmartId( nUId );
            }
            pRetStream->Read(nParams);

            USHORT nNr1 = 0;
            ULONG nLNr1 = 0;
            String aString1;
            BOOL bBool1 = FALSE;
            SbxValueRef xValue1 = new SbxValue;

            if( nParams & PARAM_USHORT_1 )
                pRetStream->Read( nNr1 );
            if( nParams & PARAM_ULONG_1 )
                pRetStream->Read( nLNr1 );
            if( nParams & PARAM_STR_1 )
            {
                pRetStream->Read( aString1 );
                ReplaceNumbers ( aString1 );
            }
            else
                aString1.Erase();
            if( nParams & PARAM_BOOL_1 )
                pRetStream->Read( bBool1 );
            if( nParams & PARAM_SBXVALUE_1 )
                pRetStream->Read( *xValue1 );
            switch (nRet)
            {
                case RET_Sequence:
                    {
                        ULONG nUId = aUId.GetNum();
                        if ( nSequence != nUId )
                        {
                            bSequenceOK = FALSE;
                            ADD_ERROR(SbxERR_BAD_ACTION, GEN_RES_STR2(S_RETURN_SEQUENCE_MISSMATCH, String::CreateFromInt64(nUId), String::CreateFromInt64(nSequence)) );
                        }
                        else
                        {
                            nClearSequence = nUId;
                        }
                    }
                    break;
                case RET_Value:
                    if ( pImpl->pNextReturn )
                    {
//                                              ULONG nUserData = pImpl->pNextReturn->GetParent()->GetUserData();
//                                              pImpl->pNextReturn->GetParent()->SetUserData(0);
//                                              if ( nUId == pImpl->pNextReturn->GetParent()->GetULong() )
                        if ( aNextReturnId.Equals( aUId ) )
                        {
                            if( nParams & PARAM_ULONG_1 )       pImpl->pNextReturn->PutULong( nLNr1 );
                            if( nParams & PARAM_USHORT_1 )      pImpl->pNextReturn->PutUShort( nNr1 );
                            if( nParams & PARAM_STR_1 )         pImpl->pNextReturn->PutString( aString1 );
                            if( nParams & PARAM_BOOL_1 )        pImpl->pNextReturn->PutBool( bBool1 );
                            if( nParams & PARAM_SBXVALUE_1 )
                            {
                                SbxValues aValues( SbxDATE );
                                xValue1->Get( aValues );
                                pImpl->pNextReturn->Put( aValues );
                            }
                        }
                        else
                        {
                            ADD_ERROR(SbxERR_BAD_ACTION, GEN_RES_STR0(S_RETURNED_VALUE_ID_MISSMATCH) )
                        }
//                                              pImpl->pNextReturn->GetParent()->SetUserData(nUserData);
                        pImpl->pNextReturn = NULL;
                    }
                    else
                    {
                        ADD_ERROR(SbxERR_BAD_ACTION, GEN_RES_STR0(S_RETURNED_VALUE_NO_RECEIVER) )
                    }
                    break;
                case RET_WinInfo:
                    {
                        if ( !pReverseControls && !pReverseControlsSon )
                            pReverseControlsKontext = NULL;

                        ReadHidLstByNumber();
                        SortControlsByNumber();

                        // Alle Slots nach Nummer Sortiert
                        if ( !pReverseSlots && pSIds )
                        {
                            pReverseSlots = new CNames;
                            USHORT nWin;
                            const String aSl('/');
                            for ( nWin = 0 ; nWin < pSIds->Count() ; nWin++ )
                            {
                                ControlItemUId *pNewItem = new ControlItemUId(pSIds->GetObject(nWin)->pData->Kurzname,pSIds->GetObject(nWin)->pData->aUId);
                                AddToListByNr( pReverseSlots, pNewItem );
                            }
                        }

                        WinInfoRec *pWinInfo = new WinInfoRec;
                        pWinInfo->aUId = aUId.GetText();
                        pWinInfo->nRType = (USHORT)nLNr1;   // just ULONG for Transport, data is always USHORT
                        pWinInfo->aRName = aString1;
                        pWinInfo->bIsReset = bBool1;
                        pWinInfo->aKurzname.Erase();
                        pWinInfo->aSlotname.Erase();

                        // eventuell den Kontext feststellen. Passiert nur beim ersten Eintrag nach reset
                        if ( !pReverseControlsKontext && pReverseControlsSon )
                        {
                            USHORT nNr;
                            ControlItem *pNewItem = new ControlItemUId( String(), aUId );
                            if ( pReverseControlsSon->Seek_Entry(pNewItem,&nNr) )
                            {
                                pReverseControlsKontext = ((ControlItemUIdSon*)pReverseControlsSon->GetObject(nNr))->GetSons();
                                pWinInfo->aKurzname = CUniString("*");
                            }
                            else
                                pReverseControlsKontext = pReverseControls;

                            delete pNewItem;
                        }

                        // Reset. Muß nach bestimmen des Kontext stehen, da sonst mit dem reset-record
                        // der Kontext falsch gesetzt wird.
                        if ( pWinInfo->bIsReset )
                            pReverseControlsKontext = NULL; // Reihenfolge wichtig!


                        // Kurzname feststellen
                        if ( pReverseControlsKontext )
                        {
                            USHORT nNr;
                            ControlItem *pNewItem = new ControlItemUId( String(), aUId );
                            if ( pReverseControlsKontext->Seek_Entry(pNewItem,&nNr) )
                            {
//                                SmartId aID = pReverseControlsKontext->GetObject(nNr)->pData->aUId;
                                pWinInfo->aKurzname += pReverseControlsKontext->GetObject(nNr)->pData->Kurzname;
                            }
                            delete pNewItem;
                        }

                        // Slotname feststellen
                        if ( pReverseSlots )
                        {
                            USHORT nNr;
                            ControlItem *pNewItem = new ControlItemUId( String(), aUId );
                            if ( pReverseSlots->Seek_Entry(pNewItem,&nNr) )
                                pWinInfo->aSlotname = pReverseSlots->GetObject(nNr)->pData->Kurzname;
                            delete pNewItem;
                        }

                        // Langname feststellen
                        if ( aUId.IsNumeric() )
                        {
                            if ( pReverseUIds )
                            {
                                USHORT nNr;
                                ControlItem *pNewItem = new ControlItemUId( String(), aUId );
                                if ( pReverseUIds->Seek_Entry(pNewItem,&nNr) )
                                    pWinInfo->aLangname = pReverseUIds->GetObject(nNr)->pData->Kurzname;
                                delete pNewItem;
                            }
                        }
                        else
                        {   // use the String ID since there is no LongName in hid.lst
                            pWinInfo->aLangname = aUId.GetStr();
                        }

                        aWinInfoHdl.Call( pWinInfo );

                        delete pWinInfo;
                    }
                    break;
                case RET_ProfileInfo:
                    {
                        ULONG nUId = aUId.GetNum();
                        if ( nParams & PARAM_STR_1 )
                        {
                            DirEntry FilePath = pImpl->aLogFileBase + DirEntry(DirEntry(aLogFileName).GetBase().AppendAscii(".prf"));
                            SvFileStream aStrm( FilePath.GetFull(), STREAM_STD_WRITE );
                            if( aStrm.IsOpen() )
                            {
                                aString1.ConvertLineEnd(LINEEND_CRLF);
                                aStrm.Seek(STREAM_SEEK_TO_END);
                                aStrm << ByteString( aString1, RTL_TEXTENCODING_UTF8 ).GetBuffer();
                                aStrm.Close();
                            }
                        }
                        if ( nParams & PARAM_ULONG_1 )
                        {
                            switch ( nUId )
                            {
                                case S_ProfileReset:    // nLNr1 = Anzahl Borders
                                {
                                    pImpl->nNumBorders = (USHORT)nLNr1;     // Borders are 0 to 4
                                    USHORT i;
                                    for ( i=0 ; i<4 ; i++ )
                                        pImpl->naValBorders[i] = 0;

                                    for ( i=0 ; i<5 ; i++ )
                                    {
                                        pImpl->naNumEntries[i] = 0;
                                        pImpl->naRemoteTime[i] = 0;
                                        pImpl->naLocalTime[i] = 0;
                                    }
                                    break;
                                }
                                case S_ProfileBorder1:  // nLNr1 = Border1 in ms
                                case S_ProfileBorder2:  // nLNr1 = Border2 in ms
                                case S_ProfileBorder3:  // nLNr1 = Border3 in ms
                                case S_ProfileBorder4:  // nLNr1 = Border4 in ms
                                {
                                    pImpl->naValBorders[ nUId - S_ProfileBorder1 ] = nLNr1;
                                    break;
                                }
                                case S_ProfileTime:     // nLNr1 = remote Zeit des Befehls
                                {
                                    USHORT i;
                                    for ( i=0 ; i<pImpl->nNumBorders &&
                                        pImpl->naValBorders[i] <= nLNr1 ; i++ ) {};

                                    pImpl->naNumEntries[ i ]++;
                                    pImpl->naRemoteTime[ i ] += nLNr1;
                                    pImpl->naLocalTime[ i ] += Time::GetSystemTicks() - pImpl->LocalStarttime;

    #if OSL_DEBUG_LEVEL > 1
                                    if ( nLNr1 > (Time::GetSystemTicks() - pImpl->LocalStarttime) )
                                    {
                                        String aLine = CUniString("Testtoolzeit(").Append(String::CreateFromInt64(Time::GetSystemTicks() - pImpl->LocalStarttime)).AppendAscii(") kleiner Officezeit(").Append(String::CreateFromInt64(nLNr1)).AppendAscii(")\n");
                                        DirEntry FilePath = pImpl->aLogFileBase + DirEntry(DirEntry(aLogFileName).GetBase().AppendAscii(".prf"));
                                        SvFileStream aStrm( FilePath.GetFull(), STREAM_STD_WRITE );
                                        if( aStrm.IsOpen() )
                                        {
                                            aLine.ConvertLineEnd(LINEEND_CRLF);
                                            aStrm.Seek(STREAM_SEEK_TO_END);
                                            aStrm << ByteString( aLine, RTL_TEXTENCODING_UTF8 ).GetBuffer();
                                            aStrm.Close();
                                        }
                                    }
    #endif

                                    break;
                                }
                                case S_ProfileDump:     // Gibt die daten aus.
                                {
                                    if ( pImpl->nNumBorders == 0 )  // Also keine alte Rückmeldung vom Office
                                        break;
                                    DirEntry FilePath = pImpl->aLogFileBase + DirEntry(DirEntry(aLogFileName).GetBase().AppendAscii(".prf"));
                                    SvFileStream aStrm( FilePath.GetFull(), STREAM_STD_WRITE );
                                    if( aStrm.IsOpen() )
                                    {
                                        String aProfile;
                                        USHORT i;

                                        aProfile += String().Expand(15);
                                        for ( i=0 ; i<pImpl->nNumBorders ; i++ )
                                            aProfile += (CUniString("< ").Append(String::CreateFromInt64(pImpl->naValBorders[i]))).Expand(20);

                                        aProfile += (CUniString(">= ").Append(TTFormat::ms2s(pImpl->naValBorders[pImpl->nNumBorders-1])));

                                        aProfile += '\n';

                                        aProfile += CUniString("Ereignisse").Expand(15);
                                        for ( i=0 ; i<=pImpl->nNumBorders ; i++ )
                                            aProfile += TTFormat::ms2s(pImpl->naNumEntries[i]).Expand(20);

                                        aProfile += '\n';

                                        aProfile += CUniString("Server Zeit").Expand(15);
                                        for ( i=0 ; i<=pImpl->nNumBorders ; i++ )
                                            aProfile += TTFormat::ms2s(pImpl->naRemoteTime[i]).Expand(20);

                                        aProfile += '\n';

                                        aProfile += CUniString("Testtool Zeit").Expand(15);
                                        for ( i=0 ; i<=pImpl->nNumBorders ; i++ )
                                            aProfile += TTFormat::ms2s(pImpl->naLocalTime[i]).Expand(20);

                                        aProfile += '\n';

                                        aProfile += CUniString("Overhead p.e.").Expand(15);
                                        for ( i=0 ; i<=pImpl->nNumBorders ; i++ )
                                        {
                                            if ( pImpl->naNumEntries[i] > 0 )
                                                aProfile += TTFormat::ms2s((pImpl->naLocalTime[i]-pImpl->naRemoteTime[i])/pImpl->naNumEntries[i]).Expand(20);
                                            else
                                                aProfile += CUniString( "??" ).Expand(20);
                                        }

                                        aProfile += '\n';

                                        aProfile.ConvertLineEnd(LINEEND_CRLF);
                                        aStrm.Seek(STREAM_SEEK_TO_END);
                                        aStrm << ByteString( aProfile, RTL_TEXTENCODING_UTF8 ).GetBuffer();
                                        aStrm.Close();
                                    }
                                    break;
                                }
                                default:
                                    DBG_ERROR1("Unbekannter Sub Return Code bei Profile: %hu", nUId );
                                    break;
                            }
                        }
                    }
                    break;
                case RET_DirectLoging:
                    {
                        ULONG nUId = aUId.GetNum();
                        switch ( nUId )
                        {
                        case S_AssertError:
                            {
                                ADD_ASSERTION_LOG( aString1 );
//                              ADD_ERROR_LOG( aString1, aRun.GetModuleName(SbxNAME_SHORT_TYPES),
//                                  aRun.GetLine(), aRun.GetCol1(), aRun.GetCol2() );
                            }
                            break;
                        case S_QAError:
                            {
                                ADD_QA_ERROR_LOG( aString1 );
                            }
                            break;
                        default:
                            ;
                        }
                    }
                    break;
                case RET_MacroRecorder:
                    {
                        SortControlsByNumber( TRUE );
                        String aCommand,aControls,aControl,aULongNames,aULongName;
                        BOOL bWriteNewKontext = FALSE;

                        aControls.Erase();
                        // Kurzname feststellen
                        if ( pReverseControls )
                        {
                            USHORT nNr;
                            ControlItem *pNewItem = new ControlItemUId( String(), aUId );
                            if ( pReverseControls->Seek_Entry(pNewItem,&nNr) )
                                aControls = pReverseControls->GetObject(nNr)->pData->Kurzname;
                            delete pNewItem;
                        }
                        if ( !aControls.Len() )
                        {
                            aControls = String::CreateFromAscii("UnknownControl:UnknownControl");
                            Sound::Beep( SOUND_WARNING );
                        }

                        aULongNames.Erase();
                        if( (nParams & PARAM_ULONG_1) && (nNr1 & M_RET_NUM_CONTROL) )
                        {
                            if ( pReverseControls )
                            {
                                USHORT nNr;
                                ControlItem *pNewItem = new ControlItemUId( String(), SmartId( nLNr1 ) );
                                if ( pReverseControls->Seek_Entry(pNewItem,&nNr) )
                                    aULongNames = pReverseControls->GetObject(nNr)->pData->Kurzname;
                                delete pNewItem;
                            }
                            if ( !aULongNames.Len() )
                            {
                                aULongNames = String::CreateFromAscii("Unknown:Unknown");
                                Sound::Beep( SOUND_WARNING );
                            }

                            // now determin the best common kontext
                            USHORT i,j;
                            BOOL bFoundUlongName = FALSE, bFoundControl = FALSE;
                            // check for current kontext
                            for ( i = 0 ; !bFoundUlongName && i < aULongNames.GetTokenCount('/') ; i++ )
                                bFoundUlongName = aLastRecordedKontext.Equals( aULongNames.GetToken(i,'/').GetToken( 0,':') );

                            for ( j = 0 ; !bFoundControl && j < aControls.GetTokenCount('/') ; j++ )
                                bFoundControl = aLastRecordedKontext.Equals( aControls.GetToken(j,'/').GetToken( 0,':') );

                            if ( bFoundUlongName && bFoundControl )
                            {
                                aULongName = aULongNames.GetToken(i-1,'/').GetToken( 1,':');
                                aControl = aControls.GetToken(j-1,'/').GetToken( 1,':');
                            }
                            else
                            {   // see if we can find common kontext
                                BOOL bFound = FALSE;

                                String aCurrentKontext;
                                for ( i = 0 ; !bFound && i < aULongNames.GetTokenCount('/') ; i++ )
                                {
                                    aCurrentKontext = aULongNames.GetToken(i,'/').GetToken( 0,':');

                                    for ( j = 0 ; !bFound && j < aControls.GetTokenCount('/') ; j++ )
                                    {
                                        if ( aCurrentKontext.Equals( aControls.GetToken(j,'/').GetToken( 0,':') ) )
                                        {
                                            bFound = TRUE;
                                            aULongName = aULongNames.GetToken(i,'/').GetToken( 1,':');
                                            aControl = aControls.GetToken(j,'/').GetToken( 1,':');
                                            aLastRecordedKontext = aCurrentKontext;
                                            bWriteNewKontext = TRUE;
                                        }
                                    }
                                }
                                if ( !bFound )
                                {
                                    // check if both contain toplevel
                                    bFoundUlongName = FALSE;
                                    bFoundControl = FALSE;
                                    for ( i = 0 ; !bFoundUlongName && i < aULongNames.GetTokenCount('/') ; i++ )
                                        bFoundUlongName = aULongNames.GetToken(i,'/').GetToken( 0,':').Equals( aULongNames.GetToken(i,'/').GetToken( 1,':') );

                                    for ( j = 0 ; !bFoundControl && j < aControls.GetTokenCount('/') ; j++ )
                                        bFoundControl = aControls.GetToken(j,'/').GetToken( 0,':').Equals( aControls.GetToken(j,'/').GetToken( 1,':') );

                                    if ( bFoundUlongName && bFoundControl )
                                    {
                                        aULongName = aULongNames.GetToken(i-1,'/').GetToken( 1,':');
                                        aControl = aControls.GetToken(j-1,'/').GetToken( 1,':');
                                        if ( aLastRecordedKontext.Len() )
                                        {
                                            aLastRecordedKontext.Erase();
                                            bWriteNewKontext = TRUE;
                                        }
                                    }
                                    else
                                    {
                                        String aComment;
                                        aComment = CUniString( "'could not Determin common kontext\n" );
                                        Sound::Beep( SOUND_WARNING );
                                        aWriteStringHdl.Call( &aComment );
                                        aULongName = aULongNames.GetToken(i,'/');
                                        aControl = aControls.GetToken(j,'/');
                                    }
                                }
                            }

                        }
                        else
                        {   // we only have a Control
                            USHORT i;
                            BOOL bFoundControl = FALSE;
                            // check for current kontext
                            for ( i = 0 ; !bFoundControl && i < aControls.GetTokenCount('/') ; i++ )
                                bFoundControl = aLastRecordedKontext.Equals( aControls.GetToken(i,'/').GetToken( 0,':') );
                            if ( bFoundControl )
                                aControl = aControls.GetToken(i-1,'/').GetToken( 1,':');
                            else
                            {
                                aLastRecordedKontext = aControls.GetToken(0,'/').GetToken( 0,':');
                                bWriteNewKontext = TRUE;
                                aControl = aControls.GetToken(0,'/').GetToken( 1,':');
                            }

                        }


                        if ( bWriteNewKontext )
                        {
                            String aCommand = CUniString( "Kontext" );
                            if ( aLastRecordedKontext.Len() )
                            {
                                aCommand.AppendAscii ( " \"" );
                                aCommand += aLastRecordedKontext;
                                aCommand.AppendAscii ( "\"" );
                            }
                            aCommand.AppendAscii( "\n" );
                            aWriteStringHdl.Call( &aCommand );
                        }

                        aCommand = aControl;

                        // Add Method name
                        String aMethod = GetMethodName( nNr1 & ~M_RET_NUM_CONTROL );
                        aCommand += '.';
                        aCommand += aMethod;

                        BOOL bWasParam = FALSE;

                        if( nParams & PARAM_STR_1 )
                        {
                            bWasParam = TRUE;
                            aCommand.AppendAscii( " \"" );
                            if ( nNr1 & M_KEY_STRING )
                            {
                                USHORT nModify = 0;
                                USHORT nNewModify = 0;
                                BOOL bIsProsa = FALSE;
                                xub_StrLen i;
                                for ( i = 0; i < aString1.Len(); i++ )
                                {
                                    if ( ((USHORT)aString1.GetChar(i)) == 1 )   // we have a spechial char
                                    {
                                        i++;
                                        if ( !bIsProsa )
                                        {
                                            aCommand.AppendAscii( "<" );
                                            bIsProsa = TRUE;
                                        }
                                        else
                                            aCommand.AppendAscii( " " );

                                        USHORT nKeyCode = (USHORT)aString1.GetChar(i) & KEY_CODE;
                                        USHORT nNewModify = (USHORT)aString1.GetChar(i) & KEY_MODTYPE;
                                        if ( nNewModify != nModify )
                                        {   // generate modifiers
                                            USHORT nChanged = ( nNewModify ^ nModify );
                                            if ( nChanged & KEY_SHIFT )
                                            {
                                                aCommand += GetKeyName( KEY_SHIFT );
                                                aCommand.AppendAscii( " " );
                                            }
                                            if ( nChanged & KEY_MOD1 )
                                            {
                                                aCommand += GetKeyName( KEY_MOD1 );
                                                aCommand.AppendAscii( " " );
                                            }
                                            if ( nChanged & KEY_MOD2 )
                                            {
                                                aCommand += GetKeyName( KEY_MOD2 );
                                                aCommand.AppendAscii( " " );
                                            }
                                        }
                                        aCommand += GetKeyName( nKeyCode );
                                        nModify = nNewModify;
                                    }
                                    else
                                    {
                                        if ( bIsProsa )
                                        {
                                            aCommand.AppendAscii( ">" );
                                            bIsProsa = FALSE;
                                        }
                                        aCommand += aString1.GetChar(i);
                                        nModify = 0;
                                    }
                                }
                                if ( bIsProsa )
                                {
                                    aCommand.AppendAscii( ">" );
                                    bIsProsa = FALSE;
                                }
                            }
                            else
                            {
                                aCommand += aString1;
                            }
                            aCommand.AppendAscii( "\"" );
                        }
                        if( nParams & PARAM_ULONG_1 )
                        {
                            if ( bWasParam )
                                aCommand.AppendAscii( ", " );
                            else
                                aCommand.AppendAscii( " " );
                            bWasParam = TRUE;
                            if ( nNr1 & M_RET_NUM_CONTROL )
                            {
                                aCommand.Append( aULongName );
                            }
                            else
                            {
                                aCommand.Append( String::CreateFromInt64( nLNr1 ) );
                            }
                        }
                        if( nParams & PARAM_BOOL_1 )
                        {
                            if ( bWasParam )
                                aCommand.AppendAscii( ", " );
                            else
                                aCommand.AppendAscii( " " );
                            bWasParam = TRUE;
                            if ( bBool1 )
                                aCommand.AppendAscii( "true" );
                            else
                                aCommand.AppendAscii( "false" );
                        }

                        aCommand.AppendAscii( "\n" );

                           aWriteStringHdl.Call( &aCommand );
                    }
                    break;
                default:
                    DBG_ERROR1( "Unbekannter Return Code: %iu", nRet );
                    break;
            }

            break;
        }
        case SIReturnError:
        {
            String aString;
            SmartId aUId;
            if ( pRetStream->GetNextType() == BinString )
            {
                String aUStrId;     // UniqueStringID Used for Mozilla Integration
                pRetStream->Read( aUStrId );
                aUId = SmartId( aUStrId );
            }
            else
            {
                ULONG nUId;
                pRetStream->Read( nUId );         // bei Sequence einfach die Sequence
                aUId = SmartId( nUId );
            }
            pRetStream->Read( aString );
            ReplaceNumbers (aString);

            String aShortName;
            aShortName = pShortNames->GetName(aUId);
            aShortName.AppendAscii( " : " );

            String aTmpStr(aShortName);
            aTmpStr += aString;
            ADD_ERROR(SbxERR_BAD_ACTION, aTmpStr/*, nNr*/);
            break;
        }
        default:
            DBG_ERROR1( "Unbekannter Request im Return Stream Nr: %iu", nId );
        break;
    }
        if( !pIn->IsEof() )
            pRetStream->Read( nId );
        else
        {
            DBG_ERROR( "truncated input stream" );
        }

    }

    delete pRetStream;
    if ( bSequenceOK )
    {
        nSequence++;
        pShortNames->Invalidate( nClearSequence - KEEP_SEQUENCES );
    }

    bReturnOK = TRUE;

    return TRUE;
} // RetService::Request()

String TestToolObj::GetMethodName( ULONG nMethodId )
{
    USHORT nElement;
    if ( !Controls::pClasses )                        // Ist static, wird also nur einmal geladen
        ReadFlatArray( Controls::arClasses, Controls::pClasses );
    if ( Controls::pClasses )
    {
        for ( nElement = 0 ; nElement < Controls::pClasses->Count() ; nElement++ )
            if ( Controls::pClasses->GetObject(nElement)->pData->aUId.Equals( nMethodId ) )
                return Controls::pClasses->GetObject(nElement)->pData->Kurzname;
    }
    return String();
}

String TestToolObj::GetKeyName( USHORT nKeyCode )
{
    USHORT nElement;
    if ( !CmdStream::pKeyCodes )                        // Ist static, wird also nur einmal geladen
        ReadFlatArray( CmdStream::arKeyCodes, CmdStream::pKeyCodes );
    if ( CmdStream::pKeyCodes )
    {
        for ( nElement = 0 ; nElement < CmdStream::pKeyCodes->Count() ; nElement++ )
            if ( CmdStream::pKeyCodes->GetObject(nElement)->pData->aUId.Equals( nKeyCode ) )
                return CmdStream::pKeyCodes->GetObject(nElement)->pData->Kurzname;
    }
    return CUniString( "UnknownKeyCode" );
}

void TestToolObj::ReplaceNumbers(String &aText)
{
static ControlDefLoad __READONLY_DATA arRes_Type [] =
#include "res_type.hxx"

    static CNames *pRTypes = NULL;
    xub_StrLen nStart;
    xub_StrLen nGleich;
    xub_StrLen nEnd;
    xub_StrLen nStartPos = 0;
    ULONG nNumber;
    String aType;
    String aResult;
    BOOL bFound;

    while ( (nStart = aText.Search(StartKenn,nStartPos)) != STRING_NOTFOUND &&
            (nGleich = aText.SearchAscii("=",nStart+StartKenn.Len())) != STRING_NOTFOUND &&
            (nEnd = aText.Search(EndKenn,nGleich+1)) != STRING_NOTFOUND)
    {
        aType = aText.Copy(nStart,nGleich-nStart);
        nNumber = (ULONG)aText.Copy(nGleich+1,nEnd-nGleich-1).ToInt64();
        bFound = FALSE;
        if ( aType.CompareTo(UIdKenn) == COMPARE_EQUAL )
        {
            aResult = pShortNames->GetName(SmartId(nNumber));
            bFound = TRUE;
        }
        if ( aType.CompareTo(MethodKenn ) == COMPARE_EQUAL )
        {
            bFound = TRUE;
            aResult = GetMethodName( nNumber );
        }
        if ( aType.CompareTo(RcKenn ) == COMPARE_EQUAL )
        {
            bFound = TRUE;
            if ( !pRCommands )                 // Ist static, wird also nur einmal geladen
                ReadFlatArray( arR_Cmds, pRCommands );

            USHORT nElement;
            if ( pRCommands )
            {
                for ( nElement = 0 ; nElement < pRCommands->Count() ; nElement++ )
                    if ( pRCommands->GetObject(nElement)->pData->aUId.Equals( nNumber ) )
                    {
                        aResult = pRCommands->GetObject(nElement)->pData->Kurzname;
                        nElement = pRCommands->Count();
                    }
            }
        }
        if ( aType.CompareTo(TypeKenn ) == COMPARE_EQUAL )
        {
            bFound = TRUE;
            if ( !pRTypes )                 // Ist static, wird also nur einmal geladen
                ReadFlatArray( arRes_Type, pRTypes );

            USHORT nElement;
            if ( pRTypes )
            {
                for ( nElement = 0 ; nElement < pRTypes->Count() ; nElement++ )
                    if ( pRTypes->GetObject(nElement)->pData->aUId.Equals( nNumber ) )
                    {
                        aResult = pRTypes->GetObject(nElement)->pData->Kurzname;
                        nElement = pRTypes->Count();
                    }
            }
        }
        if ( aType.CompareTo(SlotKenn ) == COMPARE_EQUAL )
        {
            aResult = pShortNames->GetName(SmartId(nNumber));
            bFound = TRUE;
        }
        if ( aType.CompareTo(TabKenn ) == COMPARE_EQUAL )
        {
            if ( nNumber > nStart )
                aResult.Fill( (USHORT)nNumber - nStart +1 );
            else
                aResult = CUniString(" ");
            bFound = TRUE;
        }

        nStartPos = nStart;
        if ( bFound )
        {
            aText.Erase(nStart,nEnd+EndKenn.Len()-nStart);
            aText.Insert(aResult,nStart);
            nStartPos += aResult.Len();
        }
        else
            nStartPos += StartKenn.Len();
    }
}


SbTextType TestToolObj::GetSymbolType( const String &rSymbol, BOOL bWasControl )
{
    if (    rSymbol.CompareToAscii( "try" ) == COMPARE_EQUAL
        ||  rSymbol.CompareToAscii( "catch" ) == COMPARE_EQUAL
        ||  rSymbol.CompareToAscii( "endcatch" ) == COMPARE_EQUAL
        ||  rSymbol.CompareToAscii( "testcase" ) == COMPARE_EQUAL
        ||  rSymbol.CompareToAscii( "endcase" ) == COMPARE_EQUAL )
    {
        return TT_KEYWORD;
    }


    ControlDef WhatName( rSymbol, SmartId() );

    if ( bWasControl )
    {
        if ( !Controls::pClasses )                        // Ist static, wird also nur einmal geladen
            ReadFlatArray( Controls::arClasses, Controls::pClasses );

        if ( Controls::pClasses && Controls::pClasses->Seek_Entry( &WhatName ) )
            return TT_METHOD;
        else
            return TT_NOMETHOD;
    }

    // Die Controls durchsuchen
    if ( pControls )
    {
        USHORT nWin;

        for ( nWin = 0 ; nWin < pControls->Count() ; nWin++ )
        {
            if ( ((ControlDef*)pControls->GetObject( nWin ))->SonSeek_Entry( &WhatName ) )
                return TT_CONTROL;
        }
    }

    // Die Slots durchsuchen
    if ( pSIds && pSIds->Seek_Entry( &WhatName ) )
        return TT_SLOT;

    // Ist es ein RemoteCommand
    if ( !pRCommands )                 // Ist static, wird also nur einmal geladen
        ReadFlatArray( arR_Cmds, pRCommands );
    if ( pRCommands && pRCommands->Seek_Entry( &WhatName ) )
        return TT_REMOTECMD;

    // Wenns sonst nix war, dann vielleicht ein Lokales Kommando
    SbxVariable *pVar = SbxObject::Find( rSymbol, SbxCLASS_DONTCARE );
    if ( pVar && ( pVar->ISA(SbxMethod) || pVar->ISA(SbxProperty) ) )
    {
        return TT_LOCALCMD;
    }

    return SB_SYMBOL;   // Alles was hier landet ist vom Typ SB_SYMBOL und bleibt es auch
}


#undef P_FEHLERLISTE
#define P_FEHLERLISTE ((TestToolObj*)(GetParent()))->GetFehlerListe()

Controls::Controls( String aName )
: SbxObject( aName)
{
    pMethodVar = new SbxTransportMethod( SbxVARIANT );
    pMethodVar -> SetName( CUniString("Dummy") );
    Insert( pMethodVar );
//      pMethodVar = Make( CUniString("Dummy"), SbxCLASS_PROPERTY, SbxULONG );
}


Controls::~Controls()
{}


void Controls::ChangeListener()
{
    EndListening( pMethodVar->GetBroadcaster(), TRUE );
    GetParent()->StartListening( pMethodVar->GetBroadcaster(), TRUE );
}

void Controls::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                                const SfxHint& rHint, const TypeId& )
{}



SbxVariable* Controls::Find( const String& Str, SbxClassType Type)
{
    if ( !pClasses )                        // Ist static, wird also nur einmal geladen
        ReadFlatArray( arClasses, pClasses );

    if ( GetUserData() == ID_ErrorDummy )
    {
        pMethodVar->SetName(UniString(GetName()).AppendAscii(".").Append(Str));
        pMethodVar->SetUserData( ID_ErrorDummy );
        return pMethodVar;
    }


    USHORT nElement;
    ControlDef WhatName(Str,SmartId());
    if (pClasses && pClasses->Seek_Entry(&WhatName,&nElement))
    {
        pMethodVar->SetName(Str);
        ULONG nUId = pClasses->GetObject(nElement)->pData->aUId.GetNum();
        pMethodVar->nValue = nUId;

        if ( SbxValue::GetType() == SbxSTRING )
            pMethodVar->SetUserData( ID_StringControl );
        else
            pMethodVar->SetUserData( ID_Control );
        return pMethodVar;
    }
    else
    {   // mainly for ID and name
        SbxVariableRef Old = SbxObject::Find(Str, Type );
        if (Old)
            return Old;
        else if ( Str.CompareIgnoreCaseToAscii("ID") == COMPARE_EQUAL )
            return NULL;    // suppress generation of error in this case
    }
    ADD_ERROR(SbxERR_BAD_METHOD,GEN_RES_STR2(S_UNKNOWN_METHOD, GetName(), Str));
    if ( ((TestToolObj*)(GetParent()))->bQuietErrors )
    {
        pMethodVar->SetName(UniString(GetName()).AppendAscii(".").Append(Str));
        pMethodVar->SetUserData( ID_ErrorDummy );
        pMethodVar->nValue = 0;
        return pMethodVar;
    }
    return NULL;
}


String TTFormat::ms2s( ULONG nMilliSeconds )
{
    if ( nMilliSeconds < 100000 )       // 100 Sekunden
        return String::CreateFromInt64( nMilliSeconds );
    if ( nMilliSeconds < 100000*60 )    // 100 Minuten
        return String::CreateFromInt32( nMilliSeconds / 1000 ).AppendAscii("Sec");
    return String::CreateFromInt32( nMilliSeconds / 1000 / 60 ).AppendAscii("Min");
}



