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

#include "precompile.h"

#include "hwplib.h"
#include "hinfo.h"
#include "hwpfile.h"

// Info Block


static bool HWPReadInfoBlock(void *ptr, int len, HWPFile & hwpf)
{
    hwpf.info_block_len = len;
    if (0 == len)
        return true;
    else
        return hwpf.ReadBlock(ptr, len) != 0;
}


// Document Information
HWPInfo::HWPInfo()
    : cur_col(0)
    , cur_row(0)
    , readonly(0)
    , encrypted(0)
    , beginpagenum(0)
    , beginfnnum(0)
    , countfn(0)
    , splinetext(0)
    , splinefn(0)
    , spfnfn(0)
    , fnchar(0)
    , fnlinetype(0)
    , borderline(0)
    , empty_line_hide(0)
    , table_move(0)
    , compressed(0)
    , reserved3(0)
    , info_block_len(0)
    , info_block(nullptr)
{
    back_info.isset = false;
    memset(reserved1, 0, sizeof(reserved1));
    memset(annotation, 0, sizeof(annotation));
    memset(bordermargin, 0, sizeof(bordermargin));
}

HWPInfo::~HWPInfo()
{
}


/**
 * Function for reading document information (128 bytes)
 * Document information is the information after the file identification information (30 bytes).
 */
void HWPInfo::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&cur_col, 1);                     /* When a document is saving, the paragraph number where the coursor is */
    hwpf.Read2b(&cur_row, 1);                     /* Paragraphs rows */

    hwpf.Read1b(paper.paper_kind);                /* Paper Type */
    hwpf.Read1b(paper.paper_direction);           /* Paper orientation */

// paper geometry information
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    paper.paper_height = tmp16;                   /* Paper length */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.paper_width = tmp16;                    /* Sheet width */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.top_margin = tmp16;                     /* Top margin */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.bottom_margin = tmp16;                  /* The bottom margin */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.left_margin = tmp16;                    /* Left Margin */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.right_margin = tmp16;                   /* Right margins */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.header_length = tmp16;                  /* Header length */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.footer_length = tmp16;                  /* Footer length */
    if (!hwpf.Read2b(tmp16))
        return;
    paper.gutter_length = tmp16;                  /* The binding margin */
    hwpf.Read2b(&readonly, 1);                    /* Reserve */
    hwpf.ReadBlock(reserved1, 4);                 /* Reserve */
    hwpf.Read1b(chain_info.chain_page_no);        /* Connect page number: 1-Connect, 0-newly started (used in connection printing) */
    hwpf.Read1b(chain_info.chain_footnote_no);    /* Connect footnote number: 1-connect, 0-newly started*/
                                                  /* the file name to be printed with connection */
    hwpf.ReadBlock(chain_info.chain_filename, CHAIN_MAX_PATH);

    hwpf.ReadBlock(annotation, ANNOTATION_LEN);   /* Annotation (additional information when a file is saving.) */
    hwpf.Read2b(&encrypted, 1);                   /* encrypt: 0-normal file(without password), 1-protected by password */
    hwpf.Read2b(&beginpagenum,1);                 /* Page starting number */

// footnote
    hwpf.Read2b(&beginfnnum,1);                   /* Footnote start number */
    hwpf.Read2b(&countfn,1);                      /* Number of footnote */

    if (!hwpf.Read2b(tmp16))
        return;
    splinetext = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    splinefn = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    spfnfn = tmp16;
    hwpf.Read1b(fnchar);
    hwpf.Read1b(fnlinetype);
// border layout
    for (int & ii : bordermargin)
    {
        if (!hwpf.Read2b(tmp16))
            return;
        ii = tmp16;
    }
    hwpf.Read2b(&borderline, 1);

    hwpf.Read1b(empty_line_hide);
    hwpf.Read1b(table_move);
    hwpf.Read1b(compressed);

    hwpf.Read1b(reserved3);

    hwpf.Read2b(&info_block_len, 1);
    if (hwpf.State())
        return;

/* Read the article summary. */
    if (!summary.Read(hwpf))
        return;
    if (info_block_len > 0)
    {
        info_block.reset( new unsigned char[info_block_len + 1] );

        if (!HWPReadInfoBlock(info_block.get(), info_block_len, hwpf))
            return;
    }

/* reset the value of hwpf. */
    hwpf.compressed = compressed != 0;
    hwpf.encrypted = encrypted != 0;
    hwpf.info_block_len = info_block_len;
    hwpf.SetCompressed(hwpf.compressed);
}


// Document Summary

bool HWPSummary::Read(HWPFile & hwpf)
{
    hwpf.Read2b(title, 56);
    hwpf.Read2b(subject, 56);
    hwpf.Read2b(author, 56);
    hwpf.Read2b(date, 56);
    hwpf.Read2b(keyword[0], 56);
    hwpf.Read2b(keyword[1], 56);
    hwpf.Read2b(etc[0], 56);
    hwpf.Read2b(etc[1], 56);
    hwpf.Read2b(etc[2], 56);

    return (!hwpf.State());
}

ParaShape::ParaShape()
    : index(0)
    , left_margin(0)
    , right_margin(0)
    , indent(0)
    , lspacing(0)
    , pspacing_prev(0)
    , pspacing_next(0)
    , condense(0)
    , arrange_type(0)
    , shade(0)
    , outline(0)
    , outline_continue(0)
    , pagebreak(0)
{
    reserved[0] = 0;
    reserved[1] = 0;
}

void ParaShape::Read(HWPFile & hwpf)
{
    pagebreak = 0;
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    left_margin = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    right_margin = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    indent = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    lspacing = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    pspacing_next = tmp16;

    hwpf.Read1b(condense);
    hwpf.Read1b(arrange_type);
    for (TabSet & tab : tabs)
    {
        hwpf.Read1b(tab.type);
        hwpf.Read1b(tab.dot_continue);
        if (!hwpf.Read2b(tmp16))
            return;
        tab.position = tmp16;
    }
    hwpf.Read1b(coldef.ncols);
    hwpf.Read1b(coldef.separator);
    if (!hwpf.Read2b(tmp16))
        return;
    coldef.spacing = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    coldef.columnlen = tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    coldef.columnlen0 = tmp16;
    hwpf.Read1b(shade);
    hwpf.Read1b(outline);
    hwpf.Read1b(outline_continue);
    if (!hwpf.Read2b(tmp16))
        return;
    pspacing_prev = tmp16;

    hwpf.ReadBlock(reserved, 2);
}


void CharShape::Read(HWPFile & hwpf)
{
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return;
    size = tmp16;
    hwpf.ReadBlock(font, NLanguage);
    hwpf.ReadBlock(ratio, NLanguage);
    hwpf.ReadBlock(space, NLanguage);
    hwpf.ReadBlock(color, 2);
    hwpf.Read1b(shade);
    hwpf.Read1b(attr);
    hwpf.ReadBlock(reserved, 4);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
