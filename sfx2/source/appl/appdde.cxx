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
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>

//========================================================================

OUString SfxDdeServiceName_Impl( const OUString& sIn )
{
    OUStringBuffer sReturn(sIn.getLength());

    for ( sal_uInt16 n = sIn.getLength(); n; --n )
    {
        sal_Unicode cChar = sIn[n-1];
        if (comphelper::string::isalnumAscii(cChar))
            sReturn.append(cChar);
    }

    return sReturn.makeStringAndClear();
}

#if defined( WNT )
class ImplDdeService : public DdeService
{
public:
    ImplDdeService( const OUString& rNm )
        : DdeService( rNm )
    {}
    virtual bool MakeTopic( const OUString& );

    virtual OUString  Topics();

    virtual bool SysTopicExecute( const OUString* pStr );
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
            ::ucbhelper::Content aCnt( aObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
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
            SAL_WARN( "sfx.appl", "Any other exception" );
        }

        return bRet;
    }
}

bool ImplDdeService::MakeTopic( const OUString& rNm )
{
    // Workaround for Event after Main() under OS/2
    // happens when exiting starts the App again
    if ( !Application::IsInExecute() )
        return false;

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
            bRet = true;
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
                bRet = true;
            }
        }
    }
    return bRet;
}

OUString ImplDdeService::Topics()
{
    OUString sRet;
    if( GetSysTopic() )
        sRet += GetSysTopic()->GetName();

    TypeId aType( TYPE(SfxObjectShell) );
    SfxObjectShell* pShell = SfxObjectShell::GetFirst( &aType );
    while( pShell )
    {
        if( SfxViewFrame::GetFirst( pShell ) )
        {
            if( !sRet.isEmpty() )
                sRet += "\t";
            sRet += pShell->GetTitle(SFX_TITLE_FULLNAME);
        }
        pShell = SfxObjectShell::GetNext( *pShell, &aType );
    }
    if( !sRet.isEmpty() )
        sRet += "\r\n";
    return sRet;
}

bool ImplDdeService::SysTopicExecute( const OUString* pStr )
{
    return SFX_APP()->DdeExecute( *pStr );
}
#endif

class SfxDdeTriggerTopic_Impl : public DdeTopic
{
public:
    SfxDdeTriggerTopic_Impl()
        : DdeTopic( "TRIGGER" )
        {}

    virtual bool Execute( const OUString* );
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
    virtual bool Put( const DdeData* );
    virtual bool Execute( const OUString* );
    virtual bool StartAdviseLoop();
    virtual bool MakeItem( const OUString& rItem );
};


class SfxDdeDocTopics_Impl : public std::vector<SfxDdeDocTopic_Impl*> {};

#if defined( WNT )

namespace {

/*  [Description]

    Checks if 'rCmd' of the event 'rEvent' is (without '(') and then assemble
    this data into a <ApplicationEvent>, which is then excecuted through
    <Application::AppEvent()>. If 'rCmd' is the given event 'rEvent', then
    TRUE is returned, otherwise FALSE.

    [Example]

    rCmd = "Open(\"d:\doc\doc.sdw\")"
    rEvent = "Open"
*/
sal_Bool SfxAppEvent_Impl( const OUString& rCmd, const OUString& rEvent,
                           ApplicationEvent::Type eType )
{
    OUString sEvent(rEvent + "(");
    if (rCmd.startsWithIgnoreAsciiCase(sEvent))
    {
        sal_Int32 start = sEvent.getLength();
        if ( rCmd.getLength() - start >= 2 )
        {
            // Transform into the ApplicationEvent Format
            //TODO: I /assume/ that rCmd should match the syntax of
            // <http://msdn.microsoft.com/en-us/library/ms648995.aspx>
            // "WM_DDE_EXECUTE message" but does not (handle commands enclosed
            // in [...]; handle commas separating multiple arguments; handle
            // double "", ((, )), [[, ]] in quoted arguments); see also the mail
            // thread starting at <http://lists.freedesktop.org/archives/
            // libreoffice/2013-July/054779.html> "DDE on Windows."
            std::vector<OUString> aData;
            for ( sal_Int32 n = start; n < rCmd.getLength() - 1; )
            {
                // Resiliently read arguments either starting with " and
                // spanning to the next " (if any; TODO: do we need to undo any
                // escaping within the string?) or with neither " nor SPC and
                // spanning to the next SPC (if any; TODO: is this from not
                // wrapped in "..." relevant? it would have been parsed by the
                // original code even if that was only by accident, so I left it
                // in), with runs of SPCs treated like single ones:
                switch ( rCmd[n] )
                {
                case '"':
                    {
                        sal_Int32 i = rCmd.indexOf('"', ++n);
                        if (i < 0 || i > rCmd.getLength() - 1) {
                            i = rCmd.getLength() - 1;
                        }
                        aData.push_back(rCmd.copy(n, i - n));
                        n = i + 1;
                        break;
                    }
                case ' ':
                    ++n;
                    break;
                default:
                    {
                        sal_Int32 i = rCmd.indexOf(' ', n);
                        if (i < 0 || i > rCmd.getLength() - 1) {
                            i = rCmd.getLength() - 1;
                        }
                        aData.push_back(rCmd.copy(n, i - n));
                        n = i + 1;
                        break;
                    }
                }
            }

            GetpApp()->AppEvent( ApplicationEvent(eType, aData) );
            return sal_True;
        }
    }

    return sal_False;
}

}

/*  Description]

    This method can be overloaded by application developers, to receive
    DDE-commands directed to their SfxApplication subclass.

    The base implementation understands the API functionality of the
    relevant SfxApplication subclass in BASIC syntax. Return values can
    not be transferred, unfortunately.
*/
long SfxApplication::DdeExecute( const OUString&   rCmd )  // Expressed in our BASIC-Syntax
{
    // Print or Open-Event?
    if ( !( SfxAppEvent_Impl( rCmd, "Print", ApplicationEvent::TYPE_PRINT ) ||
            SfxAppEvent_Impl( rCmd, "Open", ApplicationEvent::TYPE_OPEN ) ) )
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

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-commands directed to the their SfxApplication subclass.

    The base implementation does nothing and returns 0.
*/
long SfxObjectShell::DdeExecute( const OUString&   rCmd )  // Expressed in our BASIC-Syntax
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

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data-requests directed to their SfxApplication subclass.

    The base implementation provides no data and returns 0.
*/
long SfxObjectShell::DdeGetData( const OUString&,              // the Item to be addressed
                                 const OUString&,              // in: Format
                                 ::com::sun::star::uno::Any& )// out: requested data
{
    return 0;
}

//--------------------------------------------------------------------

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data directed to their SfxApplication subclass.

    The base implementation is not receiving any data and returns 0.
*/
long SfxObjectShell::DdeSetData( const OUString&,                    // the Item to be addressed
                                 const OUString&,                    // in: Format
                                 const ::com::sun::star::uno::Any& )// out: requested data
{
    return 0;
}

/*  [Description]

    This method can be overloaded by application developers, to establish
    a DDE-hotlink to their SfxApplication subclass.

    The base implementation is not generate a link and returns 0.
*/
::sfx2::SvLinkSource* SfxObjectShell::DdeCreateLinkSource( const OUString& ) // the Item to be addressed
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

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-commands directed to the their SfxApplication subclass.

    The base implementation understands the API functionality of the
    relevant SfxViewFrame, which is shown and the relevant SfxViewShell
    and the relevant SfxApplication subclass in BASIC syntax. Return
    values can not be transferred, unfortunately.
*/
long SfxViewFrame::DdeExecute( const OUString&   rCmd ) // Expressed in our BASIC-Syntax
{
    if ( GetObjectShell() )
        return GetObjectShell()->DdeExecute( rCmd );

    return 0;
}

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data-requests directed to their SfxApplication subclass.

    The base implementation provides no data and returns 0.
*/
long SfxViewFrame::DdeGetData( const OUString&,            // the Item to be addressed
                               const OUString&,            // in: Format
                               ::com::sun::star::uno::Any& )// out: requested data
{
    return 0;
}

/*  [Description]

    This method can be overloaded by application developers, to receive
    DDE-data directed to their SfxApplication subclass.

    The base implementation is not receiving any data and returns 0.
*/
long SfxViewFrame::DdeSetData( const OUString&,                  // the Item to be addressed
                               const OUString&,                  // in: Format
                               const ::com::sun::star::uno::Any& )// out: requested data
{
    return 0;
}

/*  [Description]

    This method can be overloaded by application developers, to establish
    a DDE-hotlink to their SfxApplication subclass.

    The base implementation is not generate a link and returns 0.
*/
::sfx2::SvLinkSource* SfxViewFrame::DdeCreateLinkSource( const OUString&  )// the Item to be addressed
{
    return 0;
}

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
        aOfficeLockFile.insertName( "soffice.lck" );
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

bool SfxDdeTriggerTopic_Impl::Execute( const OUString* )
{
    return true;
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

bool SfxDdeDocTopic_Impl::Put( const DdeData* pData )
{
    aSeq = ::com::sun::star::uno::Sequence< sal_Int8 >(
                            (sal_Int8*)(const void*)*pData, (long)*pData );
    bool bRet;
    if( aSeq.getLength() )
    {
        ::com::sun::star::uno::Any aValue;
        aValue <<= aSeq;
        String sMimeType( SotExchange::GetFormatMimeType( pData->GetFormat() ));
        bRet = 0 != pSh->DdeSetData( GetCurItem(), sMimeType, aValue );
    }
    else
        bRet = false;
    return bRet;
}

bool SfxDdeDocTopic_Impl::Execute( const OUString* pStr )
{
    long nRet = pStr ? pSh->DdeExecute( *pStr ) : 0;
    return 0 != nRet;
}

bool SfxDdeDocTopic_Impl::MakeItem( const OUString& rItem )
{
    AddItem( DdeItem( rItem ) );
    return true;
}

bool SfxDdeDocTopic_Impl::StartAdviseLoop()
{
    bool bRet = false;
    ::sfx2::SvLinkSource* pNewObj = pSh->DdeCreateLinkSource( GetCurItem() );
    if( pNewObj )
    {
        // then we also establish a corresponding SvBaseLink
        String sNm, sTmp( Application::GetAppName() );
        ::sfx2::MakeLnkName( sNm, &sTmp, pSh->GetTitle(SFX_TITLE_FULLNAME), GetCurItem() );
        new ::sfx2::SvBaseLink( sNm, OBJECT_DDE_EXTERN, pNewObj );
        bRet = true;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
