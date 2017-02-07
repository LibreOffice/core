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
#include "oox/core/fasttokenhandler.hxx"
#include <iostream>

#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>

namespace writerfilter {
namespace ooxml
{

using namespace com::sun::star;

OOXMLStreamImpl::OOXMLStreamImpl
(uno::Reference<uno::XComponentContext> const & xContext,
 uno::Reference<io::XInputStream> const & xStorageStream,
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
}

const OUString & OOXMLStreamImpl::getTarget() const
{
    return msTarget;
}

bool OOXMLStreamImpl::lcl_getTarget(const uno::Reference<embed::XRelationshipAccess>&
                                    xRelationshipAccess,
                                    StreamType_t nStreamType,
                                    const OUString & rId,
                                    OUString & rDocumentTarget)
{
    static const char sId[] = "Id";
    static const char sTarget[] = "Target";
    static const char sTargetMode[] = "TargetMode";
    static const char sExternal[] = "External";
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
    static uno::Reference<uri::XUriReferenceFactory> xFac = uri::UriReferenceFactory::create(mxContext);
    // use '/' to representent the root of the zip package ( and provide a 'file' scheme to
    // keep the XUriReference implementation happy )
    // add mspath to represent the 'source' of this stream
    uno::Reference<uri::XUriReference> xBase = xFac->parse("file:///" + msPath);

    static const char sType[] = "Type";
    static const char sDocumentType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";
    static const char sStylesType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles";
    static const char sNumberingType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering";
    static const char sFonttableType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable";
    static const char sFootnotesType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes";
    static const char sEndnotesType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes";
    static const char sCommentsType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments";
    static const char sThemeType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme";
    static const char sCustomType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml";
    static const char sCustomPropsType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXmlProps";
    static const char sActiveXType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/control";
    static const char sActiveXBinType[] = "http://schemas.microsoft.com/office/2006/relationships/activeXControlBinary";
    static const char sGlossaryType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/glossaryDocument";
    static const char sWebSettings[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings";
    static const char sSettingsType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings";
    static const char sChartType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart";
    static const char sEmbeddingsType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/package";
    static const char sFooterType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer";
    static const char sHeaderType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/header";
    static const char sOleObjectType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject";
    // OOXML strict
    static const char sDocumentTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument";
    static const char sStylesTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/styles";
    static const char sNumberingTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/numbering";
    static const char sFonttableTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/fontTable";
    static const char sFootnotesTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/footnotes";
    static const char sEndnotesTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/endnotes";
    static const char sCommentsTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/comments";
    static const char sThemeTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/theme";
    static const char sCustomTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/customXml";
    static const char sCustomPropsTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/customXmlProps";
    static const char sActiveXTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/control";
    static const char sGlossaryTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/glossaryDocument";
    static const char sWebSettingsStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/webSettings";
    static const char sSettingsTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/settings";
    static const char sChartTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/chart";
    static const char sEmbeddingsTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/package";
    static const char sFootersTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/footer";
    static const char sHeaderTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/header";
    static const char sOleObjectTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/oleObject";
    static const char sVBAProjectType[] = "http://schemas.microsoft.com/office/2006/relationships/vbaProject";

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

                if (rPair.First == sType &&
                    ( rPair.Second == sStreamType ||
                      rPair.Second == sStreamTypeStrict ))
                    bFound = true;
                else if(rPair.First == sType &&
                        ((rPair.Second == sOleObjectType ||
                          rPair.Second == sOleObjectTypeStrict) &&
                          nStreamType == EMBEDDINGS))
                {
                    bFound = true;
                }
                else if (rPair.First == sId &&
                         rPair.Second == rId)
                    bFound = true;
                else if (rPair.First == sTarget)
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
                else if (rPair.First == sTargetMode &&
                         rPair.Second == sExternal)
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
                    uno::Reference<uri::XUriReference> xPart = xFac->parse(sMyTarget);
                    uno::Reference<uri::XUriReference> xAbs = xFac->makeAbsolute(xBase, xPart, true, uri::RelativeUriExcessParentSegments_RETAIN);
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

uno::Reference<uno::XComponentContext> OOXMLStreamImpl::getContext()
{
    return mxContext;
}

uno::Reference <xml::sax::XFastTokenHandler> OOXMLStreamImpl::getFastTokenHandler()
{
    if (! mxFastTokenHandler.is())
        mxFastTokenHandler.set(new oox::core::FastTokenHandler());

    return mxFastTokenHandler;
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(const uno::Reference<uno::XComponentContext>& xContext,
 const uno::Reference<io::XInputStream>& rStream,
 bool bRepairStorage)
{
    OOXMLStreamImpl * pStream = new OOXMLStreamImpl(xContext, rStream,
                                                    OOXMLStream::DOCUMENT, bRepairStorage);
    return OOXMLStream::Pointer_t(pStream);
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(const OOXMLStream::Pointer_t& pStream,  OOXMLStream::StreamType_t nStreamType)
{
    OOXMLStream::Pointer_t pRet;
    if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
        pRet.reset(new OOXMLStreamImpl(*pImpl, nStreamType));
    return pRet;
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(const OOXMLStream::Pointer_t& pStream, const OUString & rId)
{
    OOXMLStream::Pointer_t pRet;
    if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
        pRet.reset(new OOXMLStreamImpl(*pImpl, rId));
    return pRet;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
