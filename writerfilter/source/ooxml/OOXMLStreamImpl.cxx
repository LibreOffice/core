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

#include "OOXMLStreamImpl.hxx"
#include "OOXMLFastTokenHandler.hxx"
#include "ooxmlLoggers.hxx"
#include <iostream>

#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

//#define DEBUG_STREAM

namespace writerfilter {
namespace ooxml
{

using namespace ::std;

OOXMLStreamImpl::OOXMLStreamImpl
(uno::Reference<uno::XComponentContext> xContext,
 uno::Reference<io::XInputStream> xStorageStream,
 StreamType_t nType, bool bRepairStorage)
: mxContext(xContext), mxStorageStream(xStorageStream), mnStreamType(nType)
{
    uno::Reference< lang::XMultiServiceFactory > xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);
    mxStorage.set
        (comphelper::OStorageHelper::GetStorageOfFormatFromInputStream
         (OFOPXML_STORAGE_FORMAT_STRING, mxStorageStream, xFactory, bRepairStorage));
    mxRelationshipAccess.set(mxStorage, uno::UNO_QUERY_THROW);

    init();
}

OOXMLStreamImpl::OOXMLStreamImpl
(OOXMLStreamImpl & rOOXMLStream, StreamType_t nStreamType)
: mxContext(rOOXMLStream.mxContext),
  mxStorageStream(rOOXMLStream.mxStorageStream),
  mxStorage(rOOXMLStream.mxStorage),
  mnStreamType(nStreamType),
  msPath(rOOXMLStream.msPath)
{
    mxRelationshipAccess.set(rOOXMLStream.mxDocumentStream, uno::UNO_QUERY_THROW);

    init();
}

OOXMLStreamImpl::OOXMLStreamImpl
(OOXMLStreamImpl & rOOXMLStream, const OUString & rId)
: mxContext(rOOXMLStream.mxContext),
  mxStorageStream(rOOXMLStream.mxStorageStream),
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
    debug_logger->endElement("stream");
#endif
}

const OUString & OOXMLStreamImpl::getTarget() const
{
    return msTarget;
}

bool OOXMLStreamImpl::lcl_getTarget(uno::Reference<embed::XRelationshipAccess>
                                    xRelationshipAccess,
                                    StreamType_t nStreamType,
                                    const OUString & rId,
                                    OUString & rDocumentTarget)
{
    bool bFound = false;
    static uno::Reference< com::sun::star::uri::XUriReferenceFactory > xFac =  ::com::sun::star::uri::UriReferenceFactory::create( mxContext );
    // use '/' to representent the root of the zip package ( and provide a 'file' scheme to
    // keep the XUriReference implementation happy )
    // add mspath to represent the 'source' of this stream
    uno::Reference< com::sun::star::uri::XUriReference > xBase = xFac->parse( OUString( "file:///"  ) + msPath );

    static OUString sType("Type");
    static OUString sId("Id");
    static OUString sDocumentType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
    static OUString sStylesType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles");
    static OUString sNumberingType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering");
    static OUString sFonttableType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable");
    static OUString sFootnotesType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes");
    static OUString sEndnotesType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes");
    static OUString sCommentsType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments");
    static OUString sThemeType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme");
    static OUString sSettingsType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings");
    static OUString sTarget("Target");
    static OUString sTargetMode("TargetMode");
    static OUString sExternal("External");
    static OUString sVBAProjectType("http://schemas.microsoft.com/office/2006/relationships/vbaProject");

    OUString sStreamType;

    switch (nStreamType)
    {
        case VBAPROJECT:
            sStreamType = sVBAProjectType;
            break;
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
        case SETTINGS:
            sStreamType = sSettingsType;
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
            OUString sMyTarget;
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
                    // 'Target' is a relative Uri, so a 'Target=/path'
                    // with a base Uri of file://base/foo will resolve to
                    // file://base/word. We need something more than some
                    // simple string concatination here to handle that.
                    uno::Reference< com::sun::star::uri::XUriReference > xPart = xFac->parse(  sMyTarget );
                    uno::Reference< com::sun::star::uri::XUriReference > xAbs = xFac->makeAbsolute(  xBase, xPart, sal_True,  com::sun::star::uri::RelativeUriExcessParentSegments_RETAIN );
                    rDocumentTarget = xAbs->getPath();
                    // path will start with the fragment separator. need to
                    // remove that
                    rDocumentTarget = rDocumentTarget.copy( 1 );
                }

                break;
            }
        }
    }

    return bFound;
}

OUString OOXMLStreamImpl::getTargetForId(const OUString & rId)
{
    OUString sTarget;

    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess
        (mxDocumentStream, uno::UNO_QUERY_THROW);

    if (lcl_getTarget(xRelationshipAccess, UNKNOWN, rId, sTarget))
        return sTarget;

    return OUString();
}

void OOXMLStreamImpl::init()
{
    bool bFound = lcl_getTarget(mxRelationshipAccess,
                                mnStreamType, msId, msTarget);
#ifdef DEBUG_STREAM
    debug_logger->startElement("stream");
    debug_logger->attribute("target", msTarget);
#endif

    if (bFound)
    {
        sal_Int32 nLastIndex = msTarget.lastIndexOf('/');
        if (nLastIndex >= 0)
            msPath = msTarget.copy(0, nLastIndex + 1);

        uno::Reference<embed::XHierarchicalStorageAccess>
            xHierarchicalStorageAccess(mxStorage, uno::UNO_QUERY);

        if (xHierarchicalStorageAccess.is())
        {
            uno::Any aAny(xHierarchicalStorageAccess->
                          openStreamElementByHierarchicalName
                          (msTarget, embed::ElementModes::SEEKABLEREAD));
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

uno::Reference<io::XInputStream> OOXMLStreamImpl::getStorageStream()
{
    return mxStorageStream;
}

uno::Reference<xml::sax::XParser> OOXMLStreamImpl::getParser()
{
    uno::Reference<lang::XMultiComponentFactory> xFactory =
        uno::Reference<lang::XMultiComponentFactory>
        (mxContext->getServiceManager());

    uno::Reference<xml::sax::XParser> xParser
        (xFactory->createInstanceWithContext
        ( OUString("com.sun.star.xml.sax.Parser"),
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
 bool bRepairStorage,
 OOXMLStream::StreamType_t nStreamType)
{
    OOXMLStreamImpl * pStream = new OOXMLStreamImpl(xContext, rStream,
                                                    nStreamType, bRepairStorage);
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
(OOXMLStream::Pointer_t pStream, const OUString & rId)
{
    return OOXMLStream::Pointer_t
        (new OOXMLStreamImpl(*dynamic_cast<OOXMLStreamImpl *>(pStream.get()),
                             rId));
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
