/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

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

namespace writerfilter::rtftok
{
RTFError RTFDocumentImpl::dispatchDestination(RTFKeyword nKeyword)
{
    setNeedSect(true);
    checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    RTFSkipDestination aSkip(*this);
    // special case \upr: ignore everything except nested \ud
    if (Destination::UPR == m_aStates.top().getDestination() && RTFKeyword::UD != nKeyword)
    {
        m_aStates.top().setDestination(Destination::SKIP);
        aSkip.setParsed(false);
    }
    else
        switch (nKeyword)
        {
            case RTFKeyword::RTF:
                break;
            case RTFKeyword::FONTTBL:
                m_aStates.top().setDestination(Destination::FONTTABLE);
                break;
            case RTFKeyword::COLORTBL:
                m_aStates.top().setDestination(Destination::COLORTABLE);
                break;
            case RTFKeyword::STYLESHEET:
                m_aStates.top().setDestination(Destination::STYLESHEET);
                break;
            case RTFKeyword::FIELD:
                m_aStates.top().setDestination(Destination::FIELD);
                m_aStates.top().setFieldLocked(false);
                break;
            case RTFKeyword::DOCVAR:
                m_aStates.top().setDestination(Destination::DOCVAR);
                break;
            case RTFKeyword::FLDINST:
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

                if (std::string_view(aBuf) == "INCLUDEPICTURE")
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
                    OUString aFieldCommand = OStringToOUString(aBuf, RTL_TEXTENCODING_UTF8);
                    std::tuple<OUString, std::vector<OUString>, std::vector<OUString>> aResult
                        = writerfilter::dmapper::splitFieldCommand(aFieldCommand);
                    m_aPicturePath
                        = std::get<1>(aResult).empty() ? OUString() : std::get<1>(aResult).front();
                }

                Strm().Seek(nPos);

                // Form data should be handled only for form fields if any
                if (aBuf.toString().indexOf("FORM") != -1)
                    m_bFormField = true;

                singleChar(cFieldStart);
                m_aStates.top().setDestination(Destination::FIELDINSTRUCTION);
            }
            break;
            case RTFKeyword::FLDRSLT:
                m_aStates.top().setDestination(Destination::FIELDRESULT);
                break;
            case RTFKeyword::LISTTABLE:
                m_aStates.top().setDestination(Destination::LISTTABLE);
                break;
            case RTFKeyword::LISTPICTURE:
                m_aStates.top().setDestination(Destination::LISTPICTURE);
                m_aStates.top().setInListpicture(true);
                break;
            case RTFKeyword::LIST:
                m_aStates.top().setDestination(Destination::LISTENTRY);
                break;
            case RTFKeyword::LISTNAME:
                m_aStates.top().setDestination(Destination::LISTNAME);
                break;
            case RTFKeyword::LFOLEVEL:
                m_aStates.top().setDestination(Destination::LFOLEVEL);
                m_aStates.top().getTableSprms().clear();
                break;
            case RTFKeyword::LISTOVERRIDETABLE:
                m_aStates.top().setDestination(Destination::LISTOVERRIDETABLE);
                break;
            case RTFKeyword::LISTOVERRIDE:
                m_aStates.top().setDestination(Destination::LISTOVERRIDEENTRY);
                break;
            case RTFKeyword::LISTLEVEL:
                m_aStates.top().setDestination(Destination::LISTLEVEL);
                ++m_nListLevel;
                break;
            case RTFKeyword::LEVELTEXT:
                m_aStates.top().setDestination(Destination::LEVELTEXT);
                break;
            case RTFKeyword::LEVELNUMBERS:
                m_aStates.top().setDestination(Destination::LEVELNUMBERS);
                break;
            case RTFKeyword::SHPPICT:
                resetFrame();
                m_aStates.top().setDestination(Destination::SHPPICT);
                break;
            case RTFKeyword::PICT:
                if (m_aStates.top().getDestination() != Destination::SHAPEPROPERTYVALUE)
                    m_aStates.top().setDestination(Destination::PICT); // as character
                else
                    m_aStates.top().setDestination(
                        Destination::SHAPEPROPERTYVALUEPICT); // anchored inside a shape
                break;
            case RTFKeyword::PICPROP:
                m_aStates.top().setDestination(Destination::PICPROP);
                break;
            case RTFKeyword::SP:
                m_aStates.top().setDestination(Destination::SHAPEPROPERTY);
                break;
            case RTFKeyword::SN:
                m_aStates.top().setDestination(Destination::SHAPEPROPERTYNAME);
                break;
            case RTFKeyword::SV:
                m_aStates.top().setDestination(Destination::SHAPEPROPERTYVALUE);
                break;
            case RTFKeyword::SHP:
                m_bNeedCrOrig = m_bNeedCr;
                m_aStates.top().setDestination(Destination::SHAPE);
                m_aStates.top().setInShape(true);
                break;
            case RTFKeyword::SHPINST:
                m_aStates.top().setDestination(Destination::SHAPEINSTRUCTION);
                break;
            case RTFKeyword::NESTTABLEPROPS:
                // do not set any properties of outer table at nested table!
                m_aStates.top().getTableCellSprms() = m_aDefaultState.getTableCellSprms();
                m_aStates.top().getTableCellAttributes() = m_aDefaultState.getTableCellAttributes();
                m_aNestedTableCellsSprms.clear();
                m_aNestedTableCellsAttributes.clear();
                m_nNestedCells = 0;
                m_aStates.top().setDestination(Destination::NESTEDTABLEPROPERTIES);
                break;
            case RTFKeyword::HEADER:
            case RTFKeyword::FOOTER:
            case RTFKeyword::HEADERL:
            case RTFKeyword::HEADERR:
            case RTFKeyword::HEADERF:
            case RTFKeyword::FOOTERL:
            case RTFKeyword::FOOTERR:
            case RTFKeyword::FOOTERF:
                if (!m_pSuperstream)
                {
                    Id nId = 0;
                    std::size_t nPos = m_nGroupStartPos - 1;
                    switch (nKeyword)
                    {
                        case RTFKeyword::HEADER:
                            if (!m_hasRHeader)
                            {
                                nId = NS_ooxml::LN_headerr;
                                m_hasRHeader = true;
                            }
                            break;
                        case RTFKeyword::FOOTER:
                            if (!m_hasRFooter)
                            {
                                nId = NS_ooxml::LN_footerr;
                                m_hasRFooter = true;
                            }
                            break;
                        case RTFKeyword::HEADERL:
                            nId = NS_ooxml::LN_headerl;
                            break;
                        case RTFKeyword::HEADERR:
                            nId = NS_ooxml::LN_headerr;
                            break;
                        case RTFKeyword::HEADERF:
                            if (!m_hasFHeader)
                            {
                                nId = NS_ooxml::LN_headerf;
                                m_hasFHeader = true;
                            }
                            break;
                        case RTFKeyword::FOOTERL:
                            nId = NS_ooxml::LN_footerl;
                            break;
                        case RTFKeyword::FOOTERR:
                            nId = NS_ooxml::LN_footerr;
                            break;
                        case RTFKeyword::FOOTERF:
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

                    m_aStates.top().setDestination(Destination::SKIP);
                }
                break;
            case RTFKeyword::FOOTNOTE:
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

                    if (m_aStates.top().getCurrentBuffer() == &m_aSuperBuffer)
                        m_aStates.top().setCurrentBuffer(nullptr);
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
                    m_aStates.top().setDestination(Destination::FOOTNOTE);
                    Mapper().startCharacterGroup();
                    runProps();
                    if (!m_aStates.top().getCurrentBuffer())
                        resolveSubstream(m_nGroupStartPos - 1, nId, aCustomMark);
                    else
                    {
                        RTFSprms aAttributes;
                        aAttributes.set(Id(0), new RTFValue(m_nGroupStartPos - 1));
                        aAttributes.set(Id(1), new RTFValue(nId));
                        aAttributes.set(Id(2), new RTFValue(aCustomMark));
                        m_aStates.top().getCurrentBuffer()->push_back(
                            Buf_t(BUFFER_RESOLVESUBSTREAM, new RTFValue(aAttributes), nullptr));
                    }
                    if (bCustomMark)
                    {
                        m_aStates.top().getCharacterAttributes().clear();
                        m_aStates.top().getCharacterSprms().clear();
                        auto pValue = new RTFValue(1);
                        m_aStates.top().getCharacterAttributes().set(
                            NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows, pValue);
                        text(aCustomMark);
                    }
                    Mapper().endCharacterGroup();
                    m_aStates.top().setDestination(Destination::SKIP);
                }
                break;
            case RTFKeyword::BKMKSTART:
                m_aStates.top().setDestination(Destination::BOOKMARKSTART);
                break;
            case RTFKeyword::BKMKEND:
                m_aStates.top().setDestination(Destination::BOOKMARKEND);
                break;
            case RTFKeyword::XE:
                m_aStates.top().setDestination(Destination::INDEXENTRY);
                break;
            case RTFKeyword::TC:
            case RTFKeyword::TCN:
                m_aStates.top().setDestination(Destination::TOCENTRY);
                break;
            case RTFKeyword::REVTBL:
                m_aStates.top().setDestination(Destination::REVISIONTABLE);
                break;
            case RTFKeyword::ANNOTATION:
                if (!m_pSuperstream)
                {
                    if (!m_aStates.top().getCurrentBuffer())
                    {
                        resolveSubstream(m_nGroupStartPos - 1, NS_ooxml::LN_annotation);
                    }
                    else
                    {
                        RTFSprms aAttributes;
                        aAttributes.set(Id(0), new RTFValue(m_nGroupStartPos - 1));
                        aAttributes.set(Id(1), new RTFValue(NS_ooxml::LN_annotation));
                        aAttributes.set(Id(2), new RTFValue(OUString()));
                        m_aStates.top().getCurrentBuffer()->push_back(
                            Buf_t(BUFFER_RESOLVESUBSTREAM, new RTFValue(aAttributes), nullptr));
                    }
                    m_aStates.top().setDestination(Destination::SKIP);
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
                            = new RTFReferenceProperties(std::move(aAttributes));
                        Mapper().props(pProperties);
                    }
                }
                break;
            case RTFKeyword::SHPTXT:
            case RTFKeyword::DPTXBXTEXT:
            {
                bool bPictureFrame = false;
                for (const auto& rProperty : m_aStates.top().getShape().getProperties())
                {
                    if (rProperty.first == "shapeType"
                        && rProperty.second
                               == std::u16string_view(
                                      OUString::number(ESCHER_ShpInst_PictureFrame)))
                    {
                        bPictureFrame = true;
                        break;
                    }
                }
                if (bPictureFrame)
                    // Skip text on picture frames.
                    m_aStates.top().setDestination(Destination::SKIP);
                else
                {
                    m_aStates.top().setDestination(Destination::SHAPETEXT);
                    checkFirstRun();
                    dispatchFlag(RTFKeyword::PARD);
                    m_bNeedPap = true;
                    if (nKeyword == RTFKeyword::SHPTXT)
                    {
                        if (!m_aStates.top().getCurrentBuffer())
                            m_pSdrImport->resolve(m_aStates.top().getShape(), false,
                                                  RTFSdrImport::SHAPE);
                        else
                        {
                            auto pValue = new RTFValue(m_aStates.top().getShape());
                            m_aStates.top().getCurrentBuffer()->push_back(
                                Buf_t(BUFFER_STARTSHAPE, pValue, nullptr));
                        }
                    }
                }
            }
            break;
            case RTFKeyword::FORMFIELD:
                if (m_aStates.top().getDestination() == Destination::FIELDINSTRUCTION)
                    m_aStates.top().setDestination(Destination::FORMFIELD);
                break;
            case RTFKeyword::FFNAME:
                m_aStates.top().setDestination(Destination::FORMFIELDNAME);
                break;
            case RTFKeyword::FFL:
                m_aStates.top().setDestination(Destination::FORMFIELDLIST);
                break;
            case RTFKeyword::DATAFIELD:
                m_aStates.top().setDestination(Destination::DATAFIELD);
                break;
            case RTFKeyword::INFO:
                m_aStates.top().setDestination(Destination::INFO);
                break;
            case RTFKeyword::CREATIM:
                m_aStates.top().setDestination(Destination::CREATIONTIME);
                break;
            case RTFKeyword::REVTIM:
                m_aStates.top().setDestination(Destination::REVISIONTIME);
                break;
            case RTFKeyword::PRINTIM:
                m_aStates.top().setDestination(Destination::PRINTTIME);
                break;
            case RTFKeyword::AUTHOR:
                m_aStates.top().setDestination(Destination::AUTHOR);
                break;
            case RTFKeyword::KEYWORDS:
                m_aStates.top().setDestination(Destination::KEYWORDS);
                break;
            case RTFKeyword::OPERATOR:
                m_aStates.top().setDestination(Destination::OPERATOR);
                break;
            case RTFKeyword::COMPANY:
                m_aStates.top().setDestination(Destination::COMPANY);
                break;
            case RTFKeyword::COMMENT:
                m_aStates.top().setDestination(Destination::COMMENT);
                break;
            case RTFKeyword::OBJECT:
            {
                // beginning of an OLE Object
                m_aStates.top().setDestination(Destination::OBJECT);

                // check if the object is in a special container (e.g. a table)
                if (!m_aStates.top().getCurrentBuffer())
                {
                    // the object is in a table or another container.
                    // Don't try to treat it as an OLE object (fdo#53594).
                    // Use the \result (RTFKeyword::RESULT) element of the object instead,
                    // the result element contain picture representing the OLE Object.
                    m_bObject = true;
                }
            }
            break;
            case RTFKeyword::OBJDATA:
                // check if the object is in a special container (e.g. a table)
                if (m_aStates.top().getCurrentBuffer())
                {
                    // the object is in a table or another container.
                    // Use the \result (RTFKeyword::RESULT) element of the object instead,
                    // of the \objdata.
                    m_aStates.top().setDestination(Destination::SKIP);
                }
                else
                {
                    m_aStates.top().setDestination(Destination::OBJDATA);
                }
                break;
            case RTFKeyword::OBJCLASS:
                m_aStates.top().setDestination(Destination::OBJCLASS);
                break;
            case RTFKeyword::RESULT:
                m_aStates.top().setDestination(Destination::RESULT);
                break;
            case RTFKeyword::ATNDATE:
                m_aStates.top().setDestination(Destination::ANNOTATIONDATE);
                break;
            case RTFKeyword::ATNAUTHOR:
                m_aStates.top().setDestination(Destination::ANNOTATIONAUTHOR);
                break;
            case RTFKeyword::ATNREF:
                m_aStates.top().setDestination(Destination::ANNOTATIONREFERENCE);
                break;
            case RTFKeyword::FALT:
                m_aStates.top().setDestination(Destination::FALT);
                break;
            case RTFKeyword::FLYMAINCNT:
                m_aStates.top().setDestination(Destination::FLYMAINCONTENT);
                break;
            case RTFKeyword::LISTTEXT:
            // Should be ignored by any reader that understands Word 97 through Word 2007 numbering.
            case RTFKeyword::NONESTTABLES:
                // This destination should be ignored by readers that support nested tables.
                m_aStates.top().setDestination(Destination::SKIP);
                break;
            case RTFKeyword::DO:
                m_aStates.top().setDestination(Destination::DRAWINGOBJECT);
                break;
            case RTFKeyword::PN:
                m_aStates.top().setDestination(Destination::PARAGRAPHNUMBERING);
                break;
            case RTFKeyword::PNTEXT:
                // This destination should be ignored by readers that support paragraph numbering.
                m_aStates.top().setDestination(Destination::SKIP);
                break;
            case RTFKeyword::PNTXTA:
                m_aStates.top().setDestination(Destination::PARAGRAPHNUMBERING_TEXTAFTER);
                break;
            case RTFKeyword::PNTXTB:
                m_aStates.top().setDestination(Destination::PARAGRAPHNUMBERING_TEXTBEFORE);
                break;
            case RTFKeyword::TITLE:
                m_aStates.top().setDestination(Destination::TITLE);
                break;
            case RTFKeyword::SUBJECT:
                m_aStates.top().setDestination(Destination::SUBJECT);
                break;
            case RTFKeyword::DOCCOMM:
                m_aStates.top().setDestination(Destination::DOCCOMM);
                break;
            case RTFKeyword::ATRFSTART:
                m_aStates.top().setDestination(Destination::ANNOTATIONREFERENCESTART);
                break;
            case RTFKeyword::ATRFEND:
                m_aStates.top().setDestination(Destination::ANNOTATIONREFERENCEEND);
                break;
            case RTFKeyword::ATNID:
                m_aStates.top().setDestination(Destination::ATNID);
                break;
            case RTFKeyword::MMATH:
            case RTFKeyword::MOMATHPARA:
                // Nothing to do here (just enter the destination) till RTFKeyword::MMATHPR is implemented.
                break;
            case RTFKeyword::MR:
                m_aStates.top().setDestination(Destination::MR);
                break;
            case RTFKeyword::MCHR:
                m_aStates.top().setDestination(Destination::MCHR);
                break;
            case RTFKeyword::MPOS:
                m_aStates.top().setDestination(Destination::MPOS);
                break;
            case RTFKeyword::MVERTJC:
                m_aStates.top().setDestination(Destination::MVERTJC);
                break;
            case RTFKeyword::MSTRIKEH:
                m_aStates.top().setDestination(Destination::MSTRIKEH);
                break;
            case RTFKeyword::MDEGHIDE:
                m_aStates.top().setDestination(Destination::MDEGHIDE);
                break;
            case RTFKeyword::MTYPE:
                m_aStates.top().setDestination(Destination::MTYPE);
                break;
            case RTFKeyword::MGROW:
                m_aStates.top().setDestination(Destination::MGROW);
                break;
            case RTFKeyword::MHIDETOP:
            case RTFKeyword::MHIDEBOT:
            case RTFKeyword::MHIDELEFT:
            case RTFKeyword::MHIDERIGHT:
                // SmOoxmlImport::handleBorderBox will ignore these anyway, so silently ignore for now.
                m_aStates.top().setDestination(Destination::SKIP);
                break;
            case RTFKeyword::MSUBHIDE:
                m_aStates.top().setDestination(Destination::MSUBHIDE);
                break;
            case RTFKeyword::MSUPHIDE:
                m_aStates.top().setDestination(Destination::MSUPHIDE);
                break;
            case RTFKeyword::MBEGCHR:
                m_aStates.top().setDestination(Destination::MBEGCHR);
                break;
            case RTFKeyword::MSEPCHR:
                m_aStates.top().setDestination(Destination::MSEPCHR);
                break;
            case RTFKeyword::MENDCHR:
                m_aStates.top().setDestination(Destination::MENDCHR);
                break;
            case RTFKeyword::UPR:
                m_aStates.top().setDestination(Destination::UPR);
                break;
            case RTFKeyword::UD:
                // Anything inside \ud is just normal Unicode content.
                m_aStates.top().setDestination(Destination::NORMAL);
                break;
            case RTFKeyword::BACKGROUND:
                m_aStates.top().setDestination(Destination::BACKGROUND);
                m_aStates.top().setInBackground(true);
                break;
            case RTFKeyword::SHPGRP:
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
                        uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY_THROW)
                            ->setPropertyValue("VertOrient", uno::Any(text::VertOrientation::NONE));
                        xDrawSupplier->getDrawPage()->add(xShape);
                    }
                    m_pSdrImport->pushParent(xGroupShape);
                    m_aStates.top().setCreatedShapeGroup(true);
                }
                m_aStates.top().setDestination(Destination::SHAPEGROUP);
                m_aStates.top().setInShapeGroup(true);
            }
            break;
            case RTFKeyword::FTNSEP:
                m_aStates.top().setDestination(Destination::FOOTNOTESEPARATOR);
                m_aStates.top().getCharacterAttributes().set(
                    NS_ooxml::LN_CT_FtnEdn_type,
                    new RTFValue(NS_ooxml::LN_Value_doc_ST_FtnEdn_separator));
                break;
            case RTFKeyword::USERPROPS:
                // Container of all user-defined properties.
                m_aStates.top().setDestination(Destination::USERPROPS);
                if (m_xDocumentProperties.is())
                    // Create a custom document properties to be able to process them later all at once.
                    m_xDocumentProperties = document::DocumentProperties::create(m_xContext);
                break;
            case RTFKeyword::PROPNAME:
                m_aStates.top().setDestination(Destination::PROPNAME);
                break;
            case RTFKeyword::STATICVAL:
                m_aStates.top().setDestination(Destination::STATICVAL);
                break;
            case RTFKeyword::GENERATOR:
                m_aStates.top().setDestination(Destination::GENERATOR);
                break;
            default:
            {
                // Check if it's a math token.
                RTFMathSymbol aSymbol(nKeyword);
                if (RTFTokenizer::lookupMathKeyword(aSymbol))
                {
                    m_aMathBuffer.appendOpeningTag(aSymbol.GetToken());
                    m_aStates.top().setDestination(aSymbol.GetDestination());
                    return RTFError::OK;
                }

                SAL_INFO("writerfilter",
                         "TODO handle destination '" << keywordToString(nKeyword) << "'");
                // Make sure we skip destinations (even without \*) till we don't handle them
                m_aStates.top().setDestination(Destination::SKIP);
                aSkip.setParsed(false);
            }
            break;
        }

    // new destination => use new destination text
    m_aStates.top().setCurrentDestinationText(&m_aStates.top().getDestinationText());

    return RTFError::OK;
}

} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
