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
 * 문서정보를 읽어들이는 함수 ( 128 bytes )
 * 문서정보는 파일인식정보( 30 bytes ) 다음에 위치한 정보이다.
 */
bool HWPInfo::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&cur_col, 1);                     /* 문서를 저장할 당시의 커서가 위치한 문단번호 */
    hwpf.Read2b(&cur_row, 1);                     /* 문단 칸 */

    hwpf.Read1b(&paper.paper_kind, 1);            /* 용지 종류 */
    hwpf.Read1b(&paper.paper_direction, 1);       /* 용지 방향 */

// paper geometry information
    paper.paper_height = (short) hwpf.Read2b();   /* 용지 길이 */
    paper.paper_width = (short) hwpf.Read2b();    /* 용지 너비 */
    paper.top_margin = (short) hwpf.Read2b();     /* 위쪽 여백 */
    paper.bottom_margin = (short) hwpf.Read2b();  /* 아래쪽 여백 */
    paper.left_margin = (short) hwpf.Read2b();    /* 왼쪽 여백 */
    paper.right_margin = (short) hwpf.Read2b();   /* 오른쪽 여백 */
    paper.header_length = (short) hwpf.Read2b();  /* 머리말 길이 */
    paper.footer_length = (short) hwpf.Read2b();  /* 꼬리말 길이 */
    paper.gutter_length = (short) hwpf.Read2b();  /* 제본여백 */
    hwpf.Read2b(&readonly, 1);                    /* 예약 */
    hwpf.Read1b(reserved1, 4);                    /* 예약 */
    hwpf.Read1b(&chain_info.chain_page_no, 1);    /* 쪽 번호 연결 1-연결, 0-새로시작 (연결인쇄에서 사용) */
    hwpf.Read1b(&chain_info.chain_footnote_no, 1);/* 각주번호 연결 1-연결 0-새로시작 */
                                                  /* 연결인쇄할 파일의 이름 */
    hwpf.Read1b(chain_info.chain_filename, CHAIN_MAX_PATH);

    hwpf.Read1b(annotation, ANNOTATION_LEN);      /* 덧붙이는 말 ( 파일 저장할 때 덧붙이는 말에 지정한 내용 ) */
    hwpf.Read2b(&encrypted, 1);                   /* 암호 여부 0-보통파일, 그외-암호걸린 파일 */
//hwpf.Read1b(reserved2, 6);                      /* 아래 3개의값으로 바뀌었다. */
    hwpf.Read2b(&beginpagenum,1);                 /* 페이지시작번호 */

// footnote
    hwpf.Read2b(&beginfnnum,1);                   /* 각주 시작번호 */
    hwpf.Read2b(&countfn,1);                      /* 각주 갯수 */
    splinetext = (short) hwpf.Read2b();
    splinefn = (short) hwpf.Read2b();
    spfnfn = (short) hwpf.Read2b();
    hwpf.Read1b(&fnchar, 1);
    hwpf.Read1b(&fnlinetype, 1);
// border layout
    for (int ii = 0; ii < 4; ++ii)
        bordermargin[ii] = (short) hwpf.Read2b();
    hwpf.Read2b(&borderline, 1);

    hwpf.Read1b(&empty_line_hide, 1);
    hwpf.Read1b(&table_move, 1);
    hwpf.Read1b(&compressed, 1);

    hwpf.Read1b(&reserved3, 1);

    hwpf.Read2b(&info_block_len, 1);
    if (hwpf.State())
        return false;

/* 문서 요약을 읽는다. */
    if (!summary.Read(hwpf))
        return false;
    if (info_block_len > 0)
    {
        info_block = new unsigned char[info_block_len + 1];

        if (0 == info_block ||
            !HWPReadInfoBlock(info_block, info_block_len, hwpf))
            return false;
    }

/* hwpf의 값을 재설정 한다. */
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
    left_margin = (short) hwpf.Read2b();
    right_margin = (short) hwpf.Read2b();
    indent = (short) hwpf.Read2b();
    lspacing = (short) hwpf.Read2b();
    pspacing_next = (short) hwpf.Read2b();

    hwpf.Read1b(&condense, 1);
    hwpf.Read1b(&arrange_type, 1);
    for (int ii = 0; ii < MAXTABS; ii++)
    {
        hwpf.Read1b(&tabs[ii].type, 1);
        hwpf.Read1b(&tabs[ii].dot_continue, 1);
        tabs[ii].position = (short) hwpf.Read2b();
    }
    hwpf.Read1b(&coldef.ncols, 1);
    hwpf.Read1b(&coldef.separator, 1);
    coldef.spacing = (short) hwpf.Read2b();
    coldef.columnlen = (short) hwpf.Read2b();
    coldef.columnlen0 = (short) hwpf.Read2b();
    hwpf.Read1b(&shade, 1);
    hwpf.Read1b(&outline, 1);
    hwpf.Read1b(&outline_continue, 1);
    pspacing_prev = (short) hwpf.Read2b();

    hwpf.Read1b(reserved, 2);
    return (!hwpf.State());
}


bool CharShape::Read(HWPFile & hwpf)
{
    size = (short) hwpf.Read2b();
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
