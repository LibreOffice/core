/*************************************************************************
 *
 *  $RCSfile: hwpfile.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:37:09 $
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

//
// hwpfile.h
// (C) 1998 Mizi Research, All rights are reserved
//
// $Id: hwpfile.h,v 1.1 2003-10-15 14:37:09 dvo Exp $
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
 * @ref Flush() and @ref Close().
 *
 * @short HWP file management object
 * @author Mizi Reserach
 * @version $Id: hwpfile.h,v 1.1 2003-10-15 14:37:09 dvo Exp $
 */
class DLLEXPORT HWPFile
{
    public:
/**
 * Default constructor
 */
        HWPFile( void );
/**
 * Create HWPFile object and call @ref Open() method
 */
        HWPFile( HStream& stream );

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
 * Flushs the stream of HIODev.
 */
        void Flush( void );

/**
 * Closes the stream of HIODev.
 */
        void Close( void );

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
/// 그림 등의 추가 정보를 읽는다.
/**
 * Reads additional information like embeded image of hwp file from HIODev
 */
        bool TagsRead(void);

/**
 * Get the name of current file
 */
        const char *GetFileName(void) const;

/**
 * @returns True if compressed, otherwise false
 */
        bool IsCompressedFile() const;

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

        enum Paper GetPaperKind() const;
        void GetPaperSize(hunit& Width, hunit& Height) const;
        bool GetPaperOrientation() const;

        ColumnDef* GetColumnDef(int);
          int GetPageMasterNum(int page);

          int getCurrentPage(){ return m_nCurrentPage;}
        int GetFontCount(int lang);
        const char *GetFontName(int lang, int id);
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
// hwp 파일 이름
        char      fname[256];
          int           m_nCurrentPage;
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
        LinkedList<ParaShape> pslist;             // 스타오피스의 구조상 필요
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
