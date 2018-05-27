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
        return hwpf.ReadBlock(ptr, len) ? true : false;
}


// Document Information

HWPInfo::HWPInfo(void)
{
    info_block = 0;
     back_info.isset = false;
}


HWPInfo::~HWPInfo(void)
{
    if (info_block)
        delete[]info_block;
    info_block = 0;
}


/**
 * ¹®¼­Á¤º¸¸¦ ÀĞ¾îµéÀÌ´Â ÇÔ¼ö ( 128 bytes )
 * ¹®¼­Á¤º¸´Â ÆÄÀÏÀÎ½ÄÁ¤º¸( 30 bytes ) ´ÙÀ½¿¡ À§Ä¡ÇÑ Á¤º¸ÀÌ´Ù.
 */
bool HWPInfo::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&cur_col, 1);                     /* ¹®¼­¸¦ ÀúÀåÇÒ ´ç½ÃÀÇ Ä¿¼­°¡ À§Ä¡ÇÑ ¹®´Ü¹øÈ£ */
    hwpf.Read2b(&cur_row, 1);                     /* ¹®´Ü Ä­ */

    hwpf.Read1b(&paper.paper_kind, 1);            /* ¿ëÁö Á¾·ù */
    hwpf.Read1b(&paper.paper_direction, 1);

    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.paper_height = tmp16;                   /* ìš©ì§€ ê¸¸ì´ */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.paper_width = tmp16;                    /* ìš©ì§€ ë„ˆë¹„ */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.top_margin = tmp16;                     /* ìœ„ìª½ ì—¬ë°± */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.bottom_margin = tmp16;                  /* ì•„ë˜ìª½ ì—¬ë°± */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.left_margin = tmp16;                    /* ì™¼ìª½ ì—¬ë°± */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.right_margin = tmp16;                   /* ì˜¤ë¥¸ìª½ ì—¬ë°± */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.header_length = tmp16;                  /* ë¨¸ë¦¬ë§ ê¸¸ì´ */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.footer_length = tmp16;                  /* ê¼¬ë¦¬ë§ ê¸¸ì´ */
    if (!hwpf.Read2b(tmp16))
        return false;
    paper.gutter_length = tmp16;                  /* ì œë³¸ì—¬ë°± */
    hwpf.Read2b(&readonly, 1);
    hwpf.Read1b(reserved1, 4);
    hwpf.Read1b(&chain_info.chain_page_no, 1);
    hwpf.Read1b(&chain_info.chain_footnote_no, 1);/* °¢ÁÖ¹øÈ£ ¿¬°á 1-¿¬°á 0-»õ·Î½ÃÀÛ */
                                                  /* ¿¬°áÀÎ¼âÇÒ ÆÄÀÏÀÇ ÀÌ¸§ */
    hwpf.Read1b(chain_info.chain_filename, CHAIN_MAX_PATH);

    hwpf.Read1b(annotation, ANNOTATION_LEN);      /* µ¡ºÙÀÌ´Â ¸» ( ÆÄÀÏ ÀúÀåÇÒ ¶§ µ¡ºÙÀÌ´Â ¸»¿¡ ÁöÁ¤ÇÑ ³»¿ë ) */
    hwpf.Read2b(&encrypted, 1);                   /* ¾ÏÈ£ ¿©ºÎ 0-º¸ÅëÆÄÀÏ, ±×¿Ü-¾ÏÈ£°É¸° ÆÄÀÏ */
//hwpf.Read1b(reserved2, 6);                      /* ¾Æ·¡ 3°³ÀÇ°ªÀ¸·Î ¹Ù²î¾ú´Ù. */
    hwpf.Read2b(&beginpagenum,1);                 /* ÆäÀÌÁö½ÃÀÛ¹øÈ£ */

// footnote
    hwpf.Read2b(&beginfnnum,1);
    hwpf.Read2b(&countfn,1);

    if (!hwpf.Read2b(tmp16))
        return false;
    splinetext = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    splinefn = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    spfnfn = tmp16;
    hwpf.Read1b(&fnchar, 1);
    hwpf.Read1b(&fnlinetype, 1);
// border layout
    for (int ii = 0; ii < 4; ++ii)
    {
        if (!hwpf.Read2b(tmp16))
            return false;
        bordermargin[ii] = tmp16;
    }
    hwpf.Read2b(&borderline, 1);

    hwpf.Read1b(&empty_line_hide, 1);
    hwpf.Read1b(&table_move, 1);
    hwpf.Read1b(&compressed, 1);

    hwpf.Read1b(&reserved3, 1);

    hwpf.Read2b(&info_block_len, 1);
    if (hwpf.State())
        return false;

/* ¹®¼­ ¿ä¾àÀ» ÀĞ´Â´Ù. */
    if (!summary.Read(hwpf))
        return false;
    if (info_block_len > 0)
    {
        info_block = new unsigned char[info_block_len + 1];

        if (0 == info_block ||
            !HWPReadInfoBlock(info_block, info_block_len, hwpf))
            return false;
    }

/* hwpfÀÇ °ªÀ» Àç¼³Á¤ ÇÑ´Ù. */
    hwpf.compressed = compressed ? true : false;
    hwpf.encrypted = encrypted ? true : false;
    hwpf.info_block_len = info_block_len;
    hwpf.SetCompressed(hwpf.compressed);

    return (!hwpf.State());
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


bool ParaShape::Read(HWPFile & hwpf)
{
    pagebreak = 0;
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    left_margin = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    right_margin = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    indent = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    lspacing = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    pspacing_next = tmp16;

    hwpf.Read1b(&condense, 1);
    hwpf.Read1b(&arrange_type, 1);
    for (int ii = 0; ii < MAXTABS; ii++)
    {
        hwpf.Read1b(&tabs[ii].type, 1);
        hwpf.Read1b(&tabs[ii].dot_continue, 1);
        if (!hwpf.Read2b(tmp16))
            return false;
        tabs[ii].position = tmp16;
    }
    hwpf.Read1b(&coldef.ncols, 1);
    hwpf.Read1b(&coldef.separator, 1);
    if (!hwpf.Read2b(tmp16))
        return false;
    coldef.spacing = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    coldef.columnlen = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    coldef.columnlen0 = tmp16;
    hwpf.Read1b(&shade, 1);
    hwpf.Read1b(&outline, 1);
    hwpf.Read1b(&outline_continue, 1);
    if (!hwpf.Read2b(tmp16))
        return false;
    pspacing_prev = tmp16;

    hwpf.Read1b(reserved, 2);
    return (!hwpf.State());
}


bool CharShape::Read(HWPFile & hwpf)
{
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    size = tmp16;
    hwpf.Read1b(font, NLanguage);
    hwpf.Read1b(ratio, NLanguage);
    hwpf.Read1b(space, NLanguage);
    hwpf.Read1b(color, 2);
    hwpf.Read1b(&shade, 1);
    hwpf.Read1b(&attr, 1);
    hwpf.Read1b(reserved, 4);

    return (!hwpf.State());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
