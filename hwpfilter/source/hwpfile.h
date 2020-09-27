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


// hwpfile.h
// (C) 1998 Mizi Research, All rights are reserved


#ifndef INCLUDED_HWPFILTER_SOURCE_HWPFILE_H
#define INCLUDED_HWPFILTER_SOURCE_HWPFILE_H

#include <algorithm>
#include <memory>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sal/types.h>
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
    std::shared_ptr<ColumnDef> xColdef;
    explicit ColumnInfo(int num){
        start_page = num;
        bIsSet = false;
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
        int Open( std::unique_ptr<HStream> );

/**
 * Say current state
 * @returns 0 if normal, otherwise error code. If it's  bigger than USER_ERROR_BIT, it is internally using error, otherwise it's system error which is able to get the message @ref strerror() method.
 */
        int State( void ) const { return error_code;}
/**
 * Sets the current state
 */
        int SetState(int errcode);
/**
 * Reads one byte from HIODev
 */
        bool Read1b(unsigned char &out);
        bool Read1b(char &out);
/**
 * Reads two byte from HIODev
 */
        bool Read2b(unsigned short &out);
/**
 * Reads four byte from HIODev
 */
        bool Read4b(unsigned int &out);
        bool Read4b(int &out);
/**
 * Reads nmemb short type array from HIODev
 */
        size_t Read2b(void *ptr, size_t nmemb);
/**
 * Reads nmemb long type array from HIODev
 */
        void Read4b( void *ptr, size_t nmemb );
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
        void ReadParaList(std::vector<std::unique_ptr<HWPPara>> &aplist, unsigned char flag = 0);
        void ReadParaList(std::vector<HWPPara*> &aplist);
/**
 * Sets if the stream is compressed
 */
        void SetCompressed( bool );
/**
 * Sets current HIODev
 */
        std::unique_ptr<HIODev> SetIODevice( std::unique_ptr<HIODev> hiodev );

/**
 * Reads all information of hwp file from stream
 */
        int ReadHwpFile( std::unique_ptr<HStream> );
/**
 * Reads document information of hwp file from HIODev
 */
        void InfoRead(void);
/**
 * Reads font list of hwp file from HIODev
 */
        void FontRead(void);
/**
 * Reads style list of hwp file from HIODev
 */
        void StyleRead(void);
/**
 * Reads paragraph list of hwp file from HIODev
 */
        void ParaListRead();
/* 그림 등의 추가 정보를 읽는다. */
/**
 * Reads additional information like embedded image of hwp file from HIODev
 */
        void TagsRead();

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
        void SetColumnDef(std::shared_ptr<ColumnDef> const &);
        void AddParaShape(std::shared_ptr<ParaShape> const &);
        void AddCharShape(std::shared_ptr<CharShape> const &);
        void AddFBoxStyle(FBoxStyle *);
        void AddDateFormat(DateCode *);
        void AddHeaderFooter(HeaderFooter *);
        void AddPageNumber(ShowPageNum *);
        void AddTable(std::unique_ptr<Table>);

        ColumnDef* GetColumnDef(int);
          int GetPageMasterNum(int page);

        int getCurrentPage() const{ return m_nCurrentPage;}
        HWPInfo& GetHWPInfo(void) { return _hwpInfo; }
        HWPFont& GetHWPFont(void) { return _hwpFont; }
        HWPStyle& GetHWPStyle(void) { return _hwpStyle; }
        HWPPara *GetFirstPara(void) { return !plist.empty() ? plist.front().get() : nullptr; }

        EmPicture *GetEmPicture(Picture *pic);
        EmPicture *GetEmPictureByName(char * name);
        HyperText *GetHyperText();
        ParaShape *getParaShape(int);
        CharShape *getCharShape(int);
        FBoxStyle *getFBoxStyle(int);
        DateCode *getDateCode(int);
          HeaderFooter *getHeaderFooter(int);
        ShowPageNum *getPageNumber(int);
          Table *getTable(int);

        int getParaShapeCount() const{ return pslist.size(); }
        int getCharShapeCount() const{ return cslist.size(); }
        int getFBoxStyleCount() const{ return fbslist.size(); }
        int getDateFormatCount() const{ return datecodes.size(); }
        int getHeaderFooterCount() const{ return headerfooters.size(); }
        int getPageNumberCount() const{ return pagenumbers.size(); }
        int getTableCount() const{ return tables.size(); }
        int getColumnCount() const{ return columnlist.size(); }

          int getMaxSettedPage() const{ return m_nMaxSettedPage; }
          void setMaxSettedPage(){ m_nMaxSettedPage = m_nCurrentPage; }

        void push_hpara_type(unsigned char scflag) { element_import_stack.push_back(scflag); }
        bool already_importing_type(unsigned char scflag) const
        {
            return std::find(element_import_stack.begin(), element_import_stack.end(), scflag) != element_import_stack.end();
        }
        void pop_hpara_type() { element_import_stack.pop_back(); }

    private:
        int compareCharShape(CharShape const *shape);
        int compareParaShape(ParaShape const *shape);

    public:
        int   version;
        bool  compressed;
        bool  encrypted;
        unsigned char linenumber;
        int   info_block_len;
        int   error_code;
        std::unique_ptr<OlePicture> oledata;
        unsigned char scratch[SAL_MAX_UINT16];
        int readdepth;

    private:
/* hwp 파일 이름 */
        int m_nCurrentPage;
        int m_nMaxSettedPage;
        std::unique_ptr<HIODev> hiodev;
// read hwp contents
        HWPInfo   _hwpInfo;
        HWPFont   _hwpFont;
        HWPStyle  _hwpStyle;
        std::vector<std::unique_ptr<ColumnInfo>> columnlist;
        // paragraph list
        std::vector<std::unique_ptr<HWPPara>> plist;
        // floating box list
        std::vector<FBox*> blist;
        // embedded picture list(tag data)
        std::vector<std::unique_ptr<EmPicture>> emblist;
        std::vector<std::unique_ptr<HyperText>> hyperlist;
        int currenthyper;
        std::vector<std::shared_ptr<ParaShape>> pslist;
        std::vector<std::shared_ptr<CharShape>> cslist;
        std::vector<FBoxStyle*> fbslist;
        std::vector<DateCode*> datecodes;
        std::vector<HeaderFooter*> headerfooters;
        std::vector<ShowPageNum*> pagenumbers;
        std::vector<std::unique_ptr<Table>> tables;
        //track the stack of HParas types we're currently importing
        std::vector<unsigned char> element_import_stack;

// for global document handling
        static HWPFile *cur_doc;
        friend HWPFile *GetCurrentDoc(void);
        friend HWPFile *SetCurrentDoc(HWPFile *);
};

class DLLEXPORT DepthGuard
{
private:
    HWPFile& m_rFile;
public:
    DepthGuard(HWPFile &rFile)
        : m_rFile(rFile)
    {
        ++m_rFile.readdepth;
    }
    bool toodeep() const
    {
        return m_rFile.readdepth == 512;
    }
    ~DepthGuard()
    {
        --m_rFile.readdepth;
    }
};

HWPFile *GetCurrentDoc(void);
HWPFile *SetCurrentDoc(HWPFile *hwpfp);
#endif // INCLUDED_HWPFILTER_SOURCE_HWPFILE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
