/*************************************************************************
 *
 *  $RCSfile: hpara.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:40:24 $
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

/* $Id: hpara.cpp,v 1.1 2003-10-15 14:40:24 dvo Exp $ */

#include "precompile.h"

#include "hwplib.h"
#include "hwpfile.h"
#include "hpara.h"
#include "hbox.h"
#include "hutil.h"
#include "hutil.h"

bool LineInfo::Read(HWPFile & hwpf, HWPPara *pPara)
{
    pos = hwpf.Read2b();
    space_width = (short) hwpf.Read2b();
    height = (short) hwpf.Read2b();
// internal informations
    pgy = (short) hwpf.Read2b();
    sx = (short) hwpf.Read2b();
    psx = (short) hwpf.Read2b();
    pex = (short) hwpf.Read2b();
    height_sp = 0;

    if( pex >> 15 & 0x01 )
    {
          if( pex & 0x01 )
                hwpf.AddPage();
        pPara->pshape.reserved[0] = pex & 0x01;
        pPara->pshape.reserved[1] = pex & 0x02;
    }

    return (!hwpf.State());
}


HWPPara::HWPPara(void)
{
    _next = NULL;
    linfo = NULL;
    cshapep = NULL;
    hhstr = NULL;
    pno = 0;

}


HWPPara::~HWPPara(void)
{
    int ii;

    if (linfo)
        delete[]linfo;
    if (cshapep)
        delete[]cshapep;
    if (hhstr)
    {
// virtual destructor
// C++은 null에 대해서도 동작한다.
        for (ii = 0; ii < nch; ++ii)
            delete hhstr[ii];

        delete[]hhstr;
    }

}


int HWPPara::Read(HWPFile & hwpf, unsigned char flag)
{
    unsigned char same_cshape;
    register int ii;
    scflag = flag;
// Paragraph Infomation
    hwpf.Read1b(&reuse_shape, 1);
    hwpf.Read2b(&nch, 1);
    hwpf.Read2b(&nline, 1);
    hwpf.Read1b(&contain_cshape, 1);
    hwpf.Read1b(&etcflag, 1);
    hwpf.Read4b(&ctrlflag, 1);
    hwpf.Read1b(&pstyno, 1);


// Paragraph 대표 글자
    cshape.Read(hwpf);
    if (nch > 0)
        hwpf.AddCharShape(&cshape);

// Paragraph 문단 모양
    if (nch && !reuse_shape)
    {
        pshape.Read(hwpf);
        pshape.cshape = &cshape;
          pshape.pagebreak = etcflag;
    }

    linfo = new LineInfo[nline];
    for (ii = 0; ii < nline; ii++)
    {
        linfo[ii].Read(hwpf, this);
    }
     if( etcflag & 0x04 ){
         hwpf.AddColumnInfo();
     }

    if (nch && !reuse_shape){
         if( pshape.coldef.ncols > 1 ){
             hwpf.SetColumnDef( &pshape.coldef );
         }
     }


    if( nline > 0 )
    {
        begin_ypos = linfo[0].pgy;
    }
    else
    {
        begin_ypos = 0;
    }

    if (contain_cshape)
    {
        cshapep = new CharShape[nch];
        if (!cshapep)
        {
            perror("Memory Allocation: cshape\n");
            return false;
        }

        for (ii = 0; ii < nch; ii++)
        {

            hwpf.Read1b(&same_cshape, 1);
            if (!same_cshape)
            {
                cshapep[ii].Read(hwpf);
                if (nch > 1)
                    hwpf.AddCharShape(&cshapep[ii]);
            }
            else if (ii == 0)
                cshapep[ii] = cshape;
            else
                cshapep[ii] = cshapep[ii - 1];
        }
    }
// read string
    hhstr = new HBox *[nch];
    for (ii = 0; ii < nch; ii++)
        hhstr[ii] = 0;
    ii = 0;
    while (ii < nch)
    {
        if (!(hhstr[ii] = readHBox(hwpf)))
            return false;
        if (hhstr[ii]->hh == CH_END_PARA)
            break;
          if( hhstr[ii]->hh < CH_END_PARA )
                pshape.reserved[0] = 0;
        ii += hhstr[ii]->WSize();
    }
    return nch && !hwpf.State();
}


HWPPara *HWPPara::Next(void)
{
    return _next;
}


// layout을 위한 함수

LineInfo *HWPPara::GetLineInfo(int line)
{
    if (line < 0 || line >= nline)
        line = 0;
    return linfo + line;

}


CharShape *HWPPara::GetCharShape(int pos)
{
    if (contain_cshape == 0)
        return &cshape;
    return cshapep + pos;
}


ParaShape *HWPPara::GetParaShape(void)
{
    return &pshape;
}


HBox *HWPPara::readHBox(HWPFile & hwpf)
{
    hchar hh = hwpf.Read2b();
    HBox *hbox = 0;

    if (hwpf.State() != HWP_NoError)
        return 0;
//hbox = new HBox(hh);
    if (hh > 31 || hh == CH_END_PARA)
        hbox = new HBox(hh);
    else if (IS_SP_SKIP_BLOCK(hh))
        hbox = new SkipBlock(hh);
    else
    {
        switch (hh)
        {
            case CH_FIELD:                        // 5
                hbox = new FieldCode;
                break;
            case CH_BOOKMARK:                     // 6
                hbox = new Bookmark;
                break;
            case CH_DATE_FORM:                    // 7
                hbox = new DateFormat;
                break;
            case CH_DATE_CODE:                    // 8
                hbox = new DateCode;
                break;
            case CH_TAB:                          // 9
                hbox = new Tab;
                break;
            case CH_TEXT_BOX:                     // 10
                hbox = new TxtBox;
                break;
            case CH_PICTURE:                      // 11
                hbox = new Picture;
                break;
            case CH_LINE:                         // 14
                hbox = new Line;
                break;
            case CH_HIDDEN:                       // 15
                hbox = new Hidden;
                break;
            case CH_HEADER_FOOTER:                // 16
                hbox = new HeaderFooter;
                break;
            case CH_FOOTNOTE:                     // 17
                hbox = new Footnote;
                break;
            case CH_AUTO_NUM:                     // 18
                hbox = new AutoNum;
                break;
            case CH_NEW_NUM:                      // 19
                hbox = new NewNum;
                break;
            case CH_SHOW_PAGE_NUM:                // 20
                hbox = new ShowPageNum;
                break;
            case CH_PAGE_NUM_CTRL:                // 21
                hbox = new PageNumCtrl;
                break;
            case CH_MAIL_MERGE:                   // 22
                hbox = new MailMerge;
                break;
            case CH_COMPOSE:                      // 23
                hbox = new Compose;
                break;
            case CH_HYPHEN:                       // 24
                hbox = new Hyphen;
                break;
            case CH_TOC_MARK:                     // 25
                hbox = new TocMark;
                break;
            case CH_INDEX_MARK:                   // 26
                hbox = new IndexMark;
                break;
            case CH_OUTLINE:                      // 28
                hbox = new Outline;
                break;
            case CH_KEEP_SPACE:                   // 30
                hbox = new KeepSpace;
                break;
            case CH_FIXED_SPACE:                  // 31
                hbox = new FixedSpace;
                break;
            default:
                break;
        }
    }
    if (!hbox || !hbox->Read(hwpf))
    {
        delete hbox;

        return 0;
    }
    if( hh == CH_TEXT_BOX || hh == CH_PICTURE || hh == CH_LINE )
    {
        FBox *fbox = static_cast<FBox *>(hbox);
        if( ( fbox->style.anchor_type == 1) && ( fbox->pgy >= begin_ypos) )
        {
            fbox->pgy -= begin_ypos;
        }
    }
    return hbox;
//return 0;
}
