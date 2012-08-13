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

//
// hwpfile.h
// (C) 1998 Mizi Research, All rights are reserved
//

#ifndef _HWPFILE_H_
#define _HWPFILE_H_

#include <list>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "hwplib.h"
#include "hfont.h"
#include "hstyle.h"
#include "hpara.h"

#define HWPIDLen    30
#define V20SIGNATURE    "HWP Document File V2.00 \032\1\2\3\4\5"
#define V21SIGNATURE    "HWP Document File V2.10 \032\1\2\3\4\5"
#define V30SIGNATURE    "HWP Document File V3.00 \032\1\2\3\4\5"

#define HWP_V20 20
#define HWP_V21 21
#define HWP_V30 30

int detect_hwp_version(const char *str);

struct  FBox;
struct  EmPicture;
struct  HyperText;
struct  FBoxStyle;
struct  CellLine;
struct  Cell;
struct  OlePicture;
struct  Picture;
struct  HeaderFooter;
struct  ShowPageNum;
struct  DateCode;
struct  Table;

class   HIODev;
class   HWPInfo;
class   HWPFont;
class   HWPStyle;
class   HWPPara;
class   HStream;

struct ColumnInfo{
    int start_page;
    bool bIsSet;
    ColumnDef *coldef;
    ColumnInfo(int num){
        start_page = num;
        bIsSet = false;
        coldef = 0;
    }
};

/**
 * The HWPFile class is the main class of hwp for reading file
 * information from stream
 *
 * The example is as below:
 * <pre>
 * HWPFile f;
 * f.ReadHwpFile( stream );
 * </pre>
 *
 * There are two way to read hwp information from stream, one is to read all at a time
 * to use @ref ReadhwpFile() method like above example, other is to read partial information
 * to use @ref Open(), @ref InfoRead(), @ref FontRead(), @ref StyleRead(), @ref ParaListRead(), @ref TagsRead(),
 *
 * @short HWP file management object
 * @author Mizi Reserach
 */
class DLLEXPORT HWPFile
{
    public:
/**
 * Default constructor
 */
        HWPFile();
        ~HWPFile();

    public:

/**
 * Opens HStream to use it.
 * @returns 0 if success, otherwise error code
 * @see State()
 */
        int Open( HStream & );

/**
 * Say current state
 * @returns 0 if normal, otherwise error code. If it's  bigger than USER_ERROR_BIT, it is internally using error, otherwise it's system error which is able to get the message @ref strerror() method.
 */
        int State( void ) const;
/**
 * Sets the current state
 */
        int SetState(int errcode);
/**
 * Reads one byte from HIODev
 */
        int Read1b( void );
/**
 * Reads two byte from HIODev
 */
        int Read2b( void );
/**
 * Reads four byte from HIODev
 */
        long Read4b( void );
/**
 * Reads nmemb byte array from HIODev
 */
        int Read1b( void *ptr, size_t nmemb );
/**
 * Reads nmemb short type array from HIODev
 */
        int Read2b( void *ptr, size_t nmemb );
/**
 * Reads nmemb long type array from HIODev
 */
        int Read4b( void *ptr, size_t nmemb );
/**
 * Reads some bytes from HIODev not regarding endian's way
 * @param size Amount for reading
 */
        size_t ReadBlock( void *ptr, size_t size );
/**
 * Skips some bytes from HIODev
 */
        size_t SkipBlock( size_t size );
/**
 * Reads main paragraph list
 */
        bool ReadParaList(std::list<HWPPara*> &aplist, unsigned char flag = 0);
/**
 * Sets if the stream is compressed
 */
        bool SetCompressed( bool );
/**
 * Sets current HIODev
 */
        HIODev *SetIODevice( HIODev *hiodev );

/**
 * Reads all information of hwp file from stream
 */
        int ReadHwpFile( HStream &);
/**
 * Reads document information of hwp file from HIODev
 */
        bool InfoRead(void);
/**
 * Reads font list of hwp file from HIODev
 */
        bool FontRead(void);
/**
 * Reads style list of hwp file from HIODev
 */
        bool StyleRead(void);
/**
 * Reads paragraph list of hwp file from HIODev
 */
        bool ParaListRead();
/* 그림 등의 추가 정보를 읽는다. */
/**
 * Reads additional information like embeded image of hwp file from HIODev
 */
        bool TagsRead(void);

        enum Paper
        {
            UserPaper = 0,
            Col80Paper = 1,
            Col132Paper = 2,
            A4Paper = 3,
            LetterPaper = 4,
            B5Paper = 5,
            B4Paper = 6,
            LegalPaper = 7,
            A3Paper = 8
        };

        void AddBox(FBox *);
        void AddPage(){ m_nCurrentPage++;}
        void AddColumnInfo();
        void SetColumnDef(ColumnDef *coldef);
        void AddParaShape(ParaShape *);
        void AddCharShape(CharShape *);
        void AddFBoxStyle(FBoxStyle *);
        void AddDateFormat(DateCode *);
        void AddHeaderFooter(HeaderFooter *);
        void AddPageNumber(ShowPageNum *);
        void AddTable(Table *);

        ColumnDef* GetColumnDef(int);
          int GetPageMasterNum(int page);

          int getCurrentPage(){ return m_nCurrentPage;}
        HWPInfo *GetHWPInfo(void) { return &_hwpInfo; }
        HWPFont *GetHWPFont(void) { return &_hwpFont; }
        HWPStyle *GetHWPStyle(void) { return &_hwpStyle; }
        HWPPara *GetFirstPara(void) { return plist.front(); }
        HWPPara *GetLastPara(void) { return plist.back(); }

        EmPicture *GetEmPicture(Picture *pic);
        EmPicture *GetEmPictureByName(char * name);
        HyperText *GetHyperText();
        FBox *GetBoxHead (void) { return blist.size()?blist.front():0; }
        ParaShape *getParaShape(int);
        CharShape *getCharShape(int);
        FBoxStyle *getFBoxStyle(int);
        DateCode *getDateCode(int);
          HeaderFooter *getHeaderFooter(int);
        ShowPageNum *getPageNumber(int);
          Table *getTable(int);

        int getParaShapeCount(){ return pslist.size(); }
        int getCharShapeCount(){ return cslist.size(); }
        int getFBoxStyleCount(){ return fbslist.size(); }
        int getDateFormatCount(){ return datecodes.size(); }
        int getHeaderFooterCount(){ return headerfooters.size(); }
        int getPageNumberCount(){ return pagenumbers.size(); }
        int getTableCount(){ return tables.size(); }
        int getColumnCount(){ return columnlist.size(); }

          int getMaxSettedPage(){ return m_nMaxSettedPage; }
          void setMaxSettedPage(){ m_nMaxSettedPage = m_nCurrentPage; }

    private :
        int compareCharShape(CharShape *shape);
        int compareParaShape(ParaShape *shape);

    public:
        int   version;
        bool  compressed;
        bool  encrypted;
        unsigned char linenumber;
        int   info_block_len;
        int   error_code;
        OlePicture *oledata;

    private:
/* hwp 파일 이름 */
          int           m_nCurrentPage;
          int m_nMaxSettedPage;
        HIODev    *hiodev;
// read hwp contents
        HWPInfo   _hwpInfo;
        HWPFont   _hwpFont;
        HWPStyle  _hwpStyle;
        std::list<ColumnInfo*> columnlist;
          // paragraph linked list
        std::list<HWPPara*> plist;
          // floating box linked list
        std::list<FBox*> blist;
          // embedded picture list(tag datas)
        std::list<EmPicture*> emblist;
        std::list<HyperText*> hyperlist;
        int currenthyper;
        std::list<ParaShape*> pslist;             /* 스타오피스의 구조상 필요 */
        std::list<CharShape*> cslist;
        std::list<FBoxStyle*> fbslist;
        std::list<DateCode*> datecodes;
        std::list<HeaderFooter*> headerfooters;
        std::list<ShowPageNum*> pagenumbers;
        std::list<Table*> tables;

// for global document handling
        static HWPFile *cur_doc;
        friend HWPFile *GetCurrentDoc(void);
        friend HWPFile *SetCurrentDoc(HWPFile *);
};

HWPFile *GetCurrentDoc(void);
HWPFile *SetCurrentDoc(HWPFile *hwpfp);
#endif                                            /* _HWPFILE_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
