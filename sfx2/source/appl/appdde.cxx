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
#include <tools/diagnose_ex.h>
#include <unotools/pathoptions.hxx>

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxsids.hrc>
#include "helper.hxx"
#include <sfx2/docfile.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>

//========================================================================

String SfxDdeServiceName_Impl( const String& sIn )
{
    String sReturn;

    for ( sal_uInt16 n = sIn.Len(); n; --n )
    {
        sal_Unicode cChar = sIn.GetChar(n-1);
        if (comphelper::string::isalnumAscii(cChar))
            sReturn += cChar;
    }

    return sReturn;
}

#if defined( WNT )
class ImplDdeService : public DdeService
{
public:
    ImplDdeService( const String& rNm )
        : DdeService( rNm )
    {}
    virtual sal_Bool MakeTopic( const rtl::OUString& );

    virtual String  Topics();

    virtual sal_Bool SysTopicExecute( const String* pStr );
};

//--------------------------------------------------------------------
namespace
{
    sal_Bool lcl_IsDocument( const String& rContent )
    {
        using namespace com::sun::star;

        sal_Bool bRet = sal_False;
        INetURLObject aObj( rContent );
        DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

        try
        {
            ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
            bRet = aCnt.isDocument();
        }
        catch( const ucb::CommandAbortedException& )
        {
            DBG_WARNING( "CommandAbortedException" );
        }
        catch( const ucb::IllegalIdentifierException& )
        {
            DBG_WARNING( "IllegalIdentifierException" );
        }
        catch( const ucb::ContentCreationException& )
        {
            DBG_WARNING( "IllegalIdentifierException" );
        }
        catch( const uno::Exception& )
        {
            SAL_WARN( "sfx2.appl", "Any other exception" );
        }

        return bRet;
    }
}

sal_Bool ImplDdeService::MakeTopic( const rtl::OUString& rNm )
{
    // Workaround for Event after Main() under OS/2
    // happens when exiting starts the App again
    if ( !Application::IsInExecute() )
        return sal_False;

    // The Topic rNm is sought, do we have it?
    // First only loop over the ObjectShells to find those
    // with the specific name:
    sal_Bool bRet = sal_False;
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
            bRet = sal_True;
            break;
        }
        pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }

    if( !bRet )
    {
        INetURLObject aWorkPath( SvtPathOptions().GetWorkPath() );
        INetURLObject aFile;
        if ( aWorkPath.GetNewAbsURL( rNm, &aFile ) &&
             lcl_IsDocument( aFile.GetMainURL( INetURLObject::NO_DECODE ) ) )
        {
            // File exists? then try to load it:
            SfxStringItem aName( SID_FILE_NAME, aFile.GetMainURL( INetURLObject::NO_DECODE ) );
            SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, sal_True);

            SfxBoolItem aSilent(SID_SILENT, sal_True);
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
                bRet = sal_True;
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

sal_Bool ImplDdeService::SysTopicExecute( const String* pStr )
{
    return (sal_Bool)SFX_APP()->DdeExecute( *pStr );
}
#endif

class SfxDdeTriggerTopic_Impl : public DdeTopic
{
public:
    SfxDdeTriggerTopic_Impl()
    : DdeTopic( DEFINE_CONST_UNICODE("TRIGGER") )
    {}

    virtual sal_Bool Execute( const String* );
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

    virtual DdeData* Get( sal_uIntPtr );
    virtual sal_Bool Put( const DdeData* );
    virtual sal_Bool Execute( const String* );
    virtual sal_Bool StartAdviseLoop();
    virtual sal_Bool MakeItem( const rtl::OUString& rItem );
};


class SfxDdeDocTopics_Impl : public std::vector<SfxDdeDocTopic_Impl*> {};

//========================================================================

sal_Bool SfxAppEvent_Impl( ApplicationEvent &rAppEvent,
                           const String &rCmd, const String &rEvent,
                           ApplicationEvent::Type eType )

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
        ::rtl::OUStringBuffer aData( rCmd );
        aData.remove( 0, aEvent.Len() );
        if ( aData.getLength() > 2 )
        {
            // Transform into the ApplicationEvent Format
            aData.remove( aData.getLength() - 1, 1 );
            for ( sal_Int32 n = 0; n < aData.getLength(); )
            {
                switch ( aData[n] )
                {
                case '"':
                    aData.remove( n, 1 );
                    while ( n < aData.getLength() && aData[n] != '"' )
                        ++n;
                    if ( n < aData.getLength() )
                        aData.remove( n, 1 );
                    break;
                case ' ':
                    aData[n++] = '\n';
                    break;
                default:
                    ++n;
                    break;
                }
            }

            rAppEvent = ApplicationEvent(eType, aData.makeStringAndClear());
            return sal_True;
        }
    }

    return sal_False;
}

#if defined( WNT )
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
    if ( SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Print"), ApplicationEvent::TYPE_PRINT ) ||
         SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Open"), ApplicationEvent::TYPE_OPEN ) )
        GetpApp()->AppEvent( aAppEvent );
    else
    {
        // all others are BASIC
        StarBASIC* pBasic = GetBasic();
        DBG_ASSERT( pBasic, "Where is the Basic???" );
        SbxVariable* pRet = pBasic->Execute( rCmd );
        if( !pRet )
        {
            SbxBase::ResetError();
            return 0;
        }
    }
    return 1;
}
#endif

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
#ifdef DISABLE_SCRIPTING
    (void) rCmd;
#else
    StarBASIC* pBasic = GetBasic();
    DBG_ASSERT( pBasic, "Where is the Basic???" ) ;
    SbxVariable* pRet = pBasic->Execute( rCmd );
    if( !pRet )
    {
        SbxBase::ResetError();
        return 0;
    }
#endif
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

sal_Bool SfxApplication::InitializeDde()
{
    int nError = 0;
#if defined( WNT )
    DBG_ASSERT( !pAppData_Impl->pDdeService,
                "Dde can not be initialized multiple times" );

    pAppData_Impl->pDdeService = new ImplDdeService( Application::GetAppName() );
    nError = pAppData_Impl->pDdeService->GetError();
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
#endif
    return !nError;
}

void SfxAppData_Impl::DeInitDDE()
{
    DELETEZ( pTriggerTopic );
    DELETEZ( pDdeService2 );
    DELETEZ( pDocTopics );
    DELETEZ( pDdeService );
}

#if defined( WNT )
void SfxApplication::AddDdeTopic( SfxObjectShell* pSh )
{
    DBG_ASSERT( pAppData_Impl->pDocTopics, "There is no Dde-Service" );
    //OV: DDE is disconnected in server mode!
    if( !pAppData_Impl->pDocTopics )
        return;

    // prevent double submit
    String sShellNm;
    sal_Bool bFnd = sal_False;
    for (size_t n = pAppData_Impl->pDocTopics->size(); n;)
    {
        if( (*pAppData_Impl->pDocTopics)[ --n ]->pSh == pSh )
        {
            // If the document is untitled, is still a new Topic is created!
            if( !bFnd )
            {
                bFnd = sal_True;
                (sShellNm = pSh->GetTitle(SFX_TITLE_FULLNAME)).ToLowerAscii();
            }
            String sNm( (*pAppData_Impl->pDocTopics)[ n ]->GetName() );
            if( sShellNm == sNm.ToLowerAscii() )
                return ;
        }
    }

    SfxDdeDocTopic_Impl *const pTopic = new SfxDdeDocTopic_Impl(pSh);
    pAppData_Impl->pDocTopics->push_back(pTopic);
    pAppData_Impl->pDdeService->AddTopic( *pTopic );
}
#endif

void SfxApplication::RemoveDdeTopic( SfxObjectShell* pSh )
{
    DBG_ASSERT( pAppData_Impl->pDocTopics, "There is no Dde-Service" );
    //OV: DDE is disconnected in server mode!
    if( !pAppData_Impl->pDocTopics )
        return;

    for (size_t n = pAppData_Impl->pDocTopics->size(); n; )
    {
        SfxDdeDocTopic_Impl *const pTopic = (*pAppData_Impl->pDocTopics)[ --n ];
        if (pTopic->pSh == pSh)
        {
            pAppData_Impl->pDdeService->RemoveTopic( *pTopic );
            delete pTopic;
            pAppData_Impl->pDocTopics->erase( pAppData_Impl->pDocTopics->begin() + n );
        }
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

sal_Bool SfxDdeTriggerTopic_Impl::Execute( const String* )
{
    return sal_True;
}

//--------------------------------------------------------------------
DdeData* SfxDdeDocTopic_Impl::Get( sal_uIntPtr nFormat )
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

sal_Bool SfxDdeDocTopic_Impl::Put( const DdeData* pData )
{
    aSeq = ::com::sun::star::uno::Sequence< sal_Int8 >(
                            (sal_Int8*)(const void*)*pData, (long)*pData );
    sal_Bool bRet;
    if( aSeq.getLength() )
    {
        ::com::sun::star::uno::Any aValue;
        aValue <<= aSeq;
        String sMimeType( SotExchange::GetFormatMimeType( pData->GetFormat() ));
        bRet = 0 != pSh->DdeSetData( GetCurItem(), sMimeType, aValue );
    }
    else
        bRet = sal_False;
    return bRet;
}

sal_Bool SfxDdeDocTopic_Impl::Execute( const String* pStr )
{
    long nRet = pStr ? pSh->DdeExecute( *pStr ) : 0;
    return 0 != nRet;
}

sal_Bool SfxDdeDocTopic_Impl::MakeItem( const rtl::OUString& rItem )
{
    AddItem( DdeItem( rItem ) );
    return sal_True;
}

sal_Bool SfxDdeDocTopic_Impl::StartAdviseLoop()
{
    sal_Bool bRet = sal_False;
    ::sfx2::SvLinkSource* pNewObj = pSh->DdeCreateLinkSource( GetCurItem() );
    if( pNewObj )
    {
        // then we also establish a corresponding SvBaseLink
        String sNm, sTmp( Application::GetAppName() );
        ::sfx2::MakeLnkName( sNm, &sTmp, pSh->GetTitle(SFX_TITLE_FULLNAME), GetCurItem() );
        new ::sfx2::SvBaseLink( sNm, OBJECT_DDE_EXTERN, pNewObj );
        bRet = sal_True;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
