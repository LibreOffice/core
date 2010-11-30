/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
//              rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ftp://abi:psswd@abi-1/file")),
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

//      Command aCommand(OUString(RTL_CONSTASCII_USTRINGPARAM("open")),
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
//      sal_Int32 TEST = sce.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TEST")));
//      sal_Int32 WITH = sce.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WITH")));
//      sal_Int32 FROM = sce.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FROM")));
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
//      mathLib.load(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("libm.so")));
//      typedef double (*DF)(double);
//      DF func = (DF)mathLib.getSymbol(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cos")));
//      fprintf(stderr,"-------double %f ----------\n",(*func)(2.0));

//      fprintf(stderr,"-------testing %s ----------\n",scefile);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
