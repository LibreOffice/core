/*************************************************************************
 *
 *  $RCSfile: wrtxml.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: dvo $ $Date: 2001-04-02 11:26:14 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <svx/xmlgrhlp.hxx>
#endif
#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
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
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SwXMLWriter::SwXMLWriter( sal_Bool bPl ) :
    bPlain( bPl )
{
}


__EXPORT SwXMLWriter::~SwXMLWriter()
{
}


/// export through an XML exporter component (output stream version)
sal_uInt32 WriteThroughComponent(
    Reference<io::XOutputStream> xOutputStream,
    Reference<XComponent> xComponent,
    Reference<lang::XMultiServiceFactory> & rFactory,
    const sal_Char* pComponentName,
    const Sequence<Any> & rArguments,
    const Sequence<beans::PropertyValue> & rMediaDesc,
    sal_Bool bBlockMode )
{
    ASSERT(xOutputStream.is(), "I really need an output stream!");
    ASSERT(xComponent.is(), "Need component!");
    ASSERT(NULL != pComponentName, "Need component name!");

    // get component
    Reference< io::XActiveDataSource > xSaxWriter(
        rFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Writer") ),
        UNO_QUERY );
    ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );
    if(!xSaxWriter.is())
        return ERR_SWG_WRITE_ERROR;

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    Reference<xml::sax::XDocumentHandler> xDocHandler( xSaxWriter,UNO_QUERY);
    Sequence<Any> aArgs( 1 + rArguments.getLength() );
    aArgs[0] <<= xDocHandler;
    for(sal_Int32 i = 0; i < rArguments.getLength(); i++)
        aArgs[i+1] = rArguments[i];

    // get filter component
    Reference< document::XExporter > xExporter(
        rFactory->createInstanceWithArguments(
            OUString::createFromAscii(pComponentName), aArgs), UNO_QUERY);
    ASSERT( xExporter.is(),
            "can't instantiate export filter component" );
    if( !xExporter.is() )
        return ERR_SWG_WRITE_ERROR;

    // set block mode (if appropriate)
    if( bBlockMode )
    {
        Reference<XUnoTunnel> xFilterTunnel( xExporter, UNO_QUERY );
        if (xFilterTunnel.is())
        {
            SwXMLExport *pFilter = (SwXMLExport *)xFilterTunnel->getSomething(
                                            SwXMLExport::getUnoTunnelId() );
            if (NULL != pFilter)
                pFilter->setBlockMode();
        }
    }


    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    Reference < XFilter > xFilter( xExporter, UNO_QUERY );
    xFilter->filter( rMediaDesc );

    return 0;
}

/// export through an XML exporter component (storage version)
sal_uInt32 WriteThroughComponent(
    SvStorage* pStorage,
    Reference<XComponent> xComponent,
    const sal_Char* pStreamName,
    Reference<lang::XMultiServiceFactory> & rFactory,
    const sal_Char* pComponentName,
    const Sequence<Any> & rArguments,
    const Sequence<beans::PropertyValue> & rMediaDesc,
    sal_Bool bBlockMode,
    sal_Bool bCompress=sal_True )
{
    ASSERT(NULL != pStorage, "Need storage!");
    ASSERT(NULL != pStreamName, "Need stream name!");

    Reference< io::XOutputStream > xOutputStream;
    SvStorageStreamRef xDocStream;

    // open stream
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    xDocStream = pStorage->OpenStream( sStreamName,
                                       STREAM_WRITE | STREAM_SHARE_DENYWRITE );
    DBG_ASSERT(xDocStream.Is(), "Can't create output stream in package!");
    if (! xDocStream.Is())
        return ERR_SWG_WRITE_ERROR;

    xDocStream->SetSize( 0 );

    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
    OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
    uno::Any aAny;
    aAny <<= aMime;
    xDocStream->SetProperty( aPropName, aAny );

    if( !bCompress )
    {
        aPropName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("Compressed") );
        sal_Bool bFalse = sal_False;
        aAny.setValue( &bFalse, ::getBooleanCppuType() );
        xDocStream->SetProperty( aPropName, aAny );
    }

    // set buffer and create outputstream
    xDocStream->SetBufferSize( 16*1024 );
    xOutputStream = new utl::OOutputStreamWrapper( *xDocStream );

    // write the stuff
    sal_Int32 nRet = WriteThroughComponent(
        xOutputStream, xComponent, rFactory,
        pComponentName, rArguments, rMediaDesc, bBlockMode);

    // finally, commit stream.
    if( 0 == nRet )
        xDocStream->Commit();

    return nRet;
}

sal_uInt32 SwXMLWriter::_Write()
{
    // Get service factory
    Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "SwXMLWriter::Write: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_WRITE_ERROR;

    // Get data sink ...
    Reference< io::XOutputStream > xOut;
    SvStorageStreamRef xDocStream;
    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    if( pStg )
    {
        // export graphics and objects only in packages

        pGraphicHelper = SvXMLGraphicHelper::Create( *pStg,
                                                     GRAPHICHELPER_MODE_WRITE,
                                                     sal_False );
        xGraphicResolver = pGraphicHelper;

        SvPersist *pPersist = pDoc->GetPersist();
        if( pPersist )
        {
            pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                             *pStg, *pPersist,
                                             EMBEDDEDOBJECTHELPER_MODE_WRITE,
                                             sal_False );
            xObjectResolver = pObjectHelper;
        }
    }

    // create and prepare the XPropertySet that gets passed through
    // the components, and the XStatusIndicator that shows progress to
    // the user.

    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry aInfoMap[] =
    {
        { "ProgressRange", sizeof("ProgressRange")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ProgressMax", sizeof("ProgressMax")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ProgressCurrent", sizeof("ProgressCurrent")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    // create XStatusIndicator
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    if (bShowProgress)
    {
        uno::Reference<frame::XModel> xModel( pDoc->GetDocShell()->GetModel());
        if (xModel.is())
        {
            uno::Reference<frame::XController> xController(
                xModel->getCurrentController());
            if( xController.is())
            {
                uno::Reference<frame::XFrame> xFrame( xController->getFrame());
                if( xFrame.is())
                {
                    uno::Reference<task::XStatusIndicatorFactory> xFactory(
                        xFrame, uno::UNO_QUERY );
                    if( xFactory.is())
                    {
                        xStatusIndicator = xFactory->createStatusIndicator();
                    }
                }
            }
        }

        // set progress range and start status indicator
        sal_Int32 nProgressRange(1000000);
        if (xStatusIndicator.is())
        {
            OUString sLoading(RTL_CONSTASCII_USTRINGPARAM("Saving ..."));
            xStatusIndicator->start(sLoading, nProgressRange);
        }
        uno::Any aProgRange;
        aProgRange <<= nProgressRange;
        OUString sProgressRange(RTL_CONSTASCII_USTRINGPARAM("ProgressRange"));
        xInfoSet->setPropertyValue(sProgressRange, aProgRange);
    }

    // filter arguments
    // - graphics + object resolver for styles + content
    // - status indicator
    // - info property set
    // - else empty
    Sequence < Any > aFilterArgs( 4 );
    Any *pArgs = aFilterArgs.getArray();
    *pArgs++ <<= xGraphicResolver;
    *pArgs++ <<= xObjectResolver;
    *pArgs++ <<= xStatusIndicator;
    *pArgs++ <<= xInfoSet;
    Sequence < Any > aEmptyArgs( 2 );
    pArgs = aEmptyArgs.getArray();
    *pArgs++ <<= xStatusIndicator;
    *pArgs++ <<= xInfoSet;

    //Get model
    Reference< lang::XComponent > xModelComp(
        pDoc->GetDocShell()->GetModel(), UNO_QUERY );
    ASSERT( xModelComp.is(), "XMLWriter::Write: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_WRITE_ERROR;

    PutNumFmtFontsInAttrPool();
    PutEditEngFontsInAttrPool();

    // properties
    Sequence < PropertyValue > aProps( pOrigFileName ? 1 : 0 );
    if( pOrigFileName )
    {
        PropertyValue *pProps = aProps.getArray();
        pProps->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FileName") );
        (pProps++)->Value <<= OUString( *pOrigFileName  );
    }

    // export sub streams for package, else full stream into a file
    if (NULL != pStg)
    {
        if( !bOrganizerMode )
            WriteThroughComponent(
                pStg, xModelComp, "meta.xml", xServiceFactory,
                "com.sun.star.comp.Writer.XMLMetaExporter",
                aEmptyArgs, aProps, bBlock, sal_False );

        WriteThroughComponent(
            pStg, xModelComp, "styles.xml", xServiceFactory,
            "com.sun.star.comp.Writer.XMLStylesExporter",
            aFilterArgs, aProps, bBlock );

        WriteThroughComponent(
            pStg, xModelComp, "settings.xml", xServiceFactory,
            "com.sun.star.comp.Writer.XMLSettingsExporter",
            aFilterArgs, aProps, bBlock );

        if( !bOrganizerMode )
            WriteThroughComponent(
                pStg, xModelComp, "content.xml", xServiceFactory,
                "com.sun.star.comp.Writer.XMLContentExporter",
                aFilterArgs, aProps, bBlock );

    }
    else
    {
        // create single stream and do full export
        Reference<io::XOutputStream> xOut =
            new utl::OOutputStreamWrapper( *pStrm );
        WriteThroughComponent(
            xOut, xModelComp, xServiceFactory,
            "com.sun.star.comp.Writer.XMLExporter",
            aEmptyArgs, aProps, bBlock );
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    if (xStatusIndicator.is())
    {
        xStatusIndicator->end();
    }

    return 0;
}
sal_uInt32 SwXMLWriter::WriteStream()
{
    return _Write();
}

sal_uInt32 SwXMLWriter::WriteStorage()
{
    return _Write();
}

sal_uInt32 SwXMLWriter::Write( SwPaM& rPaM, SfxMedium& rMed,
                               const String* pFileName )
{
    return IsStgWriter()
            ? ((StgWriter *)this)->Write( rPaM, *rMed.GetOutputStorage( sal_True ), pFileName )
            : ((Writer *)this)->Write( rPaM, *rMed.GetOutStream(), pFileName );
}

sal_Bool SwXMLWriter::IsStgWriter() const
{
    return !bPlain;
}

// -----------------------------------------------------------------------

void GetXMLWriter( const String& rName, WriterRef& xRet )
{
    xRet = new SwXMLWriter( rName.EqualsAscii( FILTER_XMLP ) );
}

// -----------------------------------------------------------------------

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/xml/wrtxml.cxx,v 1.23 2001-04-02 11:26:14 dvo Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.22  2001/03/19 13:45:17  mtg
      added support for export of settings.xml

      Revision 1.21  2001/03/09 14:58:43  dvo
      - fixed: unnecessary attext.xml stream removed (this is handled in core/swg)

      Revision 1.20  2001/03/07 15:23:07  mib
      set mime types

      Revision 1.19  2001/03/06 11:05:07  mib
      organizer support

      Revision 1.18  2001/03/02 21:02:30  dvo
      - changed: content and styles are written as separate streams

      Revision 1.17  2001/03/01 15:47:53  dvo
      - #84291# fixed: assertion removed (it's legal for the asserted condition to occur)

      Revision 1.16  2001/02/13 17:54:54  dvo
      - changed: in wrtxml.cxx substreams now use common code
      - added: document classes for global, label, etc. documents
      - added: support for bSaveLinkedSections flag

      Revision 1.15  2001/02/06 15:41:55  dvo
      - added: auto text event ex- and import

      Revision 1.14  2001/01/26 11:22:48  mib
      ole objects continued

      Revision 1.13  2001/01/22 12:31:45  mib
      block mode

      Revision 1.12  2001/01/17 10:55:18  mib
      XML filter now is a component

      Revision 1.11  2001/01/12 16:34:01  cl
      #82042# added support for xml filter components

      Revision 1.10  2001/01/08 09:44:55  mib
      Removed SwDoc and SvStorage members from SwXMLExport

      Revision 1.9  2001/01/03 11:40:56  mib
      support for OLE objects in XML files

      Revision 1.8  2000/12/06 08:39:34  mib
      #81388#: Content stream now is called Content.xml

      Revision 1.7  2000/12/02 10:57:15  mib
      #80795#: use packages

      Revision 1.6  2000/11/27 13:44:40  mib
      #80795#: Use packages within XML filter

      Revision 1.5  2000/11/20 11:17:53  mib
      Put edit engine's and numbering rules' fonts into the pool

      Revision 1.4  2000/11/20 09:18:37  jp
      must change: processfactory moved

      Revision 1.3  2000/11/14 08:03:32  mib
      Adding of EditEngine- and Bullet-Font-Items temporarily removed

      Revision 1.2  2000/11/13 08:44:24  mib
      font declarations and asian/complex font properties

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

