/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localfilelayer.cxx,v $
 * $Revision: 1.15 $
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
#include "precompiled_configmgr.hxx"
#include "localfilelayer.hxx"
#include "localoutputstream.hxx"
#include "oslstream.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

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

BasicCompositeLocalFileLayer::BasicCompositeLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aComponentFile)
: BasicLocalFileLayer(xFactory,aComponentFile)
{
}
//------------------------------------------------------------------------------

CompositeLocalFileLayer::CompositeLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aComponent,
        const std::vector<rtl::OUString>& aSublayerDirectories)
: BasicCompositeLocalFileLayer(xFactory,rtl::OUString())
{
    fillSubLayerLists(aSublayerDirectories, aComponent) ;
}
//------------------------------------------------------------------------------

FullCompositeLocalFileLayer::FullCompositeLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const std::vector<rtl::OUString>& aSublayerDirectories)
: BasicCompositeLocalFileLayer(xFactory,aBaseDir + aComponent)
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

FullCompositeLocalFileLayer::~FullCompositeLocalFileLayer() {}
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
    if (!xHandler.is())
    {
        rtl::OUString const sMessage(RTL_CONSTASCII_USTRINGPARAM(
            "LocalFileLayer - Cannot readData: Handler is NULL."));

        throw  lang::NullPointerException(sMessage,*this);
    }

    readEmptyLayer(xHandler) ;
}
//------------------------------------------------------------------------------

void SAL_CALL FullCompositeLocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    BasicLocalFileLayer::readData(static_cast<backend::XCompositeLayer*>(this),xHandler, getFileUrl() ) ;
}
//------------------------------------------------------------------------------

void SAL_CALL BasicCompositeLocalFileLayer::readSubLayerData(
        backend::XCompositeLayer * pContext,
        const uno::Reference<backend::XLayerHandler>& xHandler,
        const rtl::OUString& aSubLayerId)
    throw ( backend::MalformedDataException,
            lang::IllegalArgumentException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    if (!xHandler.is())
    {
        rtl::OUString const sMessage(RTL_CONSTASCII_USTRINGPARAM(
            "CompositeLocalFileLayer - Cannot readSubLayerData: Handler is NULL."));

        throw  lang::NullPointerException(sMessage,pContext);
    }

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
                                             pContext, 2) ;
    }
    if (mSubLayerFiles[i].getLength() != 0)
        BasicLocalFileLayer::readData(pContext,xHandler, mSubLayerFiles [i]) ;
    else
        readEmptyLayer(xHandler);
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
    return BasicCompositeLocalFileLayer::readSubLayerData(this,xHandler,aSubLayerId);
}
//------------------------------------------------------------------------------

void SAL_CALL FullCompositeLocalFileLayer::readSubLayerData(
        const uno::Reference<backend::XLayerHandler>& xHandler,
        const rtl::OUString& aSubLayerId)
    throw ( backend::MalformedDataException,
            lang::IllegalArgumentException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    return BasicCompositeLocalFileLayer::readSubLayerData(this,xHandler,aSubLayerId);
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

void SAL_CALL FullCompositeLocalFileLayer::replaceWith(
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
    TimeValue timevalue  = {0,0};
    sal_uInt64 aSize = FileHelper::getModifyStatus(aFileUrl,timevalue) ;
    oslDateTime fileStamp ;
    rtl::OUString retCode ;

    if (osl_getDateTimeFromTimeValue(&timevalue, &fileStamp))
    {
        // truncate to 32 bits
        unsigned long aLongSize = static_cast<sal_Int32>(aSize);

        sal_Char asciiStamp [50] ;

        sprintf(asciiStamp, "%04u%02u%02u%02u%02u%02uZ%010lu",
                unsigned(fileStamp.Year), unsigned(fileStamp.Month), unsigned(fileStamp.Day),
                unsigned(fileStamp.Hours), unsigned(fileStamp.Minutes), unsigned(fileStamp.Seconds),
                aLongSize) ;
        retCode = rtl::OUString::createFromAscii(asciiStamp) ;
    }
    return retCode ;
}
//------------------------------------------------------------------------------

rtl::OUString SimpleLocalFileLayer::getTimestamp()
    throw (uno::RuntimeException)
{
    rtl::OUString sStamp = BasicLocalFileLayer::getTimestamp(getFileUrl());

    return sStamp;
}
//------------------------------------------------------------------------------

rtl::OUString FlatLocalFileLayer::getTimestamp()
    throw (uno::RuntimeException)
{
    rtl::OUString sStamp = BasicLocalFileLayer::getTimestamp(getFileUrl());

    return sStamp;
}
//------------------------------------------------------------------------------

rtl::OUString FullCompositeLocalFileLayer::getTimestamp()
    throw (uno::RuntimeException)
{
    rtl::OUString sStamp = BasicLocalFileLayer::getTimestamp(getFileUrl());
#if 0 // thus far composite layers are only manipulated via the main layer
    for (std::vector<rtl::OUString>::const_iterator it = mSubLayerFiles.begin();
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

void BasicCompositeLocalFileLayer::fillSubLayerLists( const std::vector<rtl::OUString>& aSublayerDirectories,
                                                const rtl::OUString& aComponent)
{
    std::vector<rtl::OUString>::size_type const nSublayerCount = aSublayerDirectories.size();
    mSubLayers.realloc(nSublayerCount);
    mSubLayerFiles.resize(nSublayerCount);

    for (std::vector<rtl::OUString>::size_type i = 0; i < nSublayerCount; ++i)
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
                          std::vector<rtl::OUString>& aSublayerDirectories)
{
    if (aResDir.getLength() == 0) return false;

    // Extract the directory where the file is located
    osl::Directory directory(aResDir) ;
    if (directory.open() != osl::Directory::E_None) return false;

    osl::DirectoryItem item ;
    osl::FileStatus status(osl_FileStatus_Mask_Type |
                           osl_FileStatus_Mask_FileURL) ;

    while (directory.getNextItem(item) == osl::Directory::E_None)
    {
        if (item.getFileStatus(status) == osl::Directory::E_None)
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

uno::Reference<backend::XLayer> createReadonlyLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const rtl::OUString& aResDir)
{
    uno::Reference<backend::XLayer> xResult;

    std::vector<rtl::OUString> aSublayers;
    if (aBaseDir.getLength() == 0)
    {
        findSubLayers(aResDir,aSublayers);
        xResult.set( new CompositeLocalFileLayer(xFactory,aComponent,aSublayers) );
    }
    else if (findSubLayers(aResDir,aSublayers))
    {
        // there is no readonly full composite layer - take the updatable one
        backend::XCompositeLayer * pNewLayer =
            new FullCompositeLocalFileLayer(xFactory,aBaseDir,aComponent,aSublayers);
        xResult.set( pNewLayer );
    }
    else
    {
        xResult.set( new SimpleLocalFileLayer(xFactory,aBaseDir,aComponent) );
    }
    return xResult;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> createUpdatableLocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const rtl::OUString& aResDir)
{
    uno::Reference<backend::XUpdatableLayer> xResult;

    std::vector<rtl::OUString> aSublayers;
    if (findSubLayers(aResDir,aSublayers))
    {
        xResult.set( new FullCompositeLocalFileLayer(xFactory,aBaseDir,aComponent,aSublayers) );
    }
    else if (aBaseDir.getLength() != 0)
    {
        xResult.set( new FlatLocalFileLayer(xFactory,aBaseDir,aComponent) );
    }
    else
        OSL_ENSURE(false,"WARNING: Trying to create an updatable ressource-only layer");

    return xResult;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

enum
{
    LAYER_PROPERTY_URL = 1
};

#define PROPNAME( name ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( name ) )
#define PROPTYPE( type ) getCppuType( static_cast< type const *>( 0 ) )

// cppu::OPropertySetHelper
cppu::IPropertyArrayHelper * SAL_CALL LayerPropertyHelper::newInfoHelper()
{
    com::sun::star::beans::Property properties[] =
    {
        com::sun::star::beans::Property(PROPNAME("URL"), LAYER_PROPERTY_URL, PROPTYPE(rtl::OUString), com::sun::star::beans::PropertyAttribute::READONLY)
    };

    return new cppu::OPropertyArrayHelper(properties, sizeof(properties)/sizeof(properties[0]));
}

#define MESSAGE( text ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Layer Properties: " text ) )

void SAL_CALL LayerPropertyHelper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& /*rValue*/ )
            throw (uno::Exception)
{
    namespace beans = com::sun::star::beans;

    switch (nHandle)
    {
    case LAYER_PROPERTY_URL:
        OSL_ENSURE(false, "Error: trying to set a READONLY property");
        throw beans::PropertyVetoException(MESSAGE("Property 'URL' is read-only"),*this);

    default:
        OSL_ENSURE(false, "Error: trying to set an UNKNOWN property");
        throw beans::UnknownPropertyException(MESSAGE("Trying to set an unknown property"),*this);
    }
}

void SAL_CALL LayerPropertyHelper::getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
    case LAYER_PROPERTY_URL:
        rValue = uno::makeAny( this->getLayerUrl() );
        break;

    default:
        OSL_ENSURE(false, "Error: trying to get an UNKNOWN property");
        break;
    }
}

//------------------------------------------------------------------------------
} } // configmgr.localbe

