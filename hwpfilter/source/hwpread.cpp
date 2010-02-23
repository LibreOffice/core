/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

/* $Id: hwpread.cpp,v 1.5 2008-04-10 12:10:14 rt Exp $ */

#include "precompile.h"

#include "list.hxx"

#include "hwpfile.h"
#include "hbox.h"
#include "hpara.h"
#include "drawing.h"
#include "htags.h"
#include "hcode.h"

static short fboxnum = 1;
static int zindex = 1;
static int lnnumber = 0;

int HBox::Read(HWPFile & )
{
// already read
    return 1;
}


// skip block

int SkipBlock::Read(HWPFile & hwpf)
{
    hwpf.Read4b(&data_block_len, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(IS_SP_SKIP_BLOCK(hh) && (hh == dummy))){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }

    data_block = new char[data_block_len];

    return hwpf.Read1b(data_block, data_block_len);
}


// Field code(5)
int FieldCode::Read(HWPFile & hwpf)
{
    ulong size;
    hchar dummy;
    ulong len1;       /* hchar타입의 문자열 테이터 #1의 길이 */
    ulong len2;       /* hchar타입의 문자열 테이터 #2의 길이 */
    ulong len3;       /* hchar타입의 문자열 테이터 #3의 길이 */
    ulong binlen;     /* 임의 형식의 바이너리 데이타 길이 */

    hwpf.Read4b(&size, 1);
    hwpf.Read2b(&dummy, 1);
    hwpf.Read1b(&type, 2);
    hwpf.Read4b(reserved1, 1);
    hwpf.Read2b(&location_info, 1);
    hwpf.Read1b(reserved2, 22);
    hwpf.Read4b(&len1, 1);
    hwpf.Read4b(&len2, 1);
    hwpf.Read4b(&len3, 1);
    hwpf.Read4b(&binlen, 1);

    str1 = new hchar[len1];
    str2 = new hchar[len2];
    str3 = new hchar[len3];
    bin = new char[binlen];

    hwpf.Read2b(str1, len1/2);
    hwpf.Read2b(str2, len2/2);
    hwpf.Read2b(str3, len3/2);

    hwpf.ReadBlock(bin, binlen);

     if( type[0] == 3 && type[1] == 2 ){ /* 만든날짜로서 포맷을 생성해야 한다. */
          DateCode *pDate = new DateCode;
          for( int i = 0 ;i < (int)(len3/2) ; i++ ){
                if(str3[i] == 0 ) break;
                if( i >= DATE_SIZE ) break;
                pDate->format[i] = str3[i];
          }
          hwpf.AddDateFormat(pDate);
          m_pDate = pDate;
     }

    return true;
}


// book mark(6)
int Bookmark::Read(HWPFile & hwpf)
{
    long len;

    hwpf.Read4b(&len, 1);
    dummy = sal::static_int_cast<hchar>(hwpf.Read2b());

    if (!(len == 34))// 2 * (BMK_COMMENT_LEN + 1) + 2
     {
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    if (!(hh == dummy && dummy == CH_BOOKMARK)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }

    hwpf.Read2b(id, BMK_COMMENT_LEN + 1);
    hwpf.Read2b(&type, 1);
//return hwpf.Read2b(&type, 1);
    return 1;
}


// date format(7)

int DateFormat::Read(HWPFile & hwpf)
{
    hwpf.Read2b(format, DATE_SIZE);
    dummy = sal::static_int_cast<hchar>(hwpf.Read2b());
    if (!(hh == dummy && CH_DATE_FORM == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return true;
}


// date code(8)

int DateCode::Read(HWPFile & hwpf)
{
    hwpf.Read2b(format, DATE_SIZE);
    hwpf.Read2b(date, 6);
    dummy = sal::static_int_cast<hchar>(hwpf.Read2b());
    if (!(hh == dummy && CH_DATE_CODE == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    hwpf.AddDateFormat(this);
    return true;
}


// tab(9)

int Tab::Read(HWPFile & hwpf)
{
    width = hwpf.Read2b();
    leader = sal::static_int_cast<unsigned short>(hwpf.Read2b());
    dummy = sal::static_int_cast<hchar>(hwpf.Read2b());
    if (!(hh == dummy && CH_TAB == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return true;
}


// tbox(10) TABLE BOX MATH BUTTON HYPERTEXT

static void UpdateBBox(FBox * fbox)
{
    fbox->boundsy = fbox->pgy;
    fbox->boundey = fbox->pgy + fbox->ys - 1;
}


void Cell::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&p, 1);
    hwpf.Read2b(&color, 1);
    hwpf.Read2b(&x, 1);
    hwpf.Read2b(&y, 1);
    hwpf.Read2b(&w, 1);
    hwpf.Read2b(&h, 1);
    hwpf.Read2b(&txthigh, 1);
    hwpf.Read2b(&cellhigh, 1);

    hwpf.Read1b(&flag, 1);
    hwpf.Read1b(&changed, 1);
    hwpf.Read1b(&used, 1);
    hwpf.Read1b(&ver_align, 1);
    hwpf.Read1b(linetype, 4);
    hwpf.Read1b(&shade, 1);
    hwpf.Read1b(&diagonal, 1);
    hwpf.Read1b(&protect, 1);
}


int TxtBox::Read(HWPFile & hwpf)
{
    int ii, ncell;

    hwpf.Read2b(reserved, 2);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy && CH_TEXT_BOX == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    hwpf.AddBox(this);
    hwpf.Read2b(&style.cap_len, 1);
    hwpf.Read2b(&dummy1, 1);
    hwpf.Read2b(&next, 1);
    hwpf.Read2b(&dummy2, 1);

    style.boxnum = fboxnum++;
     zorder = zindex++;
    hwpf.Read1b(&style.anchor_type, 1);
    hwpf.Read1b(&style.txtflow, 1);
    hwpf.Read2b(&style.xpos, 1);
    hwpf.Read2b(&style.ypos, 1);
    hwpf.Read2b(&option, 1);
    hwpf.Read2b(&ctrl_ch, 1);
    hwpf.Read2b(style.margin, 12);
    hwpf.AddFBoxStyle(&style);
    hwpf.Read2b(&box_xs, 1);
    hwpf.Read2b(&box_ys, 1);
    hwpf.Read2b(&cap_xs, 1);
    hwpf.Read2b(&cap_ys, 1);
    hwpf.Read2b(&style.cap_len, 1);
    hwpf.Read2b(&xs, 1);
    hwpf.Read2b(&ys, 1);
    hwpf.Read2b(&cap_margin, 1);
    hwpf.Read1b(&xpos_type, 1);
    hwpf.Read1b(&ypos_type, 1);
    hwpf.Read1b(&smart_linesp, 1);
    hwpf.Read1b(&reserved1, 1);
    hwpf.Read2b(&pgx, 1);
    hwpf.Read2b(&pgy, 1);
    hwpf.Read2b(&pgno, 1);
     if( ( pgno +1 ) != hwpf.getCurrentPage() )
          pgno = sal::static_int_cast<short>(hwpf.getCurrentPage() -1) ;

    hwpf.Read2b(&showpg, 1);
    hwpf.Read2b(&cap_pos, 1);
    hwpf.Read2b(&num, 1);
    hwpf.Read2b(&dummy3, 1);
    hwpf.Read2b(&baseline, 1);
    hwpf.Read2b(&type, 1);
    hwpf.Read2b(&nCell, 1);
    hwpf.Read2b(&protect, 1);
    switch (type)
    {
        case 0:                                   //table
            style.boxtype = 'T';
            break;
        case 1:                                   // text-box
            style.boxtype = 'X';
            break;
        case 2:                                   // equation
            style.boxtype = 'E';
            break;
        case 3:                                   // button
            style.boxtype = 'B';
            break;
        default:                                  // other
            style.boxtype = 'O';
            break;
    }

    UpdateBBox(this);

    ncell = NCell();
    if (!(ncell > 0)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }

    cell = new Cell[ncell];
    for (ii = 0; ii < ncell; ii++)
    {
        cell[ii].Read(hwpf);
          cell[ii].key = sal::static_int_cast<unsigned char>(ii);
    }
    if (ncell == 1)
        style.cell = &cell[0];
    plists = new LinkedList < HWPPara >[ncell];
    for (ii = 0; ii < ncell; ii++)
        hwpf.ReadParaList(plists[ii]);
     // caption
    hwpf.ReadParaList(caption);

     if( type == 0 ){ // if table?
          Table *tbl = new Table;
          for( ii = 0 ; ii < ncell; ii++)
          {
                tbl->columns.insert(cell[ii].x);
                tbl->columns.insert(cell[ii].x + cell[ii].w);
                tbl->rows.insert(cell[ii].y);
                tbl->rows.insert(cell[ii].y + cell[ii].h);
          }
          TCell* *pArr = new TCell*[ncell];
          for( ii = 0 ; ii < ncell; ii++)
          {
                TCell *tcell = new TCell;
                tcell->nColumnIndex = tbl->columns.getIndex(cell[ii].x);
                tcell->nColumnSpan = tbl->columns.getIndex(cell[ii].x + cell[ii].w) -
                     tcell->nColumnIndex;
                tcell->nRowIndex = tbl->rows.getIndex(cell[ii].y);
                tcell->nRowSpan = tbl->rows.getIndex(cell[ii].y + cell[ii].h) -
                     tcell->nRowIndex;
                tcell->pCell = &cell[ii];
                pArr[ii] = tcell;
          }
          TCell *tmp;
          // Sort by row and column
          for( ii = 0 ; ii < ncell - 1; ii++ ){
                for( int jj = ii ; jj < ncell ; jj++){
                     if( pArr[ii]->nRowIndex > pArr[jj]->nRowIndex ){
                          tmp = pArr[ii];
                          pArr[ii] = pArr[jj];
                          pArr[jj] = tmp;
                     }
                }
                for( int kk = ii ; kk > 0 ; kk--){
                     if( ( pArr[kk]->nRowIndex == pArr[kk-1]->nRowIndex ) &&
                            (pArr[kk]->nColumnIndex < pArr[kk-1]->nColumnIndex )){
                          tmp = pArr[kk];
                          pArr[kk] = pArr[kk-1];
                          pArr[kk-1] = tmp;
                     }
                }
          }
          for( ii = 0 ; ii < ncell ; ii++ ){
                tbl->cells.insert(pArr[ii]);
          }
          tbl->box = this;
          hwpf.AddTable(tbl);
          m_pTable = tbl;
          delete[] pArr;
     }
     else
          m_pTable = 0L;

    return !hwpf.State();
}


// picture(11)
int Picture::Read(HWPFile & hwpf)
{
    hwpf.Read2b(reserved, 2);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy && CH_PICTURE == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    hwpf.AddBox(this);

    hwpf.Read4b(&follow_block_size, 1);
    hwpf.Read2b(&dummy1, 1);                      /* 예약 4바이트 */
    hwpf.Read2b(&dummy2, 1);

    style.boxnum = fboxnum++;
     zorder = zindex++;
    hwpf.Read1b(&style.anchor_type, 1);           /* 기준위치 */
    hwpf.Read1b(&style.txtflow, 1);               /* 그림피함. 0-2(자리차지,투명,어울림) */
    hwpf.Read2b(&style.xpos, 1);                  /* 가로위치 : 1 왼쪽, 2오른쪽, 3 가운데, 이외 = 임의 */
    hwpf.Read2b(&style.ypos, 1);                  /* 세로위치 : 1 위, 2 아래, 3 가운데, 이외 임의 */
    hwpf.Read2b(&option, 1);                      /* 기타옵션 : 테두리,그림반전,등. bit로 저장. */
    hwpf.Read2b(&ctrl_ch, 1);                     /* 항상 11 */
    hwpf.Read2b(style.margin, 12);                /* 여백 : [0-2][] out/in/셀,[][0-3] 왼/오른/위/아래 여백 */
    hwpf.Read2b(&box_xs, 1);                      /* 박스크기 가로 */
    hwpf.Read2b(&box_ys, 1);                      /* 세로 */
    hwpf.Read2b(&cap_xs, 1);                      /* 캡션 크기 가로 */
    hwpf.Read2b(&cap_ys, 1);                      /* 세로 */
    hwpf.Read2b(&style.cap_len, 1);               /* 길이 */
    hwpf.Read2b(&xs, 1);                          /* 전체 크기(박스 크기 + 캡션 + 여백) 가로 */
    hwpf.Read2b(&ys, 1);                          /* 세로 */
    hwpf.Read2b(&cap_margin, 1);                  /* 캡션 여백 */
    hwpf.Read1b(&xpos_type, 1);
    hwpf.Read1b(&ypos_type, 1);
    hwpf.Read1b(&smart_linesp, 1);                /* 줄간격 보호 : 0 미보호, 1 보호 */
    hwpf.Read1b(&reserved1, 1);
    hwpf.Read2b(&pgx, 1);                         /* 실제 계산된 박스 가로 */
    hwpf.Read2b(&pgy, 1);                         /* 세로 */
    hwpf.Read2b(&pgno, 1);                        /* 페이지 숫자 : 0부터 시작 */
    hwpf.Read2b(&showpg, 1);                      /* 박스보여줌 */
    hwpf.Read2b(&cap_pos, 1);                     /* 캡션위치 0 - 7 메뉴순서. */
    hwpf.Read2b(&num, 1);                         /* 박스번호 0부터 시작해서 매긴일련번호 */

    hwpf.Read1b(&pictype, 1);                     /* 그림종류 */

    skip[0] = (short) hwpf.Read2b();              /* 그림에서 실제 표시를 시작할 위치 가로 */
    skip[1] = (short) hwpf.Read2b();              /* 세로 */
    scale[0] = (short) hwpf.Read2b();             /* 확대비율 : 0 고정, 이외 퍼센트 단위 가로 */
    scale[1] = (short) hwpf.Read2b();             /* 세로 */

    hwpf.Read1b(picinfo.picun.path, 256);         /* 그림파일 이름 : 종류가 Drawing이 아닐때. */
    hwpf.Read1b(reserved3, 9);                    /* 밝기/명암/그림효과 등 */

    UpdateBBox(this);
    if( pictype != PICTYPE_DRAW )
        style.cell = reserved3;

    if (follow_block_size != 0)
    {
        follow = new unsigned char[follow_block_size];

        hwpf.Read1b(follow, follow_block_size);
        if (pictype == PICTYPE_DRAW)
        {
            hmem = new HMemIODev((char *) follow, follow_block_size);
            LoadDrawingObjectBlock(this);
            style.cell = picinfo.picdraw.hdo;
            delete hmem;

            hmem = 0;
        }
        else
        {
            if ((follow[3] << 24 | follow[2] << 16 | follow[1] << 8 | follow[0]) == 0x269)
            {
                ishyper = true;
            }
        }
    }

    if( pictype != 3 )
        style.boxtype = 'G';
    else
        style.boxtype = 'D';
    hwpf.AddFBoxStyle(&style);

// cation
    hwpf.ReadParaList(caption);

    return !hwpf.State();
}


// line(15)

Line::Line():FBox(CH_LINE)
{
}


int Line::Read(HWPFile & hwpf)
{
    hwpf.Read2b(reserved, 2);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy && CH_LINE == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    hwpf.AddBox(this);

    style.boxnum = fboxnum++;
     zorder = zindex++;
    style.boxtype = 'L';
    hwpf.Read1b(&reserved2, 8);
    hwpf.Read1b(&style.anchor_type, 1);
    hwpf.Read1b(&style.txtflow, 1);
    hwpf.Read2b(&style.xpos, 1);
    hwpf.Read2b(&style.ypos, 1);
    hwpf.Read2b(&option, 1);
    hwpf.Read2b(&ctrl_ch, 1);
    hwpf.Read2b(style.margin, 12);
    hwpf.AddFBoxStyle(&style);
    hwpf.Read2b(&box_xs, 1);
    hwpf.Read2b(&box_ys, 1);
    hwpf.Read2b(&cap_xs, 1);
    hwpf.Read2b(&cap_ys, 1);
    hwpf.Read2b(&style.cap_len, 1);
    hwpf.Read2b(&xs, 1);
    hwpf.Read2b(&ys, 1);
    lnnumber = style.boxnum;
    hwpf.linenumber = 1;
    hwpf.Read2b(&boundsy, 1);
    hwpf.Read2b(&boundey, 1);
    hwpf.Read1b(&boundx, 1);
    hwpf.Read1b(&draw, 1);

    hwpf.Read2b(&pgx, 1);
    hwpf.Read2b(&pgy, 1);
    hwpf.Read2b(&pgno, 1);
    hwpf.Read2b(&showpg, 1);

    hwpf.Read2b(&sx, 1);
    hwpf.Read2b(&sy, 1);
    hwpf.Read2b(&ex, 1);
    hwpf.Read2b(&sy, 1);
    hwpf.Read2b(&width, 1);
    hwpf.Read2b(&shade, 1);
    hwpf.Read2b(&color, 1);
    style.xpos = width;

    return !hwpf.State();
}


// hidden(15)
Hidden::Hidden():HBox(CH_HIDDEN)
{
}


int Hidden::Read(HWPFile & hwpf)
{
    hwpf.Read2b(reserved, 2);
    hwpf.Read2b(&dummy, 1);
    if (!(hh == dummy && CH_HIDDEN == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }

    hwpf.Read1b(info, 8);
    hwpf.ReadParaList(plist);

    return !hwpf.State();
}


// header/footer(16)
HeaderFooter::HeaderFooter():HBox(CH_HEADER_FOOTER)
{
    linenumber = 0;
}


int HeaderFooter::Read(HWPFile & hwpf)
{
    hwpf.Read2b(reserved, 2);
    hwpf.Read2b(&dummy, 1);
    if (!(hh == dummy && CH_HEADER_FOOTER == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }

    hwpf.Read1b(info, 8);
    hwpf.Read1b(&type, 1);
    hwpf.Read1b(&where, 1);
    lnnumber = 0;
    hwpf.ReadParaList(plist, CH_HEADER_FOOTER);
    linenumber = sal::static_int_cast<unsigned char>(lnnumber);
     m_nPageNumber = hwpf.getCurrentPage();
     hwpf.setMaxSettedPage();
     hwpf.AddHeaderFooter(this);

    return !hwpf.State();
}


// footnote(17)
Footnote::Footnote():HBox(CH_FOOTNOTE)
{
}


int Footnote::Read(HWPFile & hwpf)
{
    hwpf.Read2b(reserved, 2);
    hwpf.Read2b(&dummy, 1);
    if (!(hh == dummy && CH_FOOTNOTE == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }

    hwpf.Read1b(info, 8);
    hwpf.Read2b(&number, 1);
    hwpf.Read2b(&type, 1);
    width = (short) hwpf.Read2b();
    hwpf.ReadParaList(plist, CH_FOOTNOTE);

    return !hwpf.State();
}


// auto number(18)
AutoNum::AutoNum():HBox(CH_AUTO_NUM)
{
}


int AutoNum::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&type, 1);
    hwpf.Read2b(&number, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// new number(19)
NewNum::NewNum():HBox(CH_NEW_NUM)
{
}


int NewNum::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&type, 1);
    hwpf.Read2b(&number, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// show page number (20)
ShowPageNum::ShowPageNum():HBox(CH_SHOW_PAGE_NUM)
{
}


int ShowPageNum::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&where, 1);
    hwpf.Read2b(&shape, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
     m_nPageNumber = hwpf.getCurrentPage();
     hwpf.setMaxSettedPage();
    hwpf.AddPageNumber(this);
    return !hwpf.State();
}


/* 홀수쪽시작/감추기 (21) */
PageNumCtrl::PageNumCtrl():HBox(CH_PAGE_NUM_CTRL)
{
}


int PageNumCtrl::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&kind, 1);
    hwpf.Read2b(&what, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// mail merge(22)
MailMerge::MailMerge():HBox(CH_MAIL_MERGE)
{
}


int MailMerge::Read(HWPFile & hwpf)
{
    hwpf.Read1b(field_name, 20);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// char compositon(23)
Compose::Compose():HBox(CH_COMPOSE)
{
}


int Compose::Read(HWPFile & hwpf)
{
    hwpf.Read2b(compose, 3);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// hyphen(24)
Hyphen::Hyphen():HBox(CH_HYPHEN)
{
}


int Hyphen::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&width, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// toc mark(25)
TocMark::TocMark():HBox(CH_TOC_MARK)
{
}


int TocMark::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&kind, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// index mark(26)
IndexMark::IndexMark():HBox(CH_INDEX_MARK)
{
}


int IndexMark::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&keyword1, 60);
    hwpf.Read2b(&keyword2, 60);
    hwpf.Read2b(&pgno, 1);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


// outline(28)
Outline::Outline():HBox(CH_OUTLINE)
{
}


int Outline::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&kind, 1);
    hwpf.Read1b(&shape, 1);
    hwpf.Read1b(&level, 1);
    hwpf.Read2b(number, 7);
    hwpf.Read2b(user_shape, 7);
    hwpf.Read2b(deco, 14);
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


/* 묶음 빈칸(30) */
KeepSpace::KeepSpace():HBox(CH_KEEP_SPACE)
{
}


int KeepSpace::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}


/* 고정폭 빈칸(31) */
FixedSpace::FixedSpace():HBox(CH_FIXED_SPACE)
{
}


int FixedSpace::Read(HWPFile & hwpf)
{
    hwpf.Read2b(&dummy, 1);

    if (!(hh == dummy)){
        return hwpf.SetState(HWP_InvalidFileFormat);
     }
    return !hwpf.State();
}
