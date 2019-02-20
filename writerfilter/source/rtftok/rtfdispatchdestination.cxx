/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfdocumentimpl.hxx"

#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <filter/msfilter/escherex.hxx>
#include <rtl/character.hxx>
#include <tools/stream.hxx>
#include <sal/log.hxx>

#include <dmapper/DomainMapperFactory.hxx>
#include <ooxml/resourceids.hxx>

#include "rtflookahead.hxx"
#include "rtfreferenceproperties.hxx"
#include "rtfsdrimport.hxx"
#include "rtfskipdestination.hxx"
#include "rtftokenizer.hxx"

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{
RTFError RTFDocumentImpl::dispatchDestination(RTFKeyword nKeyword)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    RTFSkipDestination aSkip(*this);
    // special case \upr: ignore everything except nested \ud
    if (Destination::UPR == m_aStates.top().eDestination && RTF_UD != nKeyword)
    {
        m_aStates.top().eDestination = Destination::SKIP;
        aSkip.setParsed(false);
    }
    else
        switch (nKeyword)
        {
            case RTF_RTF:
                break;
            case RTF_FONTTBL:
                m_aStates.top().eDestination = Destination::FONTTABLE;
                break;
            case RTF_COLORTBL:
                m_aStates.top().eDestination = Destination::COLORTABLE;
                break;
            case RTF_STYLESHEET:
                m_aStates.top().eDestination = Destination::STYLESHEET;
                break;
            case RTF_FIELD:
                m_aStates.top().eDestination = Destination::FIELD;
                break;
            case RTF_FLDINST:
            {
                // Look for the field type
                sal_uInt64 const nPos = Strm().Tell();
                OStringBuffer aBuf;
                char ch = 0;
                bool bFoundCode = false;
                bool bInKeyword = false;
                while (!bFoundCode && ch != '}')
                {
                    Strm().ReadChar(ch);
                    if ('\\' == ch)
                        bInKeyword = true;
                    if (!bInKeyword && rtl::isAsciiAlphanumeric(static_cast<unsigned char>(ch)))
                        aBuf.append(ch);
                    else if (bInKeyword && rtl::isAsciiWhiteSpace(static_cast<unsigned char>(ch)))
                        bInKeyword = false;
                    if (!aBuf.isEmpty()
                        && !rtl::isAsciiAlphanumeric(static_cast<unsigned char>(ch)))
                        bFoundCode = true;
                }

                if (aBuf.toString() == "INCLUDEPICTURE")
                {
                    // Extract the field argument of INCLUDEPICTURE: we handle that
                    // at a tokenizer level, as DOCX has no such field.
                    aBuf.append(ch);
                    while (true)
                    {
                        Strm().ReadChar(ch);
                        if (ch == '}')
                            break;
                        aBuf.append(ch);
                    }
                    OUString aFieldCommand
                        = OStringToOUString(aBuf.toString(), RTL_TEXTENCODING_UTF8);
                    std::tuple<OUString, std::vector<OUString>, std::vector<OUString>> aResult
                        = writerfilter::dmapper::splitFieldCommand(aFieldCommand);
                    m_aPicturePath
                        = std::get<1>(aResult).empty() ? OUString() : std::get<1>(aResult).front();
                }

                Strm().Seek(nPos);

                // Form data should be handled only for form fields if any
                if (aBuf.toString().indexOf(OString("FORM")) != -1)
                    m_bFormField = true;

                singleChar(cFieldStart);
                m_aStates.top().eDestination = Destination::FIELDINSTRUCTION;
            }
            break;
            case RTF_FLDRSLT:
                m_aStates.top().eDestination = Destination::FIELDRESULT;
                break;
            case RTF_LISTTABLE:
                m_aStates.top().eDestination = Destination::LISTTABLE;
                break;
            case RTF_LISTPICTURE:
                m_aStates.top().eDestination = Destination::LISTPICTURE;
                m_aStates.top().bInListpicture = true;
                break;
            case RTF_LIST:
                m_aStates.top().eDestination = Destination::LISTENTRY;
                break;
            case RTF_LISTNAME:
                m_aStates.top().eDestination = Destination::LISTNAME;
                break;
            case RTF_LFOLEVEL:
                m_aStates.top().eDestination = Destination::LFOLEVEL;
                m_aStates.top().aTableSprms.clear();
                break;
            case RTF_LISTOVERRIDETABLE:
                m_aStates.top().eDestination = Destination::LISTOVERRIDETABLE;
                break;
            case RTF_LISTOVERRIDE:
                m_aStates.top().eDestination = Destination::LISTOVERRIDEENTRY;
                break;
            case RTF_LISTLEVEL:
                m_aStates.top().eDestination = Destination::LISTLEVEL;
                ++m_nListLevel;
                break;
            case RTF_LEVELTEXT:
                m_aStates.top().eDestination = Destination::LEVELTEXT;
                break;
            case RTF_LEVELNUMBERS:
                m_aStates.top().eDestination = Destination::LEVELNUMBERS;
                break;
            case RTF_SHPPICT:
                resetFrame();
                m_aStates.top().eDestination = Destination::SHPPICT;
                break;
            case RTF_PICT:
                if (m_aStates.top().eDestination != Destination::SHAPEPROPERTYVALUE)
                    m_aStates.top().eDestination = Destination::PICT; // as character
                else
                    m_aStates.top().eDestination
                        = Destination::SHAPEPROPERTYVALUEPICT; // anchored inside a shape
                break;
            case RTF_PICPROP:
                m_aStates.top().eDestination = Destination::PICPROP;
                break;
            case RTF_SP:
                m_aStates.top().eDestination = Destination::SHAPEPROPERTY;
                break;
            case RTF_SN:
                m_aStates.top().eDestination = Destination::SHAPEPROPERTYNAME;
                break;
            case RTF_SV:
                m_aStates.top().eDestination = Destination::SHAPEPROPERTYVALUE;
                break;
            case RTF_SHP:
                m_bNeedCrOrig = m_bNeedCr;
                m_aStates.top().eDestination = Destination::SHAPE;
                m_aStates.top().bInShape = true;
                break;
            case RTF_SHPINST:
                m_aStates.top().eDestination = Destination::SHAPEINSTRUCTION;
                break;
            case RTF_NESTTABLEPROPS:
                // do not set any properties of outer table at nested table!
                m_aStates.top().aTableCellSprms = m_aDefaultState.aTableCellSprms;
                m_aStates.top().aTableCellAttributes = m_aDefaultState.aTableCellAttributes;
                m_aNestedTableCellsSprms.clear();
                m_aNestedTableCellsAttributes.clear();
                m_nNestedCells = 0;
                m_aStates.top().eDestination = Destination::NESTEDTABLEPROPERTIES;
                break;
            case RTF_HEADER:
            case RTF_FOOTER:
            case RTF_HEADERL:
            case RTF_HEADERR:
            case RTF_HEADERF:
            case RTF_FOOTERL:
            case RTF_FOOTERR:
            case RTF_FOOTERF:
                if (!m_pSuperstream)
                {
                    Id nId = 0;
                    std::size_t nPos = m_nGroupStartPos - 1;
                    switch (nKeyword)
                    {
                        case RTF_HEADER:
                            if (!m_hasRHeader)
                            {
                                nId = NS_ooxml::LN_headerr;
                                m_hasRHeader = true;
                            }
                            break;
                        case RTF_FOOTER:
                            if (!m_hasRFooter)
                            {
                                nId = NS_ooxml::LN_footerr;
                                m_hasRFooter = true;
                            }
                            break;
                        case RTF_HEADERL:
                            nId = NS_ooxml::LN_headerl;
                            break;
                        case RTF_HEADERR:
                            nId = NS_ooxml::LN_headerr;
                            break;
                        case RTF_HEADERF:
                            if (!m_hasFHeader)
                            {
                                nId = NS_ooxml::LN_headerf;
                                m_hasFHeader = true;
                            }
                            break;
                        case RTF_FOOTERL:
                            nId = NS_ooxml::LN_footerl;
                            break;
                        case RTF_FOOTERR:
                            nId = NS_ooxml::LN_footerr;
                            break;
                        case RTF_FOOTERF:
                            if (!m_hasFFooter)
                            {
                                nId = NS_ooxml::LN_footerf;
                                m_hasFFooter = true;
                            }
                            break;
                        default:
                            break;
                    }

                    if (nId != 0)
                        m_nHeaderFooterPositions.push(std::make_pair(nId, nPos));

                    m_aStates.top().eDestination = Destination::SKIP;
                }
                break;
            case RTF_FOOTNOTE:
                checkFirstRun();
                if (!m_pSuperstream)
                {
                    Id nId = NS_ooxml::LN_footnote;

                    // Check if this is an endnote.
                    OStringBuffer aBuf;
                    char ch;
                    sal_uInt64 const nCurrent = Strm().Tell();
                    for (int i = 0; i < 7; ++i)
                    {
                        Strm().ReadChar(ch);
                        aBuf.append(ch);
                    }
                    Strm().Seek(nCurrent);
                    OString aKeyword = aBuf.makeStringAndClear();
                    if (aKeyword == "\\ftnalt")
                        nId = NS_ooxml::LN_endnote;

                    if (m_aStates.top().pCurrentBuffer == &m_aSuperBuffer)
                        m_aStates.top().pCurrentBuffer = nullptr;
                    bool bCustomMark = false;
                    OUString aCustomMark;
                    for (auto const& elem : m_aSuperBuffer)
                    {
                        if (std::get<0>(elem) == BUFFER_UTEXT)
                        {
                            aCustomMark = std::get<1>(elem)->getString();
                            bCustomMark = true;
                        }
                    }
                    m_aSuperBuffer.clear();
                    m_aStates.top().eDestination = Destination::FOOTNOTE;
                    Mapper().startCharacterGroup();
                    runProps();
                    if (!m_aStates.top().pCurrentBuffer)
                        resolveSubstream(m_nGroupStartPos - 1, nId, aCustomMark);
                    else
                    {
                        RTFSprms aAttributes;
                        aAttributes.set(Id(0), new RTFValue(m_nGroupStartPos - 1));
                        aAttributes.set(Id(1), new RTFValue(nId));
                        aAttributes.set(Id(2), new RTFValue(aCustomMark));
                        m_aStates.top().pCurrentBuffer->push_back(
                            Buf_t(BUFFER_RESOLVESUBSTREAM, new RTFValue(aAttributes), nullptr));
                    }
                    if (bCustomMark)
                    {
                        m_aStates.top().aCharacterAttributes.clear();
                        m_aStates.top().aCharacterSprms.clear();
                        auto pValue = new RTFValue(1);
                        m_aStates.top().aCharacterAttributes.set(
                            NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows, pValue);
                        text(aCustomMark);
                    }
                    Mapper().endCharacterGroup();
                    m_aStates.top().eDestination = Destination::SKIP;
                }
                break;
            case RTF_BKMKSTART:
                m_aStates.top().eDestination = Destination::BOOKMARKSTART;
                break;
            case RTF_BKMKEND:
                m_aStates.top().eDestination = Destination::BOOKMARKEND;
                break;
            case RTF_XE:
                m_aStates.top().eDestination = Destination::INDEXENTRY;
                break;
            case RTF_TC:
            case RTF_TCN:
                m_aStates.top().eDestination = Destination::TOCENTRY;
                break;
            case RTF_REVTBL:
                m_aStates.top().eDestination = Destination::REVISIONTABLE;
                break;
            case RTF_ANNOTATION:
                if (!m_pSuperstream)
                {
                    resolveSubstream(m_nGroupStartPos - 1, NS_ooxml::LN_annotation);
                    m_aStates.top().eDestination = Destination::SKIP;
                }
                else
                {
                    // If there is an author set, emit it now.
                    if (!m_aAuthor.isEmpty() || !m_aAuthorInitials.isEmpty())
                    {
                        RTFSprms aAttributes;
                        if (!m_aAuthor.isEmpty())
                        {
                            auto pValue = new RTFValue(m_aAuthor);
                            aAttributes.set(NS_ooxml::LN_CT_TrackChange_author, pValue);
                        }
                        if (!m_aAuthorInitials.isEmpty())
                        {
                            auto pValue = new RTFValue(m_aAuthorInitials);
                            aAttributes.set(NS_ooxml::LN_CT_Comment_initials, pValue);
                        }
                        writerfilter::Reference<Properties>::Pointer_t pProperties
                            = new RTFReferenceProperties(aAttributes);
                        Mapper().props(pProperties);
                    }
                }
                break;
            case RTF_SHPTXT:
            case RTF_DPTXBXTEXT:
            {
                bool bPictureFrame = false;
                for (auto& rProperty : m_aStates.top().aShape.getProperties())
                {
                    if (rProperty.first == "shapeType"
                        && rProperty.second == OUString::number(ESCHER_ShpInst_PictureFrame))
                    {
                        bPictureFrame = true;
                        break;
                    }
                }
                if (bPictureFrame)
                    // Skip text on picture frames.
                    m_aStates.top().eDestination = Destination::SKIP;
                else
                {
                    m_aStates.top().eDestination = Destination::SHAPETEXT;
                    checkFirstRun();
                    dispatchFlag(RTF_PARD);
                    m_bNeedPap = true;
                    if (nKeyword == RTF_SHPTXT)
                    {
                        if (!m_aStates.top().pCurrentBuffer)
                            m_pSdrImport->resolve(m_aStates.top().aShape, false,
                                                  RTFSdrImport::SHAPE);
                        else
                        {
                            auto pValue = new RTFValue(m_aStates.top().aShape);
                            m_aStates.top().pCurrentBuffer->push_back(
                                Buf_t(BUFFER_STARTSHAPE, pValue, nullptr));
                        }
                    }
                }
            }
            break;
            case RTF_FORMFIELD:
                if (m_aStates.top().eDestination == Destination::FIELDINSTRUCTION)
                    m_aStates.top().eDestination = Destination::FORMFIELD;
                break;
            case RTF_FFNAME:
                m_aStates.top().eDestination = Destination::FORMFIELDNAME;
                break;
            case RTF_FFL:
                m_aStates.top().eDestination = Destination::FORMFIELDLIST;
                break;
            case RTF_DATAFIELD:
                m_aStates.top().eDestination = Destination::DATAFIELD;
                break;
            case RTF_INFO:
                m_aStates.top().eDestination = Destination::INFO;
                break;
            case RTF_CREATIM:
                m_aStates.top().eDestination = Destination::CREATIONTIME;
                break;
            case RTF_REVTIM:
                m_aStates.top().eDestination = Destination::REVISIONTIME;
                break;
            case RTF_PRINTIM:
                m_aStates.top().eDestination = Destination::PRINTTIME;
                break;
            case RTF_AUTHOR:
                m_aStates.top().eDestination = Destination::AUTHOR;
                break;
            case RTF_KEYWORDS:
                m_aStates.top().eDestination = Destination::KEYWORDS;
                break;
            case RTF_OPERATOR:
                m_aStates.top().eDestination = Destination::OPERATOR;
                break;
            case RTF_COMPANY:
                m_aStates.top().eDestination = Destination::COMPANY;
                break;
            case RTF_COMMENT:
                m_aStates.top().eDestination = Destination::COMMENT;
                break;
            case RTF_OBJECT:
            {
                // beginning of an OLE Object
                m_aStates.top().eDestination = Destination::OBJECT;

                // check if the object is in a special container (e.g. a table)
                if (!m_aStates.top().pCurrentBuffer)
                {
                    // the object is in a table or another container.
                    // Don't try to treat it as an OLE object (fdo#53594).
                    // Use the \result (RTF_RESULT) element of the object instead,
                    // the result element contain picture representing the OLE Object.
                    m_bObject = true;
                }
            }
            break;
            case RTF_OBJDATA:
                // check if the object is in a special container (e.g. a table)
                if (m_aStates.top().pCurrentBuffer)
                {
                    // the object is in a table or another container.
                    // Use the \result (RTF_RESULT) element of the object instead,
                    // of the \objdata.
                    m_aStates.top().eDestination = Destination::SKIP;
                }
                else
                {
                    m_aStates.top().eDestination = Destination::OBJDATA;
                }
                break;
            case RTF_OBJCLASS:
                m_aStates.top().eDestination = Destination::OBJCLASS;
                break;
            case RTF_RESULT:
                m_aStates.top().eDestination = Destination::RESULT;
                break;
            case RTF_ATNDATE:
                m_aStates.top().eDestination = Destination::ANNOTATIONDATE;
                break;
            case RTF_ATNAUTHOR:
                m_aStates.top().eDestination = Destination::ANNOTATIONAUTHOR;
                break;
            case RTF_ATNREF:
                m_aStates.top().eDestination = Destination::ANNOTATIONREFERENCE;
                break;
            case RTF_FALT:
                m_aStates.top().eDestination = Destination::FALT;
                break;
            case RTF_FLYMAINCNT:
                m_aStates.top().eDestination = Destination::FLYMAINCONTENT;
                break;
            case RTF_LISTTEXT:
            // Should be ignored by any reader that understands Word 97 through Word 2007 numbering.
            case RTF_NONESTTABLES:
                // This destination should be ignored by readers that support nested tables.
                m_aStates.top().eDestination = Destination::SKIP;
                break;
            case RTF_DO:
                m_aStates.top().eDestination = Destination::DRAWINGOBJECT;
                break;
            case RTF_PN:
                m_aStates.top().eDestination = Destination::PARAGRAPHNUMBERING;
                break;
            case RTF_PNTEXT:
                // This destination should be ignored by readers that support paragraph numbering.
                m_aStates.top().eDestination = Destination::SKIP;
                break;
            case RTF_PNTXTA:
                m_aStates.top().eDestination = Destination::PARAGRAPHNUMBERING_TEXTAFTER;
                break;
            case RTF_PNTXTB:
                m_aStates.top().eDestination = Destination::PARAGRAPHNUMBERING_TEXTBEFORE;
                break;
            case RTF_TITLE:
                m_aStates.top().eDestination = Destination::TITLE;
                break;
            case RTF_SUBJECT:
                m_aStates.top().eDestination = Destination::SUBJECT;
                break;
            case RTF_DOCCOMM:
                m_aStates.top().eDestination = Destination::DOCCOMM;
                break;
            case RTF_ATRFSTART:
                m_aStates.top().eDestination = Destination::ANNOTATIONREFERENCESTART;
                break;
            case RTF_ATRFEND:
                m_aStates.top().eDestination = Destination::ANNOTATIONREFERENCEEND;
                break;
            case RTF_ATNID:
                m_aStates.top().eDestination = Destination::ATNID;
                break;
            case RTF_MMATH:
            case RTF_MOMATHPARA:
                // Nothing to do here (just enter the destination) till RTF_MMATHPR is implemented.
                break;
            case RTF_MR:
                m_aStates.top().eDestination = Destination::MR;
                break;
            case RTF_MCHR:
                m_aStates.top().eDestination = Destination::MCHR;
                break;
            case RTF_MPOS:
                m_aStates.top().eDestination = Destination::MPOS;
                break;
            case RTF_MVERTJC:
                m_aStates.top().eDestination = Destination::MVERTJC;
                break;
            case RTF_MSTRIKEH:
                m_aStates.top().eDestination = Destination::MSTRIKEH;
                break;
            case RTF_MDEGHIDE:
                m_aStates.top().eDestination = Destination::MDEGHIDE;
                break;
            case RTF_MTYPE:
                m_aStates.top().eDestination = Destination::MTYPE;
                break;
            case RTF_MGROW:
                m_aStates.top().eDestination = Destination::MGROW;
                break;
            case RTF_MHIDETOP:
            case RTF_MHIDEBOT:
            case RTF_MHIDELEFT:
            case RTF_MHIDERIGHT:
                // SmOoxmlImport::handleBorderBox will ignore these anyway, so silently ignore for now.
                m_aStates.top().eDestination = Destination::SKIP;
                break;
            case RTF_MSUBHIDE:
                m_aStates.top().eDestination = Destination::MSUBHIDE;
                break;
            case RTF_MSUPHIDE:
                m_aStates.top().eDestination = Destination::MSUPHIDE;
                break;
            case RTF_MBEGCHR:
                m_aStates.top().eDestination = Destination::MBEGCHR;
                break;
            case RTF_MSEPCHR:
                m_aStates.top().eDestination = Destination::MSEPCHR;
                break;
            case RTF_MENDCHR:
                m_aStates.top().eDestination = Destination::MENDCHR;
                break;
            case RTF_UPR:
                m_aStates.top().eDestination = Destination::UPR;
                break;
            case RTF_UD:
                // Anything inside \ud is just normal Unicode content.
                m_aStates.top().eDestination = Destination::NORMAL;
                break;
            case RTF_BACKGROUND:
                m_aStates.top().eDestination = Destination::BACKGROUND;
                m_aStates.top().bInBackground = true;
                break;
            case RTF_SHPGRP:
            {
                RTFLookahead aLookahead(Strm(), m_pTokenizer->getGroupStart());
                if (!aLookahead.hasTable())
                {
                    uno::Reference<drawing::XShapes> xGroupShape(
                        m_xModelFactory->createInstance("com.sun.star.drawing.GroupShape"),
                        uno::UNO_QUERY);
                    uno::Reference<drawing::XDrawPageSupplier> xDrawSupplier(m_xDstDoc,
                                                                             uno::UNO_QUERY);
                    if (xDrawSupplier.is())
                    {
                        uno::Reference<drawing::XShape> xShape(xGroupShape, uno::UNO_QUERY);
                        // set default VertOrient before inserting
                        uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY)
                            ->setPropertyValue("VertOrient",
                                               uno::makeAny(text::VertOrientation::NONE));
                        xDrawSupplier->getDrawPage()->add(xShape);
                    }
                    m_pSdrImport->pushParent(xGroupShape);
                    m_aStates.top().bCreatedShapeGroup = true;
                }
                m_aStates.top().eDestination = Destination::SHAPEGROUP;
                m_aStates.top().bInShapeGroup = true;
            }
            break;
            case RTF_FTNSEP:
                m_aStates.top().eDestination = Destination::FOOTNOTESEPARATOR;
                m_aStates.top().aCharacterAttributes.set(
                    NS_ooxml::LN_CT_FtnEdn_type,
                    new RTFValue(NS_ooxml::LN_Value_doc_ST_FtnEdn_separator));
                break;
            case RTF_USERPROPS:
                // Container of all user-defined properties.
                m_aStates.top().eDestination = Destination::USERPROPS;
                if (m_xDocumentProperties.is())
                    // Create a custom document properties to be able to process them later all at once.
                    m_xDocumentProperties = document::DocumentProperties::create(m_xContext);
                break;
            case RTF_PROPNAME:
                m_aStates.top().eDestination = Destination::PROPNAME;
                break;
            case RTF_STATICVAL:
                m_aStates.top().eDestination = Destination::STATICVAL;
                break;
            default:
            {
                // Check if it's a math token.
                RTFMathSymbol aSymbol(nKeyword);
                if (RTFTokenizer::lookupMathKeyword(aSymbol))
                {
                    m_aMathBuffer.appendOpeningTag(aSymbol.GetToken());
                    m_aStates.top().eDestination = aSymbol.GetDestination();
                    return RTFError::OK;
                }

                SAL_INFO("writerfilter",
                         "TODO handle destination '" << keywordToString(nKeyword) << "'");
                // Make sure we skip destinations (even without \*) till we don't handle them
                m_aStates.top().eDestination = Destination::SKIP;
                aSkip.setParsed(false);
            }
            break;
        }

    // new destination => use new destination text
    m_aStates.top().pDestinationText = &m_aStates.top().aDestinationText;

    return RTFError::OK;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
