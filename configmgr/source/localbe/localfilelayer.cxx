/*************************************************************************
 *
 *  $RCSfile: localfilelayer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-05-27 17:08:23 $
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

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_

namespace configmgr {

//==============================================================================

//------------------------------------------------------------------------------

LocalFileLayer::LocalFileLayer(
        const uno::Reference<uno::XComponentContext>& xContext,
        const rtl::OUString& aFileUrl)
: mContext(xContext), mFileUrl(aFileUrl) {
    static const rtl::OUString kXMLLayerParser(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.xml.LayerParser")) ;
    static const rtl::OUString kXMLLayerWriter(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.xml.LayerWriter")) ;

    mLayerReader = uno::Reference<backend::XLayer>::query(
            mContext->getServiceManager()->createInstanceWithContext(
                kXMLLayerParser, mContext)) ;
    mLayerWriter = uno::Reference<backend::XLayerHandler>::query(
            mContext->getServiceManager()->createInstanceWithContext(
                kXMLLayerWriter, mContext)) ;
}
//------------------------------------------------------------------------------

LocalFileLayer::~LocalFileLayer(void) {}
//------------------------------------------------------------------------------

void SAL_CALL LocalFileLayer::readData(
        const uno::Reference<backend::XLayerHandler>& xHandler) {
    osl::File blobFile(mFileUrl) ;
    osl::FileBase::RC errorCode = blobFile.open(OpenFlag_Read) ;

    if (errorCode != osl_File_E_None) { return ; }
    uno::Reference<io::XInputStream> stream =
                                        new OSLInputStreamWrapper(blobFile) ;
    uno::Sequence<uno::Any> arguments(1) ;

    arguments [0] <<= stream ;
    uno::Reference<lang::XInitialization>::query(mLayerReader)->initialize(
                                                                    arguments) ;
    mLayerReader->readData(xHandler) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayerHandler>
SAL_CALL LocalFileLayer::getWriteHandler(void) {
    uno::Reference<io::XOutputStream> stream = new LocalOutputStream(mFileUrl) ;
    uno::Sequence<uno::Any> arguments(1) ;

    arguments [0] <<= stream ;
    uno::Reference<lang::XInitialization>::query(mLayerWriter)->initialize(
                                                                    arguments) ;
    return mLayerWriter ;
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

} // configmgr

