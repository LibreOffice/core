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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <vcl/wrkwin.hxx>
#include <svl/rectitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <basic/sbstar.hxx>
#include <svl/stritem.hxx>
#include <svl/svdde.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>

#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxsids.hrc>
#include "helper.hxx"
#include <sfx2/docfile.hxx>

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
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;

    SfxDdeDocTopic_Impl( SfxObjectShell* pShell )
        : DdeTopic( pShell->GetTitle(SFX_TITLE_FULLNAME) ), pSh( pShell )
    {}

    virtual DdeData* Get( ULONG );
    virtual BOOL Put( const DdeData* );
    virtual BOOL Execute( const String* );
    virtual BOOL StartAdviseLoop();
    virtual BOOL MakeItem( const String& rItem );
};


SV_DECL_PTRARR( SfxDdeDocTopics_Impl, SfxDdeDocTopic_Impl *, 4, 4 )
SV_IMPL_PTRARR( SfxDdeDocTopics_Impl, SfxDdeDocTopic_Impl *)

//========================================================================

BOOL SfxAppEvent_Impl( ApplicationEvent &rAppEvent,
                       const String &rCmd, const String &rEvent )

/*  [Description]

    Checks if 'rCmd' of the event 'rEvent' is (without '(') and then assemble
    this data into a <ApplicationEvent>, which can be excecuted through
    <Application::AppEvent()>. If 'rCmd' is the given event 'rEvent', then
    TRUE is returned, otherwise FALSE.

    [Example]

    rCmd = "Open(\"d:\doc\doc.sdw\")"
    rEvent = "Open"
*/

{
    String aEvent( rEvent );
    aEvent += '(';
    if ( rCmd.CompareIgnoreCaseToAscii( aEvent, aEvent.Len() ) == COMPARE_EQUAL )
    {
        String aData( rCmd );
        aData.Erase( 0, aEvent.Len() );
        if ( aData.Len() > 2 )
        {
            // Transform into the ApplicationEvent Format
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
    const String&   rCmd  // Expressed in our BASIC-Syntax
)

/*  Description]

    This method can be overloaded by application developers, to receive
    DDE-commands directed to thier SfxApplication subclass.

    The base implementation understands the API functionality of the
    relevant SfxApplication subclass in BASIC syntax. Return values can
    not be transferred, unfortunately.
*/

{
    // Print or Open-Event?
    ApplicationEvent aAppEvent;
    if ( SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Print") ) ||
         SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Open") ) )
        GetpApp()->AppEvent( aAppEvent );
    else
    {
        // all others are BASIC
        EnterBasicCall();
        StarBASIC* pBasic = GetBasic();
        DBG_ASSERT( pBasic, "Where is the Basic???" );
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
    const String&,              // the Item to be addressed
    const String&,              // in: Format
    ::com::sun::star::uno::Any& // out: requested data
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data-requests directed to thier SfxApplication subclass.

    The base implementation provides no data and returns 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------

long SfxApplication::DdeSetData
(
    const String&,                    // the Item to be addressed
    const String&,                    // in: Format
    const ::com::sun::star::uno::Any& // out: requested data
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data directed to thier SfxApplication subclass.

    The base implementation is not receiving any data and returns 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------

::sfx2::SvLinkSource* SfxApplication::DdeCreateLinkSource
(
    const String&  // the Item to be addressed
)

/*  [Description]

    This method can be overloaded by application developers, to establish
    a DDE-hotlink to thier SfxApplication subclass.

    The base implementation is not generate a link and returns 0.
*/

{
    return 0;
}

//========================================================================

long SfxObjectShell::DdeExecute
(
    const String&   rCmd  // Expressed in our BASIC-Syntax
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-commands directed to the thier SfxApplication subclass.

    The base implementation does nothing and returns 0.
*/

{
    StarBASIC* pBasic = GetBasic();
    DBG_ASSERT( pBasic, "Where is the Basic???" ) ;
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
    const String&,              // the Item to be addressed
    const String&,              // in: Format
    ::com::sun::star::uno::Any& // out: requested data
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data-requests directed to thier SfxApplication subclass.

    The base implementation provides no data and returns 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------

long SfxObjectShell::DdeSetData
(
    const String&,                    // the Item to be addressed
    const String&,                    // in: Format
    const ::com::sun::star::uno::Any& // out: requested data
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data directed to thier SfxApplication subclass.

    The base implementation is not receiving any data and returns 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------
::sfx2::SvLinkSource* SfxObjectShell::DdeCreateLinkSource
(
    const String&  // the Item to be addressed
)

/*  [Description]

    This method can be overloaded by application developers, to establish
    a DDE-hotlink to thier SfxApplication subclass.

    The base implementation is not generate a link and returns 0.
*/

{
    return 0;
}

void SfxObjectShell::ReconnectDdeLink(SfxObjectShell& /*rServer*/)
{
}

void SfxObjectShell::ReconnectDdeLinks(SfxObjectShell& rServer)
{
    TypeId aType = TYPE(SfxObjectShell);
    SfxObjectShell* p = GetFirst(&aType, false);
    while (p)
    {
        if (&rServer != p)
            p->ReconnectDdeLink(rServer);

        p = GetNext(*p, &aType, false);
    }
}

//========================================================================

long SfxViewFrame::DdeExecute
(
    const String&   rCmd  // Expressed in our BASIC-Syntax
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-commands directed to the thier SfxApplication subclass.

    The base implementation understands the API functionality of the
    relevant SfxViewFrame, which is shown and the relevant SfxViewShell
    and the relevant SfxApplication subclass in BASIC syntax. Return
    values can not be transferred, unfortunately.
*/

{
    if ( GetObjectShell() )
        return GetObjectShell()->DdeExecute( rCmd );

    return 0;
}

//--------------------------------------------------------------------

long SfxViewFrame::DdeGetData
(
    const String&,              // the Item to be addressed
    const String&,              // in: Format
    ::com::sun::star::uno::Any& // out: requested data
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data-requests directed to thier SfxApplication subclass.

    The base implementation provides no data and returns 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------

long SfxViewFrame::DdeSetData
(
    const String&,                    // the Item to be addressed
    const String&,                    // in: Format
    const ::com::sun::star::uno::Any& // out: requested data
)

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data directed to thier SfxApplication subclass.

    The base implementation is not receiving any data and returns 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------

::sfx2::SvLinkSource* SfxViewFrame::DdeCreateLinkSource
(
    const String&  // the Item to be addressed
)

/*  [Description]

    This method can be overloaded by application developers, to establish
    a DDE-hotlink to thier SfxApplication subclass.

    The base implementation is not generate a link and returns 0.
*/

{
    return 0;
}

//========================================================================

BOOL SfxApplication::InitializeDde()
{
    DBG_ASSERT( !pAppData_Impl->pDdeService,
                "Dde can not be initialized multiple times" );

    pAppData_Impl->pDdeService = new ImplDdeService( Application::GetAppName() );
    int nError = pAppData_Impl->pDdeService->GetError();
    if( !nError )
    {
        pAppData_Impl->pDocTopics = new SfxDdeDocTopics_Impl;

        // we certainly want to support RTF!
        pAppData_Impl->pDdeService->AddFormat( FORMAT_RTF );

        // Config path as a topic becauseof multiple starts
        INetURLObject aOfficeLockFile( SvtPathOptions().GetUserConfigPath() );
        aOfficeLockFile.insertName( DEFINE_CONST_UNICODE( "soffice.lck" ) );
        String aService( SfxDdeServiceName_Impl(
                    aOfficeLockFile.GetMainURL(INetURLObject::DECODE_TO_IURI) ) );
        aService.ToUpperAscii();
        pAppData_Impl->pDdeService2 = new ImplDdeService( aService );
        pAppData_Impl->pTriggerTopic = new SfxDdeTriggerTopic_Impl;
        pAppData_Impl->pDdeService2->AddTopic( *pAppData_Impl->pTriggerTopic );
    }
    return !nError;
}

void SfxAppData_Impl::DeInitDDE()
{
    DELETEZ( pTriggerTopic );
    DELETEZ( pDdeService2 );
    DELETEZ( pDocTopics );
    DELETEZ( pDdeService );
}

//--------------------------------------------------------------------

void SfxApplication::AddDdeTopic( SfxObjectShell* pSh )
{
    DBG_ASSERT( pAppData_Impl->pDocTopics, "There is no Dde-Service" );
    //OV: DDE is disconnected in server mode!
    if( !pAppData_Impl->pDocTopics )
        return;

    // prevent double submit
    String sShellNm;
    BOOL bFnd = FALSE;
    for( USHORT n = pAppData_Impl->pDocTopics->Count(); n; )
        if( (*pAppData_Impl->pDocTopics)[ --n ]->pSh == pSh )
        {
            // If the document is untitled, is still a new Topic is created!
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
    DBG_ASSERT( pAppData_Impl->pDocTopics, "There is no Dde-Service" );
    //OV: DDE is disconnected in server mode!
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
    // Workaround for Event after Main() under OS/2
    // happens when exiting starts the App again
    if ( !Application::IsInExecute() )
        return FALSE;

    // The Topic rNm is sought, do we have it?
    // First only loop over the ObjectShells to find those
    // with the specific name:
    BOOL bRet = FALSE;
    String sNm( rNm );
    sNm.ToLowerAscii();
    TypeId aType( TYPE(SfxObjectShell) );
    SfxObjectShell* pShell = SfxObjectShell::GetFirst( &aType );
    while( pShell )
    {
        String sTmp( pShell->GetTitle(SFX_TITLE_FULLNAME) );
        sTmp.ToLowerAscii();
        if( sTmp == sNm )
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
             SfxContentHelper::IsDocument( aFile.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            // File exists? then try to load it:
            SfxStringItem aName( SID_FILE_NAME, aFile.GetMainURL( INetURLObject::NO_DECODE ) );
            SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, TRUE);

            SfxBoolItem aSilent(SID_SILENT, TRUE);
            SfxDispatcher* pDispatcher = SFX_APP()->GetDispatcher_Impl();
            const SfxPoolItem* pRet = pDispatcher->Execute( SID_OPENDOC,
                    SFX_CALLMODE_SYNCHRON,
                    &aName, &aNewView,
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
        if( SfxViewFrame::GetFirst( pShell ) )
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

BOOL SfxDdeTriggerTopic_Impl::Execute( const String* )
{
    return TRUE;
}

//--------------------------------------------------------------------
DdeData* SfxDdeDocTopic_Impl::Get( ULONG nFormat )
{
    String sMimeType( SotExchange::GetFormatMimeType( nFormat ));
    ::com::sun::star::uno::Any aValue;
    long nRet = pSh->DdeGetData( GetCurItem(), sMimeType, aValue );
    if( nRet && aValue.hasValue() && ( aValue >>= aSeq ) )
    {
        aData = DdeData( aSeq.getConstArray(), aSeq.getLength(), nFormat );
        return &aData;
    }
    aSeq.realloc( 0 );
    return 0;
}

BOOL SfxDdeDocTopic_Impl::Put( const DdeData* pData )
{
    aSeq = ::com::sun::star::uno::Sequence< sal_Int8 >(
                            (sal_Int8*)(const void*)*pData, (long)*pData );
    BOOL bRet;
    if( aSeq.getLength() )
    {
        ::com::sun::star::uno::Any aValue;
        aValue <<= aSeq;
        String sMimeType( SotExchange::GetFormatMimeType( pData->GetFormat() ));
        bRet = 0 != pSh->DdeSetData( GetCurItem(), sMimeType, aValue );
    }
    else
        bRet = FALSE;
    return bRet;
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
    BOOL bRet = FALSE;
    ::sfx2::SvLinkSource* pNewObj = pSh->DdeCreateLinkSource( GetCurItem() );
    if( pNewObj )
    {
        // then we also establish a corresponding SvBaseLink
        String sNm, sTmp( Application::GetAppName() );
        ::sfx2::MakeLnkName( sNm, &sTmp, pSh->GetTitle(SFX_TITLE_FULLNAME), GetCurItem() );
        new ::sfx2::SvBaseLink( sNm, OBJECT_DDE_EXTERN, pNewObj );
        bRet = TRUE;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
