/*************************************************************************
 *
 *  $RCSfile: ftpcontent.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: abi $ $Date: 2002-07-31 15:12:53 $
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

FtpContent::FtpContent( const Reference< XMultiServiceFactory >& rxSMgr,
                        FtpContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier)
    : ContentImplHelper(rxSMgr,pProvider,Identifier),
      m_pFCP(pProvider)
{
}

//=========================================================================

FtpContent::~FtpContent()
{
}


//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_4( FtpContent,
                   XTypeProvider,
                   XServiceInfo,
                   XContent,
                   XCommandProcessor);

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( FtpContent,
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

XSERVICEINFO_IMPL_1( FtpContent,
                     rtl::OUString::createFromAscii(
                         "com.sun.star.comp.FtpContent"),
                     rtl::OUString::createFromAscii(
                         "com.sun.star.ucb.FtpContent"));



//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL FtpContent::getContentType()
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
void SAL_CALL FtpContent::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
}


struct StreamContainer
{
    Reference<XOutputStream> m_out;

    StreamContainer(const Reference<XOutputStream>& out)
        : m_out(out) { }
};


struct FtpBufferContainer
{
    FtpInputStream *m_out;

    FtpBufferContainer(FtpInputStream* out)
        : m_out(out) { }
};


#ifdef __cplusplus
extern "C" {
#endif

    int write2InputStream(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        size_t ret = size*nmemb;
        if(!stream || !ret)   // OK, no error if nothing can be written.
            return ret;

        FtpBufferContainer *p = reinterpret_cast<FtpBufferContainer*>(stream);
        if(p && p->m_out)
            p->m_out->append(buffer,size,nmemb);
        return ret;
    }


    /** Callback for curl_easy_perform(),
     *  forwarding the written content to the outputstream.
     */

    int write2OutputStream(void *buffer,size_t size,size_t nmemb,void *stream)
    {
        size_t ret = size*nmemb;

        if(!stream || !ret)  // OK, no error if nothing can be written.
            return ret;

        try{
            StreamContainer *p = reinterpret_cast<StreamContainer*>(stream);
            if(p && p->m_out.is())
                p->m_out->writeBytes(
                    Sequence<sal_Int8>(static_cast<sal_Int8*>(buffer),
                                       size*nmemb)
                );
            return ret;
        } catch(const Exception&) {
            return 0;
        }
    }

#ifdef __cplusplus
}
#endif



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





// virtual
Any SAL_CALL FtpContent::execute(
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
        Sequence< Property > Properties;
        if(!( aCommand.Argument >>= Properties))
        {
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        aRet <<= getPropertyValues(Properties);
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


        CURL *curl = m_pFCP->handle();

        // Setting the header write function,
        // which receives the output of the control connection.

        std::auto_ptr<FtpInputStream> header(new FtpInputStream());
        FtpBufferContainer headerContainer(header.get());

        curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,write2InputStream);
        curl_easy_setopt(curl,CURLOPT_WRITEHEADER,&headerContainer);


        // Now setting the URL

        rtl::OUString aOUStr(m_xIdentifier->getContentIdentifier());
        rtl::OString aOStr(aOUStr.getStr(),
                           aOUStr.getLength(),
                           RTL_TEXTENCODING_UTF8); // Only ASCII in URLs
        //                                         // => UTF8 ok

        curl_easy_setopt(curl,CURLOPT_URL,aOStr.getStr());

        if(aOpenCommand.Mode == OpenMode::DOCUMENT) {
            // Open as a document
            Reference<XActiveDataSink>
                xActiveDataSink(aOpenCommand.Sink,UNO_QUERY);
            Reference< XOutputStream >
                xOutputStream(aOpenCommand.Sink,UNO_QUERY);

            if(xActiveDataSink.is()) {
                FtpBufferContainer cont(new FtpInputStream());
                curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write2InputStream);
                curl_easy_setopt(curl,CURLOPT_WRITEDATA,&cont);
                curl_easy_perform(curl);
                xActiveDataSink->setInputStream(cont.m_out);
            }
            else if(xOutputStream.is()) {
                StreamContainer cont(xOutputStream);
                curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,
                                 write2OutputStream);
                curl_easy_setopt(curl,CURLOPT_WRITEDATA,&cont);
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
            std::auto_ptr<FtpInputStream> ap(new FtpInputStream());
            FtpBufferContainer cont(ap.get());
            curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write2InputStream);
            curl_easy_setopt(curl,CURLOPT_WRITEDATA,&cont);
            curl_easy_perform(curl);

            rtl::OUString aStr((char*)header.get()->getBuffer(),
                               sal_Int32(header.get()->getLength()),
                               RTL_TEXTENCODING_UTF8);

            // Now parse the content.
            // Parsing is here somewhat ugly, because
            // the regular expression does not forward
            // the pointer to end of parsed expression.

            sal_uInt32 len = (sal_uInt32) ap.get()->getLength();
            char* fwd = (char*) ap.get()->getBuffer();
            char *p1, *p2;
            p1 = p2 = fwd;

            enum OS { DOS,UNIX,VMS,UNKNOWN };
            OS osKind(UNKNOWN);
            std::vector<FTPDirentry> resvec;
            FTPDirentry aDirEntry;

            while(true) {
                while(p2-fwd < int(len) && *p2 != '\n') ++p2;
                if(p2-fwd == int(len)) break;

                *p2 = 0;
                switch(osKind) {
                    case DOS:
                        FTPDirectoryParser::parseDOS(aDirEntry,p1);
                        break;
                    case UNIX:
                        FTPDirectoryParser::parseUNIX(aDirEntry,p1);
                        break;
                    case VMS:
                        FTPDirectoryParser::parseUNIX(aDirEntry,p1);
                        break;
                    default:
                        if(FTPDirectoryParser::parseUNIX(aDirEntry,p1))
                            osKind = UNIX;
                        else if(FTPDirectoryParser::parseDOS(aDirEntry,p1))
                            osKind = DOS;
                        else if(FTPDirectoryParser::parseVMS(aDirEntry,p1))
                            osKind = VMS;
                }
                if(osKind != int(UNKNOWN)) {
                    if(1 + aOUStr.lastIndexOf(sal_Unicode('/')) ==
                       aOUStr.getLength())
                        aDirEntry.m_aURL =
                            aOUStr +
                            aDirEntry.m_aName;
                    else
                        aDirEntry.m_aURL =
                            aOUStr +
                            sal_Unicode('/') +
                            aDirEntry.m_aName;

                    resvec.push_back(aDirEntry);
                    aDirEntry.clear();
                }

                p1 = p2 + 1;
            }

            if(osKind == int(UNKNOWN)) {
                // Ok, this was not a directory, but a file
                //todo: Check here wether our parent lists our name
            }
            else {
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


//              curl_slist_free_all(list);



rtl::OUString FtpContent::getParentURL()
{
    return rtl::OUString();
}



Reference< XRow > FtpContent::getPropertyValues(
    const Sequence< Property >& seqProp
)
{
    FTPDirentry aDirEntry;
    vos::ORef<::ucb::PropertyValueSet> xRow =
        new ::ucb::PropertyValueSet(m_xSMgr);

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
                                sal_Bool(aDirEntry.m_nMode &
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

    return Reference<XRow>(xRow.getBodyPtr());
}
