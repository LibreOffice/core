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

#include <osl/diagnose.h>

#include <comphelper/newarray.hxx>

#include "hwplib.h"
#include "hwpfile.h"
#include "hpara.h"
#include "hbox.h"
#include "hutil.h"

bool LineInfo::Read(HWPFile & hwpf, HWPPara *pPara)
{
    if (!hwpf.Read2b(pos))
        return false;
    unsigned short tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    space_width = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    height = tmp16;
// internal information
    if (!hwpf.Read2b(tmp16))
        return false;
    pgy = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    sx = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    psx = tmp16;
    if (!hwpf.Read2b(tmp16))
        return false;
    pex = tmp16;
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

HWPPara::HWPPara()
    : _next(nullptr)
    , reuse_shape(0)
    , nch(0)
    , nline(0)
    , begin_ypos(0)
    , scflag(0)
    , contain_cshape(0)
    , etcflag(0)
    , ctrlflag(0)
    , pstyno(0)
    , pno(0)
    , linfo(nullptr)
    , cshapep(nullptr)
    , hhstr(nullptr)
{
    memset(&cshape, 0, sizeof(cshape));
    memset(&pshape, 0, sizeof(pshape));
}

HWPPara::~HWPPara()
{
    delete[] linfo;
    delete[] cshapep;
    if (hhstr)
    {
// virtual destructor
/* C++은 null에 대해서도 동작한다. */
        for (int ii = 0; ii < nch; ++ii)
            delete hhstr[ii];

        delete[]hhstr;
    }

}


bool HWPPara::Read(HWPFile & hwpf, unsigned char flag)
{
    unsigned char same_cshape;
    int ii;
    scflag = flag;
// Paragraph Information
    hwpf.Read1b(&reuse_shape, 1);
    hwpf.Read2b(&nch, 1);
    hwpf.Read2b(&nline, 1);
    hwpf.Read1b(&contain_cshape, 1);
    hwpf.Read1b(&etcflag, 1);
    hwpf.Read4b(&ctrlflag, 1);
    hwpf.Read1b(&pstyno, 1);


/* Paragraph representative character */
    cshape.Read(hwpf);
    if (nch > 0)
        hwpf.AddCharShape(&cshape);

/* Paragraph paragraphs shape  */
    if (nch && !reuse_shape)
    {
        pshape.Read(hwpf);
        pshape.cshape = &cshape;
          pshape.pagebreak = etcflag;
    }

    linfo = ::comphelper::newArray_null<LineInfo>(nline);
    if (!linfo) { return false; }
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
        cshapep = ::comphelper::newArray_null<CharShape>(nch);
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
    hhstr = ::comphelper::newArray_null<HBox *>(nch);
    if (!hhstr) { return false; }
    for (ii = 0; ii < nch; ii++)
        hhstr[ii] = nullptr;
    ii = 0;
    while (ii < nch)
    {
        if (nullptr == (hhstr[ii] = readHBox(hwpf)))
            return false;
        if (hhstr[ii]->hh == CH_END_PARA)
            break;
          if( hhstr[ii]->hh < CH_END_PARA )
                pshape.reserved[0] = 0;
        ii += hhstr[ii]->WSize();
    }
    return nch && !hwpf.State();
}




CharShape *HWPPara::GetCharShape(int pos)
{
    if (contain_cshape == 0)
        return &cshape;
    return cshapep + pos;
}




HBox *HWPPara::readHBox(HWPFile & hwpf)
{
    hchar hh;
    if (!hwpf.Read2b(hh))
        return nullptr;

    HBox *hbox = nullptr;

    if (hwpf.State() != HWP_NoError)
        return nullptr;

    if (hh > 31 || hh == CH_END_PARA)
        hbox = new HBox(hh);
    else if (IS_SP_SKIP_BLOCK(hh))
        hbox = new SkipData(hh);
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

        return nullptr;
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
