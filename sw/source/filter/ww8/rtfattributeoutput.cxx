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

#include "sal/config.h"

#include <cstring>

#include "rtfattributeoutput.hxx"
#include "rtfsdrexport.hxx"
#include "writerwordglue.hxx"
#include "ww8par.hxx"
#include "fmtcntnt.hxx"
#include "fchrfmt.hxx"

#include <rtl/ustring.hxx>
#include <rtl/tencinfo.h>
#include <svtools/rtfkeywd.hxx>

#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/paravertalignitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <svtools/miscopt.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/xflgrit.hxx>

#include <docufld.hxx>
#include <fmtclds.hxx>
#include <fmtinfmt.hxx>
#include <fmtftn.hxx>
#include <fmtrowsplt.hxx>
#include <fmtline.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <htmltbl.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <pagedesc.hxx>
#include <swmodule.hxx>
#include <swtable.hxx>
#include <txtftn.hxx>
#include <txtinet.hxx>
#include <grfatr.hxx>
#include <ndole.hxx>
#include <lineinfo.hxx>
#include <rtf.hxx>

#include <vcl/cvtgrf.hxx>
#include <oox/mathml/export.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>

using ::editeng::SvxBorderLine;
using namespace nsSwDocInfoSubType;
using namespace nsFieldFlags;
using namespace sw::util;
using namespace ::com::sun::star;

static OString OutTBLBorderLine(RtfExport &rExport, const SvxBorderLine* pLine, const sal_Char* pStr)
{
    OStringBuffer aRet;
    if ( !pLine->isEmpty() )
    {
        aRet.append(pStr);
        // single line
        switch (pLine->GetBorderLineStyle())
        {
            case table::BorderLineStyle::SOLID:
                {
                    if( DEF_LINE_WIDTH_0 == pLine->GetWidth() )
                        aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRHAIR);
                    else
                        aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRS);
                }
                break;
            case table::BorderLineStyle::DOTTED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDOT);
                break;
            case table::BorderLineStyle::DASHED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDASH);
                break;
            case table::BorderLineStyle::DOUBLE:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRDB);
                break;
            case table::BorderLineStyle::THINTHICK_SMALLGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTNTHSG);
                break;
            case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTNTHMG);
                break;
            case table::BorderLineStyle::THINTHICK_LARGEGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTNTHLG);
                break;
            case table::BorderLineStyle::THICKTHIN_SMALLGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTHTNSG);
                break;
            case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTHTNMG);
                break;
            case table::BorderLineStyle::THICKTHIN_LARGEGAP:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTHTNLG);
                break;
            case table::BorderLineStyle::EMBOSSED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDREMBOSS);
                break;
            case table::BorderLineStyle::ENGRAVED:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRENGRAVE);
                break;
            case table::BorderLineStyle::OUTSET:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDROUTSET);
                break;
            case table::BorderLineStyle::INSET:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRINSET);
                break;
            case table::BorderLineStyle::NONE:
            default:
                aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRNONE);
                break;
        }

        double const fConverted( ::editeng::ConvertBorderWidthToWord(
                    pLine->GetBorderLineStyle(), pLine->GetWidth()) );
        if ( 255 >= pLine->GetWidth() ) // That value comes from RTF specs
        {
            aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRW).append(
                    static_cast<sal_Int32>(fConverted));
        }
        else
        {   // use \brdrth to double the value range...
            aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRTH OOO_STRING_SVTOOLS_RTF_BRDRW);
            aRet.append(static_cast<sal_Int32>(fConverted) / 2);
        }

        aRet.append(OOO_STRING_SVTOOLS_RTF_BRDRCF);
        aRet.append((sal_Int32)rExport.GetColor(pLine->GetColor()));
    }
    return aRet.makeStringAndClear();
}

static OString OutBorderLine(RtfExport &rExport, const SvxBorderLine* pLine,
    const sal_Char* pStr, sal_uInt16 nDist)
{
    OStringBuffer aRet;
    aRet.append(OutTBLBorderLine(rExport, pLine, pStr));
    aRet.append(OOO_STRING_SVTOOLS_RTF_BRSP);
    aRet.append((sal_Int32)nDist);
    return aRet.makeStringAndClear();
}

void RtfAttributeOutput::RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    /*
       You would have thought that
       m_rExport.Strm() << (bIsRTL ? OOO_STRING_SVTOOLS_RTF_RTLCH : OOO_STRING_SVTOOLS_RTF_LTRCH); would be sufficent here ,
       but looks like word needs to see the other directional token to be
       satisified that all is kosher, otherwise it seems in ver 2003 to go and
       semi-randomlyly stick strike through about the place. Perhaps
       strikethrough is some ms developers "something is wrong signal" debugging
       code that we're triggering ?
       */
    if (bIsRTL) {
        m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_LTRCH);
        m_aStylesEnd.append(' ');
        m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_RTLCH);
    } else {
        m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_RTLCH);
        m_aStylesEnd.append(' ');
        m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_LTRCH);
    }

    switch (nScript) {
        case i18n::ScriptType::LATIN:
            m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_LOCH);
            break;
        case i18n::ScriptType::ASIAN:
            m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_DBCH);
            break;
        case i18n::ScriptType::COMPLEX:
            /* noop */
            break;
        default:
            /* should not happen? */
            break;
    }
}

void RtfAttributeOutput::StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // Output table/table row/table cell starts if needed
    if ( pTextNodeInfo.get() )
    {
        sal_uInt32 nRow = pTextNodeInfo->getRow();
        sal_uInt32 nCell = pTextNodeInfo->getCell();

        // New cell/row?
        if ( m_nTableDepth > 0 && !m_bTableCellOpen )
        {
            ww8::WW8TableNodeInfoInner::Pointer_t pDeepInner( pTextNodeInfo->getInnerForDepth( m_nTableDepth ) );
            OSL_ENSURE( pDeepInner, "TableNodeInfoInner not found");
            // Make sure we always start a row between ending one and starting a cell.
            // In case of subtables, we may not get the first cell.
            if (pDeepInner && (pDeepInner->getCell() == 0 || m_bTableRowEnded))
            {
                m_bTableRowEnded = false;
                StartTableRow( pDeepInner );
            }

            StartTableCell( pDeepInner );
        }

        // Again, if depth was incremented, start a new table even if we skipped the first cell.
        if ((nRow == 0 && nCell == 0) || (m_nTableDepth == 0 && pTextNodeInfo->getDepth()))
        {
            // Do we have to start the table?
            // [If we are at the rigth depth already, it means that we
            // continue the table cell]
            sal_uInt32 nCurrentDepth = pTextNodeInfo->getDepth();

            if ( nCurrentDepth > m_nTableDepth )
            {
                // Start all the tables that begin here
                for ( sal_uInt32 nDepth = m_nTableDepth + 1; nDepth <= pTextNodeInfo->getDepth(); ++nDepth )
                {
                    ww8::WW8TableNodeInfoInner::Pointer_t pInner( pTextNodeInfo->getInnerForDepth( nDepth ) );

                    m_bLastTable = (nDepth == pTextNodeInfo->getDepth());
                    StartTable( pInner );
                    StartTableRow( pInner );
                    StartTableCell( pInner );
                }

                m_nTableDepth = nCurrentDepth;
            }
        }
    }

    OSL_ENSURE(m_aRun.getLength() == 0, "m_aRun is not empty");
}

void RtfAttributeOutput::EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    FinishTableRowCell( pTextNodeInfoInner );

    RtfStringBuffer aParagraph;

    aParagraph.appendAndClear(m_aRun);
    aParagraph->append(m_aAfterRuns.makeStringAndClear());
    if (m_bTblAfterCell)
        m_bTblAfterCell = false;
    else
    {
        aParagraph->append(m_rExport.sNewLine);
        aParagraph->append(OOO_STRING_SVTOOLS_RTF_PAR);
        aParagraph->append(' ');
    }
    if (m_nColBreakNeeded)
    {
        aParagraph->append(OOO_STRING_SVTOOLS_RTF_COLUMN);
        m_nColBreakNeeded = false;
    }

    if (!m_bBufferSectionHeaders)
        aParagraph.makeStringAndClear(this);
    else
        m_aSectionHeaders.append(aParagraph.makeStringAndClear());
}

void RtfAttributeOutput::EmptyParagraph()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << m_rExport.sNewLine << OOO_STRING_SVTOOLS_RTF_PAR << ' ';
}

void RtfAttributeOutput::StartParagraphProperties( const SwTxtNode& rNode )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    OSL_ENSURE(m_aStyles.getLength() == 0, "m_aStyles is not empty");

    // output page/section breaks
    SwNodeIndex aNextIndex( rNode, 1 );
    m_rExport.Strm() << m_aSectionBreaks.makeStringAndClear().getStr();
    m_bBufferSectionBreaks = true;

    // output section headers / footers
    if (!m_bBufferSectionHeaders)
        m_rExport.Strm() << m_aSectionHeaders.makeStringAndClear().getStr();

    if ( aNextIndex.GetNode().IsTxtNode() )
    {
        const SwTxtNode* pTxtNode = static_cast< SwTxtNode* >( &aNextIndex.GetNode() );
        m_rExport.OutputSectionBreaks( pTxtNode->GetpSwAttrSet(), *pTxtNode );
        // Save the current page description for now, so later we will be able to access the previous one.
        m_pPrevPageDesc = pTxtNode->FindPageDesc(sal_False);
    }
    else if ( aNextIndex.GetNode().IsTableNode() )
    {
        const SwTableNode* pTableNode = static_cast< SwTableNode* >( &aNextIndex.GetNode() );
        const SwFrmFmt *pFmt = pTableNode->GetTable().GetFrmFmt();
        m_rExport.OutputSectionBreaks( &(pFmt->GetAttrSet()), *pTableNode );
    }
    m_bBufferSectionBreaks = false;

    OStringBuffer aPar;
    if (!m_rExport.bRTFFlySyntax)
    {
        aPar.append(OOO_STRING_SVTOOLS_RTF_PARD);
        aPar.append(OOO_STRING_SVTOOLS_RTF_PLAIN);
        aPar.append(' ');
    }
    if (!m_bBufferSectionHeaders)
        m_rExport.Strm() << aPar.makeStringAndClear().getStr();
    else
        m_aSectionHeaders.append(aPar.makeStringAndClear());
}

void RtfAttributeOutput::EndParagraphProperties()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_aStyles.append(m_aStylesEnd.makeStringAndClear());
    m_rExport.Strm() << m_aStyles.makeStringAndClear().getStr();
}

void RtfAttributeOutput::StartRun( const SwRedlineData* pRedlineData, bool bSingleEmptyRun )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", bSingleEmptyRun: " << bSingleEmptyRun);

    m_bInRun = true;
    m_bSingleEmptyRun = bSingleEmptyRun;
    if (!m_bSingleEmptyRun)
        m_aRun->append('{');

    // if there is some redlining in the document, output it
    Redline( pRedlineData );

    OSL_ENSURE(m_aRunText.getLength() == 0, "m_aRunText is not empty");
}

void RtfAttributeOutput::EndRun()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_aRun->append(m_rExport.sNewLine);
    m_aRun.appendAndClear(m_aRunText);
    if (!m_bSingleEmptyRun && m_bInRun)
        m_aRun->append('}');
    m_bInRun = false;
}

void RtfAttributeOutput::StartRunProperties()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    OSL_ENSURE(m_aStyles.getLength() == 0, "m_aStyles is not empty");
}

void RtfAttributeOutput::EndRunProperties( const SwRedlineData* /*pRedlineData*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_aStyles.append(m_aStylesEnd.makeStringAndClear());
    m_aRun->append(m_aStyles.makeStringAndClear());
}

void RtfAttributeOutput::RunText( const OUString& rText, rtl_TextEncoding /*eCharSet*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", rText: " << rText);
    RawText( rText, 0, m_rExport.eCurrentEncoding );
}

OStringBuffer& RtfAttributeOutput::RunText()
{
    return m_aRunText.getLastBuffer();
}

OStringBuffer& RtfAttributeOutput::Styles()
{
    return m_aStyles;
}

void RtfAttributeOutput::RawText( const String& rText, bool /*bForceUnicode*/, rtl_TextEncoding eCharSet )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_aRunText->append(msfilter::rtfutil::OutString(rText, eCharSet));
}

void RtfAttributeOutput::StartRuby( const SwTxtNode& /*rNode*/, xub_StrLen /*nPos*/, const SwFmtRuby& /*rRuby*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::EndRuby()
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

bool RtfAttributeOutput::StartURL( const String& rUrl, const String& rTarget )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append('{');
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_FIELD);
    m_aStyles.append('{');
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_IGNORE);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_FLDINST);
    m_aStyles.append(" HYPERLINK ");

    String sURL( rUrl );
    if( sURL.Len() )
    {
        m_aStyles.append("\"");
        m_aStyles.append(msfilter::rtfutil::OutString( sURL, m_rExport.eCurrentEncoding));
        m_aStyles.append("\" ");
    }

    if( rTarget.Len() )
    {
        m_aStyles.append("\\\\t \"");
        m_aStyles.append(msfilter::rtfutil::OutString( rTarget, m_rExport.eCurrentEncoding));
        m_aStyles.append("\" ");
    }

    m_aStyles.append("}");
    m_bHadFieldResult = false;
    return true;
}

bool RtfAttributeOutput::EndURL()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // close the fldrslt group
    if (m_bHadFieldResult)
        m_aRunText->append('}');
    // close the field group
    m_aRunText->append('}');
    return true;
}

void RtfAttributeOutput::FieldVanish( const String& /*rTxt*/, ww::eField /*eType*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::Redline( const SwRedlineData* pRedline )
{
    if (!pRedline)
        return;

    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (pRedline->GetType() == nsRedlineType_t::REDLINE_INSERT)
    {
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVISED);
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVAUTH);
        m_aRun->append((sal_Int32)m_rExport.GetRedline(SW_MOD()->GetRedlineAuthor(pRedline->GetAuthor())));
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVDTTM);
    }
    else if(pRedline->GetType() == nsRedlineType_t::REDLINE_DELETE)
    {
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_DELETED);
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVAUTHDEL);
        m_aRun->append((sal_Int32)m_rExport.GetRedline(SW_MOD()->GetRedlineAuthor(pRedline->GetAuthor())));
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_REVDTTMDEL);
    }
    m_aRun->append((sal_Int32)sw::ms::DateTime2DTTM(pRedline->GetTimeStamp()));
    m_aRun->append(' ');
}

void RtfAttributeOutput::FormatDrop( const SwTxtNode& /*rNode*/, const SwFmtDrop& /*rSwFmtDrop*/, sal_uInt16 /*nStyle*/, ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/, ww8::WW8TableNodeInfoInner::Pointer_t /*pTextNodeInfoInner*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::ParagraphStyle( sal_uInt16 nStyle )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    OString *pStyle = m_rExport.GetStyle(nStyle);
    OStringBuffer aStyle;
    aStyle.append(OOO_STRING_SVTOOLS_RTF_S);
    aStyle.append((sal_Int32)nStyle);
    if (pStyle)
        aStyle.append(pStyle->getStr());
    if (!m_bBufferSectionHeaders)
        m_rExport.Strm() << aStyle.makeStringAndClear().getStr();
    else
        m_aSectionHeaders.append(aStyle.makeStringAndClear());
}

void RtfAttributeOutput::TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_INTBL);
    if ( m_nTableDepth > 1 )
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ITAP);
        m_aStyles.append((sal_Int32)m_nTableDepth);
    }
    m_bWroteCellInfo = true;
}

void RtfAttributeOutput::TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfo*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* noop */
}

void RtfAttributeOutput::TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( !m_pTableWrt )
        InitTableHelper( pTableTextNodeInfoInner );

    const SwTable *pTbl = pTableTextNodeInfoInner->getTable();
    SwFrmFmt *pFmt = pTbl->GetFrmFmt( );

    m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_TROWD);
    TableOrientation( pTableTextNodeInfoInner );
    TableBidi( pTableTextNodeInfoInner );
    TableHeight( pTableTextNodeInfoInner );
    TableCanSplit( pTableTextNodeInfoInner );

    // Cell margins
    const SvxBoxItem& rBox = pFmt->GetBox( );
    static const sal_uInt16 aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };

    static const char* aRowPadNames[] =
    {
        OOO_STRING_SVTOOLS_RTF_TRPADDT, OOO_STRING_SVTOOLS_RTF_TRPADDL, OOO_STRING_SVTOOLS_RTF_TRPADDB, OOO_STRING_SVTOOLS_RTF_TRPADDR
    };

    static const char* aRowPadUnits[] =
    {
        OOO_STRING_SVTOOLS_RTF_TRPADDFT, OOO_STRING_SVTOOLS_RTF_TRPADDFL, OOO_STRING_SVTOOLS_RTF_TRPADDFB, OOO_STRING_SVTOOLS_RTF_TRPADDFR
    };

    for (int i = 0; i < 4; ++i)
    {
        m_aRowDefs.append(aRowPadUnits[i]);
        m_aRowDefs.append((sal_Int32)3);
        m_aRowDefs.append(aRowPadNames[i]);
        m_aRowDefs.append((sal_Int32)rBox.GetDistance(aBorders[i]));
    }

    // The cell-dependent properties
    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    SwTwips nSz = 0;
    Point aPt;
    SwRect aRect( pFmt->FindLayoutRect( false, &aPt ));
    SwTwips nPageSize = aRect.Width();

    // Handle the page size when not rendered
    if( 0 == nPageSize )
    {
        const SwNode* pNode = pTableTextNodeInfoInner->getNode();
        const SwFrmFmt* pFrmFmt = GetExport().mpParentFrame ? &GetExport().mpParentFrame->GetFrmFmt() :
            GetExport().pDoc->GetPageDesc(0).GetPageFmtOfNode(*pNode, false);

        const SvxLRSpaceItem& rLR = pFrmFmt->GetLRSpace();
        nPageSize = pFrmFmt->GetFrmSize().GetWidth() -
                        rLR.GetLeft() - rLR.GetRight();
    }
    SwTwips nTblSz = pFmt->GetFrmSize().GetWidth();
    // Not using m_nTableDepth, which is not yet incremented here.
    sal_uInt32 nCurrentDepth = pTableTextNodeInfoInner->getDepth();
    m_aCells[nCurrentDepth] = pRow->GetCells().size();
    for( sal_uInt16 i = 0; i < m_aCells[nCurrentDepth]; i++ )
    {
        const SwWriteTableCell *pCell = &pRow->GetCells( )[ i ];
        const SwFrmFmt *pCellFmt = pCell->GetBox()->GetFrmFmt();

        pTableTextNodeInfoInner->setCell( i );
        TableCellProperties(pTableTextNodeInfoInner);

        // Right boundary: this can't be in TableCellProperties as the old
        // value of nSz is needed.
        nSz += pCellFmt->GetFrmSize().GetWidth();
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CELLX);
        SwTwips nCalc = nSz;
        nCalc *= nPageSize;
        nCalc /= nTblSz;
        m_aRowDefs.append( (sal_Int32)(pFmt->GetLRSpace().GetLeft() + nCalc) );
    }
}

void RtfAttributeOutput::TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /*
     * The function name is a bit misleading: given that we write borders
     * before each row, we just have borders, not default ones. Additionally,
     * this function actually writes borders for a specific cell only and is
     * called for each cell.
     */

    const SwTableBox *pTblBox = pTableTextNodeInfoInner->getTableBox( );
    SwFrmFmt *pFmt = pTblBox->GetFrmFmt( );
    const SvxBoxItem& rDefault = pFmt->GetBox( );
    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    const SwWriteTableCell *pCell = &pRow->GetCells( )[ pTableTextNodeInfoInner->getCell( ) ];
    const SwFrmFmt *pCellFmt = pCell->GetBox()->GetFrmFmt();
    const SfxPoolItem* pItem;
    if (pCellFmt->GetAttrSet().HasItem(RES_BOX, &pItem))
    {
        const SvxBoxItem& rBox = (SvxBoxItem&)*pItem;
        static const sal_uInt16 aBorders[] =
        {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
        };
        static const char* aBorderNames[] =
        {
            OOO_STRING_SVTOOLS_RTF_CLBRDRT, OOO_STRING_SVTOOLS_RTF_CLBRDRL, OOO_STRING_SVTOOLS_RTF_CLBRDRB, OOO_STRING_SVTOOLS_RTF_CLBRDRR
        };
        //Yes left and top are swapped with eachother for cell padding! Because
        //that's what the thunderingly annoying rtf export/import word xp does.
        static const char* aCellPadNames[] =
        {
            OOO_STRING_SVTOOLS_RTF_CLPADL, OOO_STRING_SVTOOLS_RTF_CLPADT, OOO_STRING_SVTOOLS_RTF_CLPADB, OOO_STRING_SVTOOLS_RTF_CLPADR
        };
        static const char* aCellPadUnits[] =
        {
            OOO_STRING_SVTOOLS_RTF_CLPADFL, OOO_STRING_SVTOOLS_RTF_CLPADFT, OOO_STRING_SVTOOLS_RTF_CLPADFB, OOO_STRING_SVTOOLS_RTF_CLPADFR
        };
        for (int i = 0; i < 4; ++i)
        {
            if (const SvxBorderLine* pLn = rBox.GetLine(aBorders[i]))
                m_aRowDefs.append(OutTBLBorderLine(m_rExport, pLn, aBorderNames[i]));
            if (rDefault.GetDistance(aBorders[i]) !=
                    rBox.GetDistance(aBorders[i]))
            {
                m_aRowDefs.append(aCellPadUnits[i]);
                m_aRowDefs.append((sal_Int32)3);
                m_aRowDefs.append(aCellPadNames[i]);
                m_aRowDefs.append((sal_Int32)rBox.GetDistance(aBorders[i]));
            }
        }
    }
}

void RtfAttributeOutput::TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    const SwWriteTableCell *pCell = &pRow->GetCells( )[ pTableTextNodeInfoInner->getCell( ) ];
    const SwFrmFmt *pCellFmt = pCell->GetBox()->GetFrmFmt();
    const SfxPoolItem* pItem;
    if (pCellFmt->GetAttrSet().HasItem(RES_BACKGROUND, &pItem))
    {
        const SvxBrushItem& rBack = (SvxBrushItem&)*pItem;
        if( !rBack.GetColor().GetTransparency() )
        {
            m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLCBPAT);
            m_aRowDefs.append((sal_Int32)m_rExport.GetColor(rBack.GetColor()));
        }
    }
}

void RtfAttributeOutput::TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrmFmt * pLineFmt = pTabLine->GetFrmFmt();
    const SwFmtFrmSize& rLSz = pLineFmt->GetFrmSize();

    if ( ATT_VAR_SIZE != rLSz.GetHeightSizeType() && rLSz.GetHeight() )
    {
        sal_Int32 nHeight = 0;

        switch ( rLSz.GetHeightSizeType() )
        {
            case ATT_FIX_SIZE: nHeight = -rLSz.GetHeight(); break;
            case ATT_MIN_SIZE: nHeight = rLSz.GetHeight(); break;
            default:           break;
        }

        if ( nHeight )
        {
            m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_TRRH);
            m_aRowDefs.append(nHeight);
        }
    }
}

void RtfAttributeOutput::TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwTableBox * pTabBox = pTableTextNodeInfoInner->getTableBox();
    const SwTableLine * pTabLine = pTabBox->GetUpper();
    const SwFrmFmt * pLineFmt = pTabLine->GetFrmFmt();
    const SwFmtRowSplit& rSplittable = pLineFmt->GetRowSplit( );

    // The rtf default is to allow a row to break
    if (rSplittable.GetValue() == 0)
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_TRKEEP);
}

void RtfAttributeOutput::TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwTable * pTable = pTableTextNodeInfoInner->getTable();
    const SwFrmFmt * pFrmFmt = pTable->GetFrmFmt();

    if ( m_rExport.TrueFrameDirection( *pFrmFmt ) != FRMDIR_HORI_RIGHT_TOP )
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_LTRROW);
    else
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_RTLROW);
}

void RtfAttributeOutput::TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwWriteTableRows& aRows = m_pTableWrt->GetRows( );
    SwWriteTableRow *pRow = aRows[ pTableTextNodeInfoInner->getRow( ) ];
    const SwWriteTableCell *pCell = &pRow->GetCells( )[ pTableTextNodeInfoInner->getCell( ) ];
    const SwFrmFmt *pCellFmt = pCell->GetBox()->GetFrmFmt();
    const SfxPoolItem* pItem;

    // vertical merges
    if (pCell->GetRowSpan() > 1)
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVMGF);
    else if (pCell->GetRowSpan() == 0)
        m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVMRG);

    // vertical alignment
    if (pCellFmt->GetAttrSet().HasItem(RES_VERT_ORIENT, &pItem))
        switch( ((SwFmtVertOrient*)pItem)->GetVertOrient() )
        {
            case text::VertOrientation::CENTER: m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVERTALC); break;
            case text::VertOrientation::BOTTOM: m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVERTALB); break;
            default:                            m_aRowDefs.append(OOO_STRING_SVTOOLS_RTF_CLVERTALT); break;
        }
}

void RtfAttributeOutput::TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t /*pNodeInfo*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* noop */
}

void RtfAttributeOutput::TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // This is called when the nested table ends in a cell, and there's no
    // paragraph benhind that; so we must check for the ends of cell, rows,
    // and tables
    // ['true' to write an empty paragraph, MS Word insists on that]
    FinishTableRowCell( pNodeInfoInner, true );
}

void RtfAttributeOutput::TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwTable *pTable = pTableTextNodeInfoInner->getTable();
    SwFrmFmt *pFmt = pTable->GetFrmFmt( );

    OStringBuffer aTblAdjust( OOO_STRING_SVTOOLS_RTF_TRQL );
    switch (pFmt->GetHoriOrient().GetHoriOrient())
    {
        case text::HoriOrientation::CENTER:
            aTblAdjust.setLength(0);
            aTblAdjust.append(OOO_STRING_SVTOOLS_RTF_TRQC);
            break;
        case text::HoriOrientation::RIGHT:
            aTblAdjust.setLength(0);
            aTblAdjust.append(OOO_STRING_SVTOOLS_RTF_TRQR);
            break;
        case text::HoriOrientation::NONE:
        case text::HoriOrientation::LEFT_AND_WIDTH:
            aTblAdjust.append(OOO_STRING_SVTOOLS_RTF_TRLEFT);
            aTblAdjust.append((sal_Int32)pFmt->GetLRSpace().GetLeft());
            break;
        default:
            break;
    }

    m_aRowDefs.append(aTblAdjust.makeStringAndClear());
}

void RtfAttributeOutput::TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::TableRowEnd( sal_uInt32 /*nDepth*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* noop, see EndTableRow() */
}

/*
 * Our private table methods.
 */

void RtfAttributeOutput::InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    sal_uInt32 nPageSize = 0;
    bool bRelBoxSize = false;

    // Create the SwWriteTable instance to use col spans
    GetTablePageSize( pTableTextNodeInfoInner.get(), nPageSize, bRelBoxSize );

    const SwTable* pTable = pTableTextNodeInfoInner->getTable( );
    const SwFrmFmt *pFmt = pTable->GetFrmFmt( );
    SwTwips nTblSz = pFmt->GetFrmSize( ).GetWidth( );

    const SwHTMLTableLayout *pLayout = pTable->GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        m_pTableWrt = new SwWriteTable( pLayout );
    else
        m_pTableWrt = new SwWriteTable( pTable->GetTabLines(), (sal_uInt16)nPageSize,
                (sal_uInt16)nTblSz, false);
}

void RtfAttributeOutput::StartTable( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // To trigger calling InitTableHelper()
    delete m_pTableWrt, m_pTableWrt = NULL;
}

void RtfAttributeOutput::StartTableRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    sal_uInt32 nCurrentDepth = pTableTextNodeInfoInner->getDepth();
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", (depth is " << nCurrentDepth << ")");

    TableDefinition(pTableTextNodeInfoInner);

    SvtMiscOptions aMiscOptions;
    if (aMiscOptions.IsExperimentalMode())
    {
        m_aTables.push_back(m_aRowDefs.toString());
        // Emit row properties at the start of the row as well for non-nested
        // tables, to support old readers.
        if ( nCurrentDepth <= 1 )
            m_rExport.Strm() << m_aRowDefs.makeStringAndClear().getStr();
        m_aRowDefs.setLength(0);
        return;
    }
    else
    {
    if (!m_bLastTable)
        m_aTables.push_back(m_aRowDefs.makeStringAndClear());

    // We'll write the table definition for nested tables later
    if ( nCurrentDepth > 1 )
        return;
    // Empty the previous row closing buffer before starting the new one,
    // necessary for subtables.
    m_rExport.Strm() << m_aAfterRuns.makeStringAndClear().getStr();
    m_rExport.Strm() << m_aRowDefs.makeStringAndClear().getStr();
    }
}

void RtfAttributeOutput::StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_bTableCellOpen = true;
}

void RtfAttributeOutput::TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    TableDefaultBorders(pTableTextNodeInfoInner);
    TableBackgrounds(pTableTextNodeInfoInner);
    TableVerticalCell(pTableTextNodeInfoInner);
}

void RtfAttributeOutput::EndTableCell( )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", (depth is " << m_nTableDepth << ")");

    if (!m_bWroteCellInfo)
    {
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_INTBL);
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_ITAP);
        m_aAfterRuns.append((sal_Int32)m_nTableDepth);
    }
    if ( m_nTableDepth > 1 )
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_NESTCELL);
    else
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_CELL);

    m_bTableCellOpen = false;
    m_bTblAfterCell = true;
    m_bWroteCellInfo = false;
    if (m_aCells[m_nTableDepth] > 0)
        m_aCells[m_nTableDepth]--;
}

void RtfAttributeOutput::EndTableRow( )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", (depth is " << m_nTableDepth << ")");

    // Trying to end the row without writing the required number of cells? Fill with empty ones.
    for( sal_uInt16 i = 0; i < m_aCells[m_nTableDepth]; i++ )
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_CELL);

    if ( m_nTableDepth > 1 )
    {
        m_aAfterRuns.append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_NESTTABLEPROPRS);
        if (!m_aRowDefs.isEmpty())
            m_aAfterRuns.append(m_aRowDefs.makeStringAndClear());
        else if (!m_aTables.empty())
        {
            m_aAfterRuns.append(m_aTables.back());
            m_aTables.pop_back();
        }
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_NESTROW "}" "{" OOO_STRING_SVTOOLS_RTF_NONESTTABLES OOO_STRING_SVTOOLS_RTF_PAR "}");
    }
    else
    {
        if (!m_aTables.empty())
        {
            m_aAfterRuns.append(m_aTables.back());
            m_aTables.pop_back();
        }
        m_aAfterRuns.append(OOO_STRING_SVTOOLS_RTF_ROW).append(OOO_STRING_SVTOOLS_RTF_PARD);
    }
    m_bTableRowEnded = true;
}

void RtfAttributeOutput::EndTable()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( m_nTableDepth > 0 ) {
        m_nTableDepth--;
        delete m_pTableWrt, m_pTableWrt = NULL;
    }

    // We closed the table; if it is a nested table, the cell that contains it
    // still continues
    m_bTableCellOpen = true;

    // Cleans the table helper
    delete m_pTableWrt, m_pTableWrt = NULL;
}

void RtfAttributeOutput::FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t pInner, bool /*bForceEmptyParagraph*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( pInner.get() )
    {
        // Where are we in the table
        sal_uInt32 nRow = pInner->getRow( );

        const SwTable *pTable = pInner->getTable( );
        const SwTableLines& rLines = pTable->GetTabLines( );
        sal_uInt16 nLinesCount = rLines.size( );

        if ( pInner->isEndOfCell() )
            EndTableCell();

        // This is a line end
        if ( pInner->isEndOfLine() )
            EndTableRow();

        // This is the end of the table
        if ( pInner->isEndOfLine( ) && ( nRow + 1 ) == nLinesCount )
            EndTable();
    }
}

void RtfAttributeOutput::StartStyles()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_rExport.Strm() << m_rExport.sNewLine << '{' << OOO_STRING_SVTOOLS_RTF_COLORTBL;
    m_rExport.OutColorTable();
    OSL_ENSURE(m_aStylesheet.getLength() == 0, "m_aStylesheet is not empty");
    m_aStylesheet.append(m_rExport.sNewLine);
    m_aStylesheet.append('{');
    m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_STYLESHEET);
}

void RtfAttributeOutput::EndStyles( sal_uInt16 /*nNumberOfStyles*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_rExport.Strm() << '}';
    m_rExport.Strm() << m_aStylesheet.makeStringAndClear().getStr();
    m_rExport.Strm() << '}';
}

void RtfAttributeOutput::DefaultStyle( sal_uInt16 /*nStyle*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /* noop, the default style is always 0 in RTF */
}

void RtfAttributeOutput::StartStyle( const String& rName, bool bPapFmt,
        sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 /*nWwId*/, sal_uInt16 nId,
        bool /* bAutoUpdate */ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", rName = '" << OUString(rName) << "'");

    m_aStylesheet.append('{');
    if (bPapFmt)
        m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_S);
    else
        m_aStylesheet.append( OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_CS);
    m_aStylesheet.append( (sal_Int32)nId );

    if ( nBase != 0x0FFF )
    {
        m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_SBASEDON);
        m_aStylesheet.append((sal_Int32)nBase);
    }

    m_aStylesheet.append(OOO_STRING_SVTOOLS_RTF_SNEXT);
    m_aStylesheet.append((sal_Int32)nNext);

    m_rStyleName = rName;
    m_nStyleId = nId;
}

void RtfAttributeOutput::EndStyle()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    m_aStyles.append(m_aStylesEnd.makeStringAndClear());
    OString aStyles = m_aStyles.makeStringAndClear();
    m_rExport.InsStyle(m_nStyleId, aStyles);
    m_aStylesheet.append(aStyles);
    m_aStylesheet.append(' ');
    m_aStylesheet.append(msfilter::rtfutil::OutString(m_rStyleName, m_rExport.eCurrentEncoding));
    m_aStylesheet.append(";}");
    m_aStylesheet.append(m_rExport.sNewLine);
}

void RtfAttributeOutput::StartStyleProperties( bool /*bParProp*/, sal_uInt16 /*nStyle*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    /* noop */
}

void RtfAttributeOutput::EndStyleProperties( bool /*bParProp*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    /* noop */
}

void RtfAttributeOutput::OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt& /*rNFmt*/, const SwFmt& /*rFmt*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( nLvl >= WW8ListManager::nMaxLevel )
        nLvl = WW8ListManager::nMaxLevel - 1;

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ILVL);
    m_aStyles.append((sal_Int32)nLvl);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_OUTLINELEVEL);
    m_aStyles.append((sal_Int32)nLvl);
}

void RtfAttributeOutput::PageBreakBefore( bool bBreak )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (bBreak)
    {
        m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_PAGEBB;
    }
}

void RtfAttributeOutput::SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    switch (nC)
    {
        case msword::ColumnBreak:
            m_nColBreakNeeded = true;
            break;
        case msword::PageBreak:
            if ( pSectionInfo )
                m_rExport.SectionProperties( *pSectionInfo );
            break;
    }
}

void RtfAttributeOutput::StartSection()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_SECT OOO_STRING_SVTOOLS_RTF_SECTD);
    if (!m_bBufferSectionBreaks)
        m_rExport.Strm() << m_aSectionBreaks.makeStringAndClear().getStr();
}

void RtfAttributeOutput::EndSection()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /*
     * noop, \sect must go to StartSection or Word won't notice multiple
     * columns...
     */
}

void RtfAttributeOutput::SectionFormProtection( bool bProtected )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_SECTUNLOCKED);
    m_aSectionBreaks.append((sal_Int32)!bProtected);
}

void RtfAttributeOutput::SectionLineNumbering( sal_uLong /*nRestartNo*/, const SwLineNumberInfo& rLnNumInfo )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LINEMOD;
    m_rExport.OutLong(rLnNumInfo.GetCountBy());
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LINEX;
    m_rExport.OutLong(rLnNumInfo.GetPosFromLeft());
    if (!rLnNumInfo.IsRestartEachPage())
        m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LINECONT;
}

void RtfAttributeOutput::SectionTitlePage()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    /*
     * noop, handled in RtfExport::WriteHeaderFooter()
     */
}

void RtfAttributeOutput::SectionPageBorders( const SwFrmFmt* pFmt, const SwFrmFmt* /*pFirstPageFmt*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SvxBoxItem& rBox = pFmt->GetBox();
    const SvxBorderLine *pLine = rBox.GetTop();
    if(pLine)
        m_aSectionBreaks.append(OutBorderLine( m_rExport, pLine,
                    OOO_STRING_SVTOOLS_RTF_PGBRDRT,
                    rBox.GetDistance(BOX_LINE_TOP) ));
    pLine = rBox.GetBottom();
    if(pLine)
        m_aSectionBreaks.append(OutBorderLine( m_rExport, pLine,
                    OOO_STRING_SVTOOLS_RTF_PGBRDRB,
                    rBox.GetDistance(BOX_LINE_BOTTOM) ));
    pLine = rBox.GetLeft();
    if(pLine)
        m_aSectionBreaks.append(OutBorderLine( m_rExport, pLine,
                    OOO_STRING_SVTOOLS_RTF_PGBRDRL,
                    rBox.GetDistance(BOX_LINE_LEFT) ));
    pLine = rBox.GetRight();
    if(pLine)
        m_aSectionBreaks.append(OutBorderLine( m_rExport, pLine,
                    OOO_STRING_SVTOOLS_RTF_PGBRDRR,
                    rBox.GetDistance(BOX_LINE_RIGHT) ));
}

void RtfAttributeOutput::SectionBiDi( bool bBiDi )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << (bBiDi ? OOO_STRING_SVTOOLS_RTF_RTLSECT : OOO_STRING_SVTOOLS_RTF_LTRSECT);
}

void RtfAttributeOutput::SectionPageNumbering( sal_uInt16 nNumType, sal_uInt16 nPageRestartNumber )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (nPageRestartNumber > 0)
    {
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGNSTARTS);
        m_aSectionBreaks.append((sal_Int32)nPageRestartNumber);
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGNRESTART);
    }

    const char* pStr = 0;
    switch ( nNumType )
    {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  pStr = OOO_STRING_SVTOOLS_RTF_PGNUCLTR; break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  pStr = OOO_STRING_SVTOOLS_RTF_PGNLCLTR; break;
        case SVX_NUM_ROMAN_UPPER:           pStr = OOO_STRING_SVTOOLS_RTF_PGNUCRM;  break;
        case SVX_NUM_ROMAN_LOWER:           pStr = OOO_STRING_SVTOOLS_RTF_PGNLCRM;  break;

        case SVX_NUM_ARABIC:                pStr = OOO_STRING_SVTOOLS_RTF_PGNDEC;     break;
    }
    if (pStr)
        m_aSectionBreaks.append(pStr);
}

void RtfAttributeOutput::SectionType( sal_uInt8 nBreakCode )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << ", nBreakCode = " << int(nBreakCode));

    /*
     * break code:   0 No break, 1 New column
     * 2 New page, 3 Even page, 4 Odd page
     */
    const char* sType = NULL;
    switch ( nBreakCode )
    {
        case 1:  sType = OOO_STRING_SVTOOLS_RTF_SBKCOL; break;
        case 2:  sType = OOO_STRING_SVTOOLS_RTF_SBKPAGE; break;
        case 3:  sType = OOO_STRING_SVTOOLS_RTF_SBKEVEN; break;
        case 4:  sType = OOO_STRING_SVTOOLS_RTF_SBKODD; break;
        default: sType = OOO_STRING_SVTOOLS_RTF_SBKNONE; break;
    }
    m_aSectionBreaks.append(sType);
    if (!m_bBufferSectionBreaks)
        m_rExport.Strm() << m_aSectionBreaks.makeStringAndClear().getStr();
}

void RtfAttributeOutput::NumberingDefinition( sal_uInt16 nId, const SwNumRule &/*rRule*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_LISTOVERRIDE;
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LISTID;
    m_rExport.OutULong(nId);
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LISTOVERRIDECOUNT << '0';
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LS;
    m_rExport.OutULong(nId) << '}';
}

void RtfAttributeOutput::StartAbstractNumbering( sal_uInt16 nId )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_LIST << OOO_STRING_SVTOOLS_RTF_LISTTEMPLATEID;
    m_rExport.OutULong( nId );
    m_nListId = nId;
}

void RtfAttributeOutput::EndAbstractNumbering()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LISTID;
    m_rExport.OutULong( m_nListId ) << '}' << m_rExport.sNewLine;
}

void RtfAttributeOutput::NumberingLevel( sal_uInt8 nLevel,
        sal_uInt16 nStart,
        sal_uInt16 nNumberingType,
        SvxAdjust eAdjust,
        const sal_uInt8 * pNumLvlPos,
        sal_uInt8 nFollow,
        const wwFont * pFont,
        const SfxItemSet * pOutSet,
        sal_Int16 nIndentAt,
        sal_Int16 nFirstLineIndex,
        sal_Int16 /*nListTabPos*/,
        const String &rNumberingString,
        const SvxBrushItem* pBrush)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << m_rExport.sNewLine;
    if( nLevel > 8 ) // RTF knows only 9 levels
        m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_SOUTLVL;

    m_rExport.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_LISTLEVEL;

    sal_uInt16 nVal = 0;
    switch( nNumberingType )
    {
        case SVX_NUM_ROMAN_UPPER:                   nVal = 1;       break;
        case SVX_NUM_ROMAN_LOWER:                   nVal = 2;       break;
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:  nVal = 3;       break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:  nVal = 4;       break;

        case SVX_NUM_BITMAP:
        case SVX_NUM_CHAR_SPECIAL:                  nVal = 23;      break;
        case SVX_NUM_NUMBER_NONE:                   nVal = 255;     break;
    }
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LEVELNFC;
    m_rExport.OutULong( nVal );

    switch( eAdjust )
    {
        case SVX_ADJUST_CENTER:             nVal = 1;       break;
        case SVX_ADJUST_RIGHT:              nVal = 2;       break;
        default:                            nVal = 0;       break;
    }
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LEVELJC;
    m_rExport.OutULong( nVal );

    // bullet
    if (nNumberingType == SVX_NUM_BITMAP && pBrush)
    {
        int nIndex = m_rExport.GetGrfIndex(*pBrush);
        if (nIndex != -1)
        {
            m_rExport.Strm() << LO_STRING_SVTOOLS_RTF_LEVELPICTURE;
            m_rExport.OutULong(nIndex);
        }
    }

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LEVELSTARTAT;
    m_rExport.OutULong( nStart );

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_LEVELFOLLOW;
    m_rExport.OutULong( nFollow );

    // leveltext group
    m_rExport.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_LEVELTEXT << ' ';

    if( SVX_NUM_CHAR_SPECIAL == nNumberingType ||
            SVX_NUM_BITMAP == nNumberingType )
    {
        m_rExport.Strm() << "\\'01";
        sal_Unicode cChar = rNumberingString.GetChar(0);
        m_rExport.Strm() << "\\u";
        m_rExport.OutULong(cChar);
        m_rExport.Strm() << " ?";
    }
    else
    {
        m_rExport.Strm() << "\\'" << msfilter::rtfutil::OutHex( rNumberingString.Len(), 2 ).getStr();
        m_rExport.Strm() << msfilter::rtfutil::OutString( rNumberingString, m_rExport.eDefaultEncoding, /*bUnicode =*/ false ).getStr();
    }

    m_rExport.Strm() << ";}";

    // write the levelnumbers
    m_rExport.Strm() << "{" << OOO_STRING_SVTOOLS_RTF_LEVELNUMBERS;
    for( sal_uInt8 i = 0; i <= nLevel && pNumLvlPos[ i ]; ++i )
    {
        m_rExport.Strm() << "\\'" << msfilter::rtfutil::OutHex(pNumLvlPos[ i ], 2).getStr();
    }
    m_rExport.Strm() << ";}";

    if( pOutSet )
    {
        if (pFont)
        {
            m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_F;
            m_rExport.OutULong(m_rExport.maFontHelper.GetId(*pFont));
        }
        m_rExport.OutputItemSet( *pOutSet, false, true, i18n::ScriptType::LATIN, m_rExport.mbExportModeRTF );
        m_rExport.Strm() << m_aStyles.makeStringAndClear().getStr();
    }

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_FI;
    m_rExport.OutLong( nFirstLineIndex ) << OOO_STRING_SVTOOLS_RTF_LI;
    m_rExport.OutLong( nIndentAt );

    m_rExport.Strm() << '}';
    if( nLevel > 8 )
        m_rExport.Strm() << '}';
}

void RtfAttributeOutput::WriteField_Impl( const SwField* pFld, ww::eField /*eType*/, const String& rFldCmd, sal_uInt8 /*nMode*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // If there are no field instructions, don't export it as a field.
    bool bHasInstructions = rFldCmd.Len() > 0;
    if (bHasInstructions)
    {
        m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD);
        m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST " ");
        m_aRunText->append(msfilter::rtfutil::OutString(rFldCmd, m_rExport.eCurrentEncoding));
        m_aRunText->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
    }
    if (pFld)
        m_aRunText->append(msfilter::rtfutil::OutString(pFld->ExpandField(true), m_rExport.eDefaultEncoding));
    if (bHasInstructions)
        m_aRunText->append("}}");
}

void RtfAttributeOutput::WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds )
{
    for ( std::vector< OUString >::const_iterator it = rStarts.begin(), end = rStarts.end(); it != end; ++it )
    {
        m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_BKMKSTART " ");
        m_aRun->append(msfilter::rtfutil::OutString(*it, m_rExport.eCurrentEncoding));
        m_aRun->append('}');
    }
    rStarts.clear();

    for ( std::vector< OUString >::const_iterator it = rEnds.begin(), end = rEnds.end(); it != end; ++it )
    {
        m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_BKMKEND " ");
        m_aRun->append(msfilter::rtfutil::OutString(*it, m_rExport.eCurrentEncoding));
        m_aRun->append('}');
    }
    rEnds.clear();
}

void RtfAttributeOutput::WriteHeaderFooter_Impl( const SwFrmFmt& rFmt, bool bHeader, const sal_Char* pStr, bool bTitlepg )
{
    OStringBuffer aSectionBreaks = m_aSectionBreaks;
    m_aSectionBreaks.setLength(0);
    RtfStringBuffer aRun = m_aRun;
    m_aRun.clear();

    m_aSectionHeaders.append(bHeader ? OOO_STRING_SVTOOLS_RTF_HEADERY : OOO_STRING_SVTOOLS_RTF_FOOTERY);
    m_aSectionHeaders.append((sal_Int32)m_rExport.pAktPageDesc->GetMaster().GetULSpace().GetUpper());
    if (bTitlepg)
        m_aSectionHeaders.append(OOO_STRING_SVTOOLS_RTF_TITLEPG);
    m_aSectionHeaders.append('{');
    m_aSectionHeaders.append(pStr);
    m_bBufferSectionHeaders = true;
    m_rExport.WriteHeaderFooterText(rFmt, bHeader);
    m_bBufferSectionHeaders = false;
    m_aSectionHeaders.append('}');

    m_aSectionBreaks = aSectionBreaks;
    m_aRun = aRun;
}

void lcl_TextFrameShadow(std::vector< std::pair<OString, OString> >& rFlyProperties, const SwFrmFmt& rFrmFmt)
{
    SvxShadowItem aShadowItem = rFrmFmt.GetShadow();
    if (aShadowItem.GetLocation() == SVX_SHADOW_NONE)
        return;

    rFlyProperties.push_back(std::make_pair<OString, OString>("fShadow", OString::number(1)));

    const Color& rColor = aShadowItem.GetColor();
    // We in fact need RGB to BGR, but the transformation is symmetric.
    rFlyProperties.push_back(std::make_pair<OString, OString>("shadowColor", OString::number(msfilter::util::BGRToRGB(rColor.GetColor()))));

    // Twips -> points -> EMUs -- hacky, the intermediate step hides rounding errors on roundtrip.
    OString aShadowWidth = OString::number(sal_Int32(aShadowItem.GetWidth() / 20) * 12700);
    OString aOffsetX;
    OString aOffsetY;
    switch (aShadowItem.GetLocation())
    {
        case SVX_SHADOW_TOPLEFT: aOffsetX = "-" + aShadowWidth; aOffsetY = "-" + aShadowWidth; break;
        case SVX_SHADOW_TOPRIGHT: aOffsetX = aShadowWidth; aOffsetY = "-" + aShadowWidth; break;
        case SVX_SHADOW_BOTTOMLEFT: aOffsetX = "-" + aShadowWidth; aOffsetY = aShadowWidth; break;
        case SVX_SHADOW_BOTTOMRIGHT: aOffsetX = aShadowWidth; aOffsetY = aShadowWidth; break;
        case SVX_SHADOW_NONE:
        case SVX_SHADOW_END:
            break;
    }
    if (!aOffsetX.isEmpty())
        rFlyProperties.push_back(std::make_pair<OString, OString>("shadowOffsetX", OString(aOffsetX)));
    if (!aOffsetY.isEmpty())
        rFlyProperties.push_back(std::make_pair<OString, OString>("shadowOffsetY", OString(aOffsetY)));
}

void RtfAttributeOutput::OutputFlyFrame_Impl( const sw::Frame& rFrame, const Point& /*rNdTopLeft*/ )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwNode *pNode = rFrame.GetContent();
    const SwGrfNode *pGrfNode = pNode ? pNode->GetGrfNode() : 0;

    switch ( rFrame.GetWriterType() )
    {
        case sw::Frame::eTxtBox:
            {
            OSL_ENSURE(m_aRunText.getLength() == 0, "m_aRunText is not empty");
            m_rExport.mpParentFrame = &rFrame;

            m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_SHP;
            m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPINST;

            // Shape properties.
            m_aFlyProperties.push_back(std::make_pair<OString, OString>("shapeType", OString::number(ESCHER_ShpInst_TextBox)));

            // When a frame has some low height, but automatically expanded due
            // to lots of contents, this size contains the real size.
            const Size aSize = rFrame.GetSize();
            m_pFlyFrameSize = &aSize;

            m_rExport.bOutFlyFrmAttrs = m_rExport.bRTFFlySyntax = true;
            m_rExport.OutputFormat( rFrame.GetFrmFmt(), false, false, true );
            m_rExport.Strm() << m_aRunText.makeStringAndClear().getStr();
            m_rExport.Strm() << m_aStyles.makeStringAndClear().getStr();
            m_rExport.bOutFlyFrmAttrs = m_rExport.bRTFFlySyntax = false;
            m_pFlyFrameSize = 0;

            const SwFrmFmt& rFrmFmt = rFrame.GetFrmFmt();
            lcl_TextFrameShadow(m_aFlyProperties, rFrmFmt);

            for (size_t i = 0; i < m_aFlyProperties.size(); ++i)
            {
                m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_SP "{";
                m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SN " ";
                m_rExport.Strm() << m_aFlyProperties[i].first.getStr();
                m_rExport.Strm() << "}{" OOO_STRING_SVTOOLS_RTF_SV " ";
                m_rExport.Strm() << m_aFlyProperties[i].second.getStr();
                m_rExport.Strm() << "}}";
            }
            m_aFlyProperties.clear();

            m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_SHPTXT;

            {
                /*
                 * Save m_aRun as we should not loose the opening brace.
                 * OTOH, just drop the contents of m_aRunText in case something
                 * would be there, causing a problem later.
                 */
                OString aSave = m_aRun.makeStringAndClear();
                // Also back m_bInRun and m_bSingleEmptyRun up.
                bool bInRunOrig = m_bInRun;
                m_bInRun = false;
                bool bSingleEmptyRunOrig = m_bSingleEmptyRun;
                m_bSingleEmptyRun = false;
                m_rExport.bRTFFlySyntax = true;

                const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();
                sal_uLong nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
                sal_uLong nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;
                m_rExport.SaveData( nStt, nEnd );
                m_rExport.mpParentFrame = &rFrame;
                m_rExport.WriteText( );
                m_rExport.RestoreData();

                m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_PARD;
                m_rExport.bRTFFlySyntax = false;
                m_aRun->append(aSave);
                m_aRunText.clear();
                m_bInRun = bInRunOrig;
                m_bSingleEmptyRun = bSingleEmptyRunOrig;
            }

            m_rExport.mpParentFrame = NULL;

            m_rExport.Strm() << '}'; // shptxt
            m_rExport.Strm() << '}'; // shpinst
            m_rExport.Strm() << '}'; // shp

            m_rExport.Strm() << RtfExport::sNewLine;
            }
            break;
        case sw::Frame::eGraphic:
            if (!rFrame.IsInline())
            {
                m_rExport.mpParentFrame = &rFrame;
                m_rExport.bRTFFlySyntax = true;
                m_rExport.OutputFormat( rFrame.GetFrmFmt(), false, false, true );
                m_rExport.bRTFFlySyntax = false;
                m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE);
                m_rExport.OutputFormat( rFrame.GetFrmFmt(), false, false, true );
                m_aRunText->append('}');
                m_rExport.mpParentFrame = NULL;
            }

            if ( pGrfNode )
                m_aRunText.append(dynamic_cast<const SwFlyFrmFmt*>( &rFrame.GetFrmFmt() ), pGrfNode);
            break;
        case sw::Frame::eDrawing:
            {
                const SdrObject* pSdrObj = rFrame.GetFrmFmt().FindRealSdrObject();
                if ( pSdrObj )
                {
                    bool bSwapInPage = false;
                    if ( !pSdrObj->GetPage() )
                    {
                        if ( SdrModel* pModel = m_rExport.pDoc->GetDrawModel() )
                        {
                            if ( SdrPage *pPage = pModel->GetPage( 0 ) )
                            {
                                bSwapInPage = true;
                                const_cast< SdrObject* >( pSdrObj )->SetPage( pPage );
                            }
                        }
                    }

                    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD "{");
                    m_aRunText->append(OOO_STRING_SVTOOLS_RTF_IGNORE);
                    m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLDINST);
                    m_aRunText->append(" SHAPE ");
                    m_aRunText->append("}" "{" OOO_STRING_SVTOOLS_RTF_FLDRSLT);

                    m_rExport.SdrExporter().AddSdrObject( *pSdrObj );

                    m_aRunText->append('}');
                    m_aRunText->append('}');

                    if ( bSwapInPage )
                        const_cast< SdrObject* >( pSdrObj )->SetPage( 0 );
                }
            }
            break;
        case sw::Frame::eFormControl:
            {
                const SwFrmFmt &rFrmFmt = rFrame.GetFrmFmt();
                const SdrObject *pObject = rFrmFmt.FindRealSdrObject();

                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_FIELD);
                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST);

                if (pObject && pObject->GetObjInventor() == FmFormInventor)
                {
                    if (SdrUnoObj *pFormObj = PTR_CAST(SdrUnoObj,pObject))
                    {
                        uno::Reference< awt::XControlModel > xControlModel =
                            pFormObj->GetUnoControlModel();
                        uno::Reference< lang::XServiceInfo > xInfo(xControlModel, uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySet> xPropSet(xControlModel, uno::UNO_QUERY);
                        uno::Reference<beans::XPropertySetInfo> xPropSetInfo = xPropSet->getPropertySetInfo();
                        OUString sName;
                        if (xInfo->supportsService("com.sun.star.form.component.CheckBox"))
                        {

                            m_aRun->append(OUStringToOString(OUString(FieldString(ww::eFORMCHECKBOX)), m_rExport.eCurrentEncoding));
                            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD "{");
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFTYPE "1"); // 1 = checkbox
                            // checkbox size in half points, this seems to be always 20, see WW8Export::DoCheckBox()
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFHPS "20");

                            OUString aStr;
                            sName = "Name";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFNAME " ");
                                m_aRun->append(OUStringToOString(aStr, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }

                            sName = "HelpText";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ");
                                m_aRun->append(OUStringToOString(aStr, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }

                            sName = "HelpF1Text";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ");
                                m_aRun->append(OUStringToOString(aStr, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }

                            sal_Int16 nTemp = 0;
                            xPropSet->getPropertyValue("DefaultState") >>= nTemp;
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFDEFRES);
                            m_aRun->append((sal_Int32)nTemp);
                            xPropSet->getPropertyValue("State") >>= nTemp;
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFRES);
                            m_aRun->append((sal_Int32)nTemp);

                            m_aRun->append("}}");

                            // field result is empty, ffres already contains the form result
                            m_aRun->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
                        }
                        else if (xInfo->supportsService("com.sun.star.form.component.TextField"))
                        {
                            OStringBuffer aBuf;
                            OString aStr;
                            OUString aTmp;
                            const sal_Char* pStr;

                            m_aRun->append(OUStringToOString(OUString(FieldString(ww::eFORMTEXT)), m_rExport.eCurrentEncoding));
                            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_DATAFIELD " ");
                            for (int i = 0; i < 8; i++) aBuf.append((sal_Char)0x00);
                            xPropSet->getPropertyValue("Name") >>= aTmp;
                            aStr = OUStringToOString(aTmp, m_rExport.eCurrentEncoding);
                            aBuf.append((sal_Char)aStr.getLength());
                            aBuf.append(aStr);
                            aBuf.append((sal_Char)0x00);
                            xPropSet->getPropertyValue("DefaultText") >>= aTmp;
                            aStr = OUStringToOString(aTmp, m_rExport.eCurrentEncoding);
                            aBuf.append((sal_Char)aStr.getLength());
                            aBuf.append(aStr);
                            for (int i = 0; i < 11; i++) aBuf.append((sal_Char)0x00);
                            aStr = aBuf.makeStringAndClear();
                            pStr = aStr.getStr();
                            for (int i = 0; i < aStr.getLength(); i++, pStr++)
                                m_aRun->append(msfilter::rtfutil::OutHex(*pStr, 2));
                            m_aRun->append('}');
                            m_aRun->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
                            xPropSet->getPropertyValue("Text") >>= aTmp;
                            m_aRun->append(OUStringToOString(aTmp, m_rExport.eCurrentEncoding));
                            m_aRun->append('}');
                            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD "{");
                            sName = "HelpText";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aTmp;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ");
                                m_aRun->append(OUStringToOString(aTmp, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }

                            sName = "HelpF1Text";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aTmp;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ");
                                m_aRun->append(OUStringToOString(aTmp, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }
                            m_aRun->append("}");
                        }
                        else if (xInfo->supportsService("com.sun.star.form.component.ListBox"))
                        {
                            OUString aStr;
                            uno::Sequence<sal_Int16> aIntSeq;
                            uno::Sequence<OUString> aStrSeq;

                            m_aRun->append(OUStringToOString(OUString(FieldString(ww::eFORMDROPDOWN)), m_rExport.eCurrentEncoding));
                            m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FORMFIELD "{");
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFTYPE "2"); // 2 = list
                            m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFHASLISTBOX);

                            xPropSet->getPropertyValue("DefaultSelection") >>= aIntSeq;
                            if( aIntSeq.getLength() )
                            {
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFDEFRES);
                                // a dropdown list can have only one 'selected item by default'
                                m_aRun->append((sal_Int32)aIntSeq[0]);
                            }

                            xPropSet->getPropertyValue("SelectedItems") >>= aIntSeq;
                            if( aIntSeq.getLength() )
                            {
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFRES);
                                // a dropdown list can have only one 'currently selected item'
                                m_aRun->append((sal_Int32)aIntSeq[0]);
                            }

                            sName = "Name";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFNAME " ");
                                m_aRun->append(OUStringToOString(aStr, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }

                            sName = "HelpText";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNHELP);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFHELPTEXT " ");
                                m_aRun->append(OUStringToOString(aStr, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }

                            sName = "HelpF1Text";
                            if (xPropSetInfo->hasPropertyByName(sName))
                            {
                                xPropSet->getPropertyValue(sName) >>= aStr;
                                m_aRun->append(OOO_STRING_SVTOOLS_RTF_FFOWNSTAT);
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFSTATTEXT " ");
                                m_aRun->append(OUStringToOString(aStr, m_rExport.eCurrentEncoding));
                                m_aRun->append('}');
                            }


                            xPropSet->getPropertyValue("StringItemList") >>= aStrSeq;
                            sal_uInt32 nListItems = aStrSeq.getLength();
                            for (sal_uInt32 i = 0; i < nListItems; i++)
                                m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FFL " ")
                                    .append(OUStringToOString(aStrSeq[i], m_rExport.eCurrentEncoding)).append('}');

                            m_aRun->append("}}");

                            // field result is empty, ffres already contains the form result
                            m_aRun->append("}{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
                        }
                        else
                            SAL_INFO("sw.rtf", OSL_THIS_FUNC << " unhandled form control: '" << xInfo->getImplementationName()<< "'");
                        m_aRun->append('}');
                    }
                }

                m_aRun->append('}');
            }
            break;
        case sw::Frame::eOle:
            {
                const SwFrmFmt &rFrmFmt = rFrame.GetFrmFmt();
                const SdrObject *pSdrObj = rFrmFmt.FindRealSdrObject();
                if ( pSdrObj )
                {
                    SwNodeIndex aIdx(*rFrmFmt.GetCntnt().GetCntntIdx(), 1);
                    SwOLENode& rOLENd = *aIdx.GetNode().GetOLENode();
                    FlyFrameOLE(dynamic_cast<const SwFlyFrmFmt*>( &rFrmFmt ), rOLENd, rFrame.GetLayoutSize());
                }
            }
            break;
        default:
            SAL_INFO("sw.rtf", OSL_THIS_FUNC << ": unknown type (" << (int)rFrame.GetWriterType() << ")");
            break;
    }
}

void RtfAttributeOutput::CharCaseMap( const SvxCaseMapItem& rCaseMap )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    switch ( rCaseMap.GetValue() )
    {
        case SVX_CASEMAP_KAPITAELCHEN:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SCAPS);
            break;
        case SVX_CASEMAP_VERSALIEN:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CAPS);
            break;
        default: // Something that rtf does not support
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SCAPS);
            m_aStyles.append((sal_Int32)0);
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CAPS);
            m_aStyles.append((sal_Int32)0);
            break;
    }
}

void RtfAttributeOutput::CharColor( const SvxColorItem& rColor )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const Color aColor( rColor.GetValue() );

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CF);
    m_aStyles.append( (sal_Int32)m_rExport.GetColor( aColor ));
}

void RtfAttributeOutput::CharContour( const SvxContourItem& rContour )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_OUTL);
    if ( !rContour.GetValue() )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharCrossedOut( const SvxCrossedOutItem& rCrossedOut )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    switch ( rCrossedOut.GetStrikeout() )
    {
        case STRIKEOUT_NONE:
            if (!m_bStrikeDouble)
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKE);
            else
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKED);
            m_aStyles.append((sal_Int32)0);
            break;
        case STRIKEOUT_DOUBLE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKED);
            m_aStyles.append((sal_Int32)1);
            break;
        default:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_STRIKE);
            break;
    }
}

void RtfAttributeOutput::CharEscapement( const SvxEscapementItem& rEsc )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    short nEsc = rEsc.GetEsc();
    if (rEsc.GetProp() == DFLT_ESC_PROP)
    {
        if ( DFLT_ESC_SUB == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SUB);
        else if ( DFLT_ESC_SUPER == nEsc || DFLT_ESC_AUTO_SUPER == nEsc )
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SUPER);
        return;
    }

    const char * pUpDn;

    SwTwips nH = ((SvxFontHeightItem&)m_rExport.GetItem( RES_CHRATR_FONTSIZE )).GetHeight();

    if( 0 < rEsc.GetEsc() )
        pUpDn = OOO_STRING_SVTOOLS_RTF_UP;
    else if( 0 > rEsc.GetEsc() )
    {
        pUpDn = OOO_STRING_SVTOOLS_RTF_DN;
        nH = -nH;
    }
    else
        return;

    short nProp = rEsc.GetProp() * 100;
    if( DFLT_ESC_AUTO_SUPER == nEsc )
    {
        nEsc = 100 - rEsc.GetProp();
        ++nProp;
    }
    else if( DFLT_ESC_AUTO_SUB == nEsc )
    {
        nEsc = - 100 + rEsc.GetProp();
        ++nProp;
    }

    m_aStyles.append('{');
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_IGNORE);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_UPDNPROP);
    m_aStyles.append( (sal_Int32)nProp );
    m_aStyles.append('}');
    m_aStyles.append(pUpDn);

    /*
     * Calculate the act. FontSize and the percentage of the displacement;
     * RTF file expects half points, while internally it's in twips.
     * Formally :            (FontSize * 1/20 ) pts         x * 2
     *                    -----------------------  = ------------
     *                      100%                       Escapement
     */

    m_aStyles.append( (sal_Int32) ( (long( nEsc ) * nH) + 500L ) / 1000L );
    // 500L to round !!
}

void RtfAttributeOutput::CharFont( const SvxFontItem& rFont)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_LOCH);
    m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_F);
    m_aStylesEnd.append((sal_Int32)m_rExport.maFontHelper.GetId(rFont));
    m_rExport.eCurrentEncoding = rtl_getTextEncodingFromWindowsCharset(rtl_getBestWindowsCharsetFromTextEncoding(rFont.GetCharSet()));
}

void RtfAttributeOutput::CharFontSize( const SvxFontHeightItem& rFontSize)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    switch ( rFontSize.Which() )
    {
        case RES_CHRATR_FONTSIZE:
            m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_FS);
            m_aStylesEnd.append((sal_Int32)(rFontSize.GetHeight() / 10 ));
            break;
        case RES_CHRATR_CJK_FONTSIZE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_FS);
            m_aStyles.append((sal_Int32)(rFontSize.GetHeight() / 10 ));
            break;
        case RES_CHRATR_CTL_FONTSIZE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_AFS);
            m_aStyles.append((sal_Int32)(rFontSize.GetHeight() / 10 ));
            break;
    }
}

void RtfAttributeOutput::CharKerning( const SvxKerningItem& rKerning )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    // in quarter points then in twips
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_EXPND);
    m_aStyles.append((sal_Int32)(rKerning.GetValue() / 5));
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_EXPNDTW);
    m_aStyles.append((sal_Int32)(rKerning.GetValue()));
}

void RtfAttributeOutput::CharLanguage( const SvxLanguageItem& rLanguage )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    switch (rLanguage.Which())
    {
        case RES_CHRATR_LANGUAGE:
            m_aStylesEnd.append(OOO_STRING_SVTOOLS_RTF_LANG);
            m_aStylesEnd.append((sal_Int32)rLanguage.GetLanguage());
            break;
        case RES_CHRATR_CJK_LANGUAGE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LANGFE);
            m_aStyles.append((sal_Int32)rLanguage.GetLanguage());
            break;
        case RES_CHRATR_CTL_LANGUAGE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ALANG);
            m_aStyles.append((sal_Int32)rLanguage.GetLanguage());
            break;
    }
}

void RtfAttributeOutput::CharPosture( const SvxPostureItem& rPosture )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_I);
    if ( rPosture.GetPosture() == ITALIC_NONE )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharShadow( const SvxShadowedItem& rShadow )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SHAD);
    if ( !rShadow.GetValue() )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharUnderline( const SvxUnderlineItem& rUnderline )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const char* pStr = 0;
    const SfxPoolItem* pItem = m_rExport.HasItem( RES_CHRATR_WORDLINEMODE );
    bool bWord = false;
    if (pItem)
        bWord = ((const SvxWordLineModeItem*)pItem)->GetValue() ? true : false;
    switch(rUnderline.GetLineStyle() )
    {
        case UNDERLINE_SINGLE:
            pStr = bWord ? OOO_STRING_SVTOOLS_RTF_ULW : OOO_STRING_SVTOOLS_RTF_UL;
            break;
        case UNDERLINE_DOUBLE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDB;
            break;
        case UNDERLINE_NONE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULNONE;
            break;
        case UNDERLINE_DOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_ULD;
            break;
        case UNDERLINE_DASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASH;
            break;
        case UNDERLINE_DASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASHD;
            break;
        case UNDERLINE_DASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULDASHDD;
            break;
        case UNDERLINE_BOLD:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTH;
            break;
        case UNDERLINE_WAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULWAVE;
            break;
        case UNDERLINE_BOLDDOTTED:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHD;
            break;
        case UNDERLINE_BOLDDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASH;
            break;
        case UNDERLINE_LONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULLDASH;
            break;
        case UNDERLINE_BOLDLONGDASH:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHLDASH;
            break;
        case UNDERLINE_BOLDDASHDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASHD;
            break;
        case UNDERLINE_BOLDDASHDOTDOT:
            pStr = OOO_STRING_SVTOOLS_RTF_ULTHDASHDD;
            break;
        case UNDERLINE_BOLDWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULHWAVE;
            break;
        case UNDERLINE_DOUBLEWAVE:
            pStr = OOO_STRING_SVTOOLS_RTF_ULULDBWAVE;
            break;
        default:
            break;
    }

    if( pStr )
    {
        m_aStyles.append(pStr);
        // NEEDSWORK looks like here rUnderline.GetColor() is always black,
        // even if the color in the odt is for example green...
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ULC);
        m_aStyles.append( (sal_Int32)m_rExport.GetColor(rUnderline.GetColor()) );
    }
}

void RtfAttributeOutput::CharWeight( const SvxWeightItem& rWeight )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_B);
    if ( rWeight.GetWeight() != WEIGHT_BOLD )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharAutoKern( const SvxAutoKernItem& rAutoKern)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_KERNING);
    m_aStyles.append((sal_Int32) (rAutoKern.GetValue() ? 1 : 0));
}

void RtfAttributeOutput::CharAnimatedText( const SvxBlinkItem& rBlink )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ANIMTEXT);
    m_aStyles.append((sal_Int32) (rBlink.GetValue() ? 2 : 0));
}

void RtfAttributeOutput::CharBackground( const SvxBrushItem& rBrush )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if( !rBrush.GetColor().GetTransparency() )
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CHCBPAT);
        m_aStyles.append((sal_Int32)m_rExport.GetColor(rBrush.GetColor()));
    }
}

void RtfAttributeOutput::CharFontCJK( const SvxFontItem& rFont )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_DBCH);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_AF);
    m_aStyles.append((sal_Int32)m_rExport.maFontHelper.GetId(rFont));
}

void RtfAttributeOutput::CharFontSizeCJK( const SvxFontHeightItem& rFontSize )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    CharFontSize( rFontSize );
}

void RtfAttributeOutput::CharLanguageCJK( const SvxLanguageItem& rLanguageItem )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    CharLanguage( rLanguageItem );
}

void RtfAttributeOutput::CharPostureCJK( const SvxPostureItem& rPosture )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_I);
    if ( rPosture.GetPosture() == ITALIC_NONE )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharWeightCJK( const SvxWeightItem& rWeight )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_B);
    if ( rWeight.GetWeight() != WEIGHT_BOLD )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharFontCTL( const SvxFontItem& rFont )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_DBCH);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_AF);
    m_aStyles.append((sal_Int32)m_rExport.maFontHelper.GetId(rFont));
}

void RtfAttributeOutput::CharFontSizeCTL( const SvxFontHeightItem& rFontSize )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    CharFontSize( rFontSize );
}

void RtfAttributeOutput::CharLanguageCTL( const SvxLanguageItem& rLanguageItem )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    CharLanguage( rLanguageItem );
}

void RtfAttributeOutput::CharPostureCTL( const SvxPostureItem& rPosture)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_AI);
    if ( rPosture.GetPosture() == ITALIC_NONE )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharWeightCTL( const SvxWeightItem& rWeight )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_AB);
    if ( rWeight.GetWeight() != WEIGHT_BOLD )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::CharRotate( const SvxCharRotateItem& rRotate)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HORZVERT);
    m_aStyles.append((sal_Int32)(rRotate.IsFitToLine() ? 1 : 0));
}

void RtfAttributeOutput::CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const sal_Char* pStr;
    switch( rEmphasisMark.GetEmphasisMark())
    {
        case EMPHASISMARK_NONE:         pStr = OOO_STRING_SVTOOLS_RTF_ACCNONE;  break;
        case EMPHASISMARK_SIDE_DOTS:    pStr = OOO_STRING_SVTOOLS_RTF_ACCCOMMA; break;
        default:                        pStr = OOO_STRING_SVTOOLS_RTF_ACCDOT;   break;
    }
    m_aStyles.append(pStr);
}

void RtfAttributeOutput::CharTwoLines( const SvxTwoLinesItem& rTwoLines )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if( rTwoLines.GetValue() )
    {
        sal_Unicode cStart = rTwoLines.GetStartBracket();
        sal_Unicode cEnd =   rTwoLines.GetEndBracket();

        sal_uInt16 nType;
        if( !cStart && !cEnd )
            nType = 0;
        else if( '{' == cStart || '}' == cEnd )
            nType = 4;
        else if( '<' == cStart || '>' == cEnd )
            nType = 3;
        else if( '[' == cStart || ']' == cEnd )
            nType = 2;
        else                            // all other kind of brackets
            nType = 1;

        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TWOINONE);
        m_aStyles.append((sal_Int32)nType);
    }
}

void RtfAttributeOutput::CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CHARSCALEX);
    m_aStyles.append((sal_Int32)rScaleWidth.GetValue());
}

void RtfAttributeOutput::CharRelief( const SvxCharReliefItem& rRelief )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const sal_Char* pStr;
    switch (rRelief.GetValue())
    {
        case RELIEF_EMBOSSED:
            pStr = OOO_STRING_SVTOOLS_RTF_EMBO;
            break;
        case RELIEF_ENGRAVED:
            pStr = OOO_STRING_SVTOOLS_RTF_IMPR;
            break;
        default:
            pStr = 0;
            break;
    }

    if (pStr)
        m_aStyles.append(pStr);
}

void RtfAttributeOutput::CharHidden( const SvxCharHiddenItem& rHidden )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_V);
    if ( !rHidden.GetValue() )
        m_aStyles.append((sal_Int32)0);
}

void RtfAttributeOutput::TextINetFormat( const SwFmtINetFmt& rURL )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if( rURL.GetValue().Len() )
    {
        const SwCharFmt* pFmt;
        const SwTxtINetFmt* pTxtAtr = rURL.GetTxtINetFmt();

        m_aStyles.append("{" OOO_STRING_SVTOOLS_RTF_FLDRSLT " ");
        m_bHadFieldResult = true;
        if( pTxtAtr && 0 != ( pFmt = pTxtAtr->GetCharFmt() ))
        {
            sal_uInt16 nStyle = m_rExport.GetId( *pFmt );
            OString* pString = m_rExport.GetStyle(nStyle);
            if (pString)
                m_aStyles.append(*pString);
        }
    }
}

void RtfAttributeOutput::TextCharFormat( const SwFmtCharFmt& rCharFmt )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    sal_uInt16 nStyle = m_rExport.GetId( *rCharFmt.GetCharFmt() );
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CS);
    m_aStyles.append((sal_Int32)nStyle);
    OString* pString = m_rExport.GetStyle(nStyle);
    if (pString)
        m_aStyles.append(*pString);
}

void RtfAttributeOutput::WriteTextFootnoteNumStr(const SwFmtFtn& rFootnote)
{
    if (!rFootnote.GetNumStr().Len())
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_CHFTN);
    else
        m_aRun->append(msfilter::rtfutil::OutString(rFootnote.GetNumStr(), m_rExport.eCurrentEncoding));
}

void RtfAttributeOutput::TextFootnote_Impl( const SwFmtFtn& rFootnote )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " start");

    m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_SUPER " ");
    WriteTextFootnoteNumStr(rFootnote);
    m_aRun->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FOOTNOTE);
    if( rFootnote.IsEndNote() )
        m_aRun->append(OOO_STRING_SVTOOLS_RTF_FTNALT);
    m_aRun->append(' ');
    WriteTextFootnoteNumStr(rFootnote);

    /*
     * The footnote contains a whole paragraph, so we have to:
     * 1) Reset, then later restore the contents of our run buffer and run state.
     * 2) Buffer the output of the whole paragraph, as we do so for section headers already.
     */
    const SwNodeIndex* pIndex = rFootnote.GetTxtFtn()->GetStartNode();
    RtfStringBuffer aRun = m_aRun;
    m_aRun.clear();
    bool bInRunOrig = m_bInRun;
    m_bInRun = false;
    bool bSingleEmptyRunOrig = m_bSingleEmptyRun;
    m_bSingleEmptyRun = false;
    m_bBufferSectionHeaders = true;
    m_rExport.WriteSpecialText( pIndex->GetIndex() + 1,
            pIndex->GetNode().EndOfSectionIndex(),
            !rFootnote.IsEndNote() ? TXT_FTN : TXT_EDN);
    m_bBufferSectionHeaders = false;
    m_bInRun = bInRunOrig;
    m_bSingleEmptyRun = bSingleEmptyRunOrig;
    m_aRun = aRun;
    m_aRun->append(m_aSectionHeaders.makeStringAndClear());

    m_aRun->append("}");
    m_aRun->append("}");

    SAL_INFO("sw.rtf", OSL_THIS_FUNC << " end");
}

void RtfAttributeOutput::ParaLineSpacing_Impl( short nSpace, short nMulti )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SL);
    m_aStyles.append((sal_Int32)nSpace);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SLMULT);
    m_aStyles.append((sal_Int32)nMulti);

}

void RtfAttributeOutput::ParaAdjust( const SvxAdjustItem& rAdjust )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    switch ( rAdjust.GetAdjust() )
    {
        case SVX_ADJUST_LEFT:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QL);
            break;
        case SVX_ADJUST_RIGHT:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QR);
            break;
        case SVX_ADJUST_BLOCKLINE:
        case SVX_ADJUST_BLOCK:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QJ);
            break;
        case SVX_ADJUST_CENTER:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_QC);
            break;
        default:
            break;
    }
}

void RtfAttributeOutput::ParaSplit( const SvxFmtSplitItem& rSplit )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if( !rSplit.GetValue() )
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_KEEP);
}

void RtfAttributeOutput::ParaWidows( const SvxWidowsItem& rWidows )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (rWidows.GetValue())
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_WIDCTLPAR);
    else
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_NOWIDCTLPAR);
}

void RtfAttributeOutput::ParaTabStop( const SvxTabStopItem& rTabStop )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    long nOffset = ((SvxLRSpaceItem&)m_rExport.GetItem( RES_LR_SPACE )).GetTxtLeft();
    for( sal_uInt16 n = 0; n < rTabStop.Count(); n++ )
    {
        const SvxTabStop & rTS = rTabStop[ n ];
        if( SVX_TAB_ADJUST_DEFAULT != rTS.GetAdjustment() )
        {
            const char* pFill = 0;
            switch( rTS.GetFill() )
            {
                case cDfltFillChar:
                    break;

                case '.':    pFill = OOO_STRING_SVTOOLS_RTF_TLDOT;    break;
                case '_':    pFill = OOO_STRING_SVTOOLS_RTF_TLUL;    break;
                case '-':    pFill = OOO_STRING_SVTOOLS_RTF_TLTH;    break;
                case '=':    pFill = OOO_STRING_SVTOOLS_RTF_TLEQ;    break;
                default:
                        break;
            }
            if( pFill )
                m_aStyles.append(pFill);

            const sal_Char* pAdjStr = 0;
            switch (rTS.GetAdjustment())
            {
                case SVX_TAB_ADJUST_RIGHT:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQR;
                    break;
                case SVX_TAB_ADJUST_DECIMAL:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQDEC;
                    break;
                case SVX_TAB_ADJUST_CENTER:
                    pAdjStr = OOO_STRING_SVTOOLS_RTF_TQC;
                    break;
                default:
                    break;
            }
            if (pAdjStr)
                m_aStyles.append(pAdjStr);
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TX);
            m_aStyles.append((sal_Int32)(rTS.GetTabPos() + nOffset));
        }
        else
        {
            m_aTabStop.append( OOO_STRING_SVTOOLS_RTF_DEFTAB );
            m_aTabStop.append( (sal_Int32)rTabStop[0].GetTabPos() );
        }
    }
}

void RtfAttributeOutput::ParaHyphenZone( const SvxHyphenZoneItem& rHyphenZone )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    sal_Int32 nFlags = rHyphenZone.IsHyphen() ? 1 : 0;
    if( rHyphenZone.IsPageEnd() )
        nFlags += 2;
    m_aStyles.append('{');
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_IGNORE);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HYPHEN);
    m_aStyles.append((sal_Int32)nFlags);
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HYPHLEAD);
    m_aStyles.append((sal_Int32)rHyphenZone.GetMinLead());
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HYPHTRAIL);
    m_aStyles.append((sal_Int32)rHyphenZone.GetMinTrail());
    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_HYPHMAX);
    m_aStyles.append((sal_Int32)rHyphenZone.GetMaxHyphens());
    m_aStyles.append('}');
}

void RtfAttributeOutput::ParaNumRule_Impl( const SwTxtNode* pTxtNd, sal_Int32 nLvl, sal_Int32 nNumId )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( USHRT_MAX == nNumId || 0 == nNumId || 0 == pTxtNd)
        return;

    const SwNumRule* pRule = pTxtNd->GetNumRule();

    if( pRule && pTxtNd->IsInList() )
    {
        OSL_ENSURE( pTxtNd->GetActualListLevel() >= 0 && pTxtNd->GetActualListLevel() < MAXLEVEL,
                "<SwRTFWriter::OutListNum(..)> - text node does not have valid list level. Serious defect -> please inform OD" );

        const bool bExportNumRule = USHRT_MAX != nNumId;
        const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
        if( !pFmt )
            pFmt = &pRule->Get( nLvl );

        const SfxItemSet& rNdSet = pTxtNd->GetSwAttrSet();

        if ( bExportNumRule ) {
            m_aStyles.append('{');
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LISTTEXT);
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_PARD);
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_PLAIN);
            m_aStyles.append(' ');
        }

        SvxLRSpaceItem aLR( (SvxLRSpaceItem&)rNdSet.Get( RES_LR_SPACE ) );
        aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetIndentAt() );
        aLR.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );

        sal_uInt16 nStyle = m_rExport.GetId( *pFmt->GetCharFmt() );
        OString* pString = m_rExport.GetStyle(nStyle);
        if (pString)
            m_aStyles.append(*pString);

        {
            String sTxt;
            if( SVX_NUM_CHAR_SPECIAL == pFmt->GetNumberingType() || SVX_NUM_BITMAP == pFmt->GetNumberingType() )
                sTxt = pFmt->GetBulletChar();
            else
                sTxt = pTxtNd->GetNumString();

            if (sTxt.Len())
            {
                m_aStyles.append(' ');
                m_aStyles.append(msfilter::rtfutil::OutString(sTxt, m_rExport.eDefaultEncoding));
            }

            if( bExportNumRule )
            {
                if( OUTLINE_RULE != pRule->GetRuleType() )
                {
                    if (sTxt.Len())
                        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TAB);
                    m_aStyles.append('}');
                    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ILVL);
                    if( nLvl > 8 )            // RTF knows only 9 levels
                    {
                        m_aStyles.append((sal_Int32)8);
                        m_aStyles.append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SOUTLVL);
                        m_aStyles.append((sal_Int32)nLvl);
                        m_aStyles.append('}');
                    }
                    else
                        m_aStyles.append((sal_Int32)nLvl);
                }
                else
                    m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TAB "}");
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LS);
                m_aStyles.append((sal_Int32)m_rExport.GetId(*pRule)+1);
                m_aStyles.append(' ');
            }
            else if( sTxt.Len() )
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_TAB);
        }
        FormatLRSpace(aLR);
    }
}

void RtfAttributeOutput::ParaScriptSpace( const SfxBoolItem& rScriptSpace )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (!rScriptSpace.GetValue( ))
        return;
    switch ( rScriptSpace.Which( ) )
    {
        case RES_PARATR_SCRIPTSPACE:
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_ASPALPHA);
            break;

        default:
            break;
    }
}

void RtfAttributeOutput::ParaVerticalAlign( const SvxParaVertAlignItem& rAlign )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const char* pStr;
    switch ( rAlign.GetValue() )
    {
        case SvxParaVertAlignItem::TOP:         pStr = OOO_STRING_SVTOOLS_RTF_FAHANG;       break;
        case SvxParaVertAlignItem::BOTTOM:      pStr = OOO_STRING_SVTOOLS_RTF_FAVAR;        break;
        case SvxParaVertAlignItem::CENTER:      pStr = OOO_STRING_SVTOOLS_RTF_FACENTER;     break;
        case SvxParaVertAlignItem::BASELINE:    pStr = OOO_STRING_SVTOOLS_RTF_FAROMAN;      break;

        default:                                pStr = OOO_STRING_SVTOOLS_RTF_FAAUTO;       break;
    }
    m_aStyles.append(pStr);
}

void RtfAttributeOutput::ParaSnapToGrid( const SvxParaGridItem& /*rGrid*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::FormatFrameSize( const SwFmtFrmSize& rSize )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (m_rExport.bOutPageDescs)
    {
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGWSXN);
        m_aSectionBreaks.append((sal_Int32)rSize.GetWidth());
        m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_PGHSXN);
        m_aSectionBreaks.append((sal_Int32)rSize.GetHeight());
        if (!m_bBufferSectionBreaks)
            m_rExport.Strm() << m_aSectionBreaks.makeStringAndClear().getStr();
    }
}

void RtfAttributeOutput::FormatPaperBin( const SvxPaperBinItem& )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::FormatLRSpace( const SvxLRSpaceItem& rLRSpace )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( !m_rExport.bOutFlyFrmAttrs )
    {
        if( m_rExport.bOutPageDescs )
        {
            if( rLRSpace.GetLeft() )
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGLSXN);
                m_aSectionBreaks.append((sal_Int32)rLRSpace.GetLeft());
            }
            if( rLRSpace.GetRight() )
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGRSXN);
                m_aSectionBreaks.append((sal_Int32)rLRSpace.GetRight());
            }
            if (!m_bBufferSectionBreaks)
                m_rExport.Strm() <<
                    m_aSectionBreaks.makeStringAndClear().getStr();
        }
        else
        {
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LI);
            m_aStyles.append( (sal_Int32) rLRSpace.GetTxtLeft() );
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_RI);
            m_aStyles.append( (sal_Int32) rLRSpace.GetRight() );
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LIN);
            m_aStyles.append( (sal_Int32) rLRSpace.GetTxtLeft() );
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_RIN);
            m_aStyles.append( (sal_Int32) rLRSpace.GetRight() );
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_FI);
            m_aStyles.append( (sal_Int32) rLRSpace.GetTxtFirstLineOfst() );
        }
    }
    else if (m_rExport.bRTFFlySyntax)
    {
        // Wrap: top and bottom spacing, convert from twips to EMUs.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dxWrapDistLeft", OString::number(rLRSpace.GetLeft() * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dxWrapDistRight", OString::number(rLRSpace.GetRight() * 635)));
    }
}

void RtfAttributeOutput::FormatULSpace( const SvxULSpaceItem& rULSpace )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( !m_rExport.bOutFlyFrmAttrs )
    {
        if( m_rExport.bOutPageDescs )
        {
            OSL_ENSURE( m_rExport.GetCurItemSet(), "Impossible" );
            if ( !m_rExport.GetCurItemSet() )
                return;

            HdFtDistanceGlue aDistances( *m_rExport.GetCurItemSet() );

            if( aDistances.dyaTop )
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGTSXN);
                m_aSectionBreaks.append((sal_Int32)aDistances.dyaTop);
            }
            if ( aDistances.HasHeader() )
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_HEADERY);
                m_aSectionBreaks.append((sal_Int32)aDistances.dyaHdrTop);
            }

            if( aDistances.dyaBottom )
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_MARGBSXN);
                m_aSectionBreaks.append((sal_Int32)aDistances.dyaBottom);
            }
            if( aDistances.HasFooter() )
            {
                m_aSectionBreaks.append(OOO_STRING_SVTOOLS_RTF_FOOTERY);
                m_aSectionBreaks.append((sal_Int32)aDistances.dyaHdrBottom);
            }
            if (!m_bBufferSectionBreaks)
                m_rExport.Strm() <<
                    m_aSectionBreaks.makeStringAndClear().getStr();
        }
        else
        {
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SB);
            m_aStyles.append( (sal_Int32) rULSpace.GetUpper() );
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_SA);
            m_aStyles.append( (sal_Int32) rULSpace.GetLower() );
            if (rULSpace.GetContext())
                m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CONTEXTUALSPACE);
        }
    }
    else if (m_rExport.bRTFFlySyntax)
    {
        // Wrap: top and bottom spacing, convert from twips to EMUs.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dyWrapDistTop", OString::number(rULSpace.GetUpper() * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dyWrapDistBottom", OString::number(rULSpace.GetLower() * 635)));
    }
}

void RtfAttributeOutput::FormatSurround( const SwFmtSurround& rSurround )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( m_rExport.bOutFlyFrmAttrs && !m_rExport.bRTFFlySyntax )
    {
        SwSurround eSurround = rSurround.GetSurround();
        sal_Bool bGold = SURROUND_IDEAL == eSurround;
        if( bGold )
            eSurround = SURROUND_PARALLEL;
        RTFSurround aMC( bGold, static_cast< sal_uInt8 >(eSurround) );
        m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYMAINCNT);
        m_aRunText->append( (sal_Int32) aMC.GetValue() );
    }
}

void RtfAttributeOutput::FormatVertOrientation( const SwFmtVertOrient& rFlyVert )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( m_rExport.bOutFlyFrmAttrs && m_rExport.bRTFFlySyntax )
    {
        switch (rFlyVert.GetRelationOrient())
        {
            case text::RelOrientation::PAGE_FRAME:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posrelv", OString::number(1)));
            break;
            default:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posrelv", OString::number(2)));
                m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SHPBYPARA << OOO_STRING_SVTOOLS_RTF_SHPBYIGNORE;
            break;
        }

        switch(rFlyVert.GetVertOrient())
        {
            case text::VertOrientation::TOP:
            case text::VertOrientation::LINE_TOP:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posv", OString::number(1)));
                break;
            case text::VertOrientation::BOTTOM:
            case text::VertOrientation::LINE_BOTTOM:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posv", OString::number(3)));
                break;
            case text::VertOrientation::CENTER:
            case text::VertOrientation::LINE_CENTER:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posv", OString::number(2)));
                break;
            default:
                break;
        }

        m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SHPTOP;
        m_rExport.OutLong(rFlyVert.GetPos());
        if (m_pFlyFrameSize)
        {
            m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SHPBOTTOM;
            m_rExport.OutLong(rFlyVert.GetPos() + m_pFlyFrameSize->Height());
        }
    }
}

void RtfAttributeOutput::FormatHorizOrientation( const SwFmtHoriOrient& rFlyHori )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( m_rExport.bOutFlyFrmAttrs && m_rExport.bRTFFlySyntax )
    {
        switch (rFlyHori.GetRelationOrient())
        {
            case text::RelOrientation::PAGE_FRAME:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posrelh", OString::number(1)));
            break;
            default:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posrelh", OString::number(2)));
                m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SHPBXCOLUMN << OOO_STRING_SVTOOLS_RTF_SHPBXIGNORE;
            break;
        }

        switch(rFlyHori.GetHoriOrient())
        {
            case text::HoriOrientation::LEFT:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posh", OString::number(1)));
                break;
            case text::HoriOrientation::CENTER:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posh", OString::number(2)));
                break;
            case text::HoriOrientation::RIGHT:
                m_aFlyProperties.push_back(std::make_pair<OString, OString>("posh", OString::number(3)));
                break;
            default:
                break;
        }

        m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SHPLEFT;
        m_rExport.OutLong(rFlyHori.GetPos());
        if (m_pFlyFrameSize)
        {
            m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_SHPRIGHT;
            m_rExport.OutLong(rFlyHori.GetPos() + m_pFlyFrameSize->Width());
        }
    }
}

void RtfAttributeOutput::FormatAnchor( const SwFmtAnchor& rAnchor )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( !m_rExport.bRTFFlySyntax )
    {
        sal_uInt16 nId = static_cast< sal_uInt16 >(rAnchor.GetAnchorId());
        m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYANCHOR);
        m_aRunText->append((sal_Int32)nId);
        switch( nId )
        {
            case FLY_AT_PAGE:
                m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYPAGE);
                m_aRunText->append((sal_Int32)rAnchor.GetPageNum());
                break;
            case FLY_AT_PARA:
            case FLY_AS_CHAR:
                m_aRunText->append(OOO_STRING_SVTOOLS_RTF_FLYCNTNT);
                break;
        }
    }
}

void RtfAttributeOutput::FormatBackground( const SvxBrushItem& rBrush )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (m_rExport.bRTFFlySyntax)
    {
        const Color& rColor = rBrush.GetColor();
        // We in fact need RGB to BGR, but the transformation is symmetric.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("fillColor", OString::number(msfilter::util::BGRToRGB(rColor.GetColor()))));
    }
    else if( !rBrush.GetColor().GetTransparency() )
    {
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_CBPAT);
        m_aStyles.append((sal_Int32)m_rExport.GetColor(rBrush.GetColor()));
    }
}

void RtfAttributeOutput::FormatFillStyle( const XFillStyleItem& rFillStyle )
{
    m_oFillStyle.reset(rFillStyle.GetValue());
}

void RtfAttributeOutput::FormatFillGradient( const XFillGradientItem& rFillGradient )
{
    if (*m_oFillStyle == XFILL_GRADIENT)
    {
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("fillType", OString::number(7))); // Shade using the fillAngle

        const XGradient& rGradient = rFillGradient.GetGradientValue();
        const Color& rStartColor = rGradient.GetStartColor();
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("fillBackColor", OString::number(msfilter::util::BGRToRGB(rStartColor.GetColor()))));

        const Color& rEndColor = rGradient.GetEndColor();
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("fillColor", OString::number(msfilter::util::BGRToRGB(rEndColor.GetColor()))));

        switch (rGradient.GetGradientStyle())
        {
            case XGRAD_LINEAR: break;
            case XGRAD_AXIAL:
               m_aFlyProperties.push_back(std::make_pair<OString, OString>("fillFocus", OString::number(50)));
               break;
            case XGRAD_RADIAL: break;
            case XGRAD_ELLIPTICAL: break;
            case XGRAD_SQUARE: break;
            case XGRAD_RECT: break;
        }
    }
}

void RtfAttributeOutput::FormatBox( const SvxBoxItem& rBox )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    static const sal_uInt16 aBorders[] = {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    static const sal_Char* aBorderNames[] = {
        OOO_STRING_SVTOOLS_RTF_BRDRT, OOO_STRING_SVTOOLS_RTF_BRDRL, OOO_STRING_SVTOOLS_RTF_BRDRB, OOO_STRING_SVTOOLS_RTF_BRDRR };

    sal_uInt16 nDist = rBox.GetDistance();

    if ( m_rExport.bRTFFlySyntax )
    {
        // Borders: spacing to contents, convert from twips to EMUs.
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dxTextLeft", OString::number(rBox.GetDistance(BOX_LINE_LEFT) * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dyTextTop", OString::number(rBox.GetDistance(BOX_LINE_TOP) * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dxTextRight", OString::number(rBox.GetDistance(BOX_LINE_RIGHT) * 635)));
        m_aFlyProperties.push_back(std::make_pair<OString, OString>("dyTextBottom", OString::number(rBox.GetDistance(BOX_LINE_BOTTOM) * 635)));

        const SvxBorderLine* pLeft = rBox.GetLine(BOX_LINE_LEFT);
        const SvxBorderLine* pRight = rBox.GetLine(BOX_LINE_RIGHT);
        const SvxBorderLine* pTop = rBox.GetLine(BOX_LINE_TOP);
        const SvxBorderLine* pBottom = rBox.GetLine(BOX_LINE_BOTTOM);
        if (pLeft && pRight && pTop && pBottom && *pLeft == *pRight && *pLeft == *pTop && *pLeft == *pBottom)
        {
            const Color& rColor = pTop->GetColor();
            // We in fact need RGB to BGR, but the transformation is symmetric.
            m_aFlyProperties.push_back(std::make_pair<OString, OString>("lineColor", OString::number(msfilter::util::BGRToRGB(rColor.GetColor()))));

            double const fConverted(editeng::ConvertBorderWidthToWord(pTop->GetBorderLineStyle(), pTop->GetWidth()));
            sal_Int32 nWidth = sal_Int32(fConverted * 635); // Twips -> EMUs
            m_aFlyProperties.push_back(std::make_pair<OString, OString>("lineWidth", OString::number(nWidth)));
        }

        return;
    }

    if( rBox.GetTop() && rBox.GetBottom() &&
            rBox.GetLeft() && rBox.GetRight() &&
            *rBox.GetTop() == *rBox.GetBottom() &&
            *rBox.GetTop() == *rBox.GetLeft() &&
            *rBox.GetTop() == *rBox.GetRight() &&
            nDist == rBox.GetDistance( BOX_LINE_TOP ) &&
            nDist == rBox.GetDistance( BOX_LINE_LEFT ) &&
            nDist == rBox.GetDistance( BOX_LINE_BOTTOM ) &&
            nDist == rBox.GetDistance( BOX_LINE_RIGHT ))
        m_aSectionBreaks.append(OutBorderLine( m_rExport, rBox.GetTop(), OOO_STRING_SVTOOLS_RTF_BOX, nDist ));
    else
    {
        const sal_uInt16* pBrd = aBorders;
        const sal_Char** pBrdNms = (const sal_Char**)aBorderNames;
        for(int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms)
        {
            if (const SvxBorderLine* pLn = rBox.GetLine(*pBrd))
            {
                m_aSectionBreaks.append(OutBorderLine(m_rExport, pLn, *pBrdNms,
                        rBox.GetDistance(*pBrd)));
            }
        }
    }

    if (!m_bBufferSectionBreaks)
        m_aStyles.append(m_aSectionBreaks.makeStringAndClear());
}

void RtfAttributeOutput::FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol& rCol, bool bEven, SwTwips nPageSize )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_COLS;
    m_rExport.OutLong( nCols );

    if( bEven )
    {
        m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_COLSX;
        m_rExport.OutLong( rCol.GetGutterWidth( sal_True ) );
    }
    else
    {
        const SwColumns & rColumns = rCol.GetColumns( );
        for( sal_uInt16 n = 0; n < nCols; )
        {
            m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_COLNO;
            m_rExport.OutLong( n+1 );

            m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_COLW;
            m_rExport.OutLong( rCol.CalcPrtColWidth( n, nPageSize ) );

            if( ++n != nCols )
            {
                m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_COLSR;
                m_rExport.OutLong( rColumns[ n-1 ].GetRight() +
                        rColumns[ n ].GetLeft() );
            }
        }
    }
}

void RtfAttributeOutput::FormatKeep( const SvxFmtKeepItem& rItem )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if( rItem.GetValue() )
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_KEEPN);
}

void RtfAttributeOutput::FormatTextGrid( const SwTextGridItem& /*rGrid*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::FormatLineNumbering( const SwFmtLineNumber& rNumbering )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if ( !rNumbering.IsCount( ) )
        m_aStyles.append(OOO_STRING_SVTOOLS_RTF_NOLINE);
}

void RtfAttributeOutput::FormatFrameDirection( const SvxFrameDirectionItem& rDirection )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (!m_rExport.bOutPageDescs)
    {
        if (rDirection.GetValue() == FRMDIR_HORI_RIGHT_TOP)
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_RTLPAR);
        else
            m_aStyles.append(OOO_STRING_SVTOOLS_RTF_LTRPAR);
    }
}

void RtfAttributeOutput::ParaGrabBag(const SfxGrabBagItem& /*rItem*/)
{
}

void RtfAttributeOutput::WriteExpand( const SwField* pFld )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
    String sCmd;        // for optional Parameters
    switch (pFld->GetTyp()->Which())
    {
        //#i119803# Export user field and DB field for RTF filter
        case RES_DBFLD:
            sCmd = FieldString(ww::eMERGEFIELD);
            // no break !!
        case RES_USERFLD:
            sCmd += pFld->GetTyp()->GetName();
            m_rExport.OutputField(pFld, ww::eNONE, sCmd);
            break;
        default:
            m_rExport.OutputField(pFld, ww::eUNKNOWN, sCmd);
            break;
    }
}

void RtfAttributeOutput::RefField( const SwField& /*rFld*/, const String& /*rRef*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::HiddenField( const SwField& /*rFld*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::SetField( const SwField& /*rFld*/, ww::eField /*eType*/, const String& /*rCmd*/ )
{
    SAL_INFO("sw.rtf", "TODO: " << OSL_THIS_FUNC);
}

void RtfAttributeOutput::PostitField( const SwField* pFld )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    const SwPostItField& rPFld = *(SwPostItField*)pFld;

    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNID " ");
    m_aRunText->append(OUStringToOString(OUString(rPFld.GetInitials()), m_rExport.eCurrentEncoding));
    m_aRunText->append("}");
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNAUTHOR " ");
    m_aRunText->append(OUStringToOString(OUString(rPFld.GetPar1()), m_rExport.eCurrentEncoding));
    m_aRunText->append("}");
    m_aRunText->append(OOO_STRING_SVTOOLS_RTF_CHATN);

    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ANNOTATION);
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNREF " ");
    m_aRunText->append(sal_Int32(m_nPostitFieldsMaxId++));
    m_aRunText->append('}');
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATNDATE " ");
    m_aRunText->append((sal_Int32)sw::ms::DateTime2DTTM(rPFld.GetDateTime()));
    m_aRunText->append('}');
    m_aRunText->append(OUStringToOString(OUString(rPFld.GetTxt()), m_rExport.eCurrentEncoding));
    m_aRunText->append('}');
}

void RtfAttributeOutput::WritePostitFieldStart()
{
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATRFSTART " ");
    m_aRunText->append(sal_Int32(m_nPostitFieldsMaxId));
    m_aRunText->append("}");
}

void RtfAttributeOutput::WritePostitFieldEnd()
{
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_ATRFEND " ");
    m_aRunText->append(sal_Int32(m_nPostitFieldsMaxId));
    m_aRunText->append("}");
}

bool RtfAttributeOutput::DropdownField( const SwField* /*pFld*/ )
{
    // this is handled in OutputFlyFrame_Impl()
    return true;
}

bool RtfAttributeOutput::PlaceholderField( const SwField* pField)
{
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_FIELD "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_FLDINST " MACROBUTTON  None ");
    RunText(pField->GetPar1());
    m_aRunText->append("}}");
    return false; // do not expand
}

RtfAttributeOutput::RtfAttributeOutput( RtfExport &rExport )
    : m_rExport( rExport ),
    m_bStrikeDouble( false ),
    m_pTableWrt( NULL ),
    m_bTableCellOpen( false ),
    m_nTableDepth( 0 ),
    m_bTblAfterCell( false ),
    m_nColBreakNeeded( false ),
    m_bBufferSectionBreaks( false ),
    m_bBufferSectionHeaders( false ),
    m_bLastTable( true ),
    m_bWroteCellInfo( false ),
    m_bHadFieldResult( false ),
    m_bTableRowEnded( false ),
    m_aCells(),
    m_bSingleEmptyRun(false),
    m_bInRun(false),
    m_nPostitFieldsMaxId(0),
    m_pFlyFrameSize(0),
    m_pPrevPageDesc(0)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
}

RtfAttributeOutput::~RtfAttributeOutput()
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);
}

MSWordExportBase& RtfAttributeOutput::GetExport()
{
    return m_rExport;
}

// These are used by wwFont::WriteRtf()

/// Start the font.
void RtfAttributeOutput::StartFont( const String& rFamilyName ) const
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << OUStringToOString( OUString( rFamilyName ), m_rExport.eCurrentEncoding ).getStr();
}

/// End the font.
void RtfAttributeOutput::EndFont() const
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << ";}";
}

/// Alternate name for the font.
void RtfAttributeOutput::FontAlternateName( const String& rName ) const
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_IGNORE << OOO_STRING_SVTOOLS_RTF_FALT << ' ';
    m_rExport.Strm() << OUStringToOString( OUString( rName ), m_rExport.eCurrentEncoding ).getStr() << '}';
}

/// Font charset.
void RtfAttributeOutput::FontCharset( sal_uInt8 nCharSet ) const
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_FCHARSET;
    m_rExport.OutULong( nCharSet );
    m_rExport.Strm() << ' ';
}

/// Font family.
void RtfAttributeOutput::FontFamilyType( FontFamily eFamily, const wwFont &rFont ) const
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << '{' << OOO_STRING_SVTOOLS_RTF_F;

    const char* pStr = OOO_STRING_SVTOOLS_RTF_FNIL;
    switch (eFamily)
    {
        case FAMILY_ROMAN:
            pStr = OOO_STRING_SVTOOLS_RTF_FROMAN;
            break;
        case FAMILY_SWISS:
            pStr = OOO_STRING_SVTOOLS_RTF_FSWISS;
            break;
        case FAMILY_MODERN:
            pStr = OOO_STRING_SVTOOLS_RTF_FMODERN;
            break;
        case FAMILY_SCRIPT:
            pStr = OOO_STRING_SVTOOLS_RTF_FSCRIPT;
            break;
        case FAMILY_DECORATIVE:
            pStr = OOO_STRING_SVTOOLS_RTF_FDECOR;
            break;
        default:
            break;
    }
    m_rExport.OutULong(m_rExport.maFontHelper.GetId(rFont)) << pStr;
}

/// Font pitch.
void RtfAttributeOutput::FontPitchType( FontPitch ePitch ) const
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_FPRQ;

    sal_uInt16 nVal = 0;
    switch (ePitch)
    {
        case PITCH_FIXED:
            nVal = 1;
            break;
        case PITCH_VARIABLE:
            nVal = 2;
            break;
        default:
            break;
    }
    m_rExport.OutULong(nVal);
}

static bool IsEMF(const sal_uInt8 *pGraphicAry, unsigned long nSize)
{
    if (pGraphicAry && (nSize > 0x2c ))
    {
        // check the magic number
        if (
                (pGraphicAry[0x28] == 0x20 ) && (pGraphicAry[0x29] == 0x45) &&
                (pGraphicAry[0x2a] == 0x4d ) && (pGraphicAry[0x2b] == 0x46)
           )
        {
            //emf detected
            return true;
        }
    }
    return false;
}

static bool StripMetafileHeader(const sal_uInt8 *&rpGraphicAry, unsigned long &rSize)
{
    if (rpGraphicAry && (rSize > 0x22))
    {
        if (
             (rpGraphicAry[0] == 0xd7) && (rpGraphicAry[1] == 0xcd) &&
             (rpGraphicAry[2] == 0xc6) && (rpGraphicAry[3] == 0x9a)
           )
        {   // we have to get rid of the metafileheader
            rpGraphicAry += 22;
            rSize -= 22;
            return true;
        }
    }
    return false;
}

OString RtfAttributeOutput::WriteHex(const sal_uInt8* pData, sal_uInt32 nSize, SvStream* pStream, sal_uInt32 nLimit)
{
    OStringBuffer aRet;

    sal_uInt32 nBreak = 0;
    for (sal_uInt32 i = 0; i < nSize; i++)
    {
        OString sNo = OString::valueOf(sal_Int32(pData[i]), 16);
        if (sNo.getLength() < 2)
        {
            if (pStream)
                (*pStream) << '0';
            else
                aRet.append('0');
        }
        if (pStream)
            (*pStream) << sNo.getStr();
        else
            aRet.append(sNo);
        if (++nBreak == nLimit)
        {
            if (pStream)
                (*pStream) << RtfExport::sNewLine;
            else
                aRet.append(RtfExport::sNewLine);
            nBreak = 0;
        }
    }

    return aRet.makeStringAndClear();
}

static void lcl_AppendSP( OStringBuffer& rBuffer,
    const char cName[],
    const OUString& rValue,
    const RtfExport& rExport )
{
    rBuffer.append( "{" OOO_STRING_SVTOOLS_RTF_SP "{" ); // "{\sp{"
    rBuffer.append( OOO_STRING_SVTOOLS_RTF_SN " " );//" \sn "
    rBuffer.append( cName ); //"PropName"
    rBuffer.append( "}{" OOO_STRING_SVTOOLS_RTF_SV " " );
// "}{ \sv "
    rBuffer.append( msfilter::rtfutil::OutString( rValue, rExport.eCurrentEncoding ) );
    rBuffer.append( "}}" );
}

static OString ExportPICT( const SwFlyFrmFmt* pFlyFrmFmt, const Size &rOrig, const Size &rRendered, const Size &rMapped,
    const SwCropGrf &rCr, const char *pBLIPType, const sal_uInt8 *pGraphicAry,
    unsigned long nSize, const RtfExport& rExport, SvStream *pStream = 0 )
{
    OStringBuffer aRet;
    bool bIsWMF = std::strcmp(pBLIPType, OOO_STRING_SVTOOLS_RTF_WMETAFILE) == 0;
    if (pBLIPType && nSize && pGraphicAry)
    {
        aRet.append("{" OOO_STRING_SVTOOLS_RTF_PICT);

        if( pFlyFrmFmt )
        {
            String sDescription = pFlyFrmFmt->GetObjDescription();
            //write picture properties - wzDescription at first
            //looks like: "{\*\picprop{\sp{\sn PropertyName}{\sv PropertyValue}}}"
            aRet.append( "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_PICPROP );//"{\*\picprop
            lcl_AppendSP( aRet, "wzDescription", sDescription, rExport );
            String sName = pFlyFrmFmt->GetObjTitle();
            lcl_AppendSP( aRet, "wzName", sName, rExport );
            aRet.append( "}" ); //"}"
        }

        long nXCroppedSize = rOrig.Width()-(rCr.GetLeft() + rCr.GetRight());
        long nYCroppedSize = rOrig.Height()-(rCr.GetTop() + rCr.GetBottom());
        /* Graphic with a zero height or width, typically copied from webpages, caused crashes. */
        if( !nXCroppedSize )
            nXCroppedSize = 100;
        if( !nYCroppedSize )
            nYCroppedSize = 100;

        //Given the original size and taking cropping into account
        //first, how much has the original been scaled to get the
        //final rendered size
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICSCALEX);
        aRet.append((sal_Int32)((100 * rRendered.Width()) / nXCroppedSize));
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICSCALEY);
        aRet.append((sal_Int32)((100 * rRendered.Height()) / nYCroppedSize));

        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPL);
        aRet.append((sal_Int32)rCr.GetLeft());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPR);
        aRet.append((sal_Int32)rCr.GetRight());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPT);
        aRet.append((sal_Int32)rCr.GetTop());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICCROPB);
        aRet.append((sal_Int32)rCr.GetBottom());

        aRet.append(OOO_STRING_SVTOOLS_RTF_PICW);
        aRet.append((sal_Int32)rMapped.Width());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICH);
        aRet.append((sal_Int32)rMapped.Height());

        aRet.append(OOO_STRING_SVTOOLS_RTF_PICWGOAL);
        aRet.append((sal_Int32)rOrig.Width());
        aRet.append(OOO_STRING_SVTOOLS_RTF_PICHGOAL);
        aRet.append((sal_Int32)rOrig.Height());

        aRet.append(pBLIPType);
        if (bIsWMF)
        {
            aRet.append((sal_Int32)8);
            StripMetafileHeader(pGraphicAry, nSize);
        }
        aRet.append(RtfExport::sNewLine);
        if (pStream)
            (*pStream) << aRet.makeStringAndClear().getStr();
        if (pStream)
            RtfAttributeOutput::WriteHex(pGraphicAry, nSize, pStream);
        else
            aRet.append(RtfAttributeOutput::WriteHex(pGraphicAry, nSize));
        aRet.append('}');
        if (pStream)
            (*pStream) << aRet.makeStringAndClear().getStr();
    }
    return aRet.makeStringAndClear();
}

void RtfAttributeOutput::FlyFrameOLEReplacement(const SwFlyFrmFmt* pFlyFrmFmt, SwOLENode& rOLENode, const Size& rSize)
{
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPPICT);
    Size aSize(sw::util::GetSwappedInSize(rOLENode));
    Size aRendered(aSize);
    aRendered.Width() = rSize.Width();
    aRendered.Height() = rSize.Height();
    const Graphic* pGraphic = rOLENode.GetGraphic();
    Size aMapped(pGraphic->GetPrefSize());
    const SwCropGrf &rCr = (const SwCropGrf &)rOLENode.GetAttr(RES_GRFATR_CROPGRF);
    const sal_Char* pBLIPType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
    const sal_uInt8* pGraphicAry = 0;
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, *pGraphic, CVT_PNG) != ERRCODE_NONE)
        OSL_FAIL("failed to export the graphic");
    aStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nSize = aStream.Tell();
    pGraphicAry = (sal_uInt8*)aStream.GetData();
    m_aRunText->append(ExportPICT( pFlyFrmFmt, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize, m_rExport ));
    m_aRunText->append("}"); // shppict
    m_aRunText->append("{" OOO_STRING_SVTOOLS_RTF_NONSHPPICT);
    pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
    SvMemoryStream aWmfStream;
    if (GraphicConverter::Export(aWmfStream, *pGraphic, CVT_WMF) != ERRCODE_NONE)
        OSL_FAIL("failed to export the graphic");
    aWmfStream.Seek(STREAM_SEEK_TO_END);
    nSize = aWmfStream.Tell();
    pGraphicAry = (sal_uInt8*)aWmfStream.GetData();
    m_aRunText->append(ExportPICT( pFlyFrmFmt, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize, m_rExport ));
    m_aRunText->append("}"); // nonshppict
}

bool RtfAttributeOutput::FlyFrameOLEMath(const SwFlyFrmFmt* pFlyFrmFmt, SwOLENode& rOLENode, const Size& rSize)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    uno::Reference <embed::XEmbeddedObject> xObj(const_cast<SwOLENode&>(rOLENode).GetOLEObj().GetOleRef());
    sal_Int64 nAspect = rOLENode.GetAspect();
    svt::EmbeddedObjectRef aObjRef(xObj, nAspect);
    SvGlobalName aObjName(aObjRef->getClassID());

    if (!SotExchange::IsMath(aObjName))
        return false;

    m_aRunText->append("{" LO_STRING_SVTOOLS_RTF_MMATH " ");
    uno::Reference<util::XCloseable> xClosable(xObj->getComponent(), uno::UNO_QUERY);
// gcc4.4 (and 4.3 and possibly older) have a problem with dynamic_cast directly to the target class,
// so help it with an intermediate cast. I'm not sure what exactly the problem is, seems to be unrelated
// to RTLD_GLOBAL, so most probably a gcc bug.
    oox::FormulaExportBase* pBase = dynamic_cast<oox::FormulaExportBase*>(dynamic_cast<SfxBaseModel*>(xClosable.get()));
    assert( pBase != NULL );
    OStringBuffer aBuf;
    pBase->writeFormulaRtf(aBuf, m_rExport.eCurrentEncoding);
    m_aRunText->append(aBuf.makeStringAndClear());
    // Replacement graphic.
    m_aRunText->append("{" LO_STRING_SVTOOLS_RTF_MMATHPICT " ");
    FlyFrameOLEReplacement(pFlyFrmFmt, rOLENode, rSize);
    m_aRunText->append("}"); // mmathPict
    m_aRunText->append("}"); // mmath

    return true;
}

void RtfAttributeOutput::FlyFrameOLE( const SwFlyFrmFmt* pFlyFrmFmt, SwOLENode& rOLENode, const Size& rSize )
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    if (FlyFrameOLEMath(pFlyFrmFmt, rOLENode, rSize))
        return;

    FlyFrameOLEReplacement(pFlyFrmFmt, rOLENode, rSize);
}

void RtfAttributeOutput::FlyFrameGraphic( const SwFlyFrmFmt* pFlyFrmFmt, const SwGrfNode* pGrfNode)
{
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

    SvMemoryStream aStream;
    const sal_uInt8* pGraphicAry = 0;
    sal_uInt32 nSize = 0;

    Graphic aGraphic(pGrfNode->GetGrf());

    // If there is no graphic there is not much point in parsing it
    if(aGraphic.GetType()==GRAPHIC_NONE)
        return;

    GfxLink aGraphicLink;
    const sal_Char* pBLIPType = 0;
    if (aGraphic.IsLink())
    {
        aGraphicLink = aGraphic.GetLink();
        nSize = aGraphicLink.GetDataSize();
        pGraphicAry = aGraphicLink.GetData();
        switch (aGraphicLink.GetType())
        {
            case GFX_LINK_TYPE_NATIVE_JPG:
                pBLIPType = OOO_STRING_SVTOOLS_RTF_JPEGBLIP;
                break;
            case GFX_LINK_TYPE_NATIVE_PNG:
                pBLIPType = OOO_STRING_SVTOOLS_RTF_PNGBLIP;
                break;
            case GFX_LINK_TYPE_NATIVE_WMF:
                pBLIPType =
                    IsEMF(pGraphicAry, nSize) ? OOO_STRING_SVTOOLS_RTF_EMFBLIP : OOO_STRING_SVTOOLS_RTF_WMETAFILE;
                break;
            default:
                break;
        }
    }

    GraphicType eGraphicType = aGraphic.GetType();
    if (!pGraphicAry)
    {
        if (ERRCODE_NONE == GraphicConverter::Export(aStream, aGraphic,
                    (eGraphicType == GRAPHIC_BITMAP) ? CVT_PNG : CVT_WMF))
        {
            pBLIPType = (eGraphicType == GRAPHIC_BITMAP) ?
                OOO_STRING_SVTOOLS_RTF_PNGBLIP : OOO_STRING_SVTOOLS_RTF_WMETAFILE;
            aStream.Seek(STREAM_SEEK_TO_END);
            nSize = aStream.Tell();
            pGraphicAry = (sal_uInt8*)aStream.GetData();
        }
    }

    Size aMapped(eGraphicType == GRAPHIC_BITMAP ? aGraphic.GetSizePixel() : aGraphic.GetPrefSize());

    const SwCropGrf &rCr = (const SwCropGrf &)pGrfNode->GetAttr(RES_GRFATR_CROPGRF);

    //Get original size in twips
    Size aSize(sw::util::GetSwappedInSize(*pGrfNode));
    Size aRendered(aSize);
    if (pFlyFrmFmt)
    {
        const SwFmtFrmSize& rS = pFlyFrmFmt->GetFrmSize();
        aRendered.Width() = rS.GetWidth();
        aRendered.Height() = rS.GetHeight();
    }

    /*
       If the graphic is not of type WMF then we will have to store two
       graphics, one in the native format wrapped in shppict, and the other in
       the wmf format wrapped in nonshppict, so as to keep wordpad happy. If its
       a wmf already then we don't need any such wrapping
       */
    bool bIsWMF = pBLIPType && std::strcmp(pBLIPType, OOO_STRING_SVTOOLS_RTF_WMETAFILE) == 0;
    if (!bIsWMF)
        m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPPICT;

    if (pBLIPType)
        ExportPICT( pFlyFrmFmt, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize, m_rExport, &m_rExport.Strm() );
    else
    {
        aStream.Seek(0);
        GraphicConverter::Export(aStream, aGraphic, CVT_WMF);
        pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
        aStream.Seek(STREAM_SEEK_TO_END);
        nSize = aStream.Tell();
        pGraphicAry = (sal_uInt8*)aStream.GetData();

        ExportPICT(pFlyFrmFmt, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize, m_rExport, &m_rExport.Strm() );
    }

    if (!bIsWMF)
    {
        m_rExport.Strm() << "}" "{" OOO_STRING_SVTOOLS_RTF_NONSHPPICT;

        aStream.Seek(0);
        GraphicConverter::Export(aStream, aGraphic, CVT_WMF);
        pBLIPType = OOO_STRING_SVTOOLS_RTF_WMETAFILE;
        aStream.Seek(STREAM_SEEK_TO_END);
        nSize = aStream.Tell();
        pGraphicAry = (sal_uInt8*)aStream.GetData();

        ExportPICT(pFlyFrmFmt, aSize, aRendered, aMapped, rCr, pBLIPType, pGraphicAry, nSize, m_rExport, &m_rExport.Strm() );

        m_rExport.Strm() << '}';
    }

    m_rExport.Strm() << m_rExport.sNewLine;
}

void RtfAttributeOutput::BulletDefinition(int /*nId*/, const Graphic& rGraphic, Size aSize)
{
    m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_IGNORE OOO_STRING_SVTOOLS_RTF_SHPPICT;
    m_rExport.Strm() << "{" OOO_STRING_SVTOOLS_RTF_PICT OOO_STRING_SVTOOLS_RTF_PNGBLIP;

    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_PICWGOAL;
    m_rExport.OutULong(aSize.Width());
    m_rExport.Strm() << OOO_STRING_SVTOOLS_RTF_PICHGOAL;
    m_rExport.OutULong(aSize.Height());

    m_rExport.Strm() << RtfExport::sNewLine;
    const sal_uInt8* pGraphicAry = 0;
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, rGraphic, CVT_PNG) != ERRCODE_NONE)
        SAL_WARN("sw.rtf", "failed to export the numbering picture bullet");
    aStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nSize = aStream.Tell();
    pGraphicAry = (sal_uInt8*)aStream.GetData();
    RtfAttributeOutput::WriteHex(pGraphicAry, nSize, &m_rExport.Strm());
    m_rExport.Strm() << "}}"; // pict, shppict
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
