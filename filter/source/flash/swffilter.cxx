/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

#include "swfexporter.hxx"
#include "swfuno.hxx"

#include <string.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::view;

using ::com::sun::star::lang::XComponent;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::container::XIndexAccess;

namespace swf {

class OslOutputStreamWrapper : public ::cppu::WeakImplHelper<css::io::XOutputStream>
{
    osl::File   mrFile;

public:
    explicit OslOutputStreamWrapper(const OUString& rFileName) : mrFile(rFileName)
    {
        osl_removeFile(rFileName.pData);
        mrFile.open( osl_File_OpenFlag_Create|osl_File_OpenFlag_Write );
    }

    // css::io::XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL closeOutput(  ) override;
};

void SAL_CALL OslOutputStreamWrapper::writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
{
    sal_uInt64 uBytesToWrite = aData.getLength();
    sal_uInt64 uBytesWritten = 0;

    sal_Int8 const * pBuffer = aData.getConstArray();

    while( uBytesToWrite )
    {
        osl::File::RC eRC = mrFile.write( pBuffer, uBytesToWrite, uBytesWritten);

        switch( eRC )
        {
        case osl::File::E_INVAL:    // the format of the parameters was not valid
        case osl::File::E_FBIG:        // File too large

        case osl::File::E_AGAIN:    // Operation would block
        case osl::File::E_BADF:        // Bad file
        case osl::File::E_FAULT:    // Bad address
        case osl::File::E_INTR:        // function call was interrupted
        case osl::File::E_IO:        // I/O error
        case osl::File::E_NOLCK:    // No record locks available
        case osl::File::E_NOLINK:    // Link has been severed
        case osl::File::E_NOSPC:    // No space left on device
        case osl::File::E_NXIO:        // No such device or address
            throw css::io::IOException();    // TODO: Better error handling
        default: break;
        }

        uBytesToWrite -= uBytesWritten;
        pBuffer += uBytesWritten;
    }
}

void SAL_CALL OslOutputStreamWrapper::flush(  )
{
}

void SAL_CALL OslOutputStreamWrapper::closeOutput(  )
{
    osl::File::RC eRC = mrFile.close();

    switch( eRC )
    {
    case osl::File::E_INVAL:    // the format of the parameters was not valid

    case osl::File::E_BADF:        // Bad file
    case osl::File::E_INTR:        // function call was interrupted
    case osl::File::E_NOLINK:    // Link has been severed
    case osl::File::E_NOSPC:    // No space left on device
    case osl::File::E_IO:        // I/O error
        throw css::io::IOException();    // TODO: Better error handling
    default: break;
    }
}


class FlashExportFilter : public cppu::WeakImplHelper
<
    css::document::XFilter,
    css::document::XExporter,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{
    Reference< XComponent > mxDoc;
    Reference< XComponentContext > mxContext;
    Reference< XStatusIndicator> mxStatusIndicator;

    // #i56084# variables for selection export
    Reference< XShapes > mxSelectedShapes;
    Reference< XDrawPage > mxSelectedDrawPage;
    bool mbExportSelection;

public:
    explicit FlashExportFilter( const Reference< XComponentContext > &rxContext);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) override;

    bool ExportAsMultipleFiles( const Sequence< PropertyValue >& aDescriptor );
    bool ExportAsSingleFile( const Sequence< PropertyValue >& aDescriptor );

    virtual void SAL_CALL cancel( ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

FlashExportFilter::FlashExportFilter(const Reference< XComponentContext > &rxContext)
    : mxDoc()
    , mxContext(rxContext)
    , mxStatusIndicator()
    , mxSelectedShapes()
    , mxSelectedDrawPage()
    , mbExportSelection(false)
{
}

OUString exportBackground(FlashExporter &aFlashExporter, const Reference< XDrawPage >& xDrawPage, const OUString& sPath, sal_uInt32 nPage, const char* suffix)
{
    OUString filename = STR("slide") + VAL(nPage+1) + STR(suffix) + STR(".swf");
    OUString fullpath = sPath + STR("/") + filename;

    // AS: If suffix is "o" then the last parameter is true (for exporting objects).
    Reference<XOutputStream> xOutputStreamWrap(*(new OslOutputStreamWrapper(fullpath)), UNO_QUERY);
    sal_uInt16 nCached = aFlashExporter.exportBackgrounds( xDrawPage, xOutputStreamWrap, sal::static_int_cast<sal_uInt16>( nPage ), *suffix == 'o' );
    aFlashExporter.Flush();
    xOutputStreamWrap.clear();

    if (nCached != nPage)
    {
        osl_removeFile(fullpath.pData);
        if ( 0xffff == nCached )
            return STR("NULL");
        else
            return STR("slide") + VAL(nCached+1) + STR(suffix) + STR(".swf");
    }

    return filename;
}

template <typename TYPE>
TYPE findPropertyValue(const Sequence< PropertyValue >& aPropertySequence, const sal_Char* name, TYPE def)
{
    TYPE temp = TYPE();

    sal_Int32 nLength = aPropertySequence.getLength();
    const PropertyValue * pValue = aPropertySequence.getConstArray();

    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name.equalsAsciiL ( name, strlen(name) ) )
        {
            pValue[i].Value >>= temp;
            return temp;
        }
    }

    return def;
}

sal_Bool SAL_CALL FlashExportFilter::filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
{
    mxStatusIndicator = findPropertyValue<Reference<XStatusIndicator> >(aDescriptor, "StatusIndicator", mxStatusIndicator);

    Sequence< PropertyValue > aFilterData;
    aFilterData = findPropertyValue<Sequence< PropertyValue > >(aDescriptor, "FilterData", aFilterData);

    // #i56084# check if selection shall be exported only; if yes, get the selected page and the selection itself
    if(findPropertyValue<bool>(aDescriptor, "SelectionOnly", false))
    {
        Reference< XDesktop2 > xDesktop(Desktop::create(mxContext));

        if(xDesktop.is())
        {
            Reference< XFrame > xFrame(xDesktop->getCurrentFrame());

            if(xFrame.is())
            {
                Reference< XController > xController(xFrame->getController());

                if(xController.is())
                {
                    Reference< XDrawView > xDrawView(xController, UNO_QUERY);

                    if(xDrawView.is())
                    {
                        mxSelectedDrawPage = xDrawView->getCurrentPage();
                    }

                    if(mxSelectedDrawPage.is())
                    {
                        Reference< XSelectionSupplier > xSelection(xController, UNO_QUERY);

                        if(xSelection.is())
                        {
                            xSelection->getSelection() >>= mxSelectedShapes;
                        }
                    }
                }
            }
        }
    }

    if(mxSelectedDrawPage.is() && mxSelectedShapes.is() && mxSelectedShapes->getCount())
    {
        // #i56084# to export selection we need the selected page and the selected shapes.
        // There must be shapes selected, else fallback to regular export (export all)
        mbExportSelection = true;
    }

    // #i56084# no multiple files (suppress) when selection since selection can only export a single page
    if (!mbExportSelection && findPropertyValue<bool>(aFilterData, "ExportMultipleFiles", false ))
    {
        ExportAsMultipleFiles(aDescriptor);
    }
    else
    {
        ExportAsSingleFile(aDescriptor);
    }

    if( mxStatusIndicator.is() )
        mxStatusIndicator->end();

    return true;
}


// AS: When exporting as multiple files, each background, object layer, and slide gets its own
//  file.  Additionally, a file called BackgroundConfig.txt is generated, indicating which
//  background and objects (if any) go with each slide.  The files are named slideNb.swf,
//  slideNo.swf, and slideNp.swf, where N is the slide number, and b=background, o=objects, and
//  p=slide contents.  Note that under normal circumstances, there will be very few b and o files.

// AS: HACK!  Right now, I create a directory as a sibling to the swf file selected in the Export
//  dialog.  This directory is called presentation.sxi-swf-files.  The name of the swf file selected
//  in the Export dialog has no impact on this.  All files created are placed in this directory.
bool FlashExportFilter::ExportAsMultipleFiles(const Sequence< PropertyValue >& aDescriptor)
{
    Reference< XDrawPagesSupplier > xDrawPagesSupplier(mxDoc, UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        return false;

    Reference< XIndexAccess > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );
    if(!xDrawPages.is())
        return false;

    Reference< XDesktop2 > rDesktop = Desktop::create( mxContext );

    Reference< XStorable > xStorable(rDesktop->getCurrentComponent(), UNO_QUERY);
    if (!xStorable.is())
        return false;

    Reference< XDrawPage > xDrawPage;

    Reference< XFrame > rFrame = rDesktop->getCurrentFrame();
    Reference< XDrawView > rDrawView( rFrame->getController(), UNO_QUERY );

    Reference< XDrawPage > rCurrentPage = rDrawView->getCurrentPage();

    Sequence< PropertyValue > aFilterData;

    aFilterData = findPropertyValue<Sequence< PropertyValue > >(aDescriptor, "FilterData", aFilterData);

    //AS: Do a bunch of path mangling to figure out where to put the files.

    OUString sOriginalPath = findPropertyValue<OUString>(aDescriptor, "URL", OUString());

    // AS: sPath is the parent directory, where everything else exists (like the sxi,
    //  the -swf-files folder, the -audio files, etc.
    sal_Int32 lastslash = sOriginalPath.lastIndexOf('/');
    OUString sPath( sOriginalPath.copy(0, lastslash) );

    OUString sPresentation(xStorable->getLocation());

    lastslash = sPresentation.lastIndexOf('/') + 1;
    sal_Int32 lastdot = sPresentation.lastIndexOf('.');

    // AS: The name of the presentation, without 3 character extension.
    OUString sPresentationName;
    if (lastdot < 0)  // fdo#71309 in case file has no name
        sPresentationName = sPresentation.copy(lastslash);
    else
        sPresentationName = sPresentation.copy(lastslash, lastdot - lastslash);

    OUString fullpath, swfdirpath, backgroundfilename, objectsfilename;

    swfdirpath = sPath + STR("/") + sPresentationName + STR(".sxi-swf-files");

    oslFileError err;
    err = osl_createDirectory( swfdirpath.pData );

    fullpath = swfdirpath + STR("/backgroundconfig.txt");

    oslFileHandle aBackgroundConfig( nullptr );

    // AS: Only export the background config if we're exporting all of the pages, otherwise we'll
    //  screw it up.
    bool bExportAll = findPropertyValue<bool>(aFilterData, "ExportAll", true);
    if (bExportAll)
    {
        osl_removeFile(fullpath.pData);
        osl_openFile( fullpath.pData, &aBackgroundConfig, osl_File_OpenFlag_Create | osl_File_OpenFlag_Write );

        sal_uInt64 bytesWritten;
        err = osl_writeFile(aBackgroundConfig, "slides=", strlen("slides="), &bytesWritten);
    }

    // TODO: check for errors
    (void) err;

    FlashExporter aFlashExporter(
        mxContext,
        mxSelectedShapes,
        mxSelectedDrawPage,
        findPropertyValue<sal_Int32>(aFilterData, "CompressMode", 75),
        findPropertyValue<bool>(aFilterData, "ExportOLEAsJPEG", false));

    const sal_Int32 nPageCount = xDrawPages->getCount();
    if ( mxStatusIndicator.is() )
        mxStatusIndicator->start( "Saving :", nPageCount);

    for(sal_Int32 nPage = 0; nPage < nPageCount; nPage++)
    {
        if ( mxStatusIndicator.is() )
            mxStatusIndicator->setValue( nPage );
        xDrawPages->getByIndex(nPage) >>= xDrawPage;

        // AS: If we're only exporting the current page, then skip the rest.
        if (!bExportAll && xDrawPage != rCurrentPage)
            continue;

        // AS: Export the background, the background objects, and then the slide contents.
        if (bExportAll || findPropertyValue<bool>(aFilterData, "ExportBackgrounds", true))
        {
            backgroundfilename = exportBackground(aFlashExporter, xDrawPage, swfdirpath, nPage, "b");
        }

        if (bExportAll || findPropertyValue<bool>(aFilterData, "ExportBackgroundObjects", true))
        {
            objectsfilename = exportBackground(aFlashExporter, xDrawPage, swfdirpath, nPage, "o");
        }

        if (bExportAll || findPropertyValue<bool>(aFilterData, "ExportSlideContents", true))
        {
            fullpath = swfdirpath + STR("/slide") + VAL(nPage+1) + STR("p.swf");

            Reference<XOutputStream> xOutputStreamWrap(*(new OslOutputStreamWrapper(fullpath)), UNO_QUERY);
            bool ret = aFlashExporter.exportSlides( xDrawPage, xOutputStreamWrap, sal::static_int_cast<sal_uInt16>( nPage ) );
            aFlashExporter.Flush();
            xOutputStreamWrap.clear();

            if (!ret)
                osl_removeFile(fullpath.pData);
        }

        // AS: Write out to the background config what backgrounds and objects this
        //  slide used.
        if (bExportAll)
        {
            OUString temp = backgroundfilename + STR("|") + objectsfilename;
            OString ASCIItemp(temp.getStr(), temp.getLength(), RTL_TEXTENCODING_ASCII_US);

            sal_uInt64 bytesWritten;
            osl_writeFile(aBackgroundConfig, ASCIItemp.getStr(), ASCIItemp.getLength(), &bytesWritten);

            if (nPage < nPageCount - 1)
                osl_writeFile(aBackgroundConfig, "|", 1, &bytesWritten);
        }
    }

    if (bExportAll)
        osl_closeFile(aBackgroundConfig);

    return true;
}

bool FlashExportFilter::ExportAsSingleFile(const Sequence< PropertyValue >& aDescriptor)
{
    Reference < XOutputStream > xOutputStream = findPropertyValue<Reference<XOutputStream> >(aDescriptor, "OutputStream", nullptr);
    Sequence< PropertyValue > aFilterData;

    if (!xOutputStream.is() )
    {
        OSL_ASSERT ( false );
        return false;
    }

    FlashExporter aFlashExporter(
        mxContext,
        mxSelectedShapes,
        mxSelectedDrawPage,
        findPropertyValue<sal_Int32>(aFilterData, "CompressMode", 75),
        findPropertyValue<bool>(aFilterData, "ExportOLEAsJPEG", false));

    return aFlashExporter.exportAll( mxDoc, xOutputStream, mxStatusIndicator );
}


void SAL_CALL FlashExportFilter::cancel(  )
{
}


// XExporter
void SAL_CALL FlashExportFilter::setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
{
    mxDoc = xDoc;
}


// XInitialization
void SAL_CALL FlashExportFilter::initialize( const css::uno::Sequence< css::uno::Any >& /* aArguments */ )
{
}

OUString FlashExportFilter_getImplementationName ()
{
    return OUString ( "com.sun.star.comp.Impress.FlashExportFilter" );
}

Sequence< OUString > SAL_CALL FlashExportFilter_getSupportedServiceNames(  )
{
    Sequence<OUString> aRet { "com.sun.star.document.ExportFilter" };
    return aRet;
}

Reference< XInterface > SAL_CALL FlashExportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new FlashExportFilter( comphelper::getComponentContext(rSMgr) ));
}

// XServiceInfo
OUString SAL_CALL FlashExportFilter::getImplementationName(  )
{
    return FlashExportFilter_getImplementationName();
}

sal_Bool SAL_CALL FlashExportFilter::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SAL_CALL FlashExportFilter::getSupportedServiceNames(  )
{
    return FlashExportFilter_getSupportedServiceNames();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
