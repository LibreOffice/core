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

//
// hwpfile.h
// (C) 1998 Mizi Research, All rights are reserved
//
// $Id: hwpfile.h,v 1.5 2008-06-04 10:01:33 vg Exp $
//

#ifndef _HWPFILE_H_
#define _HWPFILE_H_

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "hwplib.h"
#include "hfont.h"
#include "hstyle.h"
#include "hpara.h"
#include "list.hxx"

#define HWP_V20 20
#define HWP_V21 21
#define HWP_V30 30

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
 * @version $Id: hwpfile.h,v 1.5 2008-06-04 10:01:33 vg Exp $
 */
class DLLEXPORT HWPFile
{
    public:
/**
 * Default constructor
 */
        HWPFile( void );

        ~HWPFile( void );

    public:
/**
 * Initialize this object
 */
        void Init();

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
        bool ReadParaList(LinkedList<HWPPara> &plist, unsigned char flag = 0);
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
        HWPPara *GetFirstPara(void) { return plist.first(); }
        HWPPara *GetLastPara(void) { return plist.last(); }

        EmPicture *GetEmPicture(Picture *pic);
        EmPicture *GetEmPictureByName(char * name);
        HyperText *GetHyperText();
        FBox *GetBoxHead (void) { return blist.count()?blist.first():0; }
        ParaShape *getParaShape(int);
        CharShape *getCharShape(int);
        FBoxStyle *getFBoxStyle(int);
        DateCode *getDateCode(int);
          HeaderFooter *getHeaderFooter(int);
        ShowPageNum *getPageNumber(int);
          Table *getTable(int);

        int getParaShapeCount(){ return pslist.count(); }
        int getCharShapeCount(){ return cslist.count(); }
        int getFBoxStyleCount(){ return fbslist.count(); }
        int getDateFormatCount(){ return datecodes.count(); }
        int getHeaderFooterCount(){ return headerfooters.count(); }
        int getPageNumberCount(){ return pagenumbers.count(); }
        int getTableCount(){ return tables.count(); }
        int getColumnCount(){ return columnlist.count(); }

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
        char      fname[256];
          int			m_nCurrentPage;
          int m_nMaxSettedPage;
        HIODev    *hiodev;
// read hwp contents
        HWPInfo   _hwpInfo;
        HWPFont   _hwpFont;
        HWPStyle  _hwpStyle;
        LinkedList<ColumnInfo> columnlist;
          // paragraph linked list
        LinkedList<HWPPara> plist;
          // floating box linked list
        LinkedList<FBox> blist;
          // embedded picture list(tag datas)
        LinkedList<EmPicture> emblist;
        LinkedList<HyperText> hyperlist;
        int currenthyper;
        LinkedList<ParaShape> pslist;             /* 스타오피스의 구조상 필요 */
        LinkedList<CharShape> cslist;
        LinkedList<FBoxStyle> fbslist;
        LinkedList<DateCode> datecodes;
        LinkedList<HeaderFooter> headerfooters;
        LinkedList<ShowPageNum> pagenumbers;
        LinkedList<Table> tables;

// for global document handling
        static HWPFile *cur_doc;
        friend HWPFile *GetCurrentDoc(void);
        friend HWPFile *SetCurrentDoc(HWPFile *);
};

HWPFile *GetCurrentDoc(void);
HWPFile *SetCurrentDoc(HWPFile *hwpfp);
#endif                                            /* _HWPFILE_H_ */
