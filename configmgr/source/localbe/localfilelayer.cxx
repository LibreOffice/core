/*************************************************************************
 *
 *  $RCSfile: localfilelayer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jb $ $Date: 2002-07-11 17:17:41 $
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

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalFileLayer::LocalFileLayer(
        const uno::Reference<lang::XMultiServiceFactory>& xFactory,
        const rtl::OUString& aBaseDir,
        const rtl::OUString& aComponent,
        const rtl::OUString& aResDir)
: mFactory(xFactory), mFileUrl(aBaseDir + aComponent) {
    static const rtl::OUString kXMLLayerParser(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.xml.LayerParser")) ;
    static const rtl::OUString kXMLLayerWriter(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.xml.LayerWriter")) ;

    mLayerReader = uno::Reference<backend::XLayer>::query(
                                    mFactory->createInstance(kXMLLayerParser)) ;
    mLayerWriter = uno::Reference<backend::XLayerHandler>::query(
                                    mFactory->createInstance(kXMLLayerWriter)) ;
    if (aResDir.getLength() != 0) {
        fillSubLayerList(aResDir, aComponent) ;
    }
}
//------------------------------------------------------------------------------

LocalFileLayer::~LocalFileLayer(void) {}
//------------------------------------------------------------------------------

void SAL_CALL LocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    readData(xHandler, mFileUrl) ;
}
//------------------------------------------------------------------------------

void SAL_CALL LocalFileLayer::replaceWith(
        const uno::Reference<backend::XLayer>& aNewLayer)
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference<io::XActiveDataSource> xAS(mLayerWriter, uno::UNO_QUERY);
    if (!xAS.is())
    {
        rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("LocalFileLayer - Missing interface: XActiveDataSource not supported by LayerWriter"));

        throw uno::RuntimeException(sMsg,*this);
    }

    LocalOutputStream * pStream = new LocalOutputStream(mFileUrl);
    uno::Reference<io::XOutputStream> xStream( pStream );

    xAS->setOutputStream(xStream);

    aNewLayer->readData(mLayerWriter) ;

    pStream->finishOutput();

    // clear the output stream
    xStream.clear();
    xAS->setOutputStream(xStream);
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayerHandler> LocalFileLayer::createLayerWriter(void)
{
    OSL_ENSURE(false, "Creating a LayerWriter is not supported. This function should be removed");

    /* or else we need a wrapper for a kXMLLayerWriter,
        that properly calls LocalOutputStream::finishOutput at the end
    */

    rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("LocalFileLayer - Deprecated functionality: createLayerWriter is not supported any more"));

    throw uno::RuntimeException(sMsg,*this);

    return NULL;
}
//------------------------------------------------------------------------------

void SAL_CALL LocalFileLayer::readSubLayerData(
        const uno::Reference<backend::XLayerHandler>& xHandler,
        const rtl::OUString& aSubLayerId)
    throw (lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    sal_Int32 i ;

    for (i = 0 ; i < mSubLayers.getLength() ; ++ i) {
        if (mSubLayers [i].equals(aSubLayerId)) { break ; }
    }
    if (i == mSubLayers.getLength()) {
        rtl::OUStringBuffer message ;

        message.appendAscii("Sublayer Id '").append(aSubLayerId) ;
        message.appendAscii("' is unknown") ;
        throw lang::IllegalArgumentException(message.makeStringAndClear(),
                                             *this, 1) ;
    }
    readData(xHandler, mSubLayerFiles [i]) ;
}
//------------------------------------------------------------------------------

rtl::OUString LocalFileLayer::getTimestamp(const rtl::OUString& aFileUrl) {
    TimeValue timevalue = FileHelper::getModifyTime(aFileUrl) ;
    oslDateTime fileStamp ;
    rtl::OUString retCode ;

    if (osl_getDateTimeFromTimeValue(&timevalue, &fileStamp)) {
        sal_Char asciiStamp [16] ;

        sprintf(asciiStamp, "%04d%02d%02d%02d%02d%02dZ",
                fileStamp.Year, fileStamp.Month, fileStamp.Day,
                fileStamp.Hours, fileStamp.Minutes, fileStamp.Seconds) ;
        retCode = rtl::OUString::createFromAscii(asciiStamp) ;
    }
    return retCode ;
}
//------------------------------------------------------------------------------

void LocalFileLayer::fillSubLayerList(const rtl::OUString& aResDir,
                                      const rtl::OUString& aComponent) {
    // Extract the directory where the file is located
    osl::Directory directory(aResDir) ;

    if (directory.open() != osl_File_E_None) { return ; }
    osl::DirectoryItem item ;
    osl::FileStatus status(osl_FileStatus_Mask_Type |
                           osl_FileStatus_Mask_FileURL) ;
    std::vector<rtl::OUString> subLayerDirs ;

    while (directory.getNextItem(item) == osl_File_E_None) {
        if (item.getFileStatus(status) == osl_File_E_None) {
            if (status.getFileType() == osl::FileStatus::Directory) {
                // Let's check whether the sublayer exists for the
                // particular component.
                rtl::OUString subLayerFile(status.getFileURL() + aComponent) ;

                if (FileHelper::fileExists(subLayerFile)) {
                    mSubLayerFiles.push_back(subLayerFile) ;
                    subLayerDirs.push_back(
                            FileHelper::getFileName(status.getFileURL())) ;
                }
            }
        }
    }
    if (subLayerDirs.size() > 0) {
        mSubLayers.realloc(subLayerDirs.size()) ;
        std::vector<rtl::OUString>::const_iterator subLayer ;
        sal_Int32 i = 0 ;

        for (subLayer = subLayerDirs.begin() ;
                subLayer != subLayerDirs.end() ; ++ subLayer) {
            mSubLayers [i ++] = *subLayer ;
        }
    }
}
//------------------------------------------------------------------------------

void LocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler,
        const rtl::OUString& aFileUrl)
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    if (!xHandler.is())
    {
        // throw IllegalArgumentException ??
        return;
    }

    osl::File blobFile(aFileUrl) ;
    osl::File::RC errorCode = blobFile.open(OpenFlag_Read) ;

    switch (errorCode)
    {
    case osl::File::E_None: // got it
        {
            uno::Reference<io::XActiveDataSink> xAS(mLayerReader, uno::UNO_QUERY);
            if (!xAS.is())
            {
                rtl::OUString sMsg(RTL_CONSTASCII_USTRINGPARAM("LocalFileLayer - Missing interface: XActiveDataSink not supported by LayerReader"));

                throw uno::RuntimeException(sMsg,*this);
            }

            uno::Reference<io::XInputStream> xStream( new OSLInputStreamWrapper(blobFile) );

            xAS->setInputStream(xStream);

            mLayerReader->readData(xHandler) ;
        }
        break;

    case osl::File::E_NOENT: // no layer => empty layer
        xHandler->startLayer();
        xHandler->endLayer();
        break;

    default:
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("Cannot open output file \"");
            sMsg.append(aFileUrl);
            sMsg.appendAscii("\" : ");
            sMsg.append(FileHelper::createOSLErrorString(errorCode));

            io::IOException ioe(sMsg.makeStringAndClear(),*this);

            sMsg.appendAscii("LocalFileLayer - Cannot readData: ").append(ioe.Message);
            throw lang::WrappedTargetException(sMsg.makeStringAndClear(),*this,uno::makeAny(ioe));
        }
        break;

    }
}
//------------------------------------------------------------------------------

} } // configmgr.localbe

