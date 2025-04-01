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

#pragma once


#include "hwplib.h"
#include <string.h>

#include <vector>
#include <memory>

  40
  24

class HWPFile;
class CTextOut;
class CHTMLOut;

/**
 * Information of page (physical)
 */
struct PaperInfo
{
    unsigned char paper_kind;
    unsigned char paper_direction;
    hunit     paper_height;
    hunit     paper_width;
    hunit     top_margin;
    hunit     bottom_margin;
    hunit     left_margin;
    hunit     right_margin;
    hunit     header_length;
    hunit     footer_length;
    hunit     gutter_length;
    PaperInfo()
        : paper_kind(0)
        , paper_direction(0)
        , paper_height(0)
        , paper_width(0)
        , top_margin(0)
        , bottom_margin(0)
        , left_margin(0)
        , right_margin(0)
        , header_length(0)
        , footer_length(0)
        , gutter_length(0)
    {
    }
};

/* 페이지 배경색, 배경그림 정보 */
struct PaperBackInfo
{
    char type;  // 0- background color, 1 - external image, 2- embedded image
    char reserved1[8] = {};
    int luminance; /* 밝기 ( -100 ~ 100 ) */
    int contrast; /* 명암 ( -100 ~ 100 ) */
    char effect; /* 0-원래그림, 1-그레이스케일, 2-흑백 */
    char reserved2[8] = {};
    char filename[260 + 1] = {}; // filename
    unsigned char color[3] = {}; //0 - red, 1 - green, 2 - blue
    unsigned short flag; /* 0 - 바둑판식, 1 - 가운데로, 2 - 쪽크기, 3 - 용지크기 */
    int range; /* 0-전체, 1-첫페이지, 3-짝수쪽, 4-홀수쪽 */
    char reserved3[27] = {};
    int size;
    std::vector<char> data;        // image data
    bool isset;
    PaperBackInfo()
        : type(0)
        , luminance(0)
        , contrast(0)
        , effect(0)
        , flag(0)
        , range(0)
        , size(0)
        , isset(false)
    {
    }
};

/* 연결인쇄 정보 */
/**
 * Information of printing for chained page
 */
struct DocChainInfo
{
    unsigned char chain_page_no;
    unsigned char chain_footnote_no;
    unsigned char chain_filename[CHAIN_MAX_PATH] = {};
    DocChainInfo()
        : chain_page_no(0)
        , chain_footnote_no(0)
    {
    }
};

/* 문서 요약 */
/**
 * Summary of document
 */
struct HWPSummary
{
    unsigned short    title[56];
    unsigned short    subject[56];
    unsigned short    author[56];
    unsigned short    date[56];
    unsigned short    keyword[2][56];
    unsigned short    etc[3][56];

    bool Read(HWPFile &);
};

/**
  HWPInfo class는 HWPFile에 대한 정보를 담고 있는 클래스이다.
  종이나 여백 등의 레이아웃(layout) 정보는 쪽매기기에서
  사용하며, 기타 내부 정보를 포함하고 있다. \\

  HWPInfo 클래스는 HWPFile의 #GetHWPInfo()# 함수를 통해 얻을 수 있다.

  @memo Hwp 문서 정보
*/
/**
 * Contains the basic information of the hwp document:
 * margin, layout, paper and internal data.
 */
class DLLEXPORT HWPInfo
{
    public:
/**
 * The column index of a document that is saved at last
 */
        short     cur_col;
/**
 * The row index of a document that is saved at last
 */
        short     cur_row;
/**
 * Paper Information
 */
        PaperInfo paper;

        PaperBackInfo back_info;
/**
 * Sets the attribute of read-only or read/write.
 */
        short     readonly;
        unsigned char reserved1[4] = {};
/**
 * Information about document chain
 */
        DocChainInfo  chain_info;
        unsigned char annotation[ANNOTATION_LEN] = {};
        short     encrypted;
// unsigned char    reserved2[6];
        short     beginpagenum;                   /* 시작페이지 번호 */
/**
 * Information about footnote
 */
        short     beginfnnum;                     /* 각주시작번호 */
        short     countfn;                        /* 각주 갯수 */
        hunit     splinetext, splinefn;
        hunit     spfnfn;
        unsigned char fnchar;
        unsigned char fnlinetype;
// layout info
/**
 * Information about page layout
 */
        hunit     bordermargin[4] = {};
        short     borderline;

        unsigned char empty_line_hide;
        unsigned char table_move;
        unsigned char compressed;
        unsigned char reserved3;
        short     info_block_len;
/* 여기까지가 128 바이트이다 */
/**
 * Summary of document
 */
        HWPSummary    summary;
        std::unique_ptr<unsigned char[]> info_block;

        HWPInfo(void);
        ~HWPInfo(void);

        void Read(HWPFile &hwpf);

};

/* 글자 모양 구조체 */
/**
 * @short Style of character
 */
struct CharShape
{
/**
 * Index of character style
 */
    int       index;                              /* 스타일의 인덱스를 저장. */
/**
 * Font size
 */
    hunit     size;
    unsigned char color[2];
    unsigned char font;
    char space;               /* 자간 */
    unsigned char ratio;
    unsigned char shade;
    unsigned char attr;

    void Read(HWPFile &);

    CharShape();
};

/* 탭 설정에 대한 구조체 */

 40
/**
 * @short Tab properties
 */
struct TabSet
{
    unsigned char type;
    unsigned char dot_continue;
    hunit     position;
    TabSet()
        : type(0)
        , dot_continue(0)
        , position(0)
    {
    }
};

/**
 * @short Column properties
 */
struct ColumnDef
{
    unsigned char ncols;
    unsigned char separator;
    hunit     spacing;
    hunit     columnlen, columnlen0;
    ColumnDef()
        : ncols(0)
        , separator(0)
        , spacing(0)
        , columnlen(0)
        , columnlen0(0)
    {
    }
};

/**
 * @short Style of paragraph
 */
struct ParaShape
{
/**
 * Index of paragraph style
 */
    int       index;                              /* 스타일의 인덱스를 저장 */
    hunit     left_margin;
    hunit     right_margin;
    hunit     indent;
    hunit     lspacing;
    hunit     pspacing_prev;
    hunit     pspacing_next;
    unsigned char condense;
    unsigned char arrange_type;
    TabSet    tabs[MAXTABS];
    std::shared_ptr<ColumnDef> xColdef;
    unsigned char shade;
    unsigned char outline;
    unsigned char outline_continue;
    unsigned char reserved[2];
    std::shared_ptr<CharShape> cshape;
    unsigned char pagebreak;

    void  Read(HWPFile &);

    ParaShape();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
