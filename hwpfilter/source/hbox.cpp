/*************************************************************************
 *
 *  $RCSfile: hbox.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:39:19 $
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

/* $Id: hbox.cpp,v 1.1 2003-10-15 14:39:19 dvo Exp $ */

#include "precompile.h"

#include <ctype.h>

#include "hwpfile.h"
#include "hbox.h"
#include "hpara.h"
#include "hutil.h"
#include "htags.h"
#include "drawdef.h"
#include "hcode.h"

int HBox::boxCount = 0;

HBox::HBox()
{
    hh = 0;
    boxCount++;
}


HBox::HBox(hchar hch)
{
    hh = hch;
    boxCount++;
}


HBox::~HBox()
{
    boxCount--;
}


int HBox::WSize(void)
{
    static int wsize[32] =
    {
        1, 4, 4, 4, 4, 4, 4, 42,                  /* dateform */
        48, 4, 4, 4, 4, 1, 4, 4,                  /* hidden */
        4, 4, 4, 4, 4, 4, 12, 5,                  /* chcompose */
        3, 3, 123, 4, 32, 4, 2, 2
    };

    if (hh < 32)
        return wsize[hh];
    else
        return 1;
}


int HBox::GetString(hchar * hstr, int slen)
{
    *hstr++ = hh;
    *hstr = 0;
    return 1;
}


hunit HBox::Height(CharShape *csty)
{
    return( csty->size );
}


// skip block
SkipBlock::SkipBlock(hchar hch):HBox(hch)
{
    data_block = 0;
}


SkipBlock::~SkipBlock(void)
{
    delete[]data_block;
}


// FieldCode [5]
FieldCode::FieldCode(void) : HBox(CH_FIELD)
{
    str1 = 0;
    str2 = 0;
    str3 = 0;
    bin = 0;
    reserved1 = new char[4];
    reserved2 = new char[22];
     m_pDate = 0L;
}


FieldCode::~FieldCode(void)
{
    delete[] str1;
    delete[] str2;
    delete[] str3;
    delete[] bin;
    delete[] reserved1;
    delete[] reserved2;
     if( m_pDate )
          delete m_pDate;
}


// book mark(6)
Bookmark::Bookmark(void):HBox(CH_BOOKMARK)
{
}


Bookmark::~Bookmark(void)
{
}


// date format(7)
DateFormat::DateFormat(void):HBox(CH_DATE_FORM)
{
}


// date code(8)

DateCode::DateCode(void):HBox(CH_DATE_CODE)
{
}


#include "datecode.h"

int DateCode::GetString(hchar * hstr, int slen)
{
    hchar *fmt, *d;
    int i, num;
    char *form;
    char cbuf[256];
    bool is_pm, add_zero;

    add_zero = false;
    format[DATE_SIZE - 1] = 0;
    fmt = format[0] ? format : defaultform;

    d = hstr;
    for (; *fmt && ((int) (d - hstr) < DATE_SIZE) && slen > 1; fmt++)
    {
        form = const_cast < char *>((add_zero) ? "%02d" : "%d");

        add_zero = false;
        is_pm = (date[HOUR] >= 12);
        *cbuf = 0;
        num = -1;

        switch (*fmt)
        {
            case '0':
                add_zero = true;
                break;
            case '1':
                num = date[YEAR];
                form = "%04d";
                break;
            case '!':
                num = date[YEAR] % 100;
                break;
            case '2':
                num = date[MONTH];
                break;
            case '@':
                memcpy(cbuf, eng_mon + (date[MONTH] - 1) * 3, 3);
                cbuf[3] = '.';
                cbuf[4] = 0;
                break;
            case '*':
                strcpy(cbuf, en_mon[date[MONTH] - 1]);
                break;
            case '3':                             /* 'D' is day of korean */
                num = date[DAY];
                break;
            case '#':
                num = date[DAY];
                switch (date[DAY] % 10)
                {
                    case 1:
                        form = "%dst";
                        break;
                    case 2:
                        form = "%dnd";
                        break;
                    case 3:
                        form = "%drd";
                        break;
                    default:
                        form = "%dth";
                        break;
                }
                break;
            case '4':
                num = date[HOUR] - ((date[HOUR] > 12) ? 12 : 0);
                break;
            case '$':
                num = date[HOUR];
                break;
            case '5':
            case '%':
                num = date[MIN];
                break;
            case '6':
                *d++ = kor_week[date[WEEK]];
                slen--;
                break;
            case '^':
                memcpy(cbuf, eng_week + date[WEEK] * 3, 3);
                cbuf[3] = '.';
                cbuf[4] = 0;
                break;
            case '_':
                strcpy(cbuf, en_week[date[WEEK]]);
                break;
            case '7':
                if (slen > 3)
                {
                    *d++ = 0xB5A1;
                    *d++ = (is_pm) ? 0xD281 : 0xB8E5;
                }
                break;
            case '&':
                strcpy(cbuf, (is_pm) ? "p.m." : "a.m.");
                break;
            case '+':
                strcpy(cbuf, (is_pm) ? "P.M." : "A.M.");
                break;
            case '8':                             // 2.5 feature
            case '9':
#if 0
// LATER
                mkcurfilename(cbuf, *fmt);
                for (i = 0; cbuf[i] != 0 && slen > 1; i++)
                {                                 //for hangle filename
                    if (cbuf[i] & 0x80 && cbuf[i + 1] != 0)
                    {
                        *d++ = (cbuf[i] << 8) | cbuf[i + 1];
                        i++;
                    }
                    else
                        *d++ = cbuf[i];
                    slen--;
                }
#endif
                cbuf[0] = 0;
                break;
            case '~':                             // 3.0b feature
                if (fmt[1] == 0)
                    break;
                fmt++;
                if (*fmt == '6')
                {
                    *d++ = china_week[date[WEEK]];
                    slen--;
                    break;
                }
                break;
            default:
                if (*fmt == '\\' && *++fmt == 0)
                    goto done;
                *d++ = *fmt;
                slen--;
        }
        if (num != -1)
            sprintf(cbuf, form, num);
        for (i = 0; 0 != cbuf[i] && slen > 1; i++)
        {
            *d++ = *(cbuf + i);
            slen--;
        }
    }
    done:
    *d = 0;
    return hstrlen(hstr);
}


// tab(9)

Tab::Tab(void):HBox(CH_TAB)
{
}


// floating box
FBox::FBox(hchar hch):HBox(hch)
{
    prev = next = 0;
     zorder = 0;
}


FBox::~FBox()
{
}


// tbox(10) TABLE BOX MATH BUTTON HYPERTEXT

TxtBox::TxtBox(void):FBox(CH_TEXT_BOX), cell(0), plists(0)
{
}


TxtBox::~TxtBox(void)
{
    delete[]cell;

    for (int ii = 0; ii < nCell; ++ii)
    {
        LinkedListIterator < HWPPara > it(&plists[ii]);
        for (; it.current(); it++)
            delete it.current();
    }

    LinkedListIterator < HWPPara > it(&caption);
    for (; it.current(); it++)
        delete it.current();

    delete[]plists;
}


hunit TxtBox::Height(CharShape * csty)
{
    return (style.anchor_type == CHAR_ANCHOR) ? box_ys : csty->size;
}


// picture(11)

Picture::Picture(void):FBox(CH_PICTURE)
{
    follow = 0;
    ishyper = false;
}


Picture::~Picture(void)
{
    delete[]follow;
    if( pictype == PICTYPE_DRAW && picinfo.picdraw.hdo )
        delete (HWPDrawingObject *) picinfo.picdraw.hdo;

    LinkedListIterator < HWPPara > it(&caption);
    for (; it.current(); it++)
        delete it.current();
}


int Picture::Type()
{
    return pictype;
}


hunit Picture::Height(CharShape * sty)
{
    return (style.anchor_type == CHAR_ANCHOR) ? box_ys : sty->size;
}


// line(14)
// hidden(15)
Hidden::~Hidden(void)
{
    LinkedListIterator < HWPPara > it(&plist);
    for (; it.current(); it++)
        delete it.current();
}


// header/footer(16)
HeaderFooter::~HeaderFooter(void)
{
    LinkedListIterator < HWPPara > it(&plist);
    for (; it.current(); it++)
        delete it.current();
}


// footnote(17)
Footnote::~Footnote(void)
{
    LinkedListIterator < HWPPara > it(&plist);
    for (; it.current(); it++)
        delete it.current();
}


// auto number(18)
// new number(19)
// show page number (20)
// 홀수쪽시작/감추기 (21)

// mail merge(22)
int MailMerge::GetString(hchar * hstr, int slen)
{
    *hstr = 0;
    return 0;
}


// character compositon(23)
// hyphen(24)
// toc mark(25)
// index mark(26)
// outline(28)

#define OL_HANGL_JASO   0
#define OL_HANGL_KANATA 1

static hchar olHanglJaso(int num, int type)
{
    static unsigned char han_init[] =
        { "\x88\x90\x94\x9c\xa0\xa4\xac\xb4\xb8\xc0\xc4\xc8\xcc\xd0" };
    static unsigned char jung[] = { 3, 5, 7, 11, 13, 19, 20, 26, 27, 29, 30 };
    static unsigned char jung2[] = { 3, 7, 13, 20, 27, 29, 30 };

    hchar hh = 0;
    int j;

    if (type == OL_HANGL_JASO)
    {
        num = num % (14 + (sizeof(jung) / sizeof(char)));

        if (num < 14)
            hh = (han_init[num] << 8) | 'A';
        else
            hh = (jung[num - 14] << 5) | 0x8401;
    }
    else
    {
        if (num < 14)
            hh = (han_init[num] << 8) | 'a';
        else
        {
            j = (num / 14) % (sizeof(jung2) / sizeof(char));

            num = num % 14;
            hh = (han_init[num] << 8) | (jung2[j] << 5) | 1;
        }
    }
    return hh;
}


static hchar *GetOutlineStyleChars(int style)
{
    static hchar out_bul_style_entry[5][8] =      // extern
    {
        {                                         // 0 OLSTY_BULLET1
            0x2f18, 0x2f12, 0x2f08, 0x2f02, 0x2f06, 0x2f00, 0x2043, 0x0000
        },
        {                                         // 1
            0x2f18, 0x2f12, 0x2f06, 0x2f00, 0x2f36, 0x2f30, 0x2043, 0x0000
        },
        {                                         // 2
            0x2f26, 0x2f20, 0x2f06, 0x2f00, 0x2f16, 0x2f10, 0x2043, 0x0000
        },
        {                                         // 3
            0x2f18, 0x2f16, 0x2f12, 0x2f10, 0x2f06, 0x2f00, 0x2043, 0x0000
        },
        {                                         //
            0xAC61, 0xB677, 0xB861, 0xB8F7, 0xB781, 0x0000
        },
    };
    if (style >= OLSTY_BULLET1 && style <= OLSTY_BULLET5)
        return out_bul_style_entry[style - OLSTY_BULLET1];
    return NULL;
}


static void getOutlineNumStr(int style, int level, int num, hchar * hstr)
{
    enum
    {
        U_ROM = 0x01, L_ROM = 0x02, U_ENG = 0x04, L_ENG = 0x08,
        HAN = 0x10, NUM = 0x20, L_BR = 0x40, R_BR = 0x80
    };
    static unsigned char type_tbl[][MAX_OUTLINE_LEVEL] =
    {
        {
            U_ROM, HAN, NUM, HAN | R_BR, L_BR | NUM | R_BR,
            L_BR | HAN | R_BR, L_ROM | R_BR
        },
        {
            U_ROM, U_ENG, NUM, L_ENG | R_BR, L_BR | NUM | R_BR,
            L_BR | L_ENG | R_BR, L_ROM | R_BR
        },
        {
            NUM, HAN, L_BR | NUM | R_BR, L_BR | HAN | R_BR, NUM |
            R_BR, HAN | R_BR, L_ENG
        }
    };
    char fmt = type_tbl[style - OLSTY_NUMSIG1][level];
    char buf[80], *ptr;

    if (num < 1)
        num = 1;
    if (fmt & L_BR)
        *hstr++ = '(';
    if (fmt & NUM)
    {
        sprintf(buf, "%d", num);
        str2hstr(buf, hstr);
        hstr += strlen(buf);
    }
    else if (fmt & (U_ROM | L_ROM))
    {
        num2roman(num, buf);
        if (fmt & U_ROM)
        {
            ptr = buf;
            while (*ptr)
            {
                *ptr = toupper(*ptr);
                ptr++;
            }
        }
        str2hstr(buf, hstr);
        hstr += strlen(buf);
    }
    else
    {
        num = (num - 1) % 26;
        if (fmt & U_ENG)
            *hstr++ = 'A' + num;
        else if (fmt & L_ENG)
            *hstr++ = 'a' + num;
        else if (fmt & HAN)
            *hstr++ = olHanglJaso(num, OL_HANGL_KANATA);
    }
    *hstr++ = (fmt & R_BR) ? ')' : '.';
    *hstr = 0;
}


enum
{ OUTLINE_ON, OUTLINE_NUM };

// level 은 0부터 시작. 즉 1.1.1. 의 레벨은 2이다.
// number는 값이 그대로 들어가 있다. 즉, 1.2.1에는 1,2,1이 들어가 있다.
// style 은 1부터 값이 들어가 있다. hbox.h에 정의된 데로..
hchar *Outline::GetUnicode(hchar * hstr, int slen)
{
    int levelnum;
    hchar *p;
     hchar buffer[255];

    buffer[0] = 0;
    if (kind == OUTLINE_NUM)
    {
        switch (shape)
        {
            case OLSTY_NUMS1:
            case OLSTY_NUMS2:
            {
                char cur_num_str[10], buf[80];
                int i;

                buf[0] = 0;
                for (i = 0; i <= level; i++)
                {
                    levelnum = ((number[i] < 1) ? 1 : number[i]);
                    if (shape == OLSTY_NUMS2 && i && i == level)
                        sprintf(cur_num_str, "%d%c", levelnum, 0);
                    else
                        sprintf(cur_num_str, "%d%c", levelnum, '.');
                    strcat(buf, cur_num_str);
                }
                str2hstr(buf, buffer);
                     return hstr2ucsstr(buffer, hstr);
            }
            case OLSTY_NUMSIG1:
            case OLSTY_NUMSIG2:
            case OLSTY_NUMSIG3:
                {
                getOutlineNumStr(shape, level, number[level], buffer);
                     return hstr2ucsstr(buffer, hstr);
                }
            case OLSTY_BULLET1:
            case OLSTY_BULLET2:
            case OLSTY_BULLET3:
            case OLSTY_BULLET4:
            case OLSTY_BULLET5:
                {
                p = GetOutlineStyleChars(shape);
                buffer[0] = p[level];
                buffer[1] = 0;
                     return hstr2ucsstr(buffer, hstr);
                }
            case OLSTY_USER:
            case OLSTY_BULUSER:
                {
                        char dest[80];
                    int l = 0, res;
                    int i = level;
                    if( deco[i][0] ){
                        buffer[l++] = deco[i][0];
                    }
// level 은 0부터 시작. 즉 1.1.1. 의 레벨은 2이다.
// number는 값이 그대로 들어가 있다. 즉, 1.2.1에는 1,2,1이 들어가 있다.
// style 은 1부터 값이 들어가 있다. hbox.h에 정의된 데로..
                    switch( user_shape[i] )
                    {
                        case 0:
                            buffer[l++] = '1' + number[i] - 1;
                            break;
                        case 1: // 대문자로마
                        case 2: // 소문자로마
                            num2roman(number[i], dest);
                            if( user_shape[i] == 1 ){
                                char *ptr = dest;
                                while( *ptr )
                                {
                                    *ptr = toupper(*ptr);
                                    ptr++;
                                }
                            }
                            str2hstr(dest, buffer + l);
                            l += strlen(dest);
                            break;
                        case 3:
                            buffer[l++] = 'A' + number[i] -1;
                            break;
                        case 4:
                            buffer[l++] = 'a' + number[i] -1;
                            break;
                        case 5:
                            buffer[l++] = olHanglJaso(number[i] -1, OL_HANGL_KANATA);
                            break;
                        case 6:
                            buffer[l++] = olHanglJaso(number[i] -1, OL_HANGL_JASO);
                            break;
                        case 7: // 한자 숫자 : 일반 숫자로 표현
                            buffer[l++] = '1' + number[i] -1;
                            break;
                        case 8: // 원숫자
                            buffer[l++] = 0x2e00 + number[i];
                            break;
                        case 9: // 원 알파벳 소문자
                            buffer[l++] = 0x2c20 + number[i];
                            break;
                        case 10: // 원 가나다
                            buffer[l++] = 0x2c50 + number[i] -1;
                            break;
                        case 11: // 원 ㄱ ㄴ
                            buffer[l++] = 0x2c40 + number[i] -1;
                            break;
                        case 12: // 이어진 숫자.
                        {
                             char cur_num_str[10],buf[80];
                             int j;
                             buf[0] = 0;
                             for (j = 0; j <= level; j++)
                             {
                                  levelnum = ((number[j] < 1) ? 1 : number[j]);
                                  if ((j && j == level) || (j == level && deco[i][1]))
                                        sprintf(cur_num_str, "%d%c", levelnum, 0);
                                  else
                                        sprintf(cur_num_str, "%d%c", levelnum, '.');
                                  strcat(buf, cur_num_str);
                             }
                             str2hstr(buf, buffer + l);
                             l += strlen(buf);
                            break;
                        }
                        default:
                            buffer[l++] = user_shape[i];
                            break;
                    }
                    if( deco[i][1] ){
                        buffer[l++] = deco[i][1];
                    }
                    buffer[l] = 0;
                    return hstr2ucsstr(buffer, hstr);
                }
        }
    }
     return hstr2ucsstr(buffer, hstr);
}


// 묶음 빈칸(30)
// 고정폭 빈칸(31)
