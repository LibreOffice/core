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

/* $Id: hpara.cpp,v 1.6 2008-06-04 09:59:35 vg Exp $ */

#include "precompile.h"

#include "hwplib.h"
#include "hwpfile.h"
#include "hpara.h"
#include "hbox.h"
#include "hutil.h"
#include "hutil.h"

bool LineInfo::Read(HWPFile & hwpf, HWPPara *pPara)
{
    pos = sal::static_int_cast<unsigned short>(hwpf.Read2b());
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
        pPara->pshape.reserved[0] = sal::static_int_cast<unsigned char>(pex & 0x01);
        pPara->pshape.reserved[1] = sal::static_int_cast<unsigned char>(pex & 0x02);
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
/* C++?? null?? ???????? ????????. */
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


/* Paragraph ???? ???? */
    cshape.Read(hwpf);
    if (nch > 0)
        hwpf.AddCharShape(&cshape);

/* Paragraph ???? ???? */
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
        if (0 == (hhstr[ii] = readHBox(hwpf)))
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
    hchar hh = sal::static_int_cast<hchar>(hwpf.Read2b());
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
            //strange construct to compile without warning
            int nTemp = fbox->pgy;
            nTemp -= begin_ypos;
            fbox->pgy = sal::static_int_cast<short>(nTemp);
        }
    }
    return hbox;
//return 0;
}
