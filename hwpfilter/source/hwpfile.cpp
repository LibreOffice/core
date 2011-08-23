/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "precompile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "hwplib.h"
#include "hwpfile.h"
#include "hiodev.h"
#include "hfont.h"
#include "hstyle.h"
#include "hbox.h"
#include "hpara.h"
#include "htags.h"
#include "hcode.h"
#include "hstream.h"

#define HWPIDLen    30
#define HWPHeadLen  128
#define HWPSummaryLen   1008

#define V20SIGNATURE    "HWP Document File V2.00 \032\1\2\3\4\5"
#define V21SIGNATURE    "HWP Document File V2.10 \032\1\2\3\4\5"
#define V30SIGNATURE    "HWP Document File V3.00 \032\1\2\3\4\5"

#define FILESTG_SIGNATURE       0xF8995567
#define FILESTG_SIGNATURE_NORMAL    0xF8995568

HWPFile *HWPFile::cur_doc = 0;
static int ccount = 0;
static int pcount = 0;
static int datecodecount = 0;

HWPFile::HWPFile(void)
{
    Init();
}


/**
 * TODO : 추가된 스타일리스트에 대한 메모리 해제
 */
HWPFile::~HWPFile(void)
{
    if (oledata)
        delete oledata;

    if (hiodev)
        delete hiodev;

    LinkedListIterator < ColumnInfo > it_column(&columnlist);
    for (; it_column.current(); it_column++)
        delete it_column.current();

    LinkedListIterator < HWPPara > it(&plist);
    for (; it.current(); it++)
        delete it.current();

    LinkedListIterator < Table > tbl(&tables);
    for (; tbl.current(); tbl++)
        delete tbl.current();

    LinkedListIterator < HyperText > hyp(&hyperlist);
    for (; hyp.current(); hyp++)
    {
        delete hyp.current();
    }
}


void HWPFile::Init(void)
{
    version = HWP_V30;
    info_block_len = 0;
    compressed = false;
    encrypted = false;

    error_code = HWP_NoError;
    hiodev = 0;
    oledata = 0;
    SetCurrentDoc(this);
    currenthyper = 0;
     m_nCurrentPage = 1;
     m_nMaxSettedPage = 0;
}


int HWPFile::ReadHwpFile(HStream & stream)
{
    if (Open(stream) != HWP_NoError)
        return State();
//	printf("HWPFile::ReadHwpFile\n");
    InfoRead();
//	printf("HWPFile::InfoRead Done.\n");
    FontRead();
//	printf("HWPFile::FontRead Done.\n");
    StyleRead();
//	printf("HWPFile::StyleRead Done.\n");
    AddColumnInfo();
    ParaListRead();
//	printf("HWPFile::ParaListRead Done.\n");
    TagsRead();
    //printf("HWPFile::TagsRead Done. State: %d\n", State());

    return State();
}


static int hwp_version(char *str)
{
    if (memcmp(V20SIGNATURE, str, HWPIDLen) == 0)
        return HWP_V20;
    else if (memcmp(V21SIGNATURE, str, HWPIDLen) == 0)
        return HWP_V21;
    else if (memcmp(V30SIGNATURE, str, HWPIDLen) == 0)
        return HWP_V30;
    return 0;
}


// HIODev wrapper

int HWPFile::Open(HStream & stream)
{
    HStreamIODev *hstreamio;

    if (0 == (hstreamio = new HStreamIODev(stream)))
    {
        printf(" hstreamio is not instanciate \n");
        return SetState(errno);
    }
    if (!hstreamio->open())
    {
        delete hstreamio;

        return SetState(HWP_EMPTY_FILE);
    }
    SetIODevice(hstreamio);

    char idstr[HWPIDLen];

    if (ReadBlock(idstr, HWPIDLen) <= 0
        || HWP_V30 != (version = hwp_version(idstr)))
    {
        return SetState(HWP_UNSUPPORTED_VERSION);
    }
    return HWP_NoError;
}


int HWPFile::State(void) const
{
    return error_code;
}


int HWPFile::SetState(int errcode)
{
    error_code = errcode;
    return error_code;
}


int HWPFile::Read1b(void)
{
    return hiodev ? hiodev->read1b() : -1;
}


int HWPFile::Read2b(void)
{
    return hiodev ? hiodev->read2b() : -1;
}


long HWPFile::Read4b(void)
{
    return hiodev ? hiodev->read4b() : -1;
}


int HWPFile::Read1b(void *ptr, size_t nmemb)
{
    return hiodev ? hiodev->read1b(ptr, nmemb) : 0;
}


int HWPFile::Read2b(void *ptr, size_t nmemb)
{
    return hiodev ? hiodev->read2b(ptr, nmemb) : 0;
}


int HWPFile::Read4b(void *ptr, size_t nmemb)
{
    return hiodev ? hiodev->read4b(ptr, nmemb) : 0;
}


size_t HWPFile::ReadBlock(void *ptr, size_t size)
{
    return hiodev ? hiodev->readBlock(ptr, size) : 0;
}


size_t HWPFile::SkipBlock(size_t size)
{
    return hiodev ? hiodev->skipBlock(size) : 0;
}


bool HWPFile::SetCompressed(bool flag)
{
    return hiodev ? hiodev->setCompressed(flag) : false;
}


HIODev *HWPFile::SetIODevice(HIODev * new_hiodev)
{
    HIODev *old_hiodev = hiodev;

    hiodev = new_hiodev;
    return old_hiodev;
}


// end of HIODev wrapper

bool HWPFile::InfoRead(void)
{
    return _hwpInfo.Read(*this);
}


bool HWPFile::FontRead(void)
{
    return _hwpFont.Read(*this);
}


bool HWPFile::StyleRead(void)
{
    return _hwpStyle.Read(*this);
}


bool HWPFile::ParaListRead(void)
{
    return ReadParaList(plist);
}

bool HWPFile::ReadParaList(LinkedList < HWPPara > &aplist, unsigned char flag)
{
    LinkedListIterator < HWPPara > it(&aplist);

    HWPPara *spNode = new HWPPara;
     unsigned char tmp_etcflag;
     unsigned char prev_etcflag = 0;
    while (spNode->Read(*this, flag))
    {
         if( !(spNode->etcflag & 0x04) ){
          tmp_etcflag = spNode->etcflag;
          spNode->etcflag = prev_etcflag;
          prev_etcflag = tmp_etcflag;
         }
        if (spNode->nch && spNode->reuse_shape)
        {
            if (aplist.count()){
                     spNode->pshape = aplist.last()->pshape;
                }
                else{
                     spNode->nch = 0;
                     spNode->reuse_shape = 0;
                }
        }
          spNode->pshape.pagebreak = spNode->etcflag;
          if( spNode->nch )
                AddParaShape( &spNode->pshape );

        if (aplist.count())
            aplist.last()->SetNext(spNode);
        aplist.insert(spNode, -1);
        spNode = new HWPPara;
    }
    delete spNode;

    return true;
}


bool HWPFile::TagsRead(void)
{
    ulong tag;
    long size;

    while (1)
    {
        tag = Read4b();
        size = Read4b();
        if (size <= 0 && tag > 0){
            //return false;
            continue;
          }

        if (tag == FILETAG_END_OF_COMPRESSED ||
            tag == FILETAG_END_OF_UNCOMPRESSED)
            return true;
        switch (tag)
        {
            case FILETAG_EMBEDDED_PICTURE:
            {
                EmPicture *emb = new EmPicture(size);

                if (true == emb->Read(*this))
                    emblist.insert(emb, -1);
                else
                    delete emb;
            }
            break;
            case FILETAG_OLE_OBJECT:
                if (oledata)
                    delete oledata;
                oledata = new OlePicture(size);
                oledata->Read(*this);
                break;
            case FILETAG_HYPERTEXT:
            {
                if( (size % 617) != 0 )
                    SkipBlock( size );
                else
                    for( int i = 0 ; i < size/617 ; i++)
                {
                    HyperText *hypert = new HyperText;
                    hypert->Read(*this);
                    hyperlist.insert(hypert, -1);
                }
                break;
            }
                case 6:
                {
                     ReadBlock(_hwpInfo.back_info.reserved1, 8);
                     _hwpInfo.back_info.luminance = Read4b();
                     _hwpInfo.back_info.contrast = Read4b();
                     _hwpInfo.back_info.effect = sal::static_int_cast<char>(Read1b());
                     ReadBlock(_hwpInfo.back_info.reserved2, 7);
                     ReadBlock(_hwpInfo.back_info.filename, 260);
                     ReadBlock(_hwpInfo.back_info.color, 3);
                     unsigned short nFlag = sal::static_int_cast<unsigned short>(Read2b());
                     _hwpInfo.back_info.flag = nFlag >> 8 ;
                     int nRange = Read4b();
                     _hwpInfo.back_info.range = nRange >> 24;
                     ReadBlock(_hwpInfo.back_info.reserved3, 27);
                     _hwpInfo.back_info.size = Read4b();

                     _hwpInfo.back_info.data = new char[(unsigned int)_hwpInfo.back_info.size];
                     ReadBlock(_hwpInfo.back_info.data, _hwpInfo.back_info.size);

                     if( _hwpInfo.back_info.size > 0 )
                          _hwpInfo.back_info.type = 2;
                     else if( _hwpInfo.back_info.filename[0] )
                          _hwpInfo.back_info.type = 1;
                     else
                          _hwpInfo.back_info.type = 0;


                     _hwpInfo.back_info.isset = true;

                     break;
                }
            case FILETAG_PRESENTATION:
            case FILETAG_PREVIEW_IMAGE:
            case FILETAG_PREVIEW_TEXT:
            default:
                SkipBlock(size);
        }
    }
//    return false;
}


ColumnDef *HWPFile::GetColumnDef(int num)
{
    ColumnInfo *cinfo = columnlist.find(num);
    if( cinfo )
        return cinfo->coldef;
    else
        return 0;
}
/* @return 인덱스는 1부터 시작한다. */
int HWPFile::GetPageMasterNum(int page)
{
    LinkedListIterator<ColumnInfo> it(&columnlist);
    //os: unused
    //ColumnInfo *prev = 0;
    ColumnInfo *now = 0;
    int i;

    for( i = 1 ; it.current() ; it++, i++){
        now = it.current();
        if( page < now->start_page )
            return i-1;
    }
    return i-1;
}

HyperText *HWPFile::GetHyperText()
{
    return hyperlist.find(currenthyper++);
}

EmPicture *HWPFile::GetEmPicture(Picture * pic)
{
    char *name = pic->picinfo.picembed.embname;

    name[0] = 'H';
    name[1] = 'W';
    name[2] = 'P';

    LinkedListIterator < EmPicture > it(&emblist);
    for (; it.current(); it++)
        if (strcmp(name, it.current()->name) == 0)
            return it.current();
    return 0;
}

EmPicture *HWPFile::GetEmPictureByName(char * name)
{
    name[0] = 'H';
    name[1] = 'W';
    name[2] = 'P';

    LinkedListIterator < EmPicture > it(&emblist);
    for (; it.current(); it++)
        if (strcmp(name, it.current()->name) == 0)
            return it.current();
    return 0;
}


void HWPFile::AddBox(FBox * box)
{
// LATER if we don't use box->next(),
// AddBox() and GetBoxHead() are useless;
    if (blist.count())
    {
        box->prev = blist.last();
        box->prev->next = box;
    }
    else
        box->prev = 0;
    blist.insert(box, -1);
}


ParaShape *HWPFile::getParaShape(int index)
{
    return pslist.find(index);
}


CharShape *HWPFile::getCharShape(int index)
{
    return cslist.find(index);
}


FBoxStyle *HWPFile::getFBoxStyle(int index)
{
    return fbslist.find(index);
}

DateCode *HWPFile::getDateCode(int index)
{
    return datecodes.find(index);
}

HeaderFooter *HWPFile::getHeaderFooter(int index)
{
    return headerfooters.find(index);
}

ShowPageNum *HWPFile::getPageNumber(int index)
{
    return pagenumbers.find(index);
}

Table *HWPFile::getTable(int index)
{
    return tables.find(index);
}

void HWPFile::AddParaShape(ParaShape * pshape)
{
    int nscount = 0;
    for(int j = 0 ; j < MAXTABS-1 ; j++)
    {
          if( j > 0 && pshape->tabs[j].position == 0 )
                break;
          if( pshape->tabs[0].position == 0 ){
                if( pshape->tabs[j].type || pshape->tabs[j].dot_continue ||
                     (pshape->tabs[j].position != 1000 *j) )
                          nscount = j;
          }
          else{
                if( pshape->tabs[j].type || pshape->tabs[j].dot_continue ||
                     (pshape->tabs[j].position != 1000 * (j + 1)) )
                          nscount = j;
          }
    }
    if( nscount )
        pshape->tabs[MAXTABS-1].type = sal::static_int_cast<char>(nscount);
     int value = compareParaShape(pshape);
    if( value == 0 || nscount )
    {
        pshape->index = ++pcount;
        pslist.insert(pshape, -1);
    }
    else
        pshape->index = value;
}


void HWPFile::AddCharShape(CharShape * cshape)
{
    int value = compareCharShape(cshape);
    if( value == 0 )
    {
        cshape->index = ++ccount;
        cslist.insert(cshape, -1);
    }
    else
        cshape->index = value;
}

void HWPFile::AddColumnInfo()
{
    ColumnInfo *cinfo = new ColumnInfo(m_nCurrentPage);
   columnlist.insert(cinfo, -1);
    setMaxSettedPage();
}

void HWPFile::SetColumnDef(ColumnDef *coldef)
{
    ColumnInfo *cinfo = columnlist.last();
    if( cinfo->bIsSet )
        return;
    cinfo->coldef = coldef;
    cinfo->bIsSet = true;
}

void HWPFile::AddDateFormat(DateCode * hbox)
{
    hbox->key = sal::static_int_cast<char>(++datecodecount);
    datecodes.insert(hbox, -1);
}

void HWPFile::AddPageNumber(ShowPageNum * hbox)
{
    pagenumbers.insert(hbox, -1);
}

void HWPFile::AddHeaderFooter(HeaderFooter * hbox)
{
    headerfooters.insert(hbox, -1);
}

void HWPFile::AddTable(Table * hbox)
{
    tables.insert(hbox, -1);
}

void HWPFile::AddFBoxStyle(FBoxStyle * fbstyle)
{
    fbslist.insert(fbstyle, -1);
}

int HWPFile::compareCharShape(CharShape *shape)
{
    int count = cslist.count();
    if( count > 0 )
    {
        CharShape *cshape=0;
        for(int i = 0; i< count; i++)
        {
            cshape = cslist.find(i);

            if( shape->size == cshape->size &&
                shape->font[0] == cshape->font[0] &&
                shape->ratio[0] == cshape->ratio[0] &&
                shape->space[0] == cshape->space[0] &&
                shape->color[1] == cshape->color[1] &&
                shape->color[0] == cshape->color[0] &&
                shape->shade == cshape->shade &&
                shape->attr == cshape->attr )
            {
                return cshape->index;
            }
        }
    }
    return 0;
}


int HWPFile::compareParaShape(ParaShape *shape)
{
    int count = pslist.count();
    if( count > 0 )
    {
        ParaShape *pshape=0;
        for(int i = 0; i< count; i++)
        {
            pshape = pslist.find(i);
            if( shape->left_margin == pshape->left_margin &&
                shape->right_margin == pshape->right_margin &&
                shape->pspacing_prev == pshape->pspacing_prev &&
                shape->pspacing_next == pshape->pspacing_next &&
                shape->indent == pshape->indent &&
                shape->lspacing == pshape->lspacing &&
                shape->arrange_type == pshape->arrange_type &&
                shape->outline == pshape->outline  &&
                     shape->pagebreak == pshape->pagebreak)
            {
                    if( shape->cshape->size == pshape->cshape->size &&
                         shape->cshape->font[0] == pshape->cshape->font[0] &&
                         shape->cshape->ratio[0] == pshape->cshape->ratio[0] &&
                         shape->cshape->space[0] == pshape->cshape->space[0] &&
                         shape->cshape->color[1] == pshape->cshape->color[1] &&
                         shape->cshape->color[0] == pshape->cshape->color[0] &&
                         shape->cshape->shade == pshape->cshape->shade &&
                         shape->cshape->attr == pshape->cshape->attr )
                    {
                         return pshape->index;
                    }
            }
        }
    }
    return 0;
}


HWPFile *GetCurrentDoc(void)
{
    return HWPFile::cur_doc;
}


HWPFile *SetCurrentDoc(HWPFile * hwpfp)
{
    HWPFile *org = HWPFile::cur_doc;

    HWPFile::cur_doc = hwpfp;
    return org;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
