/*************************************************************************
 *
 *  $RCSfile: ftpcontent.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: abi $ $Date: 2002-08-29 09:45:03 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <memory>
#include <vector>
#include "ftpdynresultset.hxx"
#include "ftpresultsetfactory.hxx"
#include "ftpresultsetI.hxx"
#include "ftpcontent.hxx"
#include "ftpcontentprovider.hxx"
#include "ftploaderthread.hxx"
#include "ftpinpstr.hxx"
#include "ftpdirp.hxx"
#include "ftpcontentidentifier.hxx"
#include "ftpcfunc.hxx"
#include "ftpstrcont.hxx"

#include <curl/curl.h>
#include <curl/easy.h>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>


using namespace ftp;
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
                        const Reference< XContentIdentifier >& Identifier)
    : ContentImplHelper(rxSMgr,pProvider,Identifier),
      m_pFCP(pProvider)
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

XINTERFACE_IMPL_4( FTPContent,
                   XTypeProvider,
                   XServiceInfo,
                   XContent,
                   XCommandProcessor);

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( FTPContent,
                         XTypeProvider,
                         XServiceInfo,
                         XContent,
                      XCommandProcessor);

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// needed, because the service shall not be creatable!!
#undef XSERVICEINFO_CREATE_INSTANCE_IMPL
#define XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )

XSERVICEINFO_IMPL_1( FTPContent,
                     rtl::OUString::createFromAscii(
                         "com.sun.star.comp.FTPContent"),
                     rtl::OUString::createFromAscii(
                         "com.sun.star.ucb.FTPContent"));



//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL FTPContent::getContentType()
    throw( RuntimeException )
{
    return rtl::OUString::createFromAscii(MYUCP_CONTENT_TYPE);
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================


//virtual
void SAL_CALL FTPContent::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
}



/***************************************************************************/
/*                                                                         */
/*                           Interne Implklasse                            */
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


// virtual
Any SAL_CALL FTPContent::execute(
    const Command& aCommand,
    sal_Int32 CommandId,
    const Reference<
    XCommandEnvironment >& Environment
)
    throw(
        Exception,
        CommandAbortedException,
        RuntimeException
    )
{
    Any aRet;

    if(aCommand.Name.compareToAscii("getPropertyValues") == 0) {
        Sequence<Property> Properties;
        if(!(aCommand.Argument >>= Properties))
        {
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        aRet <<= getPropertyValues(Properties,Environment);
    }
    else if(aCommand.Name.compareToAscii("setPropertyValues") == 0) {
    }
    else if(aCommand.Name.compareToAscii("getCommandInfo") == 0) {
        // Note: Implemented by base class.
        aRet <<= getCommandInfo(Environment);
    }
    else if(aCommand.Name.compareToAscii("getPropertySetInfo") == 0) {
        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo(Environment);
    }
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 ) {
        OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) ) {
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        if(aOpenCommand.Mode == OpenMode::DOCUMENT) {
            // Open as a document
            CURL *curl = m_pFCP->handle();

            // Setting the header write function,
            // which receives the output of the control connection.

            std::auto_ptr<FTPInputStream> control(new FTPInputStream());
            FTPInputStreamContainer controlContainer(control.get());

            curl_easy_setopt(curl,CURLOPT_NOBODY,false);
            curl_easy_setopt(curl,
                             CURLOPT_HEADERFUNCTION,
                             ftp_write);
            curl_easy_setopt(curl,
                             CURLOPT_WRITEHEADER,
                             &controlContainer);

            // Now setting the URL

            rtl::OUString url(m_xIdentifier->getContentIdentifier());

            curl_easy_setopt(curl,
                             CURLOPT_URL,
                             rtl::OString(url.getStr(),
                                          url.getLength(),
                                          // Only ASCII in URLs => UTF8 ok
                                          RTL_TEXTENCODING_UTF8).getStr());

            curl_easy_setopt(curl,CURLOPT_POSTQUOTE,0);

            Reference<XActiveDataSink>
                xActiveDataSink(aOpenCommand.Sink,UNO_QUERY);
            Reference< XOutputStream >
                xOutputStream(aOpenCommand.Sink,UNO_QUERY);

            if(xActiveDataSink.is()) {
                FTPInputStreamContainer dataContainer(new FTPInputStream());
                curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,ftp_write);
                curl_easy_setopt(curl,CURLOPT_WRITEDATA,&dataContainer);
                curl_easy_perform(curl);
                xActiveDataSink->setInputStream(dataContainer());
            }
            else if(xOutputStream.is()) {
                FTPOutputStreamContainer dataContainer(xOutputStream);
                curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,
                                 ftp_write);
                curl_easy_setopt(curl,CURLOPT_WRITEDATA,&dataContainer);
                curl_easy_perform(curl);
            }
            else {
                aRet <<= UnsupportedDataSinkException();
                ucbhelper::cancelCommandExecution(aRet,Environment);
            }
        }
        else if(aOpenCommand.Mode == OpenMode::ALL ||
                aOpenCommand.Mode == OpenMode::DOCUMENTS ||
                aOpenCommand.Mode == OpenMode::FOLDERS ) {
            FTPURL aFTPURL(m_xIdentifier->getContentIdentifier(),
                           m_pFCP);
            try{
                std::vector<FTPDirentry> resvec =
                    aFTPURL.list(aOpenCommand.Mode);
                Reference< XDynamicResultSet > xSet
                    = new DynamicResultSet(
                        m_xSMgr,
                        this,
                        aOpenCommand,
                        Environment,
                        new ResultSetFactoryI(m_xSMgr,
                                              m_xProvider.getBodyPtr(),
                                              aOpenCommand.Mode,
                                              aOpenCommand.Properties,
                                              aOpenCommand.SortingInfo,
                                              resvec));
                aRet <<= xSet;
            } catch(const no_such_directory_exception& e) {
                if(e.code() == CURLE_COULDNT_CONNECT) {
                    InteractiveNetworkConnectException
                        excep;
                    excep.Server == aFTPURL.host();
                    aRet <<= excep;
                    ucbhelper::cancelCommandExecution(aRet,Environment);
                }
            }
        }
        else if(aOpenCommand.Mode == OpenMode::DOCUMENT_SHARE_DENY_NONE ||
                aOpenCommand.Mode == OpenMode::DOCUMENT_SHARE_DENY_WRITE) {
            // Unsupported OpenMode
            aRet <<= UnsupportedOpenModeException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }
        else {
            // IllegalArgumentException:: No OpenMode
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }
    }
    else
        throw CommandAbortedException();

    return aRet;
}



rtl::OUString FTPContent::getParentURL()
{
    return rtl::OUString();
}


extern void err_msg( const char* p,
                     const rtl::OUString& aOUString );


class FTPClientI
    : public FTPClient
{
public:

    FTPClientI(const Reference<XCommandEnvironment>& env);

    virtual rtl::OUString passwd() const;

private:

    Reference<XCommandEnvironment> m_env;
};



Reference< XRow > FTPContent::getPropertyValues(
    const Sequence< Property >& seqProp,
    const Reference<XCommandEnvironment>& environment
)
{
    vos::ORef<ucb::PropertyValueSet> xRow =
        new ucb::PropertyValueSet(m_xSMgr);

    FTPURL aFTPURL(m_xIdentifier->getContentIdentifier(),
                   m_pFCP);

    rtl::OUString passwd;
    bool retried(false);
    FTPDirentry aDirEntry;

 tryconnect:
    try {
        aDirEntry = aFTPURL.direntry(passwd);
        for(sal_Int32 i = 0; i < seqProp.getLength(); ++i) {
            const rtl::OUString& Name = seqProp[i].Name;
            if(Name.compareToAscii("ContentType") == 0)
                xRow->appendString(seqProp[i],
                                   rtl::OUString::createFromAscii(
                                       "application/ftp"));
            else if(Name.compareToAscii("Title") == 0)
                xRow->appendString(seqProp[i],aDirEntry.m_aName);
            else if(Name.compareToAscii("IsReadOnly") == 0)
                xRow->appendBoolean(seqProp[i],
                                    ! sal_Bool(aDirEntry.m_nMode &
                                               INETCOREFTP_FILEMODE_WRITE));
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
        }
    } catch(const no_such_directory_exception& e) {
        if(e.code() == CURLE_FTP_ACCESS_DENIED && !retried) {
            retried = true;
            FTPClientI aClient(environment);
            passwd = aClient.passwd();
            goto tryconnect;
        }
        else if(e.code() == CURLE_COULDNT_CONNECT) {
            InteractiveNetworkConnectException excep;
            excep.Server == aFTPURL.host();
            for(sal_Int32 i = 0; i < seqProp.getLength(); ++i) {
                xRow->appendVoid(seqProp[i]);
            }
        }
    }

    return Reference<XRow>(xRow.getBodyPtr());
}



// Some minor defs for 'ftpstrcont.hxx'


FTPOutputStreamContainer::FTPOutputStreamContainer(
    const Reference<XOutputStream>& out)
    : m_out(out) { }


int FTPOutputStreamContainer::write(
    void *buffer,size_t size,size_t nmemb)
{
    size_t ret = size*nmemb;
    if(ret && m_out.is()) {
        try {
            m_out->writeBytes(
                Sequence<sal_Int8>(static_cast<sal_Int8*>(buffer),
                                   ret)
            );
            return ret;
        } catch(const Exception&) {
        }
    }
    return 0;
}



FTPInputStreamContainer::FTPInputStreamContainer(
    FTPInputStream* out)
    : m_out(out)
{
}

int FTPInputStreamContainer::write(
    void *buffer,size_t size,size_t nmemb)
{
    size_t ret = size*nmemb;
    if(ret && m_out) {
        m_out->append(buffer,size,nmemb);
        return ret;
    }
    return 0;
}

Reference<XInputStream> FTPInputStreamContainer::operator()()
{
    return Reference<XInputStream>(m_out);
}




FTPClientI::FTPClientI(const Reference<XCommandEnvironment>& env)
    : m_env(env)
{
}

rtl::OUString FTPClientI::passwd() const
{
    return rtl::OUString::createFromAscii("abi:psswd");
}
