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

#ifndef INCLUDED_HWPFILTER_SOURCE_HBOX_H
#define INCLUDED_HWPFILTER_SOURCE_HBOX_H

#include <sal/config.h>

#include <list>
#include <memory>

#include <sal/types.h>

#include "hwplib.h"
#include "hwpfile.h"
#include "hinfo.h"
#include "hpara.h"

/**
 * The HBox class is the base class for all date classes in hwp document.
 * For example, there are special character, table, image, etc.
 * It has one character. The ascii code value of special characters are smaller than 32. General character is greater than 32.
 *
 * @short Base class for characters
 */
struct HBox
{
    public:
        hchar hh;

/**
 * Construct a HBox object with parameter hch.
 * @param hch 16bit character being able to have Korean character.
 */
        explicit HBox( hchar hch );
        virtual ~HBox();
/**
 * @returns The Size of HBox object
 */
        int           WSize();
/**
 * Read properties from HIODevice object like stream, file, memory.
 *
 * @param hwpf HWPFile Object having all information for a hwp file.
 * @returns True if reading from stream is successful.
 */
        virtual bool Read(HWPFile &hwpf);

        virtual hchar_string GetString();
    private:
        static int boxCount;
};

/**
 * @short Class for saving data to be skipped.
 */
struct SkipData: public HBox
{
    uint data_block_len;
    hchar dummy;
    std::unique_ptr<char[]> data_block;

    explicit SkipData(hchar);
    virtual ~SkipData() override;
    virtual bool Read(HWPFile &hwpf) override;
};
struct DateCode;
struct FieldCode : public HBox
{
    uchar type[2];                    /* 2/0 - Formula, 3/0-document summary, 3/1 Personal Information, 3/2-creation date, 4/0-pressing mold */
    char *reserved1;
    unsigned short location_info;     /* 0 - End code, 1 - start code */
    char *reserved2;
    hchar *str1;
    hchar *str2;
    hchar *str3;
    char *bin;

     DateCode *m_pDate;

    FieldCode();
    virtual ~FieldCode() override;
    virtual bool Read(HWPFile &hwpf) override;
};
/**
 * Kind of BOOKMARK
 */
enum
{
    BM_MARK,
    BM_BEGIN,
    BM_END
};

#define BMK_COMMENT_LEN 15

/**
 * @short Class for BOOKMARK
 */
struct Bookmark: public HBox
{
    hchar         dummy;
    hchar         id[BMK_COMMENT_LEN + 1];
    unsigned short    type;

    Bookmark();
    virtual ~Bookmark() override;
    virtual bool Read(HWPFile &hwpf) override;
};

// date format(7)
const int DATE_SIZE = 40;

/**
 * @short Class for saving date format made by user
 */
struct DateFormat: public HBox
{
    hchar format[DATE_SIZE];
    hchar dummy;

    DateFormat();
    virtual bool Read(HWPFile &hwpf) override;
};

/**
 * @short Class for current date and time with specified format.
 */
struct DateCode: public HBox
{
    enum
    {
        YEAR, MONTH, WEEK, DAY, HOUR, MIN
    };

    hchar format[DATE_SIZE];
/**
 * year/month/week/day/hour/minute
 */
    short date[6];
    hchar dummy;
    unsigned char key;

    DateCode();
    virtual bool Read(HWPFile &hwpf) override;

    virtual hchar_string GetString() override;
};

/**
 * @short Tab object
 */
struct Tab: public HBox
{
    hunit width;
    unsigned short leader;
    hchar dummy;

    Tab();
    virtual bool Read(HWPFile &hwpf) override;
};

// tbox(10) TABLE BOX MATH BUTTON HYPERTEXT

enum ttype { TBL_TYPE, TXT_TYPE, EQU_TYPE, BUTTON_TYPE, HYPERTEXT_TYPE };
enum                                              /* TxtBox->margin[n][?] */
{
    OUT_M, IN_M, CELL_M
};
enum                                              /* TxtBox->margin[?][n] */
{
    L_M, R_M, T_M, B_M
};
enum anchor { CHAR_ANCHOR, PARA_ANCHOR, PAGE_ANCHOR, PAPER_ANCHOR };
enum { TF_OCCUPY_SPACE, TF_TRANSPARENT,TF_ARROUND_TEXT };
enum
{
    CAP_OUT_BOT,  CAP_OUT_TOP,  CAP_IN_BOT,    CAP_IN_TOP,
    CAP_LEFT_BOT, CAP_LEFT_TOP, CAP_RIGHT_BOT, CAP_RIGHT_TOP,
    EQU_CAP_POS
};

struct CellLine
{
    unsigned char key;
    unsigned char top;                            // 0-No line, 1-single, 2-thick, 3-double
    unsigned char bottom;
    unsigned char left;
    unsigned char right;
    short color;                                  // cell color
    unsigned char shade;                          // <100%
};
/**
 * A cell has four properties to specify the position for itself in hwp.
 * @li xpos -  distance between left border of cell and left border of table
 * @li ypos -  distance between top border of cell and top border of table
 * @li width -  distance between left border of cell and right border of cell
 * @li height -  distance between top border of cell and bottom border of table
 * This is differ from the format of other office in fact cell has two properties
 * - rowindex and column index.
 *
 * @short Cell object
 */
struct Cell                                       // Cell
{
    unsigned char key;                            // Index value of border style
    short     p;
    short     color;                              // cell color
    short     x, y;                               // [x,y] cell pos
    short     w, h;                               // [x,y] cell size
    short     txthigh, cellhigh;                  // used maximum
    unsigned char flag, changed, used;            // unused(file)
    unsigned char ver_align;                      // vertical align  {1=center}
    unsigned char linetype[4];                    // [left,right,top,bottom]
    unsigned char shade;                          // <100%
    unsigned char diagonal;                       // { 0=none,\=1,/=2,X=3}
    unsigned char protect;

    void  Read( HWPFile &hwpf );
};

/**
 * @short Style for floating object
 */
struct FBoxStyle
{
/**
 * Anchor type : paragraph , page, char
 */
    unsigned char anchor_type;
/**
 * Kind of wrap
 */
    unsigned char txtflow;                        /* Avoid painting. 0-2 (seat occupied, transparency, harmony) */
/**
 * Horizontal alignment
 */
    short     xpos;                               // left, right, center, xx
/**
 * Vertical alignment
 */
    short     ypos;                               // page top, bottom, yy
/**
 * Every margin of border
 * [0-2][] : out/in/cell margin
 * [][0-3] : left/right/top/bottom
 */
    short     margin[3][4];                       // out : left, right, top, bottom
/**
 * Index of floating object
 */
    short     boxnum;                             /* Numbers used as style-name in Libre Office */
/**
 * Type of floating object : line, txtbox, image, table, equalizer and button
 */
    unsigned char boxtype;                        // (L)ine, t(X)tbox, Picture - (G)
    short     cap_len; /* The length of the caption */

    void *cell;

    FBoxStyle()
        : anchor_type(0)
        , txtflow(0)
        , xpos(0)
        , ypos(0)
        , boxnum(0)
        , boxtype(0)
        , cap_len(0)
        , cell(nullptr)
    {
        memset(margin, 0, sizeof(margin));
    }
};

/**
 * This object is for floating object like table, image, line and so on.
 *
 * @short floating object
 */
struct FBox: public HBox
{
    int zorder;
    short     option;                             // draw frame
    hchar     ctrl_ch;
    FBoxStyle style;

    short     box_xs, box_ys;
    short     cap_xs, cap_ys ;
    short     xs, ys;                             // ys = fig_ys + cap_ys + margin
// xs = fig_xs + cap_xs + margin
    short     cap_margin;
    char      xpos_type, ypos_type;
    unsigned char smart_linesp;

/* In tbox or pic, this data exists in memory when running, isn't written to a file.
   But in line, it will be written to a file.
 */
    short     boundsy, boundey;
    unsigned char boundx, draw;

/**
 * Physical x,y position.
 */
    short     pgx, pgy;                           // physical xpos, ypos
    short     pgno, showpg;                       // pageno where code is

    FBox      *prev, *next;

    explicit FBox( hchar hch );
    virtual ~FBox() override;
};

struct Table;
/**
 * The TxtBox class saves object properties about table, textbox, equalizer or button
 */
struct TxtBox: public FBox
{
    hchar     reserved[2];
    hchar     dummy;

    short     dummy1;                             // to not change structure size */
    short     cap_len;
    short     next_box;
    short     dummy2;                             // to not change structure size */
    unsigned char reserved1;
/**
 * caption position
 */
    short     cap_pos;                            // caption pos
    short     num;                                // numbering

    short     dummy3;
    short     baseline;                           //(for equ)

/**
 * The value of type indicates as the below: zero is table, one is
 * textbox, two is equalizer and three is button.
 */
    short     type;                               // 0-table, 1-textbox, 2-수식, 3-button
/**
 * nCell is greater than one only for table, otherwise it is 1.
 */
    short     nCell;                              //:=1    offset 80
/**
 * If value of protect is 1, size of cell cann't change.
 */
    short     protect;                            //1=size lock

    Cell      *cell;
     Table *m_pTable;
/**
 * Paragraph list
 */
    std::list<HWPPara*> *plists;
/**
 * Caption
 */
    std::list<HWPPara*> caption;

    TxtBox();
    virtual ~TxtBox() override;

    virtual bool Read(HWPFile &hwpf) override;
};

#define ALLOWED_GAP 5
#define INIT_SIZE 20
#define ADD_AMOUNT 10

struct Columns
{
     std::unique_ptr<int[]> data;
     size_t nCount;
     size_t nTotal;
     Columns(){
          nCount = 0;
          nTotal = INIT_SIZE;
          data.reset(new int[nTotal]);
     }

     void AddColumnsSize(){
          if (nTotal + ADD_AMOUNT < nTotal) // overflow
          {
              throw ::std::bad_alloc();
          }
          int* tmp = new int[nTotal + ADD_AMOUNT];
          for (size_t i = 0 ; i < nTotal ; i++)
                tmp[i] = data[i];
          nTotal += ADD_AMOUNT;
          data.reset(tmp);
     }

     void insert(int pos){
          if( nCount == 0 ){
                data[nCount++] = pos;
                return;
          }
          for (size_t i = 0 ; i < nCount; i++ ) {
                if( pos < data[i] + ALLOWED_GAP && pos > data[i] - ALLOWED_GAP )
                     return;  // Already exist;
                if( pos < data[i] ){
                     if( nCount == nTotal )
                          AddColumnsSize();
                     for (size_t j = nCount ; j > i ; j--)
                          data[j] = data[j-1];
                     data[i] = pos;
                     nCount++;
                     return;
                }
          }
          // last position.
          if( nCount == nTotal )
                AddColumnsSize();
          data[nCount++] = pos;
     }

     int getIndex(int pos)
     {
          if( pos == 0 )
                return 0;
          for (size_t i = 0 ; i < nCount; i++) {
                if( pos < data[i] + ALLOWED_GAP && pos > data[i] - ALLOWED_GAP )
                     return i;
          }
          return -1;
     }
};

struct Rows
{
     std::unique_ptr<int[]> data;
     size_t nCount;
     size_t nTotal;
     Rows(){
          nCount = 0;
          nTotal = INIT_SIZE;
          data.reset( new int[nTotal] );
     }

     void AddRowsSize(){
          if (nTotal + ADD_AMOUNT < nTotal) // overflow
          {
              throw ::std::bad_alloc();
          }
          int* tmp = new int[nTotal + ADD_AMOUNT];
          for (size_t i = 0 ; i < nTotal ; i++)
                tmp[i] = data[i];
          nTotal += ADD_AMOUNT;
          data.reset(tmp);
     }

     void insert(int pos){
          if( nCount == 0 ){
                data[nCount++] = pos;
                return;
          }
          for (size_t i = 0 ; i < nCount; i++) {
                if( pos < data[i] + ALLOWED_GAP && pos > data[i] - ALLOWED_GAP )
                     return;  // Already exist;
                if( pos < data[i] ){
                     if( nCount == nTotal )
                          AddRowsSize();
                     for (size_t j = nCount ; j > i ; j--)
                          data[j] = data[j-1];
                     data[i] = pos;
                     nCount++;
                     return;
                }
          }
          // last position.
          if( nCount == nTotal )
                AddRowsSize();
          data[nCount++] = pos;
     }

     int getIndex(int pos)
     {
          if( pos == 0 )
                return 0;
          for (size_t i = 0 ; i < nCount; i++) {
                if( pos < data[i] + ALLOWED_GAP && pos > data[i] - ALLOWED_GAP )
                     return i;
          }
          return -1;
     }
};

struct TCell
{
     int nColumnIndex;
     int nRowIndex;
     int nColumnSpan;
     int nRowSpan;
     Cell *pCell;
};

struct Table
{
     Table() : box(nullptr) {};
     ~Table() {
          std::list<TCell*>::iterator it = cells.begin();
          for( ; it != cells.end(); ++it)
                delete *it;
     };

     Columns columns;
     Rows    rows;
     std::list<TCell*> cells;
     TxtBox *box;
};

/* picture (11) graphics, OLE graphics, inserted graphics, drawing */
enum pictype
{
    PICTYPE_FILE, PICTYPE_OLE, PICTYPE_EMBED,
    PICTYPE_DRAW, PICTYPE_UNKNOWN
};
/**
 * @short External image file
 */
struct PicDefFile
{
    char path[256];
    void *img;
    bool skipfind;
};

/**
 * @short Embedded image file
 */
struct PicDefEmbed
{
    char  embname[16];
};

/**
 * @short Win32 ole object
 */
struct PicDefOle
{
    char  embname[16];
    void  *hwpole;
};

/**
 * @short Drawing object of hwp
 */
struct PicDefDraw
{
    void      *hdo;
    uint      zorder;
    ZZRect    vrect;
    int       mbrcnt;
};

/**
 * @short For using common case
 */
struct PicDefUnknown
{
    char  path[256];
};

typedef union
{
    PicDefFile        picfile;
    PicDefEmbed       picembed;
    PicDefOle     picole;
    PicDefDraw        picdraw;
    PicDefUnknown     picun;
} PicDef;

/**
 * There are four kinds of image.
 * @li External image
 * @li Embedded image
 * @li Win32 ole object
 * @li Drawing object of hwp
 *
 * @short Image object
 */
struct Picture: public FBox
{
    hchar     reserved[2];
    hchar     dummy;
/**
 * follow_block_size is the size information of the Drawing object of hwp.
 * It's value is greater than 0 if the pictype is PICTYPE_DRAW.
 */
    uint      follow_block_size;                  /* Additional information length. */
    short     dummy1;                             // to not change structure size */
    short     dummy2;                             // to not change structure size */
    uchar     reserved1;
/**
 * Position of caption
 */
    short     cap_pos;                            // caption pos
/**
 * Index of current Picture object
 */
    short     num;                                // numbering

/**
 * Type of this object
 * It is one of external/ole/embedded/drawing picture
 */
    uchar     pictype;
    hunit     skip[2];
/**
 * Ratio of magnification or reduction.
 */
    hunit     scale[2];
    PicDef    picinfo;
    char      reserved3[9];

    std::list<HWPPara*> caption;
/**
 * It's for the Drawing object
 */
    unsigned char *follow;                        /* When the type of image is drawing, gives additional information. */

    bool ishyper;

    Picture();
    virtual ~Picture() override;

    virtual bool Read    (HWPFile &hwpf) override;
};

// line (14)
/**
 * @short Line
 */
struct Line: public FBox
{
    hchar     reserved[2];
    hchar     dummy;

    char      reserved2[8];

    short     sx, sy, ex, ey;
    short     width, shade, color;

    Line();

    virtual bool Read(HWPFile &hwpf) override;
};

// hidden(15)
/**
 * @short Hidden section
 */
struct Hidden: public HBox
{
    hchar     reserved[2];
    hchar     dummy;

    unsigned char info[8];                        // h, next, dummy
    std::list<HWPPara*> plist;

    Hidden();
    virtual ~Hidden() override;

    virtual bool Read(HWPFile &hwpf) override;
};

/**
 * @short Header or footer
 */
struct HeaderFooter: public HBox
{
    hchar     reserved[2];
    hchar     dummy;

    unsigned char info[8];
/**
 * Header or footer
 */
    unsigned char type;
    unsigned char where;
    unsigned char linenumber;

     unsigned int m_nPageNumber;

/**
 * Paragraph list of header or footer
 */
    std::list<HWPPara*> plist;

    HeaderFooter();
    virtual ~HeaderFooter() override;

    virtual bool Read(HWPFile &hwpf) override;
};

/**
 * Both footnote and endnote are comment. Footnote is located at the end of paragraph; endnote is located at the end of page. The Footnote class represents footnote and endnote.
 * @short Footnote or endnote
 */
struct Footnote: public HBox
{
    hchar     reserved[2];
    hchar     dummy;

    unsigned char  info[8];
/**
 * The number of current footnote/endnote
 */
    unsigned short number;
/**
 * Set the type of Footnote either footnote or endnote.
 */
    unsigned short type;
/**
 * The width of the Footnote object.
 */
    hunit     width;
/**
 * Paragraph list of Footnote objects
 */
    std::list<HWPPara*> plist;

    Footnote();
    virtual ~Footnote() override;

    virtual bool Read(HWPFile &hwpf) override;
};

// auto number(18)
/**
 * Kind of auto input number
 */
enum
{
    PGNUM_AUTO,
    FNNUM_AUTO,
    ENNUM_AUTO,
    PICNUM_AUTO,
    TBLNUM_AUTO,
    EQUNUM_AUTO
};

/**
 * @short Input current index of page,comment,table and picture.
 */
struct AutoNum: public HBox
{
    unsigned short type;
    unsigned short number;
    hchar      dummy;

    AutoNum();

    virtual bool Read(HWPFile &hwpf) override;
};

/**
 * @short Input new number as current index of page,comment,table and picture.
 */
struct NewNum: public HBox
{
    unsigned short type;
    unsigned short number;
    hchar      dummy;

    NewNum();

    virtual bool Read(HWPFile &hwpf) override;
};

// page number(20)
/**
 * @short Input page index in footer or header
 */
struct ShowPageNum: public HBox
{
/**
 * Location of page number to be inserted.
 */
    unsigned short where;
     unsigned int m_nPageNumber;
/**
 * Shape of page number to be inserted.
 */
    unsigned short shape;
    hchar      dummy;

    ShowPageNum();

    virtual bool Read(HWPFile &hwpf) override;
};

/* Start odd side (21) */
/**
 * Controls the display of page number, header, footer and border.
 */
struct PageNumCtrl: public HBox
{
/**
 * object type
 */
    unsigned short kind;
/**
 * control command.
 */
    unsigned short what;
    hchar      dummy;

    PageNumCtrl();

    virtual bool Read(HWPFile &hwpf) override;
};

// mail merge(22)
/**
 * Generates the mailing list automatically using address book and mail body format.
 * @short Generates mailing list
 */
struct MailMerge: public HBox
{
    unsigned char field_name[20];
    hchar     dummy;

    MailMerge();

    virtual bool Read(HWPFile &hwpf) override;
    virtual hchar_string GetString() override;
};

// char composition(23)
/**
 * The compose struct displays characters at position. The maximum character count for composition is three.
 * @short Composition several characters
 */
struct Compose: public HBox
{
    hchar     compose[3];
    hchar     dummy;

    Compose();

    virtual bool Read(HWPFile &hwpf) override;
};

// hyphen(24)
/**
 * @short Hyphen
 */
struct Hyphen: public HBox
{
/**
 * Width of hyphen
 */
    hchar     width;
    hchar     dummy;

    Hyphen();

    virtual bool Read(HWPFile &hwpf) override;
};

// toc mark(25)
/**
 * The TocMark class is for making the content of a table.
 * When you set TocMark on current position, hwp makes it as toc automatically.
 * @short Table of contents
 */
struct TocMark: public HBox
{
    hchar     kind;
    hchar     dummy;

    TocMark();

    virtual bool Read(HWPFile &hwpf) override;
};

// index mark(26)
/**
 * IndexMark marks the table of search.
 * If you set IndexMark at current position, hwp make it as search index.
 * @short Table of search
 */
struct IndexMark: public HBox
{
    hchar     keyword1[60];
    hchar     keyword2[60];
    unsigned short pgno;
    hchar     dummy;

    IndexMark();

    virtual bool Read(HWPFile &hwpf) override;
};

// outline(28)
#define MAX_OUTLINE_LEVEL   7

enum
{
    OLSTY_USER = 0,
    OLSTY_NUMS1 = 1,
    OLSTY_NUMS2 = 2,
    OLSTY_NUMSIG1 = 3,
    OLSTY_NUMSIG2 = 4,
    OLSTY_NUMSIG3 = 5,
    OLSTY_BULUSER = 128,
    OLSTY_BULLET1 = 129,
    OLSTY_BULLET2 = 130,
    OLSTY_BULLET3 = 131,
    OLSTY_BULLET4 = 132,
    OLSTY_BULLET5 = 133
};

// value is in style->userchar[level];
enum
{
    UDO_NUM,
    UDO_UROM,
    UDO_LROM,
    UDO_UENG,
    UDO_LENG,
    UDO_SYLL,
    UDO_JAMO,
    UDO_HANJA,
    UDO_SP_CNUM,
    UDO_SP_CLENG,
    UDO_SP_CSYLL,
    UDO_SP_CJAMO,
    N_UDO
};
/**
 * Number and format of title.
 * @short Number and format of title
 */
class Outline: public HBox
{
    public:
/**
 * kind of numbering format
 */
        unsigned short kind;
        unsigned char  shape;
/**
 * level of number, Ex) The level of 1.3.2.4 is four
 */
        unsigned char  level;
/**
 * value of level
 */
        unsigned short number[MAX_OUTLINE_LEVEL];
/**
 * shape of level
 */
        hchar     user_shape[MAX_OUTLINE_LEVEL];
/**
 * decoration character for the level type
 */
        hchar     deco[MAX_OUTLINE_LEVEL][2];     /* Prefix/postfix for Customize */
        hchar     dummy;

        Outline();

        virtual bool Read(HWPFile &hwpf) override;
        hchar_string GetUnicode() const;
};

/* Bundle of spaces (30) */
/**
 * The Special space to be treated non-space when a string is
 * cut at the end of line
 * @short Special space
 */
struct KeepSpace: public HBox
{
    hchar dummy;

    KeepSpace();

    virtual bool Read(HWPFile &hwpf) override;
};

/* Fixed-width spaces (31) */
/**
 * @short Space with always same width not relation with fonts.
 */
struct FixedSpace: public HBox
{
    hchar     dummy;

    FixedSpace();

    virtual bool Read(HWPFile &hwpf) override;
};
#endif // INCLUDED_HWPFILTER_SOURCE_HBOX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
