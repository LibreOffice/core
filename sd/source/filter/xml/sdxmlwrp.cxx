/*************************************************************************
 *
 *  $RCSfile: sdxmlwrp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:45 $
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

#pragma hdrstop

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef _SDXMLWRP_HXX
#include "sdxmlwrp.hxx"
#endif

#ifndef _SDXMLIMP_HXX
#include <xmloff/sdxmlimp.hxx>
#endif

#ifndef _SDXMLEXP_HXX
#include <xmloff/sdxmlexp.hxx>
#endif

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>  //! only necessary until SfxMedium has a uno3 DataSource / DataSink
#endif

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#include <xmloff/pkgurl.hxx>

using namespace com::sun::star;
using namespace rtl;

//////////////////////////////////////////////////////////////////////////////

char __READONLY_DATA sXML_draw[] = "draw";
char __READONLY_DATA sXML_impress[] = "presentation";

//////////////////////////////////////////////////////////////////////////////

SdXMLWrapper::SdXMLWrapper(
    uno::Reference<frame::XModel>& xRef,
    SfxMedium& rMedium,
    BOOL bIsDraw, BOOL bShowProg)
:   mxLocalModel(xRef),
    mrMedium(rMedium),
    mbIsDraw(bIsDraw),
    mbShowProgress(bShowProg)
{
}

BOOL SdXMLWrapper::Import()
{
    if(!mxLocalModel.is())
    {
        DBG_ERROR("Got NO Model in XMLImport");
        return FALSE;
    }

    uno::Reference<lang::XServiceInfo> xServiceInfo(mxLocalModel, uno::UNO_QUERY);
    if(!xServiceInfo.is() || !xServiceInfo->supportsService(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"))))
    {
        DBG_ERROR("Model is no DrawingDocument in XMLImport");
        return FALSE;
    }

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(utl::getProcessServiceFactory());
    if(!xServiceFactory.is())
    {
        DBG_ERROR("XMLReader::Read: got no service manager");
        return FALSE;
    }

    // Get data source ...
    uno::Reference<io::XActiveDataSource> xSource = mrMedium.GetDataSource();

    if(!xSource.is())
    {
//      // If we didn't get a data source from a medium, we have to create one
//      XInterfaceRef xFactory = xServiceFactory->createInstance(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.DataSourceFactory"));
//      if(xFactory.is())
//      {
//          XMultiServiceFactoryRef xMFactory(xFactory, USR_QUERY);
//
//          if(xMFactory.is())
//          {
//              UString sURL(S2WS(mrMedium.GetName()));
//              Sequence<Any> aArgs(1);
//              Any* pArgs = aArgs.getArray();
//
//              pArgs->setString(sURL);
//
//              XInterfaceRef xSrc = xMFactory->createInstanceWithArguments(sURL, aArgs);
//
//              if(xSrc.is())
//              {
//                  xSrc->queryInterface(XActiveDataSource::getSmartUik(), xSource);
//              }
//          }
//      }
    }

    // get data source
    if(!xSource.is())
    {
        DBG_ERROR("XMLReader::Read: data source missing");
        return FALSE;
    }

    // get parser
    uno::Reference<uno::XInterface> xXMLParser(xServiceFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser"))));
    if(!xXMLParser.is())
    {
        DBG_ERROR("com.sun.star.xml.sax.Parser service missing");
        return FALSE;
    }

    // get a pipe for connecting the data source to the parser
    uno::Reference<uno::XInterface> xPipe(xServiceFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pipe"))));
    if(!xPipe.is())
    {
        DBG_ERROR("com.sun.star.io.Pipe service missing");
        return FALSE;
    }

    UINT16 nStyleFamilyMask(0);
    BOOL bLoadDoc(TRUE);

// this is stuff for loading only styles or add-load documents, needed later
//  USHORT nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
//  BOOL bInsert;
//  if( aOpt.IsFmtsOnly() )
//  {
//      bLoadDoc = FALSE;
//      bInsert = aOpt.IsMerge();
//      nStyleFamilyMask = 0U;
//      if( aOpt.IsFrmFmts() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_FRAME;
//      if( aOpt.IsPageDescs() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_PAGE;
//      if( aOpt.IsTxtFmts() )
//          nStyleFamilyMask |= (SFX_STYLE_FAMILY_CHAR|SFX_STYLE_FAMILY_PARA);
//      if( aOpt.IsNumRules() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;
//  }
//  else
//  {
//      bLoadDoc = TRUE;
//      bInsert = bInsertMode;
//      nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
//  }
//  aOpt.ResetAllFmtsOnly();

    // get filter
    uno::Reference<xml::sax::XDocumentHandler> xFilter =
        new SdXMLImport(mxLocalModel, bLoadDoc, nStyleFamilyMask, mbShowProgress, IsDraw());

    // connect pipe's output stream to the data source
    uno::Reference<io::XOutputStream> xPipeOutput(xPipe, uno::UNO_QUERY);
    xSource->setOutputStream(xPipeOutput);

    // connect pipe's input stream to the parser
    xml::sax::InputSource aParserInput;
    uno::Reference<io::XInputStream> xPipeInput(xPipe, uno::UNO_QUERY);
    aParserInput.aInputStream = xPipeInput;
    aParserInput.sSystemId = mrMedium.GetName();
    OUString sFileName = mrMedium.GetName();

    // connect parser and filter
    uno::Reference<xml::sax::XParser> xParser(xXMLParser, uno::UNO_QUERY);

//      uno::Reference<xml::sax::XDocumentHandler> xPacker =
//    new URLPacker(sFileName, xFilter, sal_True, sal_False);
//  xParser->setDocumentHandler(xPacker);
    xParser->setDocumentHandler(xFilter);

    // parse
    uno::Reference<io::XActiveDataControl> xSourceControl(xSource, uno::UNO_QUERY);
    xSourceControl->start();
    BOOL bRetval(TRUE);

    try
    {
        xParser->parseStream(aParserInput);
    }
    catch(xml::sax::SAXParseException)
    {
        bRetval = FALSE;
    }
    catch(xml::sax::SAXException)
    {
        bRetval = FALSE;
    }
    catch(io::IOException)
    {
        bRetval = FALSE;
    }

    return bRetval;
}

BOOL SdXMLWrapper::Export()
{
    if(!mxLocalModel.is())
    {
        DBG_ERROR("Got NO Model in XMLExport");
        return FALSE;
    }

    uno::Reference<lang::XServiceInfo> xServiceInfo(mxLocalModel, uno::UNO_QUERY);
    if(!xServiceInfo.is() || !xServiceInfo->supportsService(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"))))
    {
        DBG_ERROR("Model is no DrawingDocument in XMLExport");
        return FALSE;
    }

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(utl::getProcessServiceFactory());
    if(!xServiceFactory.is())
    {
        DBG_ERROR("got no service manager");
        return FALSE;
    }

    uno::Reference<uno::XInterface> xWriter(xServiceFactory->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
    if(!xWriter.is())
    {
        DBG_ERROR("com.sun.star.xml.sax.Writer service missing");
        return FALSE;
    }

    // smart -> uno3 conversion only until SfxMedium has a uno3 DataSink
    uno::Reference<io::XOutputStream> xOut = mrMedium.GetDataSink();
    uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
    xSrc->setOutputStream(xOut);

    uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);
    OUString sFileName = mrMedium.GetName();
//  uno::Reference<xml::sax::XDocumentHandler> xPacker =
//      new URLPacker(sFileName, xHandler, sal_False, sal_True);
//  SdXMLExport aExp(mxLocalModel, sFileName, xPacker, mbShowProgress, IsDraw());
    SdXMLExport aExp(mxLocalModel, sFileName, xHandler, mbShowProgress, IsDraw());

    // give string descriptor as parameter for doc type
    BOOL bRet = (0 == aExp.exportDoc( IsDraw() ? sXML_draw : sXML_impress ));

    return bRet;
}


