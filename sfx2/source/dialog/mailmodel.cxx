/*************************************************************************
 *
 *  $RCSfile: mailmodel.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: pb $ $Date: 2001-02-07 09:46:53 $
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

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDATACONTAINER_HPP_
#include <com/sun/star/ucb/XDataContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#include <unotools/streamhelper.hxx>
#endif

#include "mailmodel.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"
#include "objshimp.hxx"

#include "sfxsids.hrc"

#include <unotools/tempfile.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <tools/urlobj.hxx>

using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

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
        // create temp file name with leading chars and extension
        String aLeadingStr( DEFINE_CONST_UNICODE("smail") );
        sal_Bool bHasName = xDocShell->HasName();
        if ( !bHasName )
            aLeadingStr = String( DEFINE_CONST_UNICODE("noname") );
        String* pExt = NULL;
        if ( pFilter )
        {
            pExt = new String( pFilter->GetWildcard()().GetToken(0) );
            // erase the '*' from the extension (e.g. "*.sdw")
            pExt->Erase( 0, 1 );
        }

        ::utl::TempFile aTempFile( aLeadingStr, pExt );
        rFileName = aTempFile.GetURL();

        // save document to temp file
        SfxStringItem aFileName( SID_FILE_NAME, rFileName );
        SfxBoolItem aPicklist( SID_PICKLIST, FALSE );
        SfxBoolItem aSaveTo( SID_SAVETO, TRUE );
        SfxStringItem* pFilterName = NULL;
        if ( pFilter && bHasFilter )
            pFilterName = new SfxStringItem( SID_FILTER_NAME, pFilter->GetName() );
        pDisp->Execute( SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_API,
                        &aFileName, &aPicklist, &aSaveTo, pFilterName, 0L );
        delete pFilterName;
        if ( pFilter )
        {
            // detect content type and expand with the file name
            rType = pFilter->GetMimeType();
            rType += DEFINE_CONST_UNICODE("; name =\"");
            INetURLObject aFileObj = bHasName ? xDocShell->GetMedium()->GetURLObject()
                                              : INetURLObject( rFileName );
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
    String aFileNameText, aContentTypeText;
    if ( SaveDocument( aFileName, aContentType ) )
    {
        SvStream* pStream = new SvFileStream( aFileName, STREAM_STD_READ );
        Reference < XMultiServiceFactory > xMgr = ::comphelper::getProcessServiceFactory();
        Reference < XDataContainer > xData(
            xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.DataContainer") ), UNO_QUERY );
        if ( xData.is() )
        {
            xData->setContentType( aContentType );
            SvLockBytesRef xLockBytes = new SvLockBytes( pStream );
            Reference < XInputStream > xStream = new ::utl::OInputStreamHelper( xLockBytes, 8192 );
            Reference < XActiveDataSink > xSink( xData, UNO_QUERY );
            if ( xSink.is() )
                xSink->setInputStream( xStream );
            else
            {
                try
                {
                    Sequence< sal_Int8 > aData( 65536 );
                    sal_Int8* pData = aData.getArray();
                    Sequence< sal_Int8 > aBuffer;
                    sal_Int32 nPos = 0, nRead = xStream->readSomeBytes( aBuffer, 65536 );
                    while ( nRead > 0 )
                    {
                        if ( aData.getLength() < ( nPos + nRead ) )
                            aData.realloc( nPos + nRead );

                        aBuffer.realloc( nRead );
                        rtl_copyMemory( (void*)pData, (const void*)aBuffer.getConstArray(), nRead );
                        pData += nRead;
                        nPos += nRead;

                        aBuffer.realloc( 0 );
                        nRead = xStream->readSomeBytes( aBuffer, 65536 );
                    }

                    aData.realloc( nPos );
                    xData->setData( aData );
                }
                catch ( NotConnectedException const & ) {}
                catch ( BufferSizeExceededException const & ) {}
                catch ( IOException const & ) {}
            }

            try
            {
                Reference< XCommandEnvironment > aCmdEnv;
                String aURL = ::rtl::OUString( DEFINE_CONST_UNICODE("vnd.sun.staroffice.out:///~") );
                ::ucb::Content aOutbox( aURL, aCmdEnv );
                ::ucb::Content aMessage( aURL, aCmdEnv );

                sal_Int32 nIdx = 0, nCount = 4;
                sal_Bool bCc = sal_False, bBcc = sal_False;
                if ( mpCcList && mpCcList->Count() > 0 )
                {
                    bCc = sal_True;
                    nCount++;
                }
                if ( mpBccList && mpBccList->Count() > 0 )
                {
                    bBcc = sal_True;
                    nCount++;
                }
                Sequence < ::rtl::OUString > aNamesList(nCount);
                ::rtl::OUString* pNames = aNamesList.getArray();
                pNames[nIdx++] = ::rtl::OUString::createFromAscii( "Title" );
                pNames[nIdx++] = ::rtl::OUString::createFromAscii( "MessageFrom" );
                pNames[nIdx++] = ::rtl::OUString::createFromAscii( "MessageTo" );
                pNames[nIdx++] = ::rtl::OUString::createFromAscii( "MessageBody" );
                if ( bCc )
                    pNames[nIdx++] = ::rtl::OUString::createFromAscii( "MessageCC" );
                if ( bBcc )
                    pNames[nIdx++] = ::rtl::OUString::createFromAscii( "MessageBCC" );

                nIdx = 0;
                Sequence < Any > aValuesList(nCount);
                Any* pValues = aValuesList.getArray();
                pValues[nIdx++] = makeAny( ::rtl::OUString( maSubject ) );
                pValues[nIdx++] = makeAny( ::rtl::OUString( maFromAddress ) );
                String aValueList;
                MakeValueList( mpToList, aValueList );
                pValues[nIdx++] = makeAny( ::rtl::OUString( aValueList ) );
                pValues[nIdx++] = makeAny( xData );
                if ( bCc )
                {
                    MakeValueList( mpCcList, aValueList );
                    pValues[nIdx++] = makeAny( ::rtl::OUString( aValueList ) );
                }
                if ( bBcc )
                {
                    MakeValueList( mpBccList, aValueList );
                    pValues[nIdx++] = makeAny( ::rtl::OUString( aValueList ) );
                }

                bSend = aOutbox.insertNewContent(
                    ::rtl::OUString( DEFINE_CONST_UNICODE("application/vnd.sun.staroffice.message") ),
                    aNamesList, aValuesList, aMessage );
            }
            catch( com::sun::star::ucb::ContentCreationException& )
            {
                DBG_ERRORFILE( "ContentCreationException" );
            }
            catch( CommandAbortedException& e )
            {
                ByteString aError( UniString( e.Message ), RTL_TEXTENCODING_MS_1252 );
                DBG_ERRORFILE( aError.GetBuffer() );
            }
            catch( Exception& )
            {
                DBG_ERRORFILE( "Any other exception" );
            }
        }

        delete pStream;
    }

    return bSend;
}

