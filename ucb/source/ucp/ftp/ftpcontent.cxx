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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "ftpdynresultset.hxx"
#include "ftpresultsetfactory.hxx"
#include "ftpresultsetI.hxx"
#include "ftpcontent.hxx"
#include "ftpcontentprovider.hxx"
#include "ftpdirp.hxx"
#include "ftpcontentidentifier.hxx"
#include "ftpcfunc.hxx"
#include "ftpstrcont.hxx"
#include "ftpintreq.hxx"

#include <memory>
#include <vector>
#include <string.h>
#include "curl.hxx"
#include <curl/easy.h>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/fd_inputstream.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>

using namespace ftp;
using namespace com::sun::star::task;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::sdbc;



//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

FTPContent::FTPContent( const Reference< XMultiServiceFactory >& rxSMgr,
                        FTPContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier,
                        const FTPURL& aFTPURL)
    : ContentImplHelper(rxSMgr,pProvider,Identifier),
      m_pFCP(pProvider),
      m_aFTPURL(aFTPURL),
      m_bInserted(false),
      m_bTitleSet(false)
{
}



FTPContent::FTPContent( const Reference< XMultiServiceFactory >& rxSMgr,
                        FTPContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier,
                        const ContentInfo& Info)
    : ContentImplHelper(rxSMgr,pProvider,Identifier),
      m_pFCP(pProvider),
      m_aFTPURL(Identifier->getContentIdentifier(),
                pProvider),
      m_bInserted(true),
      m_bTitleSet(false),
      m_aInfo(Info)
{
}



//=========================================================================

FTPContent::~FTPContent()
{
}


//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_6( FTPContent,
                   XTypeProvider,
                   XServiceInfo,
                   XContent,
                   XCommandProcessor,
                   XContentCreator,
                   XChild);

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_6( FTPContent,
                          XTypeProvider,
                          XServiceInfo,
                          XContent,
                      XCommandProcessor,
                      XContentCreator,
                      XChild);

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// needed, because the service shall not be creatable!!
#undef XSERVICEINFO_CREATE_INSTANCE_IMPL
#define XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )

XSERVICEINFO_IMPL_1( FTPContent,
                     rtl::OUString( "com.sun.star.comp.FTPContent"),
                     rtl::OUString( "com.sun.star.ucb.FTPContent"));



//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL FTPContent::getContentType()
    throw( RuntimeException )
{
    return rtl::OUString(FTP_CONTENT_TYPE);
}


//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================


//virtual
void SAL_CALL FTPContent::abort( sal_Int32 /*CommandId*/ )
    throw( RuntimeException )
{
}



/***************************************************************************/
/*                                                                         */
/*                     Internal implementation class.                      */
/*                                                                         */
/***************************************************************************/


class ResultSetFactoryI
    : public ResultSetFactory
{
public:

    ResultSetFactoryI(const Reference<XMultiServiceFactory >&  xSMgr,
                      const Reference<XContentProvider >&  xProvider,
                      sal_Int32 nOpenMode,
                      const Sequence<Property>& seq,
                      const Sequence<NumberedSortingInfo>& seqSort,
                      const std::vector<FTPDirentry>& dirvec)
        : m_xSMgr(xSMgr),
          m_xProvider(xProvider),
          m_nOpenMode(nOpenMode),
          m_seq(seq),
          m_seqSort(seqSort),
          m_dirvec(dirvec)
    {
    }

    virtual ResultSetBase* createResultSet()
    {
        return new ResultSetI(m_xSMgr,
                              m_xProvider,
                              m_nOpenMode,
                              m_seq,
                              m_seqSort,
                              m_dirvec);
    }

public:

    Reference< XMultiServiceFactory >               m_xSMgr;
    Reference< XContentProvider >                   m_xProvider;
    sal_Int32                                       m_nOpenMode;
    Sequence< Property >                            m_seq;
    Sequence< NumberedSortingInfo >                 m_seqSort;
    std::vector<FTPDirentry>                        m_dirvec;
};



//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

enum ACTION { NOACTION,
              THROWAUTHENTICATIONREQUEST,
              THROWACCESSDENIED,
              THROWINTERACTIVECONNECT,
              THROWRESOLVENAME,
              THROWQUOTE,
              THROWNOFILE,
              THROWGENERAL };


// virtual
Any SAL_CALL FTPContent::execute(
    const Command& aCommand,
    sal_Int32 /*CommandId*/,
    const Reference<
    XCommandEnvironment >& Environment
)
    throw(
        Exception,
        CommandAbortedException,
        RuntimeException
    )
{
    ACTION action(NOACTION);
    Any aRet;

    while(true)
        try {
            if(action == THROWAUTHENTICATIONREQUEST) {
                // try to get a continuation first
                rtl::OUString aRealm,aPassword,aAccount;
                m_pFCP->forHost(m_aFTPURL.host(),
                                m_aFTPURL.port(),
                                m_aFTPURL.username(),
                                aPassword,
                                aAccount);
                rtl::Reference<ucbhelper::SimpleAuthenticationRequest>
                    p( new ucbhelper::SimpleAuthenticationRequest(
                        m_aFTPURL.ident(false, false),
                        m_aFTPURL.host(),      // ServerName
                        ucbhelper::SimpleAuthenticationRequest::ENTITY_NA,
                        aRealm,
                        ucbhelper::SimpleAuthenticationRequest
                        ::ENTITY_FIXED,
                        m_aFTPURL.username(),
                        ucbhelper::SimpleAuthenticationRequest
                        ::ENTITY_MODIFY,
                        aPassword));

                Reference<XInteractionHandler> xInteractionHandler;
                if(Environment.is())
                    xInteractionHandler =
                        Environment->getInteractionHandler();

                if( xInteractionHandler.is()) {
                    xInteractionHandler->handle(p.get());

                    Reference<XInterface> xSelection(
                        p->getSelection().get());

                    if(Reference<XInteractionRetry>(
                        xSelection,UNO_QUERY).is())
                        action = NOACTION;
                    else if(Reference<XInteractionSupplyAuthentication>(
                        xSelection,UNO_QUERY).is()) {
                        m_pFCP->setHost(
                            m_aFTPURL.host(),
                            m_aFTPURL.port(),
                            m_aFTPURL.username(),
                            p->getAuthenticationSupplier()->getPassword(),
                            aAccount);
                        action = NOACTION;
                    }
                }
                aRet = p->getRequest();
            }

//              if(aCommand.Name.compareToAscii(
//                  "getPropertyValues") == 0 &&
//                 action != NOACTION) {
//                  // It is not allowed to throw if
//                  // command is getPropertyValues
//                  rtl::Reference<ucbhelper::PropertyValueSet> xRow =
//                      new ucbhelper::PropertyValueSet(m_xSMgr);
//                  Sequence<Property> Properties;
//                  aCommand.Argument >>= Properties;
//                  for(int i = 0; i < Properties.getLength(); ++i)
//                      xRow->appendVoid(Properties[i]);
//                  aRet <<= Reference<XRow>(xRow.get());
//                  return aRet;
//              }

            switch (action)
            {
            case NOACTION:
                break;

            case THROWAUTHENTICATIONREQUEST:
                ucbhelper::cancelCommandExecution(
                    aRet,
                    Reference<XCommandEnvironment>(0));
                break;

            case THROWACCESSDENIED:
                {
                    Sequence<Any> seq(1);
                    PropertyValue value;
                    value.Name = rtl::OUString("Uri");
                    value.Handle = -1;
                    value.Value <<= m_aFTPURL.ident(false,false);
                    value.State = PropertyState_DIRECT_VALUE;
                    seq[0] <<= value;
                    ucbhelper::cancelCommandExecution(
                        IOErrorCode_ACCESS_DENIED,
                        seq,
                        Environment);
                    break;
                }
            case THROWINTERACTIVECONNECT:
                {
                    InteractiveNetworkConnectException excep;
                    excep.Server = m_aFTPURL.host();
                    aRet <<= excep;
                    ucbhelper::cancelCommandExecution(
                        aRet,
                        Environment);
                    break;
                }
            case THROWRESOLVENAME:
                {
                    InteractiveNetworkResolveNameException excep;
                    excep.Server = m_aFTPURL.host();
                    aRet <<= excep;
                    ucbhelper::cancelCommandExecution(
                        aRet,
                        Environment);
                    break;
                }
            case THROWNOFILE:
                {
                    Sequence<Any> seq(1);
                    PropertyValue value;
                    value.Name = rtl::OUString("Uri");
                    value.Handle = -1;
                    value.Value <<= m_aFTPURL.ident(false,false);
                    value.State = PropertyState_DIRECT_VALUE;
                    seq[0] <<= value;
                    ucbhelper::cancelCommandExecution(
                        IOErrorCode_NO_FILE,
                        seq,
                        Environment);
                    break;
                }
            case THROWQUOTE:
            case THROWGENERAL:
                ucbhelper::cancelCommandExecution(
                    IOErrorCode_GENERAL,
                    Sequence<Any>(0),
                    Environment);
                break;
            }

            if(aCommand.Name.compareToAscii("getPropertyValues") == 0) {
                Sequence<Property> Properties;
                if(!(aCommand.Argument >>= Properties))
                {
                    aRet <<= IllegalArgumentException(
                                rtl::OUString( "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >(this),
                                -1);
                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }

                aRet <<= getPropertyValues(Properties,Environment);
            }
            else if(aCommand.Name.compareToAscii("setPropertyValues") == 0)
            {
                Sequence<PropertyValue> propertyValues;

                if( ! ( aCommand.Argument >>= propertyValues ) ) {
                    aRet <<= IllegalArgumentException(
                                rtl::OUString( "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >(this),
                                -1);
                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }

                aRet <<= setPropertyValues(propertyValues);
            }
            else if(aCommand.Name.compareToAscii("getCommandInfo") == 0) {
                // Note: Implemented by base class.
                aRet <<= getCommandInfo(Environment);
            }
            else if(aCommand.Name.compareToAscii("getPropertySetInfo") == 0) {
                // Note: Implemented by base class.
                aRet <<= getPropertySetInfo(Environment);
            }
            else if(aCommand.Name.compareToAscii( "insert" ) == 0)
            {
                InsertCommandArgument aInsertArgument;
                if ( ! ( aCommand.Argument >>= aInsertArgument ) ) {
                    aRet <<= IllegalArgumentException(
                                rtl::OUString( "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >(this),
                                -1);
                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }
                insert(aInsertArgument,Environment);
            }
            else if(aCommand.Name.compareToAscii("delete") == 0) {
                m_aFTPURL.del();
                deleted();
            }
            else if(aCommand.Name.compareToAscii( "open" ) == 0) {
                OpenCommandArgument2 aOpenCommand;
                if ( !( aCommand.Argument >>= aOpenCommand ) ) {
                    aRet <<= IllegalArgumentException(
                                rtl::OUString( "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >(this),
                                -1);

                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }

                if(aOpenCommand.Mode == OpenMode::DOCUMENT) {
                    // Open as a document
                    Reference<XActiveDataSink>
                        xActiveDataSink(aOpenCommand.Sink,UNO_QUERY);
                    Reference< XOutputStream >
                        xOutputStream(aOpenCommand.Sink,UNO_QUERY);

                    if(xActiveDataSink.is()) {
                        xActiveDataSink->setInputStream(
                            new ucbhelper::FdInputStream(m_aFTPURL.open()));
                    }
                    else if(xOutputStream.is()) {
                        Reference<XInputStream> xStream(
                            new ucbhelper::FdInputStream(m_aFTPURL.open()));
                        Sequence<sal_Int8> byte_seq(4096);
                        sal_Int32 n = 1000; // value does not matter here
                        for (;;) {
                            n = xStream->readBytes(byte_seq,4096);
                            if (n == 0) {
                                break;
                            }
                            try {
                                if(byte_seq.getLength() != n)
                                    byte_seq.realloc(n);
                                xOutputStream->writeBytes(byte_seq);
                            } catch(const NotConnectedException&) {

                            } catch(const BufferSizeExceededException&) {

                            } catch(const IOException&) {

                            }
                        }
                        if(n) {
                            Sequence<Any> seq(1);
                            PropertyValue value;
                            value.Name = rtl::OUString("Uri");
                            value.Handle = -1;
                            value.Value <<= m_aFTPURL.ident(false,false);
                            value.State = PropertyState_DIRECT_VALUE;
                            seq[0] <<= value;
                            ucbhelper::cancelCommandExecution(
                                IOErrorCode_UNKNOWN,
                                seq,
                                Environment);
                        }
                    }
                    else {
                        aRet <<= UnsupportedDataSinkException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >(this),
                            aOpenCommand.Sink);
                        ucbhelper::cancelCommandExecution(aRet,Environment);
                    }
                }
                else if(aOpenCommand.Mode == OpenMode::ALL ||
                        aOpenCommand.Mode == OpenMode::DOCUMENTS ||
                        aOpenCommand.Mode == OpenMode::FOLDERS ) {
                    std::vector<FTPDirentry> resvec =
                        m_aFTPURL.list(sal_Int16(aOpenCommand.Mode));
                    Reference< XDynamicResultSet > xSet
                        = new DynamicResultSet(
                            m_xSMgr,
                            this,
                            aOpenCommand,
                            Environment,
                            new ResultSetFactoryI(m_xSMgr,
                                                  m_xProvider.get(),
                                                  aOpenCommand.Mode,
                                                  aOpenCommand.Properties,
                                                  aOpenCommand.SortingInfo,
                                                  resvec));
                    aRet <<= xSet;
                }
                else if(aOpenCommand.Mode ==
                        OpenMode::DOCUMENT_SHARE_DENY_NONE ||
                        aOpenCommand.Mode ==
                        OpenMode::DOCUMENT_SHARE_DENY_WRITE) {
                    // Unsupported OpenMode
                    aRet <<= UnsupportedOpenModeException(
                        rtl::OUString(),
                        static_cast< cppu::OWeakObject * >(this),
                        static_cast< sal_Int16 >(aOpenCommand.Mode));
                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }
                else {
                    aRet <<= IllegalArgumentException(
                                rtl::OUString( "Unexpected OpenMode!" ),
                                static_cast< cppu::OWeakObject * >(this),
                                -1);

                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }
            } else if(aCommand.Name.compareToAscii("createNewContent") == 0) {
                ContentInfo aArg;
                if (!(aCommand.Argument >>= aArg)) {
                    ucbhelper::cancelCommandExecution(
                        makeAny(
                            IllegalArgumentException(
                                rtl::OUString( "Wrong argument type!" ),
                                static_cast< cppu::OWeakObject * >(this),
                                -1)),
                        Environment);
                    // Unreachable
                }
                aRet <<= createNewContent(aArg);
            } else {
                aRet <<= UnsupportedCommandException(
                    aCommand.Name,
                    static_cast< cppu::OWeakObject * >(this));
                ucbhelper::cancelCommandExecution(aRet,Environment);
            }

            return aRet;
        } catch(const curl_exception& e) {
            if(e.code() == CURLE_COULDNT_CONNECT)
                action = THROWINTERACTIVECONNECT;
            else if(e.code() == CURLE_COULDNT_RESOLVE_HOST )
                action = THROWRESOLVENAME;
            else if(e.code() == CURLE_FTP_USER_PASSWORD_INCORRECT ||
                    e.code() == CURLE_LOGIN_DENIED ||
                    e.code() == CURLE_BAD_PASSWORD_ENTERED ||
                    e.code() == CURLE_FTP_WEIRD_PASS_REPLY)
                action = THROWAUTHENTICATIONREQUEST;
            else if(e.code() == CURLE_FTP_ACCESS_DENIED)
                action = THROWACCESSDENIED;
            else if(e.code() == CURLE_FTP_QUOTE_ERROR)
                action = THROWQUOTE;
            else if(e.code() == CURLE_FTP_COULDNT_RETR_FILE)
                action = THROWNOFILE;
            else
                // nothing known about the cause of the error
                action = THROWGENERAL;
        }
}

#define FTP_FILE rtl::OUString( "application/vnd.sun.staroffice.ftp-file")

#define FTP_FOLDER rtl::OUString( "application/vnd.sun.staroffice.ftp-folder")

Sequence<ContentInfo > SAL_CALL
FTPContent::queryCreatableContentsInfo(  )
    throw (RuntimeException)
{
    return queryCreatableContentsInfo_Static();
}

// static
Sequence<ContentInfo >
FTPContent::queryCreatableContentsInfo_Static(  )
    throw (RuntimeException)
{
    Sequence< ContentInfo > seq(2);

    seq[0].Type = FTP_FILE;
    seq[0].Attributes = ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
        | ContentInfoAttribute::KIND_DOCUMENT;
    Sequence< Property > props( 1 );
    props[0] = Property(
        rtl::OUString("Title"),
        -1,
        getCppuType( static_cast< rtl::OUString* >( 0 ) ),
        PropertyAttribute::MAYBEVOID
        | PropertyAttribute::BOUND );
    seq[0].Properties = props;

    // folder
    seq[1].Type       = FTP_FOLDER;
    seq[1].Attributes = ContentInfoAttribute::KIND_FOLDER;
    seq[1].Properties = props;

    return seq;
}

Reference<XContent > SAL_CALL
FTPContent::createNewContent( const ContentInfo& Info )
    throw (RuntimeException)
{
    if( Info.Type =="application/vnd.sun.staroffice.ftp-file" || Info.Type == "application/vnd.sun.staroffice.ftp-folder" )
        return new FTPContent(m_xSMgr,
                              m_pFCP,
                              m_xIdentifier,Info);
    else
        return Reference<XContent>(0);
}




Reference<XInterface > SAL_CALL
FTPContent::getParent(  )
    throw (RuntimeException)
{
    Reference<XContentIdentifier>
        xIdent(new FTPContentIdentifier(m_aFTPURL.parent(false)));
    Reference<XContent> xContent(m_xProvider->queryContent(xIdent));
    return Reference<XInterface>(xContent,UNO_QUERY);
}


void SAL_CALL
FTPContent::setParent(const Reference<XInterface >& /*Parent*/ )
    throw (NoSupportException,
           RuntimeException)
{
    throw NoSupportException();
}



rtl::OUString FTPContent::getParentURL()
{
    return m_aFTPURL.parent();
}


class InsertData
    : public CurlInput {

public:

    InsertData(const Reference<XInputStream>& xInputStream)
        : m_xInputStream(xInputStream) { }
    virtual ~InsertData() {}

    // returns the number of bytes actually read
    virtual sal_Int32 read(sal_Int8 *dest,sal_Int32 nBytesRequested);

private:

    Reference<XInputStream> m_xInputStream;
};



sal_Int32 InsertData::read(sal_Int8 *dest,sal_Int32 nBytesRequested)
{
    sal_Int32 m = 0;

    if(m_xInputStream.is()) {
            Sequence<sal_Int8> seq(nBytesRequested);
        m = m_xInputStream->readBytes(seq,nBytesRequested);
        memcpy(dest,seq.getConstArray(),m);
    }
    return m;
}


void FTPContent::insert(const InsertCommandArgument& aInsertCommand,
                        const Reference<XCommandEnvironment>& Env)
{
    osl::MutexGuard aGuard(m_aMutex);

    if(m_bInserted && !m_bTitleSet) {
        MissingPropertiesException excep;
        excep.Properties.realloc(1);
        excep.Properties[0] = rtl::OUString("Title");
        Any aAny; aAny <<= excep;
        ucbhelper::cancelCommandExecution(aAny,Env);
    }

    if(m_bInserted &&
       m_aInfo.Type == FTP_FILE &&
       !aInsertCommand.Data.is())
    {
        MissingInputStreamException excep;
        Any aAny; aAny <<= excep;
        ucbhelper::cancelCommandExecution(aAny,Env);
    }

    bool bReplace(aInsertCommand.ReplaceExisting);

 retry:
    try {
        if(m_aInfo.Type == FTP_FILE) {
            InsertData data(aInsertCommand.Data);
            m_aFTPURL.insert(bReplace,&data);
        } else if(m_aInfo.Type == FTP_FOLDER)
            m_aFTPURL.mkdir(bReplace);
    } catch(const curl_exception& e) {
        if(e.code() == FILE_EXIST_DURING_INSERT ||
           e.code() == FOLDER_EXIST_DURING_INSERT) {
            // Deprecated, not used anymore:
            NameClashException excep;
            excep.Name = m_aFTPURL.child();
            Any aAny;
            aAny <<= excep;
            ucbhelper::cancelCommandExecution(aAny,Env);
        } else if(e.code() == FOLDER_MIGHT_EXIST_DURING_INSERT ||
                  e.code() == FILE_MIGHT_EXIST_DURING_INSERT) {
            // Interact
            Reference<XInteractionHandler> xInt;
            if(Env.is())
                xInt = Env->getInteractionHandler();

            UnsupportedNameClashException excep;
            excep.NameClash = 0; //NameClash::ERROR;

            if(!xInt.is()) {
                Any aAny;
                aAny <<= excep;
                ucbhelper::cancelCommandExecution(aAny,Env);
            }

            XInteractionRequestImpl* p =
                new XInteractionRequestImpl(m_aFTPURL.child());
            Reference<XInteractionRequest> req(p);
            xInt->handle(req);
            if(p->approved()) {
                bReplace = true;
                goto retry;
            }
            else
                throw excep;
        }
        else
            throw;
    }

    // May not be reached, because both mkdir and insert can throw curl-
    // exceptions
    m_bInserted = false;
    inserted();
}



Reference< XRow > FTPContent::getPropertyValues(
    const Sequence< Property >& seqProp,
    const Reference<XCommandEnvironment>& /*environment*/
)
{
    rtl::Reference<ucbhelper::PropertyValueSet> xRow =
        new ucbhelper::PropertyValueSet(m_xSMgr);

    FTPDirentry aDirEntry = m_aFTPURL.direntry();

    for(sal_Int32 i = 0; i < seqProp.getLength(); ++i) {
        const rtl::OUString& Name = seqProp[i].Name;
        if(Name.compareToAscii("Title") == 0)
            xRow->appendString(seqProp[i],aDirEntry.m_aName);
        else if(Name.compareToAscii("CreatableContentsInfo") == 0)
            xRow->appendObject(seqProp[i],
                               makeAny(queryCreatableContentsInfo()));
        else if(aDirEntry.m_nMode != INETCOREFTP_FILEMODE_UNKNOWN) {
            if(Name.compareToAscii("ContentType") == 0)
                xRow->appendString(seqProp[i],
                                   aDirEntry.m_nMode&INETCOREFTP_FILEMODE_ISDIR
                                   ? FTP_FOLDER
                                   : FTP_FILE );
            else if(Name.compareToAscii("IsReadOnly") == 0)
                xRow->appendBoolean(seqProp[i],
                                    aDirEntry.m_nMode
                                    & INETCOREFTP_FILEMODE_WRITE
                                    ? 0
                                    : 1 );
            else if(Name.compareToAscii("IsDocument") == 0)
                xRow->appendBoolean(seqProp[i],
                                    ! sal_Bool(aDirEntry.m_nMode &
                                               INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.compareToAscii("IsFolder") == 0)
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(aDirEntry.m_nMode &
                                             INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.compareToAscii("Size") == 0)
                xRow->appendLong(seqProp[i],
                                 aDirEntry.m_nSize);
            else if(Name.compareToAscii("DateCreated") == 0)
                xRow->appendTimestamp(seqProp[i],
                                      aDirEntry.m_aDate);
            else
                xRow->appendVoid(seqProp[i]);
        } else
            xRow->appendVoid(seqProp[i]);
    }

    return Reference<XRow>(xRow.get());
}



Sequence<Any> FTPContent::setPropertyValues(
    const Sequence<PropertyValue>& seqPropVal)
{
    Sequence<Any> ret(seqPropVal.getLength());
    Sequence<PropertyChangeEvent > evt;

    osl::MutexGuard aGuard(m_aMutex);
    for(sal_Int32 i = 0; i < ret.getLength(); ++i) {
        if ( seqPropVal[i].Name == "Title" ) {
            rtl::OUString Title;
            if(!(seqPropVal[i].Value >>= Title)) {
                ret[i] <<= IllegalTypeException();
                continue;
            } else if(Title.isEmpty()) {
                ret[i] <<= IllegalArgumentException();
                continue;
            }

            if(m_bInserted) {
                m_aFTPURL.child(Title);
                m_xIdentifier =
                    new FTPContentIdentifier(m_aFTPURL.ident(false,false));
                m_bTitleSet = true;
            } else
                try {
                    rtl::OUString OldTitle = m_aFTPURL.ren(Title);
                    evt.realloc(1);
                    evt[0].PropertyName =
                        rtl::OUString("Title");
                    evt[0].Further = false;
                    evt[0].PropertyHandle = -1;
                    evt[0].OldValue <<= OldTitle;
                    evt[0].NewValue <<= Title;
                } catch(const curl_exception&) {
                    InteractiveIOException excep;
                    // any better possibility here?
                    // ( the error code is always CURLE_FTP_QUOTE_ERROR )
                    excep.Code = IOErrorCode_ACCESS_DENIED;
                    ret[i] <<= excep;
                }
        } else {
            Sequence<Property> props =
                getProperties(Reference<XCommandEnvironment>(0));

            // either unknown or read-only
            ret[i] <<= UnknownPropertyException();
            for(sal_Int32 j = 0; j < props.getLength(); ++j)
                if(props[j].Name == seqPropVal[i].Name) {
                    ret[i] <<= IllegalAccessException(
                        rtl::OUString( "Property is read-only!"),
                            //props[j].Attributes & PropertyAttribute::READONLY
                            //    ? "Property is read-only!"
                            //    : "Access denied!"),
                        static_cast< cppu::OWeakObject * >( this ));
                    break;
                }
        }
    }

    if(evt.getLength()) {
        // title has changed
        notifyPropertiesChange(evt);
        exchange(new FTPContentIdentifier(m_aFTPURL.ident(false,false)));
    }

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
