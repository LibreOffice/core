/*************************************************************************
 *
 *  $RCSfile: fontmap.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:38:55 $
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

#include <stdio.h>
#ifdef SOLARIS
#include <strings.h>
#else
#include <string.h>
#endif

struct FontEntry
{
    const char *familyname;
    int key;
     double ratio;
};
/**
 * ratio는 한글 70%, 숫자 10% 영문 20%의 비율로 구성되었다는 가정하에 정해진다.
 */
const struct FontEntry FontMapTab[] =
{
    {"명조",0, 0.97},
    {"고딕",1, 0.97},
    {"샘물",1, 0.97},
    {"필기",0, 0.97},
    {"시스템",1, 0.84},
    {"시스템 약자",1, 0.84},
    {"시스템 간자",1, 0.84},
    {"HY둥근 고딕",2, 0.97},
    {"옛한글",0, 0.97},
    {"가는공한",0, 0.72},
    {"중간공한",0, 0.72},
    {"굵은공한",0, 0.72},
    {"가는한",0, 0.72},
    {"중간한",0, 0.72},
    {"굵은한",0, 0.72},
    {"휴먼명조",0, 0.97},
    {"휴먼고딕",1, 0.97},
    {"가는안상수체",0, 0.55},
    {"중간안상수체",0, 0.637},
    {"굵은안상수체",0, 0.63},
    {"휴먼가는샘체",0, 0.666},
    {"휴먼중간샘체",0, 0.685},
    {"휴먼굵은샘체",0, 0.727},
    {"휴먼가는팸체",0, 0.666},
    {"휴먼중간팸체",0, 0.685},
    {"휴먼굵은팸체",0, 0.727},
    {"휴먼옛체",3, 0.97},
    {"한양신명조",0, 0.97},
    {"한양견명조",1, 0.97},
    {"한양중고딕",1, 0.97},
    {"한양견고딕",1, 0.97},
    {"한양그래픽",0, 0.97},
    {"한양궁서",3, 0.97},
    {"문화바탕",0, 0.97},
    {"문화바탕제목",1, 0.97},
    {"문화돋움",1, 0.97},
    {"문화돋움제목",0, 0.97},
    {"문화쓰기",0, 0.97},
    {"문화쓰기흘림",0, 0.97},
    {"펜흘림",0, 0.97},
    {"복숭아",0, 0.827},
    {"옥수수",0, 0.97},
    {"오이",0, 0.97},
    {"가지",0, 0.97},
    {"강낭콩",2, 0.97},
    {"딸기",3, 0.97},
    {"타이프",0, 0.987},
    {"태 나무",1, 0.97},
    {"태 헤드라인",0, 0.97},
    {"태 가는 헤드라인",0, 0.97},
    {"태 헤드라인T",0, 0.97},
    {"태 가는 헤드라인T",0, 0.97},
    {"양재 다운명조M",0, 0.97},
    {"양재 본목각M",0, 0.97},
    {"양재 소슬",1, 0.97},
    {"양재 매화",1, 0.987},
    {"양재 튼튼",0, 0.97},
    {"양재 참숯",1, 0.97},
    {"양재 둘기",0, 0.97},
    {"양재 샤넬",1, 0.97},
    {"양재 와당",1, 0.97},
    {"신명 세명조",0, 0.97},
    {"신명 신명조",0, 0.97},
    {"신명 신신명조",0, 0.97},
    {"신명 중명조",0, 0.97},
    {"신명 태명조",0, 0.97},
    {"신명 견명조",0, 0.97},
    {"신명 신문명조",0, 0.97},
    {"신명 순명조",0, 0.97},
    {"신명 세고딕",1, 0.97},
    {"신명 중고딕",1, 0.97},
    {"신명 태고딕",1, 0.97},
    {"신명 견고딕",1, 0.97},
    {"신명 세나루",2, 0.97},
    {"신명 디나루",2, 0.97},
    {"신명 신그래픽",2, 0.97},
    {"신명 태그래픽",2, 0.97},
    {"신명 궁서",3, 0.97}
};

#define FONTCOUNT 4
#ifndef WIN32
#if defined(LINUX)
char* RepFontTab[] =
{
    "백묵 바탕",                                     /* 0 */
    "백묵 돋움",                                      /* 1 */
    "백묵 굴림",                                      /* 2 */
    "백묵 헤드라인"                                      /* 3 */
};
#else
char* RepFontTab[] =
{
    "Batang",                                     /* 0 */
    "Dotum",                                      /* 1 */
    "Gulim",                                      /* 2 */
    "Gungso"                                      /* 3 */
};
#endif
#else
char* RepFontTab[] =
{
    "바탕",                                       /* 0 */
    "돋움",                                       /* 1 */
    "굴림",                                       /* 2 */
    "궁서"                                        /* 3 */
};
#endif

int getRepFamilyName(const char* orig, char *buf, double &ratio)
{
    int i;
    int size = sizeof(FontMapTab)/sizeof(FontEntry);
    for( i = 0 ; i < size ; i++)
    {
        if( !strcmp(orig, FontMapTab[i].familyname) ){
                ratio = FontMapTab[i].ratio;
            return strlen( strcpy(buf,RepFontTab[FontMapTab[i].key]) );
          }
    }
     ratio = FontMapTab[0].ratio;
    return strlen( strcpy(buf, RepFontTab[0] ) );
}
