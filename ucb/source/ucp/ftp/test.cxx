/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:53:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "test_ftpurl.hxx"
#include <stdio.h>
#include <unistd.h>

//  #include <vos/process.hxx>
//  #include <osl/process.h>


//  #include <osl/thread.hxx>

//  #include <ucbhelper/configurationkeys.hxx>
//  #include <cppuhelper/bootstrap.hxx>
//  #include <cppuhelper/servicefactory.hxx>
//  #include <comphelper/processfactory.hxx>
//  #include <comphelper/regpathhelper.hxx>
//  #include <com/sun/star/lang/XComponent.hpp>
//  #include <com/sun/star/lang/XMultiServiceFactory.hpp>
//  #include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
//  #include <com/sun/star/ucb/XContent.hpp>
//  #include <com/sun/star/ucb/XContentProvider.hpp>
//  #include <com/sun/star/ucb/XCommandProcessor.hpp>
//  #include <com/sun/star/ucb/OpenMode.hpp>
//  #include <com/sun/star/ucb/Command.hpp>
//  #include <com/sun/star/ucb/OpenCommandArgument2.hpp>
//  #include <com/sun/star/io/XActiveDataSink.hpp>
//  #include <com/sun/star/beans/Property.hpp>


//  #include "debughelper.hxx"
//  #include "test_activedatasink.hxx"
//  #include "test_ftpurl.hxx"
//  #include "test_multiservicefac.hxx"
//  #include "ftpcontentprovider.hxx"
//  #include "ftpcontentidentifier.hxx"


//  using namespace test_ftp;
//  using namespace rtl;
//  using namespace com::sun::star::uno;
//  using namespace com::sun::star::lang;
//  using namespace com::sun::star::ucb;
//  using namespace com::sun::star::beans;
//  using namespace com::sun::star::io;



//  class FTPThread
//      : public osl::Thread
//  {
//  public:

//      FTPThread();

//      int number_of_errors() { return _number_of_errors; }

//  protected:

//      void SAL_CALL run();


//  private:

//      ftp::FTPContentProvider    *pProvider;
//      Reference<XContentProvider> xProvider;
//      Reference<XMultiServiceFactory> xFac;

//      int _number_of_errors;
//  };


//  Reference< XMultiServiceFactory > createApplicationServiceManager()
//  {
//      try
//      {
//          Reference<XComponentContext> xComponentContext =
//              ::cppu::defaultBootstrap_InitialComponentContext();
//          Reference<XMultiServiceFactory> xMS(
//              xComponentContext->getServiceManager(),
//              UNO_QUERY);

//          return xMS;
//      }
//      catch( ::com::sun::star::uno::Exception& )
//      {
//          return Reference< XMultiServiceFactory >();
//      }
//  }


//  FTPThread::FTPThread()
//      : _number_of_errors(0),
//        xFac(createApplicationServiceManager())
//  {
//      pProvider = new ftp::FTPContentProvider(xFac);
//      xProvider = Reference<XContentProvider>(pProvider);

//      if(!xProvider.is())
//          ++_number_of_errors;
//  }


//  void FTPThread::run() {
//      if(!xFac.is()) {
//          ++_number_of_errors;
//          exit(1);
//      }

//      Reference<XContentIdentifier> xIdent(
//          new ftp::FTPContentIdentifier(
//              rtl::OUString::createFromAscii("ftp://abi:psswd@abi-1/file"),
//              pProvider));

//      Reference<XContent> xContent = xProvider->queryContent(xIdent);
//      if(!xContent.is())
//      {
//          err_msg("no content",OUString());
//          exit(1);
//      }


//      Reference<XActiveDataSink> xActDS(new Test_ActiveDataSink());
//      OpenCommandArgument2 aOpenCommand(OpenMode::DOCUMENT,
//                                        0,
//                                        Reference<XInterface>(xActDS,UNO_QUERY),
//                                        Sequence<Property>(0),
//                                        Sequence<NumberedSortingInfo>(0));
//      Any aAny; aAny <<= aOpenCommand;

//      Command aCommand(OUString::createFromAscii("open"),
//                       -1,
//                       aAny);

//      Reference<XCommandProcessor> xCommandProcessor(xContent,UNO_QUERY);
//      if(!xCommandProcessor.is())
//      {
//          err_msg("no command_processor",OUString());
//          exit(1);
//      }
//      xCommandProcessor->execute(aCommand,
//                                 0,Reference<XCommandEnvironment>(0));

//      Reference<XInputStream> xInputStream = xActDS->getInputStream();
//      if(!xInputStream.is())
//          ;
//  }


int main(int argc,char* argv[])
{
//      FTPThread aThread;
//      aThread.create();
//      aThread.join();

    typedef int (*INT_FUNC)(void);
    INT_FUNC tests[] = { test_ftpurl,
                   test_ftpparent,
                   test_ftpproperties,
                   test_ftpopen,
                   test_ftplist,
                   0 };  // don't remove ending zero

    int err_level = 0;

    fprintf(stderr,"-------       Testing       ----------\n");

    int i = -1;
    do {
        INT_FUNC f = tests[++i];
        if(f) {
            err_level += (*f)();
            fprintf(stderr,".");
        } else
            break;
    } while(true);

    if(err_level) {
        fprintf(stderr,"number of failed tests: %d\n",err_level);
        fprintf(stderr,"----------------------------------\n");
    } else
        fprintf(stderr,"no errors\n");

    return err_level;
}



//      char *scefile = 0;
//      if(strcmp(argv[1],"-sce") == 0)
//          scefile = argv[2];

//      if(!scefile) {
//          fprintf(stderr,"usage: ftptest -sce filename\n");
//          return 1;
//      }

//      rtl::OUString sceurl;
//      osl::FileBase::RC err =
//          osl::FileBase::getFileURLFromSystemPath(
//              rtl::OUString(scefile,
//                            strlen(scefile),
//                            RTL_TEXTENCODING_UTF8),
//              sceurl);

//      osl::File aFile(sceurl);
//      err = aFile.open(OpenFlag_Read);
//      if(err != osl::FileBase::E_None) {
//          fprintf(stderr,"could not open sce-file %s\n",scefile);
//          return 1;
//      }

//      sal_uInt64 n;
//      char buffer[256];
//      rtl::OUStringBuffer bff;
//      do {
//          err = aFile.read((void*)buffer,256,n);
//          bff.appendAscii(buffer,sal_Int32(n));
//      } while(err == osl::FileBase::E_None && n == 256);

//      aFile.close();

//      rtl::OUString sce = bff.makeStringAndClear();

//      fprintf(stderr,rtl::OString(sce.getStr(),
//                                  sce.getLength(),
//                                  RTL_TEXTENCODING_UTF8).getStr());

//      rtl::OUString lib,tmp,testlib;
//      std::vector<rtl::OUString> tests;
//      sal_Int32 TEST = sce.indexOf(rtl::OUString::createFromAscii("TEST"));
//      sal_Int32 WITH = sce.indexOf(rtl::OUString::createFromAscii("WITH"));
//      sal_Int32 FROM = sce.indexOf(rtl::OUString::createFromAscii("FROM"));
//      lib = sce.copy(TEST+4,WITH-TEST-4).trim();

//      tmp = sce.copy(WITH+4,FROM-WITH-4).trim();
//      sal_Int32 nIndex = 0;
//      do
//      {
//          rtl::OUString token = tmp.getToken(0,';',nIndex).trim();
//          if(token.getLength())
//              tests.push_back(token);
//      } while(nIndex >= 0);

//      testlib = sce.copy(FROM+4).trim();

//  //      fprintf(stderr,"%s\n",
//  //              rtl::OString(token.getStr(),
//  //                           token.getLength(),
//  //                           RTL_TEXTENCODING_UTF8).getStr());

//      osl::Module testLib;
//      if(!testLib.load(testlib)) {
//          fprintf(stderr,"library not found: %s\n",
//                  rtl::OString(testlib.getStr(),
//                               testlib.getLength(),
//                               RTL_TEXTENCODING_UTF8).getStr());
//          return 1;
//      }

//      osl::Module mathLib;
//      mathLib.load(rtl::OUString::createFromAscii("libm.so"));
//      typedef double (*DF)(double);
//      DF func = (DF)mathLib.getSymbol(rtl::OUString::createFromAscii("cos"));
//      fprintf(stderr,"-------double %f ----------\n",(*func)(2.0));

//      fprintf(stderr,"-------testing %s ----------\n",scefile);
