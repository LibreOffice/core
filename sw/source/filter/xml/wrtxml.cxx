/*************************************************************************
 *
 *  $RCSfile: wrtxml.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-13 08:44:24 $
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

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <sfx2/docfile.hxx>
#endif
#ifndef _PAM_HXX //autogen wg. SwPaM
#include <pam.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX //autogen wg. SwDoc
#include <docsh.hxx>
#endif

#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _WRTXML_HXX
#include <wrtxml.hxx>
#endif
#ifndef _XMLEXP_HXX
#include <xmlexp.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SwXMLWriter::SwXMLWriter()
{
}


__EXPORT SwXMLWriter::~SwXMLWriter()
{
}

sal_uInt32 SwXMLWriter::WriteStream()
{
    ASSERT( !this, "SwXMLWriter::WriteStream: use Write!" );

    return ERR_SWG_WRITE_ERROR;
}

sal_uInt32 SwXMLWriter::Write( SwPaM& rPaM, SfxMedium& rMed,
                               const String* pFileName )
{
    Reference< lang::XMultiServiceFactory > xServiceFactory =
            utl::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "SwXMLWriter::Write: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_WRITE_ERROR;

    Reference< XInterface > xWriter = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Writer") );
    ASSERT( xWriter.is(),
            "SwXMLWriter::Write: com.sun.star.xml.sax.Writer service missing" );
    if(!xWriter.is())
        return ERR_SWG_WRITE_ERROR;

    Reference< frame::XModel > xModel = rPaM.GetDoc()->GetDocShell()->GetModel();
    ASSERT( xModel.is(),
            "XMLWriter::Write: got no model" );
    if( !xModel.is() )
        return ERR_SWG_WRITE_ERROR;

    pDoc = rPaM.GetDoc();
    PutNumFmtFontsInAttrPool();
    PutEditEngFontsInAttrPool();

    Reference< io::XOutputStream > xOut = rMed.GetDataSink();
    Reference< io::XActiveDataSource > xSrc( xWriter, UNO_QUERY );
    xSrc->setOutputStream( xOut );

    Reference< xml::sax::XDocumentHandler > xHandler( xWriter, UNO_QUERY );

    SwXMLExport *pExp = new SwXMLExport( xModel, rPaM, *pFileName, xHandler,
                                         bWriteAll, bWriteOnlyFirstTable,
                                         bShowProgress );

    sal_uInt32 nRet = pExp->exportDoc( sXML_text );

    delete pExp;

    ResetWriter();

    return nRet;
}

// -----------------------------------------------------------------------

void GetXMLWriter( const String&, WriterRef& xRet )
{
    xRet = new SwXMLWriter;
}

// -----------------------------------------------------------------------

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/wrtxml.cxx,v 1.2 2000-11-13 08:44:24 mib Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.17  2000/09/18 16:05:04  willem.vandorp
      OpenOffice header added.

      Revision 1.16  2000/07/21 12:55:15  mib
      text import/export using StarOffice API

      Revision 1.15  2000/06/08 09:45:54  aw
      changed to use functionality from xmloff project now

      Revision 1.14  2000/05/03 12:08:05  mib
      unicode

      Revision 1.13  2000/03/21 15:10:56  os
      UNOIII

      Revision 1.12  2000/03/13 14:33:44  mib
      UNO3

      Revision 1.11  2000/03/03 16:07:54  pl
      #73771# workaround for c50 intel compiler

      Revision 1.10  2000/02/11 14:40:52  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.9  1999/11/26 11:09:47  mib
      progress, export-flags

      Revision 1.8  1999/11/19 16:40:21  os
      modules renamed

      Revision 1.7  1999/10/26 13:34:30  mib
      removed 'using namespace' from header files

      Revision 1.6  1999/10/25 10:41:48  mib
      Using new OUString ASCII methods

      Revision 1.5  1999/10/15 14:48:25  hr
      export() -> exportDoc()

      Revision 1.4  1999/10/15 12:36:39  mib
      added document class attribute

      Revision 1.3  1999/10/08 11:47:06  mib
      moved some file to SVTOOLS/SVX

      Revision 1.2  1999/09/22 11:56:36  mib
      string -> wstring

      Revision 1.1  1999/08/12 10:28:26  MIB
      Initial revision.


      Rev 1.0   12 Aug 1999 12:28:26   MIB
   Initial revision.

*************************************************************************/

