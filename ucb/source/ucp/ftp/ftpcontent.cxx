/*************************************************************************
 *
 *  $RCSfile: ftpcontent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: abi $ $Date: 2002-06-24 15:17:55 $
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

#ifndef _FTP_FTPCONTENT_HXX_
#include "ftpcontent.hxx"
#endif
#ifndef _FTP_FTPCONTENTPROVIDER_HXX_
#include "ftpcontentprovider.hxx"
#endif
#ifndef _FTP_FTPLOADERTHREAD_HXX_
#include "ftploaderthread.hxx"
#endif
#ifndef _FTP_FTPINPSTR_HXX_
#include "ftpinpstr.hxx"
#endif
#ifndef __CURL_CURL_H
#include <curl/curl.h>
#endif
#ifndef __CURL_EASY_H
#include <curl/easy.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif

using namespace ftp;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;


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
                     rtl::OUString::createFromAscii("com.sun.star.comp.FtpContent"),
                     rtl::OUString::createFromAscii("com.sun.star.ucb.FtpContent"));



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
            p->m_out->append(buffer,ret);
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
                p->m_out->writeBytes(Sequence<sal_Int8>(static_cast<sal_Int8*>(buffer),
                                                      size*nmemb));
            return ret;
        } catch(const Exception&) {
            return 0;
        }
    }

#ifdef __cplusplus
}
#endif


// virtual
Any SAL_CALL FtpContent::execute( const Command& aCommand,
                                  sal_Int32 CommandId,
                                  const Reference<
                                  XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{

    Any aRet;

    if( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 ) {
    }
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 ) {
        OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
            throw IllegalArgumentException();

        Reference< XActiveDataStreamer > activeDataStreamer( aOpenCommand.Sink,UNO_QUERY );
        if(activeDataStreamer.is())
            throw UnsupportedDataSinkException();

        CURL *curl = m_pFCP->handle();

        /** Now setting the URL
         */

        rtl::OUString aOUStr(m_xIdentifier->getContentIdentifier());
        rtl::OString aOStr(aOUStr.getStr(),
                           aOUStr.getLength(),
                           RTL_TEXTENCODING_UTF8); // Only ASCII in URLs => UTF8 ok
        curl_easy_setopt(curl,CURLOPT_URL,aOStr.getStr());

        Reference<XActiveDataSink> activeDataSink(aOpenCommand.Sink,UNO_QUERY);
        if(activeDataSink.is()) {
            FtpBufferContainer cont(new FtpInputStream());
            curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write2InputStream);
            curl_easy_setopt(curl,CURLOPT_WRITEDATA,&cont);
            curl_easy_perform(curl);
            activeDataSink->setInputStream(cont.m_out);
        }

        Reference< XOutputStream > xOutputStream(aOpenCommand.Sink,UNO_QUERY);
        if(xOutputStream.is()) {
            StreamContainer cont(xOutputStream);
            curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write2OutputStream);
            curl_easy_setopt(curl,CURLOPT_WRITEDATA,&cont);
            curl_easy_perform(curl);
        }
    }
    else
        throw CommandAbortedException();

    return aRet;
}


//              curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,write2OutputStream);
//              curl_easy_setopt(curl,CURLOPT_WRITEHEADER,&cont);


//              curl_slist_free_all(list);


Sequence<Property> FtpContent::getProperties(const Reference<XCommandEnvironment>& xEnv)
{
    Sequence<Property> ret(0);
    return ret;
}



Sequence<CommandInfo> FtpContent::getCommands(const Reference<XCommandEnvironment> & xEnv)
{
    return Sequence<CommandInfo>(0);
}



rtl::OUString FtpContent::getParentURL()
{
    return rtl::OUString();
}

