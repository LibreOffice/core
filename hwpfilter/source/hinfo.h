/*************************************************************************
 *
 *  $RCSfile: hinfo.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:36:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* $Id: hinfo.h,v 1.1 2003-10-15 14:36:05 dvo Exp $ */

#ifndef _HWPINFO_H_
#define _HWPINFO_H_

#include "hwplib.h"

#define CHAIN_MAX_PATH  40
#define ANNOTATION_LEN  24

class HWPFile;
class CTextOut;
class CHTMLOut;

/**
 * Information of page (phisical)
 */
typedef struct
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
} PaperInfo;

// 페이지 배경색, 배경그림 정보
typedef struct
{
     char type;  // 0- background color, 1 - external image, 2- embeded image
     char reserved1[8];
     int luminance; // 밝기 ( -100 ~ 100 )
     int contrast; // 명암 ( -100 ~ 100 )
     char effect; // 0-원래그림, 1-그레이스케일, 2-흑백
     char reserved2[8];
     char filename[260 + 1]; // filename
     unsigned char color[3]; //0 - red, 1 - green, 2 - blue
     unsigned short flag; // 0 - 바둑판식, 1 - 가운데로, 2 - 쪽크기, 3 - 용지크기
     int range; // 0-전체, 1-첫페이지, 3-짝수쪽, 4-홀수쪽
     char reserved3[27];
     int size;
     char *data;        // image data
     bool isset;
} PaperBackInfo;

//연결인쇄 정보
/**
 * Information of printing for chained page
 */
typedef struct
{
    unsigned char chain_page_no;
    unsigned char chain_footnote_no;
    unsigned char chain_filename[CHAIN_MAX_PATH];
} DocChainInfo;

// 문서 요약
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
        unsigned char reserved1[4];
/**
 * Information about document chain
 */
        DocChainInfo  chain_info;
        unsigned char annotation[ANNOTATION_LEN];
        short     encrypted;
// unsigned char    reserved2[6];
        short     beginpagenum;                   //시작페이지 번호
/**
 * Information about footnote
 */
        short     beginfnnum;                     // 각주시작번호
        short     countfn;                        // 각주 갯수
        hunit     splinetext, splinefn;
        hunit     spfnfn;
        unsigned char fnchar;
        unsigned char fnlinetype;
// layout info
/**
 * Information about page layout
 */
        hunit     bordermargin[4];
        short     borderline;

        unsigned char empty_line_hide;
        unsigned char table_move;
        unsigned char compressed;
        unsigned char reserved3;
        short     info_block_len;
// 여기까지가 128 바이트이다
/**
 * Summary of document
 */
        HWPSummary    summary;
        unsigned char *info_block;

        HWPInfo(void);
        ~HWPInfo(void);

        bool Read(HWPFile &hwpf);
        bool Write(CTextOut &txtf);
        bool Write(CHTMLOut &html);

/// 용지 방향을 고려한 종이의 넓이를 계산한다.
/**
 * Returns the width of paper regarding page orientation
 */
        hunit PageWid(void);
/// 용지 방향을 고려한 종이의 길이를 계산한다.
/**
 * Returns the length of paper regarding page orientation
 */
        hunit PageLen(void);
/**
 * Returns the edit-enable width of paper
 */
        hunit PageEditWid(void);
/**
 * Returns the edit-enable length of paper
 */
        hunit PageEditLen(void);
};

// 글자 모양 구조체
/**
 * @short Style of character
 */
struct CharShape
{
/**
 * Index of character style
 */
    int       index;                              // 스타일의 인덱스를 저장.
/**
 * Font size
 */
    hunit     size;
    unsigned char font[NLanguage];
    unsigned char ratio[NLanguage];
    signed char   space[NLanguage];               // 자간
    unsigned char color[2];
    unsigned char shade;
    unsigned char attr;
    unsigned char reserved[4];

    bool Read(HWPFile &);
};

// 탭 설정에 대한 구조체

#define MAXTABS 40
/**
 * @short Tab properties
 */
typedef struct
{
    unsigned char type;
    unsigned char dot_continue;
    hunit     position;
} TabSet;

/**
 * @short Column properties
 */
typedef struct
{
    unsigned char ncols;
    unsigned char separator;
    hunit     spacing;
    hunit     columnlen, columnlen0;
} ColumnDef;

/**
 * @short Style of paragraph
 */
struct ParaShape
{
/**
 * Index of paragraph style
 */
    int       index;                              // 스타일의 인덱스를 저장
    hunit     left_margin;
    hunit     right_margin;
    hunit     indent;
    hunit     lspacing;
    hunit     pspacing_prev;
    hunit     pspacing_next;
    unsigned char condense;
    unsigned char arrange_type;
    TabSet    tabs[MAXTABS];
    ColumnDef coldef;
    unsigned char shade;
    unsigned char outline;
    unsigned char outline_continue;
    unsigned char reserved[2];
    CharShape *cshape;
     unsigned char pagebreak;

    bool  Read(HWPFile &);
//  virtual ~ParaShape();
};
#endif                                            /* _HWPINFO_H_ */
