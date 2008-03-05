/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLStreamImpl.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:06:34 $
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
#include "OOXMLFastTokenHandler.hxx"
#include <iostream>

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALSTORAGEACCESS_HPP_
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#endif

//#define DEBUG_STREAM

namespace writerfilter {
namespace ooxml
{

using namespace ::std;

OOXMLStreamImpl::OOXMLStreamImpl
(uno::Reference<uno::XComponentContext> xContext,
 uno::Reference<embed::XStorage> xStorage, StreamType_t nType)
: mxContext(xContext), mxStorage(xStorage), mnStreamType(nType)
{
    mxRelationshipAccess.set(mxStorage, uno::UNO_QUERY_THROW);

    init();
}

OOXMLStreamImpl::OOXMLStreamImpl
(OOXMLStreamImpl & rOOXMLStream, StreamType_t nStreamType)
: mxContext(rOOXMLStream.mxContext),
  mxStorage(rOOXMLStream.mxStorage),
  mnStreamType(nStreamType),
  msPath(rOOXMLStream.msPath)
{
    mxRelationshipAccess.set(rOOXMLStream.mxDocumentStream, uno::UNO_QUERY_THROW);

    init();
}

OOXMLStreamImpl::OOXMLStreamImpl
(uno::Reference<uno::XComponentContext> xContext,
 uno::Reference<embed::XStorage> xStorage, const rtl::OUString & rId)
: mxContext(xContext),
  mxStorage(xStorage),
  mnStreamType(UNKNOWN),
  msId(rId)
{
    mxRelationshipAccess.set(mxStorage, uno::UNO_QUERY_THROW);

    init();
}

OOXMLStreamImpl::OOXMLStreamImpl
(OOXMLStreamImpl & rOOXMLStream, const rtl::OUString & rId)
: mxContext(rOOXMLStream.mxContext),
  mxStorage(rOOXMLStream.mxStorage),
  mnStreamType(UNKNOWN),
  msId(rId),
  msPath(rOOXMLStream.msPath)
{
    mxRelationshipAccess.set(rOOXMLStream.mxDocumentStream, uno::UNO_QUERY_THROW);

    init();
}

OOXMLStreamImpl::~OOXMLStreamImpl()
{
#ifdef DEBUG_STREAM
    logger("DEBUG", "</stream>");
#endif
}

bool OOXMLStreamImpl::getTarget(uno::Reference<embed::XRelationshipAccess>
                                xRelationshipAccess,
                                StreamType_t nStreamType,
                                const ::rtl::OUString & rId,
                                ::rtl::OUString & rDocumentTarget)
{
    bool bFound = false;

    static rtl::OUString sType(RTL_CONSTASCII_USTRINGPARAM("Type"));
    static rtl::OUString sId(RTL_CONSTASCII_USTRINGPARAM("Id"));
    static rtl::OUString sDocumentType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"));
    static rtl::OUString sStylesType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"));
    static rtl::OUString sNumberingType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"));
    static rtl::OUString sFonttableType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable"));
    static rtl::OUString sFootnotesType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes"));
    static rtl::OUString sEndnotesType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes"));
    static rtl::OUString sCommentsType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments"));
    static rtl::OUString sThemeType(RTL_CONSTASCII_USTRINGPARAM("http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme"));
    static rtl::OUString sTarget(RTL_CONSTASCII_USTRINGPARAM("Target"));
    static rtl::OUString sTargetMode(RTL_CONSTASCII_USTRINGPARAM("TargetMode"));
    static rtl::OUString sExternal(RTL_CONSTASCII_USTRINGPARAM("External"));

    rtl::OUString sStreamType;

    switch (nStreamType)
    {
    case DOCUMENT:
        sStreamType = sDocumentType;
        break;
    case STYLES:
        sStreamType = sStylesType;
        break;
    case NUMBERING:
        sStreamType = sNumberingType;
        break;
    case FONTTABLE:
        sStreamType = sFonttableType;
        break;
    case FOOTNOTES:
        sStreamType = sFootnotesType;
        break;
    case ENDNOTES:
        sStreamType = sEndnotesType;
        break;
    case COMMENTS:
        sStreamType = sCommentsType;
        break;
    case THEME:
        sStreamType = sThemeType;
    break;
    default:
        break;
    }

    if (xRelationshipAccess.is())
    {
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
            xRelationshipAccess->getAllRelationships();

        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];

            bool bExternalTarget = false;
            ::rtl::OUString sMyTarget;
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];

                if (aPair.First.compareTo(sType) == 0 &&
                    aPair.Second.compareTo(sStreamType) == 0)
                    bFound = true;
                else if (aPair.First.compareTo(sId) == 0 &&
                         aPair.Second.compareTo(rId) == 0)
                    bFound = true;
                else if (aPair.First.compareTo(sTarget) == 0)
                    sMyTarget = aPair.Second;
                else if (aPair.First.compareTo(sTargetMode) == 0 &&
                         aPair.Second.compareTo(sExternal) == 0)
                    bExternalTarget = true;

            }

            if (bFound)
            {
                if (bExternalTarget)
                    rDocumentTarget = sMyTarget;
                else
                {
                    rDocumentTarget = msPath;
                    rDocumentTarget += sMyTarget;
                }

                break;
            }
        }
    }

    return bFound;
}

::rtl::OUString OOXMLStreamImpl::getTargetForId(const ::rtl::OUString & rId)
{
    ::rtl::OUString sTarget;

    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess
        (mxDocumentStream, uno::UNO_QUERY_THROW);

    if (getTarget(xRelationshipAccess, UNKNOWN, rId, sTarget))
        return sTarget;

    return ::rtl::OUString();
}

void OOXMLStreamImpl::init()
{
    ::rtl::OUString sDocumentTarget;

    bool bFound = getTarget(mxRelationshipAccess,
                            mnStreamType, msId, sDocumentTarget);
#ifdef DEBUG_STREAM
    logger("DEBUG", string("<stream>")
           + OUStringToOString(sDocumentTarget,
                               RTL_TEXTENCODING_ASCII_US).getStr());
#endif

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

uno::Reference<io::XInputStream> OOXMLStreamImpl::getDocumentStream()
{
    uno::Reference<io::XInputStream> xResult;

    if (mxDocumentStream.is())
        xResult = mxDocumentStream->getInputStream();

    return xResult;
}

uno::Reference<io::XInputStream> OOXMLStreamImpl::getInputStream()
{
    return mxInputStream;
}

void OOXMLStreamImpl::setInputStream
(uno::Reference<io::XInputStream> rxInputStream)
{
    mxInputStream = rxInputStream;
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

uno::Reference <xml::sax::XFastTokenHandler>
OOXMLStreamImpl::getFastTokenHandler
(uno::Reference<uno::XComponentContext> xContext)
{
    if (! mxFastTokenHandler.is())
        mxFastTokenHandler.set(new OOXMLFastTokenHandler(xContext));

    return mxFastTokenHandler;
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

    OOXMLStreamImpl * pStream = new OOXMLStreamImpl(xContext, xStorage,
                                                    nStreamType);
    pStream->setInputStream(rStream);

    return OOXMLStream::Pointer_t(pStream);
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(OOXMLStream::Pointer_t pStream,  OOXMLStream::StreamType_t nStreamType)
{
    return OOXMLStream::Pointer_t
        (new OOXMLStreamImpl(*dynamic_cast<OOXMLStreamImpl *>(pStream.get()),
                             nStreamType));
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(OOXMLStream::Pointer_t pStream, const rtl::OUString & rId)
{
    return OOXMLStream::Pointer_t
        (new OOXMLStreamImpl(*dynamic_cast<OOXMLStreamImpl *>(pStream.get()),
                             rId));
}

}}
