/*************************************************************************
 *
 *  $RCSfile: swxml.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:59 $
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


#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <sfx2/docfile.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif
#ifndef _FLTINI_HXX //autogen wg. XMLReader
#include <fltini.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX //autogen wg. SwDoc
#include <docsh.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

XMLReader::XMLReader()
{
}

sal_uInt32 XMLReader::Read( SwDoc &rDoc, SwPaM &rPaM, const String & rName )
{
    Reference< lang::XMultiServiceFactory > xServiceFactory =
            utl::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_READ_ERROR;

    // Get model
    SwDocShell *pDocSh = rDoc.GetDocShell();
    ASSERT( pDocSh, "XMLReader::Read: got no doc shell" );
    if( !pDocSh )
        return ERR_SWG_READ_ERROR;

    Reference< frame::XModel > xModel = pDocSh->GetModel();
    ASSERT( xModel.is(),
            "XMLReader::Read: got no model" );
    if( !xModel.is() )
        return ERR_SWG_READ_ERROR;

    // Get data source ...
    Reference< io::XActiveDataSource > xSource;
    if( pMedium /* && pMedium->GetLoadEnvironment_Impl() */ )
    {
        // if there is a medium and if this medium has a load environment,
        // we get an active data source from the medium.
        xSource = pMedium->GetDataSource();
        ASSERT( xSource.is(), "XMLReader:: got no data source from medium" );
    }
    if( !xSource.is() )
    {
        // If we didn't get a data source from a medium, we have to create
        // one.
        Reference< XInterface > xFactory = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.frame.DataSourceFactory") );
        if( xFactory.is() )
        {
            Reference< lang::XMultiServiceFactory > xMFactory( xFactory,
                                                               UNO_QUERY );
            if( xMFactory.is() )
            {
                OUString sURL( rName );
                Sequence< Any > aArgs(1);
                Any* pArgs = aArgs.getArray();
                *pArgs <<= sURL;

                Reference< XInterface > xSrc =
                    xMFactory->createInstanceWithArguments( sURL, aArgs );
                if( xSrc.is() )
                {
                    Reference< io::XActiveDataSource > xTmp( xSrc, UNO_QUERY );
                    xSource = xTmp;
                }
            }
        }
    }

    // get data source
    ASSERT( xSource.is(), "XMLReader::Read: data source missing" );
    if( !xSource.is() )
        return ERR_SWG_READ_ERROR;

    // get parser
    Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
    ASSERT( xXMLParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
    if( !xXMLParser.is() )
        return ERR_SWG_READ_ERROR;

    // get a pipe for connecting the data source to the parser
    Reference< XInterface > xPipe = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.io.Pipe") );
    ASSERT( xPipe.is(),
            "XMLReader::Read: com.sun.star.io.Pipe service missing" );
    if( !xPipe.is() )
        return ERR_SWG_READ_ERROR;

    sal_uInt16 nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
    sal_Bool bLoadDoc;
    sal_Bool bInsert;
    if( aOpt.IsFmtsOnly() )
    {
        bLoadDoc = sal_False;
        bInsert = aOpt.IsMerge();
        nStyleFamilyMask = 0U;
        if( aOpt.IsFrmFmts() )
            nStyleFamilyMask |= SFX_STYLE_FAMILY_FRAME;
        if( aOpt.IsPageDescs() )
            nStyleFamilyMask |= SFX_STYLE_FAMILY_PAGE;
        if( aOpt.IsTxtFmts() )
            nStyleFamilyMask |= (SFX_STYLE_FAMILY_CHAR|SFX_STYLE_FAMILY_PARA);
        if( aOpt.IsNumRules() )
            nStyleFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;
    }
    else
    {
        bLoadDoc = sal_True;
        bInsert = bInsertMode;
        nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
    }
    aOpt.ResetAllFmtsOnly();

    // get filter
    Reference< xml::sax::XDocumentHandler > xFilter =
            new SwXMLImport( rDoc, rPaM, bLoadDoc, bInsert, nStyleFamilyMask,
                               xModel );

    // connect pipe's output stream to the data source
    Reference< io::XOutputStream > xPipeOutput( xPipe, UNO_QUERY );
    xSource->setOutputStream( xPipeOutput );

    // connect pipe's input stream to the parser
     xml::sax::InputSource aParserInput;
    Reference< io::XInputStream > xPipeInput( xPipe, UNO_QUERY );
    aParserInput.aInputStream = xPipeInput;
    aParserInput.sSystemId = rName;

    // connect parser and filter
    Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
    xParser->setDocumentHandler( xFilter );

    rDoc.AddLink(); // prevent deletion
    sal_uInt32 nRet = 0;

    // parse
    Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
    xSourceControl->start();
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& )
    {
        nRet = ERR_SWG_READ_ERROR;
    }
    catch( xml::sax::SAXException&)
    {
        nRet = ERR_SWG_READ_ERROR;
    }
    catch( io::IOException& )
    {
        nRet = ERR_SWG_READ_ERROR;
    }

    rDoc.RemoveLink();

    return nRet;
}


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.13  2000/09/18 16:05:04  willem.vandorp
      OpenOffice header added.

      Revision 1.12  2000/07/07 13:58:36  mib
      text styles using StarOffice API

      Revision 1.11  2000/05/03 12:08:05  mib
      unicode

      Revision 1.10  2000/03/21 15:10:56  os
      UNOIII

      Revision 1.9  2000/03/13 14:33:43  mib
      UNO3

      Revision 1.8  2000/02/11 14:40:44  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.7  1999/11/26 11:15:20  mib
      loading of styles only and insert mode

      Revision 1.6  1999/11/19 16:40:21  os
      modules renamed

      Revision 1.5  1999/11/19 15:27:05  mib
      Opt: using OUString constructor instead of StringToOUString

      Revision 1.4  1999/09/23 11:53:47  mib
      i18n, token maps and hard paragraph attributes

      Revision 1.3  1999/08/19 14:51:30  HR
      #65293#: fixed exception macro usage


      Rev 1.2   19 Aug 1999 16:51:30   HR
   #65293#: fixed exception macro usage

      Rev 1.1   17 Aug 1999 16:31:26   MIB
   import

      Rev 1.0   12 Aug 1999 12:28:08   MIB
   Initial revision.

*************************************************************************/

