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

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <osl/file.hxx>

#include "swfexporter.hxx"

#include <string.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::task;

using ::rtl::OUString;
using ::rtl::OString;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::container::XIndexAccess;
using ::osl::FileBase;
using ::com::sun::star::frame::XModel;

namespace swf {

typedef ::cppu::WeakImplHelper1<com::sun::star::io::XOutputStream> OslOutputStreamWrapper_Base;
    // needed for some compilers
class OslOutputStreamWrapper : public OslOutputStreamWrapper_Base
{
    osl::File   mrFile;

public:
    OslOutputStreamWrapper(const OUString& sFileName) : mrFile(sFileName)
    {
        osl_removeFile(sFileName.pData);
        mrFile.open( osl_File_OpenFlag_Create|osl_File_OpenFlag_Write );
    }

    // stario::XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

void SAL_CALL OslOutputStreamWrapper::writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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
            throw com::sun::star::io::IOException();    // TODO: Better error handling
        default: break;
        }

        uBytesToWrite -= uBytesWritten;
        pBuffer += uBytesWritten;
    }
}

void SAL_CALL OslOutputStreamWrapper::flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL OslOutputStreamWrapper::closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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
        throw com::sun::star::io::IOException();    // TODO: Better error handling
    default: break;
    }
}

// -----------------------------------------------------------------------------

class FlashExportFilter : public cppu::WeakImplHelper4
<
    com::sun::star::document::XFilter,
    com::sun::star::document::XExporter,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
    Reference< XComponent > mxDoc;
    Reference< XMultiServiceFactory > mxMSF;
    Reference< XStatusIndicator> mxStatusIndicator;

public:
    FlashExportFilter( const Reference< XMultiServiceFactory > &rxMSF);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) throw(RuntimeException);

    sal_Bool ExportAsMultipleFiles( const Sequence< PropertyValue >& aDescriptor );
    sal_Bool ExportAsSingleFile( const Sequence< PropertyValue >& aDescriptor );

    virtual void SAL_CALL cancel( ) throw (RuntimeException);

    // XExporter
    virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw(RuntimeException);
};

// -----------------------------------------------------------------------------

FlashExportFilter::FlashExportFilter(const Reference< XMultiServiceFactory > &rxMSF)
:   mxMSF( rxMSF )
{
}


// -----------------------------------------------------------------------------

OUString exportBackground(FlashExporter &aFlashExporter, Reference< XDrawPage > xDrawPage, OUString sPath, sal_uInt32 nPage, const char* suffix)
{
    OUString filename = STR("slide") + VAL(nPage+1) + STR(suffix) + STR(".swf");
    OUString fullpath = sPath + STR("/") + filename;

    // AS: If suffix is "o" then the last paramter is true (for exporting objects).
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

sal_Bool SAL_CALL FlashExportFilter::filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    mxStatusIndicator = findPropertyValue<Reference<XStatusIndicator> >(aDescriptor, "StatusIndicator", mxStatusIndicator);

    Sequence< PropertyValue > aFilterData;
    aFilterData = findPropertyValue<Sequence< PropertyValue > >(aDescriptor, "FilterData", aFilterData);

    if (findPropertyValue<sal_Bool>(aFilterData, "ExportMultipleFiles", false ))
    {
        ExportAsMultipleFiles(aDescriptor);
    }
    else
    {
        ExportAsSingleFile(aDescriptor);
    }

    if( mxStatusIndicator.is() )
        mxStatusIndicator->end();

    return sal_True;
}


// AS: When exporting as multiple files, each background, object layer, and slide gets its own
//  file.  Additionally, a file called BackgroundConfig.txt is generated, indicating which
//  background and objects (if any) go with each slide.  The files are named slideNb.swf,
//  slideNo.swf, and slideNp.swf, where N is the slide number, and b=background, o=objects, and
//  p=slide contents.  Note that under normal circumstances, there will be very few b and o files.

// AS: HACK!  Right now, I create a directory as a sibling to the swf file selected in the Export
//  dialog.  This directory is called presentation.sxi-swf-files.  The name of the swf file selected
//  in the Export dialog has no impact on this.  All files created are placed in this directory.
sal_Bool FlashExportFilter::ExportAsMultipleFiles(const Sequence< PropertyValue >& aDescriptor)
{
    Reference< XDrawPagesSupplier > xDrawPagesSupplier(mxDoc, UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        return sal_False;

    Reference< XIndexAccess > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );
    if(!xDrawPages.is())
        return sal_False;

    Reference< XDesktop > rDesktop( mxMSF->createInstance(OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY);
    if (!rDesktop.is())
        return sal_False;

    Reference< XStorable > xStorable(rDesktop->getCurrentComponent(), UNO_QUERY);
    if (!xStorable.is())
        return sal_False;

    Reference< XDrawPage > xDrawPage;

    Reference< XFrame > rFrame = rDesktop->getCurrentFrame();
    Reference< XDrawView > rDrawView = Reference< XDrawView >( rFrame->getController(), UNO_QUERY );

    Reference< XDrawPage > rCurrentPage = rDrawView->getCurrentPage();

    Sequence< PropertyValue > aFilterData;

    aFilterData = findPropertyValue<Sequence< PropertyValue > >(aDescriptor, "FilterData", aFilterData);

    //AS: Do a bunch of path mangling to figure out where to put the files.

    OUString sOriginalPath = findPropertyValue<OUString>(aDescriptor, "URL", OUString());

    // AS: sPath is the parent directory, where everything else exists (like the sxi,
    //  the -swf-files folder, the -audio files, etc.
    int lastslash = sOriginalPath.lastIndexOf('/');
    OUString sPath( sOriginalPath.copy(0, lastslash) );

    OUString sPresentation(xStorable->getLocation());

    lastslash = sPresentation.lastIndexOf('/') + 1;
    int lastdot = sPresentation.lastIndexOf('.');

    // AS: The name of the presentation, without 3 character extension.
    OUString sPresentationName = sPresentation.copy(lastslash, lastdot - lastslash);

    OUString fullpath, swfdirpath, backgroundfilename, objectsfilename;

    swfdirpath = sPath + STR("/") + sPresentationName + STR(".sxi-swf-files");

    oslFileError err;
    err = osl_createDirectory( swfdirpath.pData );

    fullpath = swfdirpath + STR("/backgroundconfig.txt");

    oslFileHandle xBackgroundConfig( 0 );

    // AS: Only export the background config if we're exporting all of the pages, otherwise we'll
    //  screw it up.
    sal_Bool bExportAll = findPropertyValue<sal_Bool>(aFilterData, "ExportAll", true);
    if (bExportAll)
    {
        osl_removeFile(fullpath.pData);
        osl_openFile( fullpath.pData, &xBackgroundConfig, osl_File_OpenFlag_Create | osl_File_OpenFlag_Write );

        sal_uInt64 bytesWritten;
        err = osl_writeFile(xBackgroundConfig, "slides=", strlen("slides="), &bytesWritten);
    }

    // TODO: check for errors
    (void) err;

    FlashExporter aFlashExporter( mxMSF, findPropertyValue<sal_Int32>(aFilterData, "CompressMode", 75),
                                         findPropertyValue<sal_Bool>(aFilterData, "ExportOLEAsJPEG", false));

    const sal_Int32 nPageCount = xDrawPages->getCount();
    if ( mxStatusIndicator.is() )
        mxStatusIndicator->start(OUString( RTL_CONSTASCII_USTRINGPARAM( "Saving :" )), nPageCount);

    for(sal_Int32 nPage = 0; nPage < nPageCount; nPage++)
    {
        if ( mxStatusIndicator.is() )
            mxStatusIndicator->setValue( nPage );
        xDrawPages->getByIndex(nPage) >>= xDrawPage;

        // AS: If we're only exporting the current page, then skip the rest.
        if (!bExportAll && xDrawPage != rCurrentPage)
            continue;

        // AS: Export the background, the background objects, and then the slide contents.
        if (bExportAll || findPropertyValue<sal_Bool>(aFilterData, "ExportBackgrounds", true))
        {
            backgroundfilename = exportBackground(aFlashExporter, xDrawPage, swfdirpath, nPage, "b");
        }

        if (bExportAll || findPropertyValue<sal_Bool>(aFilterData, "ExportBackgroundObjects", true))
        {
            objectsfilename = exportBackground(aFlashExporter, xDrawPage, swfdirpath, nPage, "o");
        }

        if (bExportAll || findPropertyValue<sal_Bool>(aFilterData, "ExportSlideContents", true))
        {
            fullpath = swfdirpath + STR("/slide") + VAL(nPage+1) + STR("p.swf");

            Reference<XOutputStream> xOutputStreamWrap(*(new OslOutputStreamWrapper(fullpath)), UNO_QUERY);
            sal_Bool ret = aFlashExporter.exportSlides( xDrawPage, xOutputStreamWrap, sal::static_int_cast<sal_uInt16>( nPage ) );
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
            osl_writeFile(xBackgroundConfig, ASCIItemp.getStr(), ASCIItemp.getLength(), &bytesWritten);

            if (nPage < nPageCount - 1)
                osl_writeFile(xBackgroundConfig, "|", 1, &bytesWritten);
        }

#ifdef AUGUSTUS
        if (findPropertyValue<sal_Bool>(aFilterData, "ExportSound", true))
        {
            fullpath = swfdirpath + STR("/slide") + VAL(nPage+1) + STR("s.swf");

            OUString wavpath = sPath + STR("/") + sPresentationName + STR(".ppt-audio/slide") + VAL(nPage+1) + STR(".wav");
            FileBase::getSystemPathFromFileURL(wavpath, wavpath);
            OString sASCIIPath(wavpath.getStr(), wavpath.getLength(), RTL_TEXTENCODING_ASCII_US);

            Reference<XOutputStream> xOutputStreamWrap(*(new OslOutputStreamWrapper(fullpath)), UNO_QUERY);
            sal_Bool ret = aFlashExporter.exportSound(xOutputStreamWrap, sASCIIPath.getStr());
            aFlashExporter.Flush();
            xOutputStreamWrap.clear();

            if (!ret)
                osl_removeFile(fullpath.pData);
        }
#endif // defined AUGUSTUS
    }

    if (bExportAll)
        osl_closeFile(xBackgroundConfig);

    return sal_True;
}

sal_Bool FlashExportFilter::ExportAsSingleFile(const Sequence< PropertyValue >& aDescriptor)
{
    Reference < XOutputStream > xOutputStream = findPropertyValue<Reference<XOutputStream> >(aDescriptor, "OutputStream", 0);
    Sequence< PropertyValue > aFilterData;

    if (!xOutputStream.is() )
    {
        OSL_ASSERT ( 0 );
        return sal_False;
    }

    FlashExporter aFlashExporter( mxMSF, findPropertyValue<sal_Int32>(aFilterData, "CompressMode", 75),
                                         findPropertyValue<sal_Bool>(aFilterData, "ExportOLEAsJPEG", false));

    return aFlashExporter.exportAll( mxDoc, xOutputStream, mxStatusIndicator );
}

// -----------------------------------------------------------------------------

void SAL_CALL FlashExportFilter::cancel(  )
    throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

// XExporter
void SAL_CALL FlashExportFilter::setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDoc = xDoc;
}

// -----------------------------------------------------------------------------

// XInitialization
void SAL_CALL FlashExportFilter::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& /* aArguments */ )
    throw (Exception, RuntimeException)
{
}

// -----------------------------------------------------------------------------

OUString FlashExportFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Impress.FlashExportFilter" ) );
}

// -----------------------------------------------------------------------------

#define SERVICE_NAME "com.sun.star.document.ExportFilter"

sal_Bool SAL_CALL FlashExportFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName == SERVICE_NAME;
}

// -----------------------------------------------------------------------------

Sequence< OUString > SAL_CALL FlashExportFilter_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL FlashExportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new FlashExportFilter( rSMgr );
}

// -----------------------------------------------------------------------------

// XServiceInfo
OUString SAL_CALL FlashExportFilter::getImplementationName(  )
    throw (RuntimeException)
{
    return FlashExportFilter_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL FlashExportFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return FlashExportFilter_supportsService( rServiceName );
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL FlashExportFilter::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return FlashExportFilter_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
