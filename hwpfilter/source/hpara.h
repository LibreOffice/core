/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hpara.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:37:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* $Id: hpara.h,v 1.3 2005-09-07 16:37:21 rt Exp $ */

#ifndef _HWPPARA_H_
#define _HWPPARA_H_

#include <hwplib.h>
#include <hwpfile.h>
#include <hinfo.h>

struct HBox;

/**
 * etc flag
 * 0 bit : Use page columns
 * 1 bit : Use page lows
 * 2 bit : Use section break
 * 3 bit : Block start
 * 4 bit : In Block
 * 5 bit : Block end
 * 6 bit : Preserve widow orphan
 * 7 bit : Reserved
 */
enum
{
    PA_USER_COLUMN    =   1,
    PA_USER_PAGE      =   2,                      /* p user_page definiton */
    PA_SECTION_BREAK  =   4,
    PA_BLOCK_BEGIN    =   8,
    PA_IN_BLOCK       =   16,
    PA_BLOCK_END      =   32,
    PA_WIDOW_ORPHAN   =   64
};

class HWPPara;
#define FIXED_SPACING_BIT (0x8000)

struct LineInfo
{
/* 시작하는 글자의 위치 : 콘트롤은 여러 길이를 가진다 */
/**
 * Starting character position
 */
    unsigned short    pos;
    hunit         space_width;
    hunit         height;
    hunit         pgy;                            /* internal */
    hunit         sx;                             /* internal */
    hunit         psx;                            /* internal */
    hunit         pex;                            /* internal */
// for formating
    hunit         height_sp;
    unsigned short    softbreak;                  // column, page, section

    bool  Read(HWPFile &hwpf, HWPPara *para);
};
/**
 * It represents the paragraph.
 * @short Paragraph
 */
class DLLEXPORT HWPPara
{
    private:
        HWPPara       *_next;

    public:
// paragraph information
/**
 * Zero is for the new paragraph style.
 */
        unsigned char     reuse_shape;            /* 0이면 새모양 */
        unsigned short    nch;
        unsigned short    nline;

// realking
        hunit         begin_ypos;
        unsigned char     scflag;

/**
 * If the value is 0, all character of paragraph have same style given cshape
 */
        unsigned char     contain_cshape;         /* 0이면 모든 글자가 대표 글자 모양 */
        unsigned char     etcflag;
/**
 * Checks the special characters in the paragraph
 */
        unsigned long     ctrlflag;
        unsigned char     pstyno;
        CharShape     cshape;                     /* 글자가 모두 같은 모양일때  */
        ParaShape     pshape;                     /* reuse flag가 0이면     */
        int           pno;                        /* run-time only      */

        LineInfo      *linfo;
        CharShape     *cshapep;
/**
 * Box object list
 */
        HBox          **hhstr;

        HWPPara(void);
        ~HWPPara(void);

        int   Read(HWPFile &hwpf, unsigned char flag = 0);
        int   Write(CTextOut &txtf);
        int   Write(CHTMLOut &html);

        void  SetNext(HWPPara *n) { _next = n; };

/* layout을 위한 함수 */
/**
 * Get line information of given line
 */
        LineInfo *GetLineInfo(int line);
/**
 * Returns the character sytle of paragraph.
 */
        CharShape *GetCharShape(int pos);
/**
 * Returns the sytle of paragraph.
 */
        ParaShape *GetParaShape(void);

/**
 * Returns previous paragraph.
 */
        HWPPara *Prev(void);
/**
 * Returns next paragraph.
 */
        HWPPara *Next(void);

        int HomePos(int line) const;
        int EndPos(int line) const;
        int LineLen(int line) const;

    private:
        HBox *readHBox(HWPFile &);
};

// inline functions

inline int HWPPara::HomePos(int line) const
{
    if( nline < line + 1 ) return nch;
    return linfo[line].pos;
}


inline int HWPPara::EndPos(int line) const
{
    if( nline <= line + 1 ) return nch;
    else return HomePos(line + 1);
}


inline int HWPPara::LineLen(int line) const
{
    return EndPos(line) - HomePos(line);
}
#endif                                            /* _HWPPARA_H_ */
