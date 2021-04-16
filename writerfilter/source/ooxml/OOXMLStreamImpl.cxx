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
#include <oox/core/fasttokenhandler.hxx>

#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <comphelper/storagehelper.hxx>

namespace writerfilter::ooxml
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
(OOXMLStreamImpl const & rOOXMLStream, StreamType_t nStreamType)
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
(OOXMLStreamImpl const & rOOXMLStream, const OUString & rId)
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
        const uno::Sequence< uno::Sequence<beans::StringPair> >aSeqs = xRelationshipAccess->getAllRelationships();
        for (const uno::Sequence<beans::StringPair>& rSeq : aSeqs)
        {
            OUString aId;
            OUString aTarget;
            bool bExternal = false;
            for (const beans::StringPair& rPair : rSeq)
            {
                if (rPair.First == sId)
                    aId = rPair.Second;
                else if (rPair.First == sTarget)
                    aTarget = rPair.Second;
                else if (rPair.First == sTargetMode && rPair.Second == sExternal)
                    bExternal = true;
            }
            // Only cache external targets, internal ones are more complex (see below)
            if (bExternal || aTarget.startsWith("#"))
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
    static const OUStringLiteral sDocumentType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";
    static const OUStringLiteral sStylesType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles";
    static const OUStringLiteral sNumberingType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering";
    static const OUStringLiteral sFonttableType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable";
    static const OUStringLiteral sFootnotesType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes";
    static const OUStringLiteral sEndnotesType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/endnotes";
    static const OUStringLiteral sCommentsType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments";
    static const OUStringLiteral sThemeType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme";
    static const OUStringLiteral sCustomType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml";
    static const OUStringLiteral sCustomPropsType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXmlProps";
    static const OUStringLiteral sGlossaryType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/glossaryDocument";
    static const OUStringLiteral sWebSettings = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings";
    static const OUStringLiteral sSettingsType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings";
    static const OUStringLiteral sChartType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart";
    static const OUStringLiteral sEmbeddingsType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package";
    static const OUStringLiteral sFooterType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer";
    static const OUStringLiteral sHeaderType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/header";
    static const OUStringLiteral sOleObjectType = u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject";
    static const OUStringLiteral sCommentsExtendedType = u"http://schemas.microsoft.com/office/2011/relationships/commentsExtended";
    // OOXML strict
    static const OUStringLiteral sDocumentTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument";
    static const OUStringLiteral sStylesTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/styles";
    static const OUStringLiteral sNumberingTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/numbering";
    static const OUStringLiteral sFonttableTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/fontTable";
    static const OUStringLiteral sFootnotesTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/footnotes";
    static const OUStringLiteral sEndnotesTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/endnotes";
    static const OUStringLiteral sCommentsTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/comments";
    static const OUStringLiteral sThemeTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/theme";
    static const OUStringLiteral sCustomTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/customXml";
    static const OUStringLiteral sCustomPropsTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/customXmlProps";
    static const OUStringLiteral sGlossaryTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/glossaryDocument";
    static const OUStringLiteral sWebSettingsStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/webSettings";
    static const OUStringLiteral sSettingsTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/settings";
    static const OUStringLiteral sChartTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/chart";
    static const OUStringLiteral sEmbeddingsTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/package";
    static const OUStringLiteral sFootersTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/footer";
    static const OUStringLiteral sHeaderTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/header";
    static const OUStringLiteral sOleObjectTypeStrict = u"http://purl.oclc.org/ooxml/officeDocument/relationships/oleObject";
    static const OUStringLiteral sVBAProjectType = u"http://schemas.microsoft.com/office/2006/relationships/vbaProject";
    static const OUStringLiteral sVBADataType = u"http://schemas.microsoft.com/office/2006/relationships/wordVbaData";

    OUString sStreamType;
    OUString sStreamTypeStrict;

    switch (nStreamType)
    {
        case VBAPROJECT:
            sStreamType = sVBAProjectType;
            sStreamTypeStrict = sVBAProjectType;
            break;
        case VBADATA:
            sStreamType = sVBADataType;
            sStreamTypeStrict = sVBADataType;
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
        case COMMENTS_EXTENDED:
            sStreamType = sCommentsExtendedType;
            sStreamTypeStrict = sCommentsExtendedType;
            break;
        default:
            break;
    }

    if (xRelationshipAccess.is())
    {
        const uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
            xRelationshipAccess->getAllRelationships();

        for (const uno::Sequence< beans::StringPair > &rSeq : aSeqs)
        {
            bool bExternalTarget = false;
            OUString sMyTarget;
            for (const beans::StringPair &rPair : rSeq)
            {
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
                    // checking item[n].xml is not visited already.
                    if(customTarget != rPair.Second && (sStreamType == sCustomType || sStreamType == sChartType || sStreamType == sFooterType || sStreamType == sHeaderType))
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
                    // simple string concatenation here to handle that.
                    uno::Reference<uri::XUriReference> xPart = xFac->parse(sMyTarget);
                    uno::Reference<uri::XUriReference> xAbs = xFac->makeAbsolute(xBase, xPart, true, uri::RelativeUriExcessParentSegments_RETAIN);
                    if (!xAbs)
                    {
                        //it was invalid gibberish
                        bFound = false;
                    }
                    else
                    {
                        rDocumentTarget = xAbs->getPath();
                        // path will start with the fragment separator. need to
                        // remove that
                        rDocumentTarget = rDocumentTarget.copy( 1 );
                        if(sStreamType == sEmbeddingsType)
                            embeddingsTarget = rDocumentTarget;
                    }
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

    if (!bFound)
        return;

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

    if (nStreamType != OOXMLStream::VBADATA)
    {
        if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
            pRet = new OOXMLStreamImpl(*pImpl, nStreamType);
    }
    else
    {
        // VBADATA is not a relation of the document, but of the VBAPROJECT stream.
        if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
        {
            std::unique_ptr<OOXMLStreamImpl> pProject(new OOXMLStreamImpl(*pImpl, OOXMLStream::VBAPROJECT));
            pRet = new OOXMLStreamImpl(*pProject, OOXMLStream::VBADATA);
        }
    }

    return pRet;
}

OOXMLStream::Pointer_t
OOXMLDocumentFactory::createStream
(const OOXMLStream::Pointer_t& pStream, const OUString & rId)
{
    OOXMLStream::Pointer_t pRet;
    if (OOXMLStreamImpl* pImpl = dynamic_cast<OOXMLStreamImpl *>(pStream.get()))
        pRet = new OOXMLStreamImpl(*pImpl, rId);
    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
