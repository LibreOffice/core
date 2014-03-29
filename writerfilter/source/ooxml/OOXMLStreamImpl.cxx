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
#include <com/sun/star/xml/sax/Parser.hpp>

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
    mxStorage.set
        (comphelper::OStorageHelper::GetStorageOfFormatFromInputStream
         (OFOPXML_STORAGE_FORMAT_STRING, mxStorageStream, xContext, bRepairStorage));
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
    static OUString sId("Id");
    static OUString sTarget("Target");
    static OUString sTargetMode("TargetMode");
    static OUString sExternal("External");
    if (maIdCache.empty())
    {
        // Cache is empty? Then let's build it!
        uno::Sequence< uno::Sequence<beans::StringPair> >aSeqs = xRelationshipAccess->getAllRelationships();
        for (sal_Int32 i = 0; i < aSeqs.getLength(); ++i)
        {
            const uno::Sequence<beans::StringPair>& rSeq = aSeqs[i];
            OUString aId;
            OUString aTarget;
            bool bExternal = false;
            for (sal_Int32 j = 0; j < rSeq.getLength(); ++j)
            {
                const beans::StringPair& rPair = rSeq[j];
                if (rPair.First == sId)
                    aId = rPair.Second;
                else if (rPair.First == sTarget)
                    aTarget = rPair.Second;
                else if (rPair.First == sTargetMode && rPair.Second == sExternal)
                    bExternal = true;
            }
            // Only cache external targets, internal ones are more complex (see below)
            if (bExternal)
                maIdCache[aId] = aTarget;
        }
    }

    if (maIdCache.find(rId) != maIdCache.end())
    {
        rDocumentTarget = maIdCache[rId];
        return true;
    }

    bool bFound = false;
    static uno::Reference< com::sun::star::uri::XUriReferenceFactory > xFac =  ::com::sun::star::uri::UriReferenceFactory::create( mxContext );
    // use '/' to representent the root of the zip package ( and provide a 'file' scheme to
    // keep the XUriReference implementation happy )
    // add mspath to represent the 'source' of this stream
    uno::Reference< com::sun::star::uri::XUriReference > xBase = xFac->parse( OUString( "file:///"  ) + msPath );

    static OUString sType("Type");
    static OUString sDocumentType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument");
    static OUString sStylesType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles");
    static OUString sNumberingType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering");
    static OUString sFonttableType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable");
    static OUString sFootnotesType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes");
    static OUString sEndnotesType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes");
    static OUString sCommentsType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments");
    static OUString sThemeType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme");
    static OUString sCustomType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml");
    static OUString sCustomPropsType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXmlProps");
    static OUString sActiveXType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/control");
    static OUString sActiveXBinType("http://schemas.microsoft.com/office/2006/relationships/activeXControlBinary");
    static OUString sGlossaryType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/glossaryDocument");
    static OUString sWebSettings("http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings");
    static OUString sSettingsType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings");
    static OUString sChartType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart");
    static OUString sEmbeddingsType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/package");
    static OUString sFooterType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer");
    static OUString sHeaderType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/header");
    static OUString sOleObjectType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject");
    // OOXML strict
    static OUString sDocumentTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument");
    static OUString sStylesTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/styles");
    static OUString sNumberingTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/numbering");
    static OUString sFonttableTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/fontTable");
    static OUString sFootnotesTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/footnotes");
    static OUString sEndnotesTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/endnotes");
    static OUString sCommentsTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/comments");
    static OUString sThemeTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/theme");
    static OUString sCustomTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/customXml");
    static OUString sCustomPropsTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/customXmlProps");
    static OUString sActiveXTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/control");
    static OUString sGlossaryTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/glossaryDocument");
    static OUString sWebSettingsStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/webSettings");
    static OUString sSettingsTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/settings");
    static OUString sChartTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/chart");
    static OUString sEmbeddingsTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/package");
    static OUString sFootersTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/footer");
    static OUString sHeaderTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/header");
    static OUString sOleObjectTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/oleObject");
    static OUString sVBAProjectType("http://schemas.microsoft.com/office/2006/relationships/vbaProject");

    OUString sStreamType;
    OUString sStreamTypeStrict;

    switch (nStreamType)
    {
        case VBAPROJECT:
            sStreamType = sVBAProjectType;
            sStreamTypeStrict = sVBAProjectType;
            break;
        case DOCUMENT:
            sStreamType = sDocumentType;
            sStreamTypeStrict = sDocumentTypeStrict;
            break;
        case STYLES:
            sStreamType = sStylesType;
            sStreamTypeStrict = sStylesTypeStrict;
            break;
        case NUMBERING:
            sStreamType = sNumberingType;
            sStreamTypeStrict = sNumberingTypeStrict;
            break;
        case FONTTABLE:
            sStreamType = sFonttableType;
            sStreamTypeStrict = sFonttableTypeStrict;
            break;
        case FOOTNOTES:
            sStreamType = sFootnotesType;
            sStreamTypeStrict = sFootnotesTypeStrict;
            break;
        case ENDNOTES:
            sStreamType = sEndnotesType;
            sStreamTypeStrict = sEndnotesTypeStrict;
            break;
        case COMMENTS:
            sStreamType = sCommentsType;
            sStreamTypeStrict = sCommentsTypeStrict;
            break;
        case THEME:
            sStreamType = sThemeType;
            sStreamTypeStrict = sThemeTypeStrict;
            break;
        case CUSTOMXML:
            sStreamType = sCustomType;
            sStreamTypeStrict = sCustomTypeStrict;
            break;
        case CUSTOMXMLPROPS:
            sStreamType = sCustomPropsType;
            sStreamTypeStrict = sCustomPropsTypeStrict;
            break;
        case ACTIVEX:
            sStreamType = sActiveXType;
            sStreamTypeStrict = sActiveXTypeStrict;
            break;
        case ACTIVEXBIN:
            sStreamType = sActiveXBinType;
            sStreamTypeStrict = sActiveXBinType;
            break;
        case SETTINGS:
            sStreamType = sSettingsType;
            sStreamTypeStrict = sSettingsTypeStrict;
            break;
        case GLOSSARY:
            sStreamType = sGlossaryType;
            sStreamTypeStrict = sGlossaryTypeStrict;
            break;
        case WEBSETTINGS:
            sStreamType = sWebSettings;
            sStreamTypeStrict = sWebSettingsStrict;
          break;
        case CHARTS:
            sStreamType = sChartType;
            sStreamTypeStrict = sChartTypeStrict;
          break;
        case EMBEDDINGS:
            sStreamType = sEmbeddingsType;
            sStreamTypeStrict = sEmbeddingsTypeStrict;
          break;
        case FOOTER:
            sStreamType = sFooterType;
            sStreamTypeStrict = sFootersTypeStrict;
          break;
        case HEADER:
            sStreamType = sHeaderType;
            sStreamTypeStrict = sHeaderTypeStrict;
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
            const uno::Sequence< beans::StringPair > &rSeq = aSeqs[j];

            bool bExternalTarget = false;
            OUString sMyTarget;
            for (sal_Int32 i = 0; i < rSeq.getLength(); i++)
            {
                const beans::StringPair &rPair = rSeq[i];

                if (rPair.First.compareTo(sType) == 0 &&
                    ( rPair.Second.compareTo(sStreamType) == 0 ||
                      rPair.Second.compareTo(sStreamTypeStrict) == 0))
                    bFound = true;
                else if(rPair.First.compareTo(sType) == 0 &&
                        ((rPair.Second.compareTo(sOleObjectType) == 0 ||
                          rPair.Second.compareTo(sOleObjectTypeStrict) == 0) &&
                          nStreamType == EMBEDDINGS))
                {
                    bFound = true;
                }
                else if (rPair.First.compareTo(sId) == 0 &&
                         rPair.Second.compareTo(rId) == 0)
                    bFound = true;
                else if (rPair.First.compareTo(sTarget) == 0)
                {
                    // checking item[n].xml or activex[n].xml is not visited already.
                    if(customTarget != rPair.Second && (sStreamType == sCustomType || sStreamType == sActiveXType || sStreamType == sChartType || sStreamType == sFooterType || sStreamType == sHeaderType))
                    {
                        bFound = false;
                    }
                    else
                    {
                        sMyTarget = rPair.Second;
                    }
                }
                else if (rPair.First.compareTo(sTargetMode) == 0 &&
                         rPair.Second.compareTo(sExternal) == 0)
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
                    if(sStreamType == sEmbeddingsType)
                        embeddingsTarget = rDocumentTarget;
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
            // Non-cached ID lookup works by accessing mxDocumentStream as an embed::XRelationshipAccess.
            // So when it changes, we should empty the cache.
            maIdCache.clear();
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

// Giving access to mxDocumentStream. It is needed by resolving custom xml to get list of customxml's used in document.
uno::Reference<io::XStream> OOXMLStreamImpl::accessDocumentStream()
{
    return mxDocumentStream;
}

uno::Reference<io::XInputStream> OOXMLStreamImpl::getStorageStream()
{
    return mxStorageStream;
}

uno::Reference<xml::sax::XParser> OOXMLStreamImpl::getParser()
{
    uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(mxContext);
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
    OOXMLStream::Pointer_t pRet;
    if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
        pRet.reset(new OOXMLStreamImpl(*pImpl, nStreamType));
    return pRet;
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(OOXMLStream::Pointer_t pStream, const OUString & rId)
{
    OOXMLStream::Pointer_t pRet;
    if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
        pRet.reset(new OOXMLStreamImpl(*pImpl, rId));
    return pRet;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
