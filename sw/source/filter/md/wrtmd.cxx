/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <editeng/formatbreakitem.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/Writer.hxx>

#include <fmtpdsc.hxx>
#include <mdiexp.hxx>
#include <ndtxt.hxx>
#include <strings.hrc>
#include "wrtmd.hxx"

namespace
{
/* Output of the nodes*/
void OutMarkdown_SwTextNode(SwMDWriter& /*rWrt*/, const SwTextNode& /*rNode*/) {}
}

SwMDWriter::SwMDWriter(const OUString& rBaseURL) { SetBaseURL(rBaseURL); }

ErrCode SwMDWriter::WriteStream()
{
    if (m_bShowProgress)
        ::StartProgress(STR_STATSTR_W4WWRITE, 0, sal_Int32(m_pDoc->GetNodes().Count()),
                        m_pDoc->GetDocShell());

    // respect table and section at document beginning
    {
        if (m_bWriteAll)
        {
            while (const SwStartNode* pTNd = m_pCurrentPam->GetPointNode().FindTableBoxStartNode())
            {
                // start with table node !!
                m_pCurrentPam->GetPoint()->Assign(*pTNd->FindTableNode());

                if (m_bWriteOnlyFirstTable)
                    m_pCurrentPam->GetMark()->Assign(
                        *m_pCurrentPam->GetPointNode().EndOfSectionNode());
            }
        }

        // first node (which can contain a page break)
        m_nStartNodeIndex = m_pCurrentPam->GetPoint()->GetNode().GetIndex();

        for (SwSectionNode* pSNd = m_pCurrentPam->GetPointNode().FindSectionNode(); pSNd;
             pSNd = pSNd->StartOfSectionNode()->FindSectionNode())
        {
            if (m_bWriteAll)
            {
                // start with section node !!
                m_pCurrentPam->GetPoint()->Assign(*pSNd);
            }
        }
    }

    Out_SwDoc(m_pOrigPam);

    if (m_bShowProgress)
        ::EndProgress(m_pDoc->GetDocShell());
    return ERRCODE_NONE;
}

void SwMDWriter::Out_SwDoc(SwPaM* pPam)
{
    bool bSaveWriteAll = m_bWriteAll;
    bool bIncludeHidden
        = officecfg::Office::Writer::FilterFlags::Markdown::IncludeHiddenText::get();
    bool bFirstLine = true;

    do
    {
        m_bWriteAll = bSaveWriteAll;

        while (*m_pCurrentPam->GetPoint() <= *m_pCurrentPam->GetMark())
        {
            SwNode& rNd = m_pCurrentPam->GetPointNode();

            SAL_WARN_IF(rNd.IsGrfNode() || rNd.IsOLENode(), "sw.md",
                        "Unexpected Grf- or OLE-Node here");

            if (SwTextNode* pTextNd = rNd.GetTextNode())
            {
                if (bIncludeHidden || !pTextNd->IsHidden())
                {
                    if (!bFirstLine)
                        m_pCurrentPam->GetPoint()->SetContent(0);

                    OutMarkdown_SwTextNode(*this, *pTextNd);
                }
            }
            else if (rNd.IsTableNode())
            {
                // TODO
            }
            else if (rNd.IsSectionNode())
            {
                SwSectionNode* pSectionNode = rNd.GetSectionNode();
                if (!pSectionNode->GetSection().IsHiddenFlag() || bIncludeHidden)
                {
                    // TODO
                }
            }
            else if (&rNd == &m_pDoc->GetNodes().GetEndOfContent())
                break;

            m_pCurrentPam->GetPoint()->Adjust(SwNodeOffset(+1)); // move
            SwNodeOffset nPos = m_pCurrentPam->GetPoint()->GetNodeIndex();

            if (m_bShowProgress)
                ::SetProgressState(sal_Int32(nPos), m_pDoc->GetDocShell()); // How far ?

            /* If only the selected area should be saved, so only the complete
             * nodes should be saved, this means the first and n-th node
             * partly, the 2nd till n-1 node complete. (complete means with
             * all formats!)
             */
            m_bWriteAll = bSaveWriteAll || nPos != m_pCurrentPam->GetMark()->GetNodeIndex();
            bFirstLine = false;
        }
    } while (CopyNextPam(&pPam)); // until all PaM's processed

    m_bWriteAll = bSaveWriteAll; // reset to old values
}

void GetMDWriter(std::u16string_view /*rFilterOptions*/, const OUString& rBaseURL, WriterRef& xRet)
{
    xRet = new SwMDWriter(rBaseURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
