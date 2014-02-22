/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    pos = sal::static_int_cast<unsigned short>(hwpf.Read2b());
    space_width = (short) hwpf.Read2b();
    height = (short) hwpf.Read2b();

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
    if (linfo)
        delete[]linfo;
    if (cshapep)
        delete[]cshapep;
    if (hhstr)
    {

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

    hwpf.Read1b(&reuse_shape, 1);
    hwpf.Read2b(&nch, 1);
    hwpf.Read2b(&nline, 1);
    hwpf.Read1b(&contain_cshape, 1);
    hwpf.Read1b(&etcflag, 1);
    hwpf.Read4b(&ctrlflag, 1);
    hwpf.Read1b(&pstyno, 1);


/* Paragraph 대표 글자 */
    cshape.Read(hwpf);
    if (nch > 0)
        hwpf.AddCharShape(&cshape);

/* Paragraph 문단 모양 */
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

    hhstr = ::comphelper::newArray_null<HBox *>(nch);
    if (!hhstr) { return false; }
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

    if (hh > 31 || hh == CH_END_PARA)
        hbox = new HBox(hh);
    else if (IS_SP_SKIP_BLOCK(hh))
        hbox = new SkipData(hh);
    else
    {
        switch (hh)
        {
            case CH_FIELD:                        
                hbox = new FieldCode;
                break;
            case CH_BOOKMARK:                     
                hbox = new Bookmark;
                break;
            case CH_DATE_FORM:                    
                hbox = new DateFormat;
                break;
            case CH_DATE_CODE:                    
                hbox = new DateCode;
                break;
            case CH_TAB:                          
                hbox = new Tab;
                break;
            case CH_TEXT_BOX:                     
                hbox = new TxtBox;
                break;
            case CH_PICTURE:                      
                hbox = new Picture;
                break;
            case CH_LINE:                         
                hbox = new Line;
                break;
            case CH_HIDDEN:                       
                hbox = new Hidden;
                break;
            case CH_HEADER_FOOTER:                
                hbox = new HeaderFooter;
                break;
            case CH_FOOTNOTE:                     
                hbox = new Footnote;
                break;
            case CH_AUTO_NUM:                     
                hbox = new AutoNum;
                break;
            case CH_NEW_NUM:                      
                hbox = new NewNum;
                break;
            case CH_SHOW_PAGE_NUM:                
                hbox = new ShowPageNum;
                break;
            case CH_PAGE_NUM_CTRL:                
                hbox = new PageNumCtrl;
                break;
            case CH_MAIL_MERGE:                   
                hbox = new MailMerge;
                break;
            case CH_COMPOSE:                      
                hbox = new Compose;
                break;
            case CH_HYPHEN:                       
                hbox = new Hyphen;
                break;
            case CH_TOC_MARK:                     
                hbox = new TocMark;
                break;
            case CH_INDEX_MARK:                   
                hbox = new IndexMark;
                break;
            case CH_OUTLINE:                      
                hbox = new Outline;
                break;
            case CH_KEEP_SPACE:                   
                hbox = new KeepSpace;
                break;
            case CH_FIXED_SPACE:                  
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
            
            int nTemp = fbox->pgy;
            nTemp -= begin_ypos;
            fbox->pgy = sal::static_int_cast<short>(nTemp);
        }
    }
    return hbox;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
