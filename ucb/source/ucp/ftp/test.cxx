/*************************************************************************
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: abi $ $Date: 2002-07-31 15:13:38 $
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


#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#include <comphelper/regpathhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif


#ifndef _TEST_ACTIVEDATASINK_HXX_
#include "test_activedatasink.hxx"
#endif
#ifndef _TEST_MULTISERVICEFAC_HXX_
#include "test_multiservicefac.hxx"
#endif
#ifndef _FTP_CONTENTPROVIDER_HXX_
#include "ftpcontentprovider.hxx"
#endif
#ifndef _FTP_CONTENTIDENTIFIER_HXX_
#include "ftpcontentidentifier.hxx"
#endif


using namespace test_ftp;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;



class FtpThread
    : public osl::Thread
{
public:

    FtpThread(const Reference<XContentProvider>& xProvider);


protected:

    void SAL_CALL run();


private:

    Reference<XContentProvider> m_xProvider;
};




FtpThread::FtpThread(const Reference<XContentProvider>& xProvider)
    : m_xProvider(xProvider) { }


void FtpThread::run() {
//      Reference<XContentIdentifierFactory> xFactory(m_xProvider,
//                                                    UNO_QUERY);
//      xFactory->createContentIdentifier(
//          rtl::OUString::createFromAscii("ftp://chagall/"));

    Reference<XContentIdentifier> xIdent(
        new ftp::FtpContentIdentifier(
            rtl::OUString::createFromAscii("ftp://chagall/")));


    Reference<XContent> xContent = m_xProvider->queryContent(xIdent);

    Reference<XActiveDataSink> xActDS(new Test_ActiveDataSink());
    OpenCommandArgument2 aOpenCommand(OpenMode::DOCUMENTS,
                                      0,
                                      Reference<XInterface>(xActDS,UNO_QUERY),
                                      Sequence<Property>(0),
                                      Sequence<NumberedSortingInfo>(0));
    Any aAny; aAny <<= aOpenCommand;

    Command aCommand(OUString::createFromAscii("open"),
                     -1,
                     aAny);

    Reference<XCommandProcessor> xCommandProcessor(xContent,UNO_QUERY);
    xCommandProcessor->execute(aCommand,0,Reference<XCommandEnvironment>(0));

    int n;
    Reference<XInputStream> xInputStream = xActDS->getInputStream();
    if(xInputStream.is())
        do {
            Sequence<sal_Int8> seq(0);
            n = xInputStream->readBytes(seq,2048);
            fprintf(stdout,OString(reinterpret_cast<const sal_Char*>(seq.getConstArray()),
                                   seq.getLength()).getStr());
        } while(n == 2048);

}



int main(int argc,char* argv[])
{
    Reference< XMultiServiceFactory > xFac;

    try {
        OUString systemRegistryPath = comphelper::getPathToSystemRegistry();
        OString path(systemRegistryPath.getStr(),
                     systemRegistryPath.getLength(),
                     RTL_TEXTENCODING_UTF8);
        fprintf(stdout,"\nsystem registry path: %s\n",path.getStr());

        xFac = cppu::createRegistryServiceFactory(systemRegistryPath,
                                                  OUString(),
                                                  true);
        if(!xFac.is()) {
            fprintf(stderr,"\ncould not create ServiceFactory");
            return 1;
        }

        comphelper::setProcessServiceFactory(xFac);

        Reference< XMultiServiceFactory > xFac(new Test_MultiServiceFactory());
        Reference< XContentProvider> xProvider(new ftp::FtpContentProvider(xFac));
        FtpThread aThread(xProvider);
        aThread.create();
        aThread.join();

        Reference< XComponent > xComponent(xFac,UNO_QUERY);
        if(xComponent.is())
            xComponent->dispose();
        return 0;
    } catch(const Exception& e) {
        fprintf(stderr,"\nERROR: any other error");
        fprintf(stderr,"\nERROR: P0-bug to ABI\n");
        return 1;
    }
}
