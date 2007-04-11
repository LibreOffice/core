/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLStreamImpl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-04-11 10:42:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "OOXMLStreamImpl.hxx"
#include <iostream>

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALSTORAGEACCESS_HPP_
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#endif

namespace ooxml
{

using namespace ::std;

OOXMLStreamImpl::OOXMLStreamImpl
(uno::Reference<uno::XComponentContext> xContext,
 uno::Reference<embed::XStorage> xStorage, StreamType_t nType)
: mxContext(xContext),  mxStorage(xStorage), mnStreamType(nType)
{
    mxRelationshipAccess = uno::Reference<embed::XRelationshipAccess>
        (mxStorage, uno::UNO_QUERY);

    init();
}

OOXMLStreamImpl::OOXMLStreamImpl
(OOXMLStreamImpl & rOOXMLStream, StreamType_t nStreamType)
: mxContext(rOOXMLStream.mxContext), mxStorage(rOOXMLStream.mxStorage),
  msPath(rOOXMLStream.msPath), mnStreamType(nStreamType)
{
    mxRelationshipAccess = uno::Reference<embed::XRelationshipAccess>
        (rOOXMLStream.mxDocumentStream, uno::UNO_QUERY);

    init();
}

OOXMLStreamImpl::~OOXMLStreamImpl()
{
}

void OOXMLStreamImpl::init()
{
    static rtl::OUString sType(RTL_CONSTASCII_USTRINGPARAM("Type"));
    static rtl::OUString sDocumentType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"));
    static rtl::OUString sStylesType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"));
    static rtl::OUString sNumberingType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"));
    static rtl::OUString sTarget(RTL_CONSTASCII_USTRINGPARAM("Target"));

    rtl::OUString sStreamType(sDocumentType);

    switch (mnStreamType)
    {
    case TYPES:
        sStreamType = sStylesType;
        break;
    case NUMBERING:
        sStreamType = sNumberingType;
        break;
    default:
        break;
    }

    if (mxRelationshipAccess.is())
    {
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
            mxRelationshipAccess->getAllRelationships();

        bool bFound = false;

        rtl::OUString sDocumentTarget;

        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];

            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];

                if (aPair.First.compareTo(sType) == 0 &&
                    aPair.Second.compareTo(sStreamType) == 0)
                    bFound = true;
                else if (aPair.First.compareTo(sTarget) == 0)
                {
                    sDocumentTarget = msPath;
                    sDocumentTarget += aPair.Second;
                }

                clog << "\""
                     << OUStringToOString(aPair.First,
                                          RTL_TEXTENCODING_ASCII_US).getStr()
                     << ","
                     << OUStringToOString(aPair.Second,
                                          RTL_TEXTENCODING_ASCII_US).getStr()
                     << "\"" << endl;
            }

            if (bFound)
                break;
        }

        clog << OUStringToOString(sDocumentTarget,
                                  RTL_TEXTENCODING_ASCII_US).getStr() << endl;

        if (bFound)
        {
            sal_Int32 nLastIndex = sDocumentTarget.lastIndexOf('/');
            if (nLastIndex >= 0)
                msPath = sDocumentTarget.copy(0, nLastIndex + 1);

            uno::Reference<embed::XHierarchicalStorageAccess>
                xHierarchicalStorageAccess(mxStorage, uno::UNO_QUERY);

            if (xHierarchicalStorageAccess.is())
            {
                uno::Any aAny(xHierarchicalStorageAccess->
                              openStreamElementByHierarchicalName
                              (sDocumentTarget, embed::ElementModes::READ));
                aAny >>= mxDocumentStream;
            }
        }
    }
}

uno::Reference<io::XInputStream> OOXMLStreamImpl::getInputStream()
{
    uno::Reference<io::XInputStream> xResult;

    if (mxDocumentStream.is())
        xResult = mxDocumentStream->getInputStream();

    return xResult;
}

uno::Reference<xml::sax::XParser> OOXMLStreamImpl::getParser()
{
    uno::Reference<lang::XMultiComponentFactory> xFactory =
        uno::Reference<lang::XMultiComponentFactory>
        (mxContext->getServiceManager());

    uno::Reference<xml::sax::XParser> xParser
        (xFactory->createInstanceWithContext
        ( rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Parser" ),
          mxContext ),
        uno::UNO_QUERY );

    return xParser;
}

uno::Reference<uno::XComponentContext> OOXMLStreamImpl::getContext()
{
    return mxContext;
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(uno::Reference<uno::XComponentContext> xContext,
 uno::Reference<io::XInputStream> rStream,
 OOXMLStream::StreamType_t nStreamType)
{
    uno::Reference<embed::XStorage> xStorage =
        comphelper::OStorageHelper::GetStorageOfFormatFromInputStream
        (OFOPXML_STORAGE_FORMAT_STRING, rStream);

    return OOXMLStream::Pointer_t(new OOXMLStreamImpl(xContext, xStorage,
                                                      nStreamType));
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(OOXMLStream::Pointer_t pStream,  OOXMLStream::StreamType_t nStreamType)
{
    return OOXMLStream::Pointer_t
        (new OOXMLStreamImpl(*dynamic_cast<OOXMLStreamImpl *>(pStream.get()),
                             nStreamType));
}

}
