/*************************************************************************
 *
 *  $RCSfile: mailmodel.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: cd $ $Date: 2001-06-26 08:39:52 $
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

// includes --------------------------------------------------------------

#ifndef  _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef  _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef  _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XDATACONTAINER_HPP_
#include <com/sun/star/ucb/XDataContainer.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef  _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XSIMPLEMAILCLIENTSUPPLIER_HPP_
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SIMPLEMAILCLIENTFLAGS_HPP_
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#endif


#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#include <unotools/streamhelper.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#include <mailmodel.hxx>
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"

#include "sfxsids.hrc"

#include <unotools/tempfile.hxx>
#include <vcl/svapp.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/useroptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>

// --------------------------------------------------------------
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::mozilla;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::rtl;

// --------------------------------------------------------------
#define SEND_MAIL_COMMAND   "?cmd2.officeMail=1"
#define SEND_MAIL_FROM      "MESSAGE_FROM="
#define SEND_MAIL_TO        "MESSAGE_TO="
#define SEND_MAIL_CC        "MESSAGE_CC="
#define SEND_MAIL_BCC       "MESSAGE_BCC="
#define SEND_MAIL_SUBJECT   "MESSAGE_SUBJECT="
#define SEND_MAIL_FILEURL   "file_1="
#define SEND_MAIL_SEP       '&'

// class DefaultMailer_Impl ------------------------------------------------

class DefaultMailerConfig_Impl : public utl::ConfigItem
{
    public:
        DefaultMailerConfig_Impl();
        virtual ~DefaultMailerConfig_Impl();

    sal_Bool    GetUseDefaultMailer();
};

DefaultMailerConfig_Impl::DefaultMailerConfig_Impl() : ConfigItem( String::CreateFromAscii( "Office.Common/ExternalMailer" ))
{
}

DefaultMailerConfig_Impl::~DefaultMailerConfig_Impl()
{
}

sal_Bool DefaultMailerConfig_Impl::GetUseDefaultMailer()
{
#ifdef UNIX
    sal_Bool bUseDefaultMailer = sal_False;
#else
    sal_Bool bUseDefaultMailer = sal_True;
#endif

    Sequence< ::rtl::OUString > aPropertyNames( 1 );
    ::rtl::OUString* pPropertyNames = aPropertyNames.getArray();
    pPropertyNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseDefaultMailer" ));
    Sequence< Any > aPropertyValues = GetProperties( aPropertyNames );

    const Any* pPropertyValues = aPropertyValues.getConstArray();
    if ( aPropertyValues.getLength() == 1 &&
         pPropertyValues[0].hasValue() )
    {

        try
        {
            bUseDefaultMailer = ::cppu::any2bool(pPropertyValues[0]);
        }
        catch(const ::com::sun::star::lang::IllegalArgumentException&)
        {
        }
    }

    return bUseDefaultMailer;
}

// class AddressList_Impl ------------------------------------------------

typedef String* AddressItemPtr_Impl;
DECLARE_LIST( AddressList_Impl, AddressItemPtr_Impl );

// class SfxMailModel_Impl -----------------------------------------------

void SfxMailModel_Impl::ClearList( AddressList_Impl* pList )
{
    if ( pList )
    {
        ULONG i, nCount = pList->Count();
        for ( i = 0; i < nCount; ++i )
            delete pList->GetObject(i);
        pList->Clear();
    }
}

void SfxMailModel_Impl::MakeValueList( AddressList_Impl* pList, String& rValueList )
{
    rValueList.Erase();
    if ( pList )
    {
        ULONG i, nCount = pList->Count();
        for ( i = 0; i < nCount; ++i )
        {
            if ( rValueList.Len() > 0 )
                rValueList += ',';
            rValueList += *pList->GetObject(i);
        }
    }
}

sal_Bool SfxMailModel_Impl::SaveDocument( String& rFileName, String& rType )
{
    sal_Bool bRet = sal_False;
    SfxViewFrame* pTopViewFrm = mpBindings->GetDispatcher_Impl()->GetFrame()->GetTopViewFrame();
    SfxObjectShellRef xDocShell = pTopViewFrm->GetObjectShell();

    // save the document
    if ( xDocShell.Is() && xDocShell->GetMedium() )
    {
        // save old settings
        BOOL bOldDidDangerousSave = xDocShell->Get_Impl()->bDidDangerousSave;
        BOOL bModified = xDocShell->IsModified();
        // prepare for mail export
        SfxDispatcher* pDisp = pTopViewFrm->GetDispatcher();
        pDisp->Execute( SID_MAIL_PREPAREEXPORT, SFX_CALLMODE_SYNCHRON );
        // detect filter
        const SfxFilter* pFilter = xDocShell->GetMedium()->GetFilter();
        sal_Bool bHasFilter = pFilter ? sal_True : sal_False;
        if ( !pFilter )
        {
            SfxFilterMatcher aFilterMatcher( xDocShell->GetFactory().GetFilterContainer() );
            pFilter = aFilterMatcher.GetDefaultFilter();
        }
//#if 0
        // create temp file name with leading chars and extension
        sal_Bool    bHasName = xDocShell->HasName();
        String      aLeadingStr;
        String*     pExt = NULL;

        if ( !bHasName )
            aLeadingStr = String( DEFINE_CONST_UNICODE("noname") );
        else
        {
            INetURLObject aFileObj = xDocShell->GetMedium()->GetURLObject();
            String aName;
            if ( aFileObj.hasExtension() )
            {
                pExt = new String( String::CreateFromAscii( "." ) + aFileObj.getExtension() );
                aFileObj.removeExtension();
                aLeadingStr = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                aLeadingStr += String::CreateFromAscii( "_" );
            }
            else
            {
                aLeadingStr = aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                aLeadingStr += String::CreateFromAscii( "_" );
            }
        }

        if ( pFilter && !pExt )
        {
            pExt = new String( pFilter->GetWildcard()().GetToken(0) );
            // erase the '*' from the extension (e.g. "*.sdw")
            pExt->Erase( 0, 1 );
        }

        ::utl::TempFile aTempFile( aLeadingStr, pExt );

        delete pExt;

        rFileName = aTempFile.GetURL();
        // save document to temp file
        SfxStringItem aFileName( SID_FILE_NAME, rFileName );
        SfxBoolItem aPicklist( SID_PICKLIST, FALSE );
        SfxBoolItem aSaveTo( SID_SAVETO, TRUE );
//#endif
        SfxStringItem* pFilterName = NULL;
        if ( pFilter && bHasFilter )
            pFilterName = new SfxStringItem( SID_FILTER_NAME, pFilter->GetName() );
//      pDisp->Execute( SID_SAVEDOC, SFX_CALLMODE_SYNCHRON,
//                      pFilterName, 0L );
        pDisp->Execute( SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, &aFileName, &aPicklist, &aSaveTo,
                        pFilterName, 0L );

//        rFileName = xDocShell->GetMedium()->GetName();

        delete pFilterName;
        if ( pFilter )
        {
            // detect content type and expand with the file name
            rType = pFilter->GetMimeType();
            rType += DEFINE_CONST_UNICODE("; name =\"");
            INetURLObject aFileObj = xDocShell->GetMedium()->GetURLObject();
            rType += aFileObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
            rType += '\"';
        }
        // restore old settings
        if ( !bModified && xDocShell->IsEnableSetModified() )
            xDocShell->SetModified( FALSE );
        if ( !bOldDidDangerousSave )
            xDocShell->Get_Impl()->bDidDangerousSave = sal_False;
        bRet = sal_True;
    }
    return bRet;
}

IMPL_LINK_INLINE_START( SfxMailModel_Impl, DoneHdl, void*, EMPTYARG )
{
    mbLoadDone = sal_True;
    return 0;
}
IMPL_LINK_INLINE_END( SfxMailModel_Impl, DoneHdl, void*, EMPTYARG )

SfxMailModel_Impl::SfxMailModel_Impl( SfxBindings* pBinds ) :

    mpToList    ( NULL ),
    mpCcList    ( NULL ),
    mpBccList   ( NULL ),
    mpBindings  ( pBinds ),
    mePriority  ( PRIO_NORMAL ),
    mbLoadDone  ( sal_True )

{
}

SfxMailModel_Impl::~SfxMailModel_Impl()
{
    ClearList( mpToList );
    delete mpToList;
    ClearList( mpCcList );
    delete mpCcList;
    ClearList( mpBccList );
    delete mpBccList;
}

void SfxMailModel_Impl::AddAddress( const String& rAddress, AddressRole eRole )
{
    // don't add a empty address
    if ( rAddress.Len() > 0 )
    {
        AddressList_Impl* pList = NULL;
        if ( ROLE_TO == eRole )
        {
            if ( !mpToList )
                // create the list
                mpToList = new AddressList_Impl;
            pList = mpToList;
        }
        else if ( ROLE_CC == eRole )
        {
            if ( !mpCcList )
                // create the list
                mpCcList = new AddressList_Impl;
            pList = mpCcList;
        }
        else if ( ROLE_BCC == eRole )
        {
            if ( !mpBccList )
                // create the list
                mpBccList = new AddressList_Impl;
            pList = mpBccList;
        }
        else
        {
            DBG_ERRORFILE( "invalid address role" );
        }

        if ( pList )
        {
            // add address to list
            AddressItemPtr_Impl pAddress = new String( rAddress );
            pList->Insert( pAddress, LIST_APPEND );
        }
    }
}

sal_Bool SfxMailModel_Impl::Send()
{
    sal_Bool bSend = sal_False;
    String aFileName, aContentType;

    if ( SaveDocument( aFileName, aContentType ) )
    {
        SfxFrame* pViewFrm = mpBindings->GetDispatcher_Impl()->GetFrame()->GetFrame();
        Reference < XPluginInstance > xPlugin;

        if ( pViewFrm )
            xPlugin = Reference < XPluginInstance > ( pViewFrm->GetFrameInterface(), UNO_QUERY );

        if ( xPlugin.is() )
        {
            OUString aURL = aFileName;

            // Create the parameter
            ULONG i, nCount;
            OUString aSep( SEND_MAIL_SEP );
            OUString aParam( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_COMMAND ) );
            aParam += aSep;

            if ( maFromAddress.Len() || CreateFromAddress_Impl( maFromAddress ) )
            {
                aParam += OUString( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_FROM ) );
                aParam += maFromAddress;
                aParam += aSep;
            }

            nCount = mpToList ? mpToList->Count() : 0;
            for ( i = 0; i < nCount; ++i )
            {
                aParam += OUString( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_TO ) );
                aParam += *mpToList->GetObject(i);;
                aParam += aSep;
            }

            nCount = mpCcList ? mpCcList->Count() : 0;
            for ( i = 0; i < nCount; ++i )
            {
                aParam += OUString( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_CC ) );
                aParam += *mpCcList->GetObject(i);;
                aParam += aSep;
            }

            nCount = mpBccList ? mpBccList->Count() : 0;
            for ( i = 0; i < nCount; ++i )
            {
                aParam += OUString( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_BCC ) );
                aParam += *mpBccList->GetObject(i);;
                aParam += aSep;
            }

            if ( maSubject.Len() )
            {
                aParam += OUString( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_SUBJECT ) );
                aParam += maSubject;
                aParam += aSep;
            }

            aParam += OUString( RTL_CONSTASCII_USTRINGPARAM( SEND_MAIL_FILEURL ) );
            aParam += aFileName;
            aParam += aSep;

            // now we dispatch the new created URL so the document will be send.

            aURL += aParam;

            URL aTargetURL;
            aTargetURL.Complete = aURL;
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
            xTrans->parseStrict( aTargetURL );

            Reference < XDispatchProvider > xProv( xPlugin, UNO_QUERY );
            Reference < XDispatch > xDisp;
            if ( xProv.is() )
                xDisp = xProv->queryDispatch( aTargetURL, OUString(), 0 );

            if ( xDisp.is() )
            {
                Sequence < PropertyValue > aArgs;
                xDisp->dispatch( aTargetURL, aArgs );
                bSend = sal_True;
            }
        }
        else
        {
            Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
            if ( xMgr.is() )
            {
                // read configuration to choose between "SimpleCommandMail" or "SimpleSystemMail"!
                DefaultMailerConfig_Impl                aMailConfig;
                Reference< XSimpleMailClientSupplier >  xSimpleMailClientSupplier;

                if ( aMailConfig.GetUseDefaultMailer() )
                    xSimpleMailClientSupplier = Reference< XSimpleMailClientSupplier >(
                                                        xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleSystemMail" ))),
                                                    UNO_QUERY );
                else
                    xSimpleMailClientSupplier = Reference< XSimpleMailClientSupplier >(
                                                        xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SimpleCommandMail" ))),
                                                    UNO_QUERY );

                if ( xSimpleMailClientSupplier.is() )
                {
                    Reference< XSimpleMailClient > xSimpleMailClient = xSimpleMailClientSupplier->querySimpleMailClient();

                    if ( !xSimpleMailClient.is() )
                    {
                        // no mail client support => message box!
                        return sal_False;
                    }

                    // we have a simple mail client
                    Reference< XSimpleMailMessage > xSimpleMailMessage = xSimpleMailClient->createSimpleMailMessage();
                    if ( xSimpleMailMessage.is() )
                    {
                        sal_Int32 nSendFlags = SimpleMailClientFlags::DEFAULTS;
                        if ( maFromAddress.Len() == 0 )
                        {
                            // from address not set, try figure out users e-mail address
                            CreateFromAddress_Impl( maFromAddress );
                        }
                        xSimpleMailMessage->setOriginator( maFromAddress );

                        sal_Int32 nToCount      = mpToList ? mpToList->Count() : 0;
                        sal_Int32 nCcCount      = mpCcList ? mpCcList->Count() : 0;
                        sal_Int32 nCcSeqCount   = nCcCount;

                        // set recipient (only one) for this simple mail server!!
                        if ( nToCount > 1 )
                        {
                            nCcSeqCount = nToCount - 1 + nCcCount;
                            xSimpleMailMessage->setRecipient( *mpToList->GetObject( 0 ));
                            nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                        }
                        else if ( nToCount == 1 )
                        {
                            xSimpleMailMessage->setRecipient( *mpToList->GetObject( 0 ));
                            nSendFlags = SimpleMailClientFlags::NO_USER_INTERFACE;
                        }

                        // all other recipient must be handled with CC recipients!
                        if ( nCcSeqCount > 0 )
                        {
                            sal_Int32               nIndex = 0;
                            Sequence< OUString >    aCcRecipientSeq;

                            aCcRecipientSeq.realloc( nCcSeqCount );
                            if ( nCcSeqCount > nCcCount )
                            {
                                for ( sal_Int32 i = 1; i < nToCount; ++i )
                                {
                                    aCcRecipientSeq[nIndex++] = *mpToList->GetObject(i);
                                }
                            }

                            for ( sal_Int32 i = 0; i < nCcCount; i++ )
                            {
                                aCcRecipientSeq[nIndex++] = *mpCcList->GetObject(i);
                            }
                            xSimpleMailMessage->setCcRecipient( aCcRecipientSeq );
                        }

                        sal_Int32 nBccCount = mpBccList ? mpBccList->Count() : 0;
                        if ( nBccCount > 0 )
                        {
                            Sequence< OUString > aBccRecipientSeq( nBccCount );
                            for ( sal_Int32 i = 0; i < nBccCount; ++i )
                            {
                                aBccRecipientSeq[i] = *mpBccList->GetObject(i);
                            }
                            xSimpleMailMessage->setBccRecipient( aBccRecipientSeq );
                        }

                        Sequence< OUString > aAttachmentSeq( 1 );
                        aAttachmentSeq[0] = aFileName;

                        xSimpleMailMessage->setSubject( maSubject );
                        xSimpleMailMessage->setAttachement( aAttachmentSeq );
/*
                        SfxViewFrame* pTopViewFrm = mpBindings->GetDispatcher_Impl()->GetFrame()->GetTopViewFrame();
                        SfxObjectShellRef xDocShell = pTopViewFrm->GetObjectShell();
                        SvStorageRef xStorage = xDocShell->GetStorage();
                        xDocShell->DoHandsOff();
*/
                        try
                        {
                            xSimpleMailClient->sendSimpleMailMessage( xSimpleMailMessage, nSendFlags );
                            bSend = sal_True;
                        }
                        catch ( IllegalArgumentException& )
                        {
                            bSend = sal_False;
                        }
                        catch ( Exception& )
                        {
                            bSend = sal_False;
                        }

//                      xDocShell->DoSaveCompleted( xStorage );
                    }
                }
            }
        }
    }

    return bSend;
}

// functions -------------------------------------------------------------

BOOL CreateFromAddress_Impl( String& rFrom )

/*  [Beschreibung]

    Diese Funktion versucht mit Hilfe des IniManagers eine From-Adresse
    zu erzeugen. daf"ur werden die Felder 'Vorname', 'Name' und 'EMail'
    aus der Applikations-Ini-Datei ausgelesen. Sollten diese Felder
    nicht gesetzt sein, wird FALSE zur"uckgegeben.

    [R"uckgabewert]

    TRUE:   Adresse konnte erzeugt werden.
    FALSE:  Adresse konnte nicht erzeugt werden.
*/

{
#if SUPD<613//MUSTINI
    SfxIniManager* pIni = SFX_INIMANAGER();
    String aName = pIni->Get( SFX_KEY_USER_NAME );
    String aFirstName = pIni->Get( SFX_KEY_USER_FIRSTNAME );
#else
    SvtUserOptions aUserCFG;
    String aName        = aUserCFG.GetLastName  ();
    String aFirstName   = aUserCFG.GetFirstName ();
#endif
    if ( aFirstName.Len() || aName.Len() )
    {
        if ( aFirstName.Len() )
        {
            rFrom = TRIM( aFirstName );

            if ( aName.Len() )
                rFrom += ' ';
        }
        rFrom += TRIM( aName );
        // unerlaubte Zeichen entfernen
        rFrom.EraseAllChars( '<' );
        rFrom.EraseAllChars( '>' );
        rFrom.EraseAllChars( '@' );
    }
#if SUPD<613//MUSTINI
    String aEmailName = pIni->GetAddressToken( ADDRESS_EMAIL );
#else
    String aEmailName = aUserCFG.GetEmail();
#endif
    // unerlaubte Zeichen entfernen
    aEmailName.EraseAllChars( '<' );
    aEmailName.EraseAllChars( '>' );

    if ( aEmailName.Len() )
    {
        if ( rFrom.Len() )
            rFrom += ' ';
        ( ( rFrom += '<' ) += TRIM( aEmailName ) ) += '>';
    }
    else
        rFrom.Erase();
    return ( rFrom.Len() > 0 );
}

