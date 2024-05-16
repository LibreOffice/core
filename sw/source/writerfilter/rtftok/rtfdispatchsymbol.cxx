/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfdocumentimpl.hxx"

#include <com/sun/star/io/WrongFormatException.hpp>
#include <svl/lngmisc.hxx>

#include <ooxml/resourceids.hxx>

#include <sal/log.hxx>

#include "rtfreferenceproperties.hxx"
#include "rtfskipdestination.hxx"

using namespace com::sun::star;

namespace writerfilter::rtftok
{
RTFError RTFDocumentImpl::dispatchSymbol(RTFKeyword nKeyword)
{
    setNeedSect(true);
    if (nKeyword != RTFKeyword::HEXCHAR)
        checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    else
        checkUnicode(/*bUnicode =*/true, /*bHex =*/false);
    RTFSkipDestination aSkip(*this);

    if (RTFKeyword::LINE == nKeyword)
    {
        // very special handling since text() will eat lone '\n'
        singleChar('\n', /*bRunProps=*/true);
        return RTFError::OK;
    }
    // Trivial symbols
    sal_uInt8 cCh = 0;
    switch (nKeyword)
    {
        case RTFKeyword::TAB:
            cCh = '\t';
            break;
        case RTFKeyword::BACKSLASH:
            cCh = '\\';
            break;
        case RTFKeyword::LBRACE:
            cCh = '{';
            break;
        case RTFKeyword::RBRACE:
            cCh = '}';
            break;
        case RTFKeyword::EMDASH:
            cCh = 151;
            break;
        case RTFKeyword::ENDASH:
            cCh = 150;
            break;
        case RTFKeyword::BULLET:
            cCh = 149;
            break;
        case RTFKeyword::LQUOTE:
            cCh = 145;
            break;
        case RTFKeyword::RQUOTE:
            cCh = 146;
            break;
        case RTFKeyword::LDBLQUOTE:
            cCh = 147;
            break;
        case RTFKeyword::RDBLQUOTE:
            cCh = 148;
            break;
        default:
            break;
    }
    if (cCh > 0)
    {
        OUString aStr(OStringToOUString(OStringChar(char(cCh)), RTL_TEXTENCODING_MS_1252));
        text(aStr);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTFKeyword::IGNORE:
        {
            m_bSkipUnknown = true;
            aSkip.setReset(false);
            return RTFError::OK;
        }
        break;
        case RTFKeyword::PAR:
        {
            if (m_aStates.top().getDestination() == Destination::FOOTNOTESEPARATOR)
                break; // just ignore it - only thing we read in here is CHFTNSEP
            checkFirstRun();
            checkNeedPap();
            runProps(); // tdf#152872 paragraph marker formatting
            if (!m_aStates.top().getCurrentBuffer())
            {
                parBreak();
                // Not in table? Reset max width.
                if (m_nCellxMax)
                {
                    // Was in table, but not anymore -> tblEnd.
                    RTFSprms aAttributes;
                    RTFSprms aSprms;
                    aSprms.set(NS_ooxml::LN_tblEnd, new RTFValue(1));
                    writerfilter::Reference<Properties>::Pointer_t pProperties
                        = new RTFReferenceProperties(std::move(aAttributes), std::move(aSprms));
                    Mapper().props(pProperties);
                }
                m_nCellxMax = 0;
            }
            else if (m_aStates.top().getDestination() != Destination::SHAPETEXT)
            {
                RTFValue::Pointer_t pValue;
                m_aStates.top().getCurrentBuffer()->push_back(Buf_t(BUFFER_PAR, pValue, nullptr));
            }
            // but don't emit properties yet, since they may change till the first text token arrives
            m_bNeedPap = true;
            if (!m_aStates.top().getFrame().hasProperties())
                m_bNeedPar = false;
            m_bNeedFinalPar = false;
        }
        break;
        case RTFKeyword::SECT:
        {
            m_bHadSect = true;
            if (m_bIgnoreNextContSectBreak || m_aStates.top().getFrame().hasProperties())
            {
                // testContSectionPageBreak: need \par now
                dispatchSymbol(RTFKeyword::PAR);
                m_bIgnoreNextContSectBreak = false;
            }
            else
            {
                bool bPendingFloatingTable = false;
                RTFValue::Pointer_t pTblpPr
                    = m_aStates.top().getTableRowSprms().find(NS_ooxml::LN_CT_TblPrBase_tblpPr);
                if (pTblpPr)
                {
                    // We have a pending floating table, provide an anchor for it still in this
                    // section.
                    bPendingFloatingTable = true;
                }

                if (m_bNeedCr || bPendingFloatingTable)
                { // tdf#158586 don't dispatch \par here, it eats deferred page breaks
                    setNeedPar(true);
                }

                sectBreak();
                if (m_nResetBreakOnSectBreak != RTFKeyword::invalid)
                {
                    // this should run on _second_ \sect after \page
                    dispatchFlag(m_nResetBreakOnSectBreak); // lazy reset
                    m_nResetBreakOnSectBreak = RTFKeyword::invalid;
                    m_bNeedSect = false; // dispatchSymbol set it
                }
                setNeedPar(true); // testFdo52052: need \par at end of document
                // testNestedTable: but not m_bNeedCr, that creates a page break
            }
        }
        break;
        case RTFKeyword::NOBREAK:
        {
            OUString aStr(SVT_HARD_SPACE);
            text(aStr);
        }
        break;
        case RTFKeyword::NOBRKHYPH:
        {
            OUString aStr(SVT_HARD_HYPHEN);
            text(aStr);
        }
        break;
        case RTFKeyword::OPTHYPH:
        {
            OUString aStr(SVT_SOFT_HYPHEN);
            text(aStr);
        }
        break;
        case RTFKeyword::HEXCHAR:
            m_aStates.top().setInternalState(RTFInternalState::HEX);
            break;
        case RTFKeyword::CELL:
        case RTFKeyword::NESTCELL:
        {
            checkFirstRun();
            if (m_bNeedPap)
            {
                // There were no runs in the cell, so we need to send paragraph and character properties here.
                auto pPValue = new RTFValue(m_aStates.top().getParagraphAttributes(),
                                            m_aStates.top().getParagraphSprms());
                bufferProperties(m_aTableBufferStack.back(), pPValue, nullptr);
                auto pCValue = new RTFValue(m_aStates.top().getCharacterAttributes(),
                                            m_aStates.top().getCharacterSprms());
                bufferProperties(m_aTableBufferStack.back(), pCValue, nullptr);
            }

            RTFValue::Pointer_t pValue;
            m_aTableBufferStack.back().emplace_back(Buf_t(BUFFER_CELLEND, pValue, nullptr));
            m_bNeedPap = true;
        }
        break;
        case RTFKeyword::NESTROW:
        {
            tools::SvRef<TableRowBuffer> const pBuffer(
                new TableRowBuffer(m_aTableBufferStack.back(), m_aNestedTableCellsSprms,
                                   m_aNestedTableCellsAttributes, m_nNestedCells));
            prepareProperties(m_aStates.top(), pBuffer->GetParaProperties(),
                              pBuffer->GetFrameProperties(), pBuffer->GetRowProperties(),
                              m_nNestedCells, m_nNestedCurrentCellX - m_nNestedTRLeft);

            if (m_aTableBufferStack.size() == 1 || !m_aStates.top().getCurrentBuffer())
            {
                throw io::WrongFormatException(
                    u"mismatch between \\itap and number of \\nestrow"_ustr, nullptr);
            }
            assert(m_aStates.top().getCurrentBuffer() == &m_aTableBufferStack.back());
            // note: there may be several states pointing to table buffer!
            for (std::size_t i = 0; i < m_aStates.size(); ++i)
            {
                if (m_aStates[i].getCurrentBuffer() == &m_aTableBufferStack.back())
                {
                    m_aStates[i].setCurrentBuffer(
                        &m_aTableBufferStack[m_aTableBufferStack.size() - 2]);
                }
            }
            m_aTableBufferStack.pop_back();
            m_aTableBufferStack.back().emplace_back(
                Buf_t(BUFFER_NESTROW, RTFValue::Pointer_t(), pBuffer));

            m_aNestedTableCellsSprms.clear();
            m_aNestedTableCellsAttributes.clear();
            m_nNestedCells = 0;
            m_bNeedPap = true;
        }
        break;
        case RTFKeyword::ROW:
        {
            if (m_aStates.top().getTableRowWidthAfter() > 0)
            {
                // Add fake cellx / cell, RTF equivalent of
                // OOXMLFastContextHandlerTextTableRow::handleGridAfter().
                auto pXValue = new RTFValue(m_aStates.top().getTableRowWidthAfter());
                m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue,
                                                       RTFOverwrite::NO_APPEND);
                dispatchSymbol(RTFKeyword::CELL);

                // Adjust total width, which is done in the \cellx handler for normal cells.
                m_nTopLevelCurrentCellX += m_aStates.top().getTableRowWidthAfter();

                m_aStates.top().setTableRowWidthAfter(0);
            }

            bool bRestored = false;
            // Ending a row, but no cells defined?
            // See if there was an invalid table row reset, so we can restore cell infos to help invalid documents.
            if (!m_nTopLevelCurrentCellX && m_nBackupTopLevelCurrentCellX)
            {
                restoreTableRowProperties();
                bRestored = true;
            }

            // If the right edge of the last cell (row width) is smaller than the width of some other row, mimic WW8TabDesc::CalcDefaults(): resize the last cell
            const int MINLAY = 23; // sw/inc/swtypes.hxx, minimal possible size of frames.
            if ((m_nCellxMax - m_nTopLevelCurrentCellX) >= MINLAY)
            {
                auto pXValueLast = m_aStates.top().getTableRowSprms().find(
                    NS_ooxml::LN_CT_TblGridBase_gridCol, false);
                const int nXValueLast = pXValueLast ? pXValueLast->getInt() : 0;
                auto pXValue = new RTFValue(nXValueLast + m_nCellxMax - m_nTopLevelCurrentCellX);
                m_aStates.top().getTableRowSprms().eraseLast(NS_ooxml::LN_CT_TblGridBase_gridCol);
                m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue,
                                                       RTFOverwrite::NO_APPEND);
                m_nTopLevelCurrentCellX = m_nCellxMax;
            }

            if (m_nTopLevelCells)
            {
                // Make a backup before we start popping elements
                m_aTableInheritingCellsSprms = m_aTopLevelTableCellsSprms;
                m_aTableInheritingCellsAttributes = m_aTopLevelTableCellsAttributes;
                m_nInheritingCells = m_nTopLevelCells;
            }
            else
            {
                // No table definition? Then inherit from the previous row
                m_aTopLevelTableCellsSprms = m_aTableInheritingCellsSprms;
                m_aTopLevelTableCellsAttributes = m_aTableInheritingCellsAttributes;
                m_nTopLevelCells = m_nInheritingCells;
            }

            while (m_aTableBufferStack.size() > 1)
            {
                SAL_WARN("writerfilter.rtf", "dropping extra table buffer");
                // note: there may be several states pointing to table buffer!
                for (std::size_t i = 0; i < m_aStates.size(); ++i)
                {
                    if (m_aStates[i].getCurrentBuffer() == &m_aTableBufferStack.back())
                    {
                        m_aStates[i].setCurrentBuffer(&m_aTableBufferStack.front());
                    }
                }
                m_aTableBufferStack.pop_back();
            }

            replayRowBuffer(m_aTableBufferStack.back(), m_aTopLevelTableCellsSprms,
                            m_aTopLevelTableCellsAttributes, m_nTopLevelCells);

            // The scope of the table cell defaults is one row.
            m_aDefaultState.getTableCellSprms().clear();
            m_aStates.top().getTableCellSprms() = m_aDefaultState.getTableCellSprms();
            m_aStates.top().getTableCellAttributes() = m_aDefaultState.getTableCellAttributes();

            writerfilter::Reference<Properties>::Pointer_t paraProperties;
            writerfilter::Reference<Properties>::Pointer_t frameProperties;
            writerfilter::Reference<Properties>::Pointer_t rowProperties;
            prepareProperties(m_aStates.top(), paraProperties, frameProperties, rowProperties,
                              m_nTopLevelCells, m_nTopLevelCurrentCellX - m_nTopLevelTRLeft);
            sendProperties(paraProperties, frameProperties, rowProperties);

            m_bNeedPap = true;
            m_bNeedFinalPar = true;
            m_aTableBufferStack.back().clear();
            m_nTopLevelCells = 0;

            if (bRestored)
                // We restored cell definitions, clear these now.
                // This is necessary, as later cell definitions want to overwrite the restored ones.
                resetTableRowProperties();
        }
        break;
        case RTFKeyword::COLUMN:
        {
            bool bColumns = false; // If we have multiple columns
            RTFValue::Pointer_t pCols
                = m_aStates.top().getSectionSprms().find(NS_ooxml::LN_EG_SectPrContents_cols);
            if (pCols)
            {
                RTFValue::Pointer_t pNum = pCols->getAttributes().find(NS_ooxml::LN_CT_Columns_num);
                if (pNum && pNum->getInt() > 1)
                    bColumns = true;
            }
            checkFirstRun();
            if (bColumns)
            {
                sal_uInt8 const sBreak[] = { 0xe };
                Mapper().startCharacterGroup();
                Mapper().text(sBreak, 1);
                Mapper().endCharacterGroup();
            }
            else
                dispatchSymbol(RTFKeyword::PAGE);
        }
        break;
        case RTFKeyword::CHFTN:
        {
            if (m_aStates.top().getCurrentBuffer() == &m_aSuperBuffer)
                // Stop buffering, there will be no custom mark for this footnote or endnote.
                m_aStates.top().setCurrentBuffer(nullptr);
            break;
        }
        case RTFKeyword::PAGE:
        {
            // Ignore page breaks inside tables.
            if (m_aStates.top().getCurrentBuffer() == &m_aTableBufferStack.back())
                break;

            // If we're inside a continuous section, we should send a section break, not a page one.
            RTFValue::Pointer_t pBreak
                = m_aStates.top().getSectionSprms().find(NS_ooxml::LN_EG_SectPrContents_type);
            // Unless we're on a title page.
            RTFValue::Pointer_t pTitlePg
                = m_aStates.top().getSectionSprms().find(NS_ooxml::LN_EG_SectPrContents_titlePg);
            if (((pBreak
                  && pBreak->getInt()
                         == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous)
                  && m_bHadSect) // tdf#158983 before first \sect, ignore \sbknone!
                 || m_nResetBreakOnSectBreak == RTFKeyword::SBKNONE)
                && !(pTitlePg && pTitlePg->getInt()))
            {
                if (m_bWasInFrame)
                {
                    dispatchSymbol(RTFKeyword::PAR);
                    m_bWasInFrame = false;
                }
                sectBreak();
                // note: this will not affect the following section break
                // but the one just pushed
                dispatchFlag(RTFKeyword::SBKPAGE);
                if (m_bNeedPar)
                    dispatchSymbol(RTFKeyword::PAR);
                m_bIgnoreNextContSectBreak = true;
                // arrange to clean up the synthetic RTFKeyword::SBKPAGE
                m_nResetBreakOnSectBreak = RTFKeyword::SBKNONE;
            }
            else
            {
                bool bFirstRun = m_bFirstRun;
                checkFirstRun();
                if (bFirstRun || m_bNeedCr)
                {
                    // Only send the paragraph properties early if we'll create a new paragraph in a
                    // bit anyway.
                    checkNeedPap();
                    // flush previously deferred break - needed for testFdo49893_2
                    // which has consecutive \page with no text between
                    sal_Unicode const nothing[] = { 0 /*MSVC doesn't allow it to be empty*/ };
                    Mapper().utext(nothing, 0);
                }
                sal_uInt8 const sBreak[] = { 0xc };
                Mapper().text(sBreak, 1);
                // testFdo81892 don't do another \par break directly; because of
                // GetSplitPgBreakAndParaMark() it does finishParagraph *twice*
                m_bNeedCr = true;
            }
        }
        break;
        case RTFKeyword::CHPGN:
        {
            OUString aStr(u"PAGE"_ustr);
            singleChar(cFieldStart);
            text(aStr);
            singleChar(cFieldSep, true);
            singleChar(cFieldEnd);
        }
        break;
        case RTFKeyword::CHFTNSEP:
        {
            static const sal_Unicode uFtnEdnSep = 0x3;
            Mapper().utext(&uFtnEdnSep, 1);
        }
        break;
        default:
        {
            SAL_INFO("writerfilter.rtf",
                     "TODO handle symbol '" << keywordToString(nKeyword) << "'");
            aSkip.setParsed(false);
        }
        break;
    }
    return RTFError::OK;
}

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
