/*************************************************************************
 *
 *  $RCSfile: localfilelayer.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:29:20 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_
#include "localfilelayer.hxx"
#endif // CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_

#ifndef CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_
#include "localoutputstream.hxx"
#endif // CONFIGMGR_LOCALBE_LOCALOUTPUTSTREAM_HXX_

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#include "oslstream.hxx"
#endif // _CONFIGMGR_OSLSTREAM_HXX_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

BasicLocalFileLayer::BasicLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aComponentFile)
: mFactory(xFactory)
, mFileUrl(aComponentFile)
{
    static const rtl::OUString kXMLLayerParser(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.xml.LayerParser")) ;

    mLayerReader = uno::Reference<backend::XLayer>::query(
                                    mFactory->createInstance(kXMLLayerParser)) ;

}
//------------------------------------------------------------------------------

SimpleLocalFileLayer::SimpleLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aComponentFile)
: BasicLocalFileLayer(xFactory,aComponentFile)
{
}
//------------------------------------------------------------------------------

SimpleLocalFileLayer::SimpleLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent)
: BasicLocalFileLayer(xFactory,aBaseDir + aComponent)
{
}
//------------------------------------------------------------------------------

FlatLocalFileLayer::FlatLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent)
: BasicLocalFileLayer(xFactory,aBaseDir + aComponent)
, mLayerWriter( createLayerWriter() )
{
}
//------------------------------------------------------------------------------

CompositeLocalFileLayer::CompositeLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const SubLayerFiles& aSublayerDirectories)
: BasicLocalFileLayer(xFactory,aBaseDir + aComponent)
, mLayerWriter( createLayerWriter() )
{
    fillSubLayerLists(aSublayerDirectories, aComponent) ;
}
//------------------------------------------------------------------------------

BasicLocalFileLayer::~BasicLocalFileLayer() {}
//------------------------------------------------------------------------------

SimpleLocalFileLayer::~SimpleLocalFileLayer() {}
//------------------------------------------------------------------------------

FlatLocalFileLayer::~FlatLocalFileLayer() {}
//------------------------------------------------------------------------------

CompositeLocalFileLayer::~CompositeLocalFileLayer() {}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayerHandler> BasicLocalFileLayer::createLayerWriter()
{
    static const rtl::OUString kXMLLayerWriter(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.xml.LayerWriter")) ;

    uno::Reference< uno::XInterface > xWriter = mFactory->createInstance(kXMLLayerWriter);

    return uno::Reference<backend::XLayerHandler>(xWriter,uno::UNO_REF_QUERY_THROW) ;
}
//------------------------------------------------------------------------------

static inline void readEmptyLayer(const uno::Reference<backend::XLayerHandler>& xHandler)
{
    OSL_ASSERT(xHandler.is());
    xHandler->startLayer();
    xHandler->endLayer();
}
//------------------------------------------------------------------------------

void BasicLocalFileLayer::readData(
        backend::XLayer * pContext,
        const uno::Reference<backend::XLayerHandler>& xHandler,
        const rtl::OUString& aFileUrl)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    if (!xHandler.is())
    {
        rtl::OUString const sMessage(RTL_CONSTASCII_USTRINGPARAM(
            "LocalFileLayer - Cannot readData: Handler is NULL."));

        throw  lang::NullPointerException(sMessage,pContext);
    }

    osl::File blobFile(aFileUrl) ;
    osl::File::RC errorCode = blobFile.open(OpenFlag_Read) ;

    switch (errorCode)
    {
    case osl::File::E_None: // got it
        {
            uno::Reference<io::XActiveDataSink> xAS(mLayerReader, uno::UNO_QUERY_THROW);

            uno::Reference<io::XInputStream> xStream( new OSLInputStreamWrapper(blobFile) );

            xAS->setInputStream(xStream);

            mLayerReader->readData(xHandler) ;
        }
        break;

    case osl::File::E_NOENT: // no layer => empty layer
        readEmptyLayer(xHandler);
        break;

    default:
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("LocalFile Layer: Cannot open input file \"");
            sMsg.append(aFileUrl);
            sMsg.appendAscii("\" : ");
            sMsg.append(FileHelper::createOSLErrorString(errorCode));

            io::IOException ioe(sMsg.makeStringAndClear(),pContext);

            sMsg.appendAscii("LocalFileLayer - Cannot readData: ").append(ioe.Message);
            throw backend::BackendAccessException(sMsg.makeStringAndClear(),pContext,uno::makeAny(ioe));
        }
        break;

    }
}
//------------------------------------------------------------------------------

void SAL_CALL SimpleLocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    BasicLocalFileLayer::readData(this,xHandler, getFileUrl()) ;
}
//------------------------------------------------------------------------------

void SAL_CALL FlatLocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    BasicLocalFileLayer::readData(this,xHandler, getFileUrl() ) ;
}
//------------------------------------------------------------------------------

void SAL_CALL CompositeLocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    BasicLocalFileLayer::readData(static_cast<backend::XCompositeLayer*>(this),xHandler, getFileUrl() ) ;
}
//------------------------------------------------------------------------------

void SAL_CALL CompositeLocalFileLayer::readSubLayerData(
        const uno::Reference<backend::XLayerHandler>& xHandler,
        const rtl::OUString& aSubLayerId)
    throw ( backend::MalformedDataException,
            lang::IllegalArgumentException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    sal_Int32 i ;

    for (i = 0 ; i < mSubLayers.getLength() ; ++ i) {
        if (mSubLayers [i].equals(aSubLayerId)) { break ; }
    }
    if (i == mSubLayers.getLength())
    {
        rtl::OUStringBuffer message ;

        message.appendAscii("Sublayer Id '").append(aSubLayerId) ;
        message.appendAscii("' is unknown") ;
        throw lang::IllegalArgumentException(message.makeStringAndClear(),
                                             *this, 2) ;
    }
    if (mSubLayerFiles[i].getLength() != 0)
        BasicLocalFileLayer::readData(static_cast<backend::XCompositeLayer*>(this),xHandler, mSubLayerFiles [i]) ;
    else
        readEmptyLayer(xHandler);
}
//------------------------------------------------------------------------------

void SAL_CALL FlatLocalFileLayer::replaceWith(
        const uno::Reference<backend::XLayer>& aNewLayer)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    if (!aNewLayer.is())
    {
        rtl::OUString const sMessage(RTL_CONSTASCII_USTRINGPARAM(
            "LocalFileLayer - Cannot replaceWith: Replacement layer is NULL."));

        throw  lang::NullPointerException(sMessage,*this);
    }
    OSL_ENSURE( !uno::Reference<backend::XCompositeLayer>::query(aNewLayer).is(),
                "Warning: correct updates with composite layers are not implemented");

    uno::Reference<io::XActiveDataSource> xAS(mLayerWriter, uno::UNO_QUERY_THROW);

    LocalOutputStream * pStream = new LocalOutputStream(getFileUrl());
    uno::Reference<io::XOutputStream> xStream( pStream );

    xAS->setOutputStream(xStream);

    aNewLayer->readData(mLayerWriter) ;

    pStream->finishOutput();

    // clear the output stream
    xStream.clear();
    xAS->setOutputStream(xStream);
}
//------------------------------------------------------------------------------

void SAL_CALL CompositeLocalFileLayer::replaceWith(
        const uno::Reference<backend::XLayer>& aNewLayer)
    throw (backend::MalformedDataException, lang::NullPointerException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if (!aNewLayer.is())
    {
        rtl::OUString const sMessage(RTL_CONSTASCII_USTRINGPARAM(
            "LocalFileLayer - Cannot replaceWith: Replacement layer is NULL."));

        throw  lang::NullPointerException(sMessage,*this);
    }
    OSL_ENSURE( !uno::Reference<backend::XCompositeLayer>::query(aNewLayer).is(),
                "Warning: correct updates with composite layers are not implemented");

    uno::Reference<io::XActiveDataSource> xAS(mLayerWriter, uno::UNO_QUERY_THROW);

    LocalOutputStream * pStream = new LocalOutputStream(getFileUrl());
    uno::Reference<io::XOutputStream> xStream( pStream );

    xAS->setOutputStream(xStream);

    aNewLayer->readData(mLayerWriter) ;

    pStream->finishOutput();

    // clear the output stream
    xStream.clear();
    xAS->setOutputStream(xStream);
}
//------------------------------------------------------------------------------

rtl::OUString BasicLocalFileLayer::getTimestamp(const rtl::OUString& aFileUrl)
{
    TimeValue timevalue = FileHelper::getModifyTime(aFileUrl) ;
    oslDateTime fileStamp ;
    rtl::OUString retCode ;

    if (osl_getDateTimeFromTimeValue(&timevalue, &fileStamp)) {
        sal_Char asciiStamp [20] ;

        sprintf(asciiStamp, "%04d%02d%02d%02d%02d%02dZ",
                fileStamp.Year, fileStamp.Month, fileStamp.Day,
                fileStamp.Hours, fileStamp.Minutes, fileStamp.Seconds) ;
        retCode = rtl::OUString::createFromAscii(asciiStamp) ;
    }
    return retCode ;
}
//------------------------------------------------------------------------------

rtl::OUString FlatLocalFileLayer::getTimestamp()
    throw (uno::RuntimeException)
{
    rtl::OUString sStamp = BasicLocalFileLayer::getTimestamp(getFileUrl());

    return sStamp;
}
//------------------------------------------------------------------------------

rtl::OUString CompositeLocalFileLayer::getTimestamp()
    throw (uno::RuntimeException)
{
    rtl::OUString sStamp = BasicLocalFileLayer::getTimestamp(getFileUrl());
#if 0 // thus far composite layers are only manipulated via the main layer
    for (SubLayerFiles::const_iterator it = mSubLayerFiles.begin();
         it != mSubLayerFiles.end();
         ++it)
    {
        rtl::OUString sSublayerTime = BasicLocalFileLayer::getTimestamp(*it);
        if (sStamp < sSublayerTime)
            sStamp = sSublayerTime;
    }
#endif
    return sStamp;
}
//------------------------------------------------------------------------------

void CompositeLocalFileLayer::fillSubLayerLists( const SubLayerFiles& aSublayerDirectories,
                                                const rtl::OUString& aComponent)
{
    SubLayerFiles::size_type const nSublayerCount = aSublayerDirectories.size();
    mSubLayers.realloc(nSublayerCount);
    mSubLayerFiles.resize(nSublayerCount);

    for (SubLayerFiles::size_type i = 0; i < nSublayerCount; ++i)
    {
        mSubLayers[i] = FileHelper::getFileName(aSublayerDirectories[i]);

        // Let's check whether the sublayer exists for the
        // particular component.
        rtl::OUString subLayerFile(aSublayerDirectories[i] + aComponent) ;
        if (FileHelper::fileExists(subLayerFile))
        {
            mSubLayerFiles[i] =  subLayerFile;
        }
        else
            OSL_ASSERT(mSubLayerFiles[i].getLength() == 0);
    }
}
//------------------------------------------------------------------------------

static bool findSubLayers(const rtl::OUString& aResDir,
                          CompositeLocalFileLayer::SubLayerFiles& aSublayerDirectories)
{
    if (aResDir.getLength() == 0) return false;

    // Extract the directory where the file is located
    osl::Directory directory(aResDir) ;
    if (directory.open() != osl_File_E_None) return false;

    osl::DirectoryItem item ;
    osl::FileStatus status(osl_FileStatus_Mask_Type |
                           osl_FileStatus_Mask_FileURL) ;

    while (directory.getNextItem(item) == osl_File_E_None)
    {
        if (item.getFileStatus(status) == osl_File_E_None)
        {
            if (status.getFileType() == osl::FileStatus::Directory)
            {
                aSublayerDirectories.push_back(status.getFileURL()) ;
            }
        }
    }
    return !aSublayerDirectories.empty();
}
//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> createLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const rtl::OUString& aResDir)
{
    uno::Reference<backend::XUpdatableLayer> xResult;

    CompositeLocalFileLayer::SubLayerFiles aSublayers;
    if (findSubLayers(aResDir,aSublayers))
    {
        xResult.set( new CompositeLocalFileLayer(xFactory,aBaseDir,aComponent,aSublayers) );
    }
    else
    {
        xResult.set( new FlatLocalFileLayer(xFactory,aBaseDir,aComponent) );
    }
    return xResult;
}
//------------------------------------------------------------------------------

} } // configmgr.localbe

