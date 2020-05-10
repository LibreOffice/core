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
    if (nKeyword != RTF_HEXCHAR)
        checkUnicode(/*bUnicode =*/true, /*bHex =*/true);
    else
        checkUnicode(/*bUnicode =*/true, /*bHex =*/false);
    RTFSkipDestination aSkip(*this);

    if (RTF_LINE == nKeyword)
    {
        // very special handling since text() will eat lone '\n'
        singleChar('\n');
        return RTFError::OK;
    }
    // Trivial symbols
    sal_uInt8 cCh = 0;
    switch (nKeyword)
    {
        case RTF_TAB:
            cCh = '\t';
            break;
        case RTF_BACKSLASH:
            cCh = '\\';
            break;
        case RTF_LBRACE:
            cCh = '{';
            break;
        case RTF_RBRACE:
            cCh = '}';
            break;
        case RTF_EMDASH:
            cCh = 151;
            break;
        case RTF_ENDASH:
            cCh = 150;
            break;
        case RTF_BULLET:
            cCh = 149;
            break;
        case RTF_LQUOTE:
            cCh = 145;
            break;
        case RTF_RQUOTE:
            cCh = 146;
            break;
        case RTF_LDBLQUOTE:
            cCh = 147;
            break;
        case RTF_RDBLQUOTE:
            cCh = 148;
            break;
        default:
            break;
    }
    if (cCh > 0)
    {
        OUString aStr(OStringToOUString(OString(cCh), RTL_TEXTENCODING_MS_1252));
        text(aStr);
        return RTFError::OK;
    }

    switch (nKeyword)
    {
        case RTF_IGNORE:
        {
            m_bSkipUnknown = true;
            aSkip.setReset(false);
            return RTFError::OK;
        }
        break;
        case RTF_PAR:
        {
            if (m_aStates.top().getDestination() == Destination::FOOTNOTESEPARATOR)
                break; // just ignore it - only thing we read in here is CHFTNSEP
            checkFirstRun();
            bool bNeedPap = m_bNeedPap;
            checkNeedPap();
            if (bNeedPap)
                runProps();
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
                        = new RTFReferenceProperties(aAttributes, aSprms);
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
            if (!m_aStates.top().getFrame().inFrame())
                m_bNeedPar = false;
            m_bNeedFinalPar = false;
        }
        break;
        case RTF_SECT:
        {
            m_bHadSect = true;
            if (m_bIgnoreNextContSectBreak)
                m_bIgnoreNextContSectBreak = false;
            else
            {
                sectBreak();
                if (m_nResetBreakOnSectBreak != RTF_invalid)
                {
                    // this should run on _second_ \sect after \page
                    dispatchSymbol(m_nResetBreakOnSectBreak); // lazy reset
                    m_nResetBreakOnSectBreak = RTF_invalid;
                    m_bNeedSect = false; // dispatchSymbol set it
                }
            }
        }
        break;
        case RTF_NOBREAK:
        {
            OUString aStr(SVT_HARD_SPACE);
            text(aStr);
        }
        break;
        case RTF_NOBRKHYPH:
        {
            OUString aStr(SVT_HARD_HYPHEN);
            text(aStr);
        }
        break;
        case RTF_OPTHYPH:
        {
            OUString aStr(SVT_SOFT_HYPHEN);
            text(aStr);
        }
        break;
        case RTF_HEXCHAR:
            m_aStates.top().setInternalState(RTFInternalState::HEX);
            break;
        case RTF_CELL:
        case RTF_NESTCELL:
        {
            if (nKeyword == RTF_CELL)
                m_bAfterCellBeforeRow = true;

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
        case RTF_NESTROW:
        {
            tools::SvRef<TableRowBuffer> const pBuffer(
                new TableRowBuffer(m_aTableBufferStack.back(), m_aNestedTableCellsSprms,
                                   m_aNestedTableCellsAttributes, m_nNestedCells));
            prepareProperties(m_aStates.top(), pBuffer->GetParaProperties(),
                              pBuffer->GetFrameProperties(), pBuffer->GetRowProperties(),
                              m_nNestedCells, m_nNestedCurrentCellX - m_nNestedTRLeft);

            if (m_aTableBufferStack.size() == 1 || !m_aStates.top().getCurrentBuffer())
            {
                throw io::WrongFormatException("mismatch between \\itap and number of \\nestrow",
                                               nullptr);
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
        case RTF_ROW:
        {
            m_bAfterCellBeforeRow = false;
            if (m_aStates.top().getTableRowWidthAfter() > 0)
            {
                // Add fake cellx / cell, RTF equivalent of
                // OOXMLFastContextHandlerTextTableRow::handleGridAfter().
                auto pXValue = new RTFValue(m_aStates.top().getTableRowWidthAfter());
                m_aStates.top().getTableRowSprms().set(NS_ooxml::LN_CT_TblGridBase_gridCol, pXValue,
                                                       RTFOverwrite::NO_APPEND);
                dispatchSymbol(RTF_CELL);

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
        case RTF_COLUMN:
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
                dispatchSymbol(RTF_PAGE);
        }
        break;
        case RTF_CHFTN:
        {
            if (m_aStates.top().getCurrentBuffer() == &m_aSuperBuffer)
                // Stop buffering, there will be no custom mark for this footnote or endnote.
                m_aStates.top().setCurrentBuffer(nullptr);
            break;
        }
        case RTF_PAGE:
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
                         == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous))
                 || m_nResetBreakOnSectBreak == RTF_SBKNONE)
                && !(pTitlePg && pTitlePg->getInt()))
            {
                if (m_bWasInFrame)
                {
                    dispatchSymbol(RTF_PAR);
                    m_bWasInFrame = false;
                }
                sectBreak();
                // note: this will not affect the following section break
                // but the one just pushed
                dispatchFlag(RTF_SBKPAGE);
                if (m_bNeedPar)
                    dispatchSymbol(RTF_PAR);
                m_bIgnoreNextContSectBreak = true;
                // arrange to clean up the synthetic RTF_SBKPAGE
                m_nResetBreakOnSectBreak = RTF_SBKNONE;
            }
            else
            {
                checkFirstRun();
                checkNeedPap();
                sal_uInt8 const sBreak[] = { 0xc };
                Mapper().text(sBreak, 1);
                if (!m_bNeedPap)
                {
                    parBreak();
                    m_bNeedPap = true;
                }
                m_bNeedCr = true;
            }
        }
        break;
        case RTF_CHPGN:
        {
            OUString aStr("PAGE");
            singleChar(cFieldStart);
            text(aStr);
            singleChar(cFieldSep, true);
            singleChar(cFieldEnd);
        }
        break;
        case RTF_CHFTNSEP:
        {
            static const sal_Unicode uFtnEdnSep = 0x3;
            Mapper().utext(reinterpret_cast<const sal_uInt8*>(&uFtnEdnSep), 1);
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
