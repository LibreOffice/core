/*************************************************************************
 *
 *  $RCSfile: ww8struc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-24 14:01:34 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _WW8STRUC_HXX
#define _WW8STRUC_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif


// max. Anzahl der Listen-Level in WW8: 1..9
#define nWW8MaxListLevel 9


inline BYTE Get_Byte( BYTE *& p )
    { BYTE n = SVBT8ToByte( *(SVBT8*)p );       p += 1; return n; }

inline short Get_Short( BYTE *& p )
    { short n = SVBT16ToShort( *(SVBT16*)p );   p += 2; return n; }
inline USHORT Get_UShort( BYTE *& p )
    { USHORT n = SVBT16ToShort( *(SVBT16*)p );  p += 2; return n; }

inline long Get_Long( BYTE *& p )
    { long n = SVBT32ToLong( *(SVBT32*)p );     p += 4; return n; }
inline ULONG Get_ULong( BYTE *& p )
    { ULONG n = SVBT32ToLong( *(SVBT32*)p );        p += 4; return n; }

inline void Set_UInt8( BYTE *& p, UINT8 n )
    { ByteToSVBT8( n, *(SVBT8*)p );  p+= 1; }

inline void Set_UInt16( BYTE *& p, UINT16 n )
    { ShortToSVBT16( n, *(SVBT16*)p );  p+= 2; }

inline void Set_UInt32( BYTE *& p, UINT32 n )
    { LongToSVBT32( n, *(SVBT32*)p );  p+= 4; }


#if defined  __BIGENDIAN || __ALIGNMENT4 > 2 || defined UNX
#define __WW8_NEEDS_COPY
#else
#if defined WNT || defined WIN || defined OS2
#define __WW8_NEEDS_PACK
#pragma pack(2)
#endif
#endif

typedef INT16 WW8_PN;
typedef INT32 WW8_FC;
typedef INT32 WW8_CP;

// STD: STyle Definition
//   The STD contains the entire definition of a style.
//   It has two parts, a fixed-length base (cbSTDBase bytes long)
//   and a variable length remainder holding the name, and the upx and upe
//   arrays (a upx and upe for each type stored in the style, std.cupx)
//   Note that new fields can be added to the BASE of the STD without
//   invalidating the file format, because the STSHI contains the length
//   that is stored in the file.  When reading STDs from an older version,
//   new fields will be zero.
struct WW8_STD
{
    // Base part of STD:
    UINT16  sti : 12;          // invariant style identifier
    UINT16  fScratch : 1;      // spare field for any temporary use,
                                                         // always reset back to zero!
    UINT16  fInvalHeight : 1;  // PHEs of all text with this style are wrong
    UINT16  fHasUpe : 1;       // UPEs have been generated
    UINT16  fMassCopy : 1;     // std has been mass-copied; if unused at
                                                         // save time, style should be deleted
    UINT16  sgc : 4;           // style type code
    UINT16  istdBase : 12;     // base style
    UINT16  cupx : 4;          // # of UPXs (and UPEs)
    UINT16  istdNext : 12;     // next style
    UINT16  bchUpe;            // offset to end of upx's, start of upe's
    //-------- jetzt neu:
    // ab Ver8 gibts zwei Felder mehr:
  UINT16    fAutoRedef : 1;    /* auto redefine style when appropriate */
  UINT16    fHidden : 1;       /* hidden from UI? */
  UINT16    : 14;              /* unused bits */

    // Variable length part of STD:
    //  UINT8   stzName[2];        /* sub-names are separated by chDelimStyle
    // char grupx[];
            // the UPEs are not stored on the file; they are a cache of the based-on
        // chain
    // char grupe[];
};

/*
    Basis zum Einlesen UND zum Arbeiten (wird jeweils unter
    schiedlich beerbt)
*/
struct WW8_FFN_BASE     // Font Descriptor
{
    // ab Ver6
    BYTE    cbFfnM1;        //  0x0     total length of FFN - 1.

    BYTE    prg: 2;         //  0x1:03  pitch request
    BYTE    fTrueType : 1;  //  0x1:04  when 1, font is a TrueType font
                            //  0x1:08  reserved
    BYTE    ff : 3;         //  0x1:70  font family id
                            //  0x1:80  reserved

    short wWeight;          //  0x2     base weight of font
    BYTE    chs;            //  0x4     character set identifier
    BYTE    ibszAlt;        //  0x5     index into ffn.szFfn to the name of the alternate font
};

/*
    Hiermit arbeiten wir im Parser (und Dumper)
*/
struct WW8_FFN : public WW8_FFN_BASE
{
    // ab Ver8 als Unicode
    UniString sFontname;// 0x6 bzw. 0x40 ab Ver8 zero terminated string that
                                        // records name of font.
                                        // Maximal size of szFfn is 65 characters.
                                        // Vorsicht: Dieses Array kann auch kleiner sein!!!
                                        // Possibly followed by a second sz which records the
                                        // name of an alternate font to use if the first named
                                        // font does not exist on this system.
};



struct WW8_BRCVer6  // alter Border Code
{
    SVBT16 aBits1;
//  UINT16 dxpLineWidth : 3;// 0007 When dxpLineWidth is 0, 1, 2, 3, 4, or 5, this field is the width of
                            //      a single line of border in units of 0.75 points
                            //      Must be nonzero when brcType is nonzero.
                            //      6 == dotted, 7 == dashed.
//  UINT16 brcType : 2;     // 0018 border type code: 0 == none, 1 == single, 2 == thick, 3 == double
//  UINT16 fShadow : 1;     // 0020 when 1, border is drawn with shadow. Must be 0 when BRC is a substructure of the TC
//  UINT16 ico : 5;         // 07C0 color code (see chp.ico)
//  UINT16 dxpSpace : 5;    // F800 width of space to maintain between border and text within border.
                            //      Must be 0 when BRC is a substructure of the TC.  Stored in points for Windows.
};

struct WW8_BRC : public WW8_BRCVer6 // Border Code
{
    SVBT16 aBits2;
//  UINT16 dxpLineWidth : 3;// 0007 When dxpLineWidth is 0, 1, 2, 3, 4, or 5, this field is the width of
                            //      a single line of border in units of 0.75 points
                            //      Must be nonzero when brcType is nonzero.
                            //      6 == dotted, 7 == dashed.
//  UINT16 brcType : 2;     // 0018 border type code: 0 == none, 1 == single, 2 == thick, 3 == double
//  UINT16 fShadow : 1;     // 0020 when 1, border is drawn with shadow. Must be 0 when BRC is a substructure of the TC
//  UINT16 ico : 5;         // 07C0 color code (see chp.ico)
//  UINT16 dxpSpace : 5;    // F800 width of space to maintain between border and text within border.
                            //      Must be 0 when BRC is a substructure of the TC.  Stored in points for Windows.
};

typedef WW8_BRC WW8_BRC5[5];        // 5 * Border Code

#define WW8_TOP 0
#define WW8_LEFT 1
#define WW8_BOT 2
#define WW8_RIGHT 3
#define WW8_BETW 4





struct WW8_BordersSO            // fuer StarOffice-Border Code
{
    USHORT Out;
    USHORT In;
    USHORT Dist;
};


/*
// Linien-Defaults in Twips: fruehere Writer-Defaults,
//                           siehe auch <svx/boxitem.hxx>
#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        20
#define DEF_LINE_WIDTH_2        50
#define DEF_LINE_WIDTH_3        80
#define DEF_LINE_WIDTH_4        100

#define DEF_MAX_LINE_WIDHT      DEF_LINE_WIDTH_4
#define DEF_MAX_LINE_DIST       DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE0_OUT    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE0_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE0_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE1_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE1_IN     DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE1_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE2_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE2_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE2_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE3_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE3_IN     DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE3_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE4_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE4_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE4_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE5_OUT    DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE5_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE5_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE6_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE6_IN     DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE6_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE7_OUT    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE7_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE7_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE8_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE8_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE8_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE9_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE9_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE9_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE10_OUT   DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE10_IN    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE10_DIST  DEF_LINE_WIDTH_2
*/
// Deklarationen gemaess BOXITEM.HXX
#define WW8_DECL_LINETAB_ARRAY                                               \
    static WW8_BordersSO __READONLY_DATA nLineTabVer8[] =                    \
    {                                                                        \
/* 0*/  { DEF_LINE_WIDTH_0, 0, 0 },                                          \
/* 1*/  { DEF_LINE_WIDTH_1, 0, 0 },                                          \
/* 2*/  { DEF_LINE_WIDTH_2, 0, 0 },                                          \
/* 3*/  { DEF_LINE_WIDTH_3, 0, 0 },                                          \
/* 4*/  { DEF_LINE_WIDTH_4, 0, 0 },                                          \
/* 5*/  { DEF_DOUBLE_LINE0_OUT, DEF_DOUBLE_LINE0_IN, DEF_DOUBLE_LINE0_DIST },\
/* 6*/  { DEF_DOUBLE_LINE1_OUT, DEF_DOUBLE_LINE1_IN, DEF_DOUBLE_LINE1_DIST },\
/* 7*/  { DEF_DOUBLE_LINE2_OUT, DEF_DOUBLE_LINE2_IN, DEF_DOUBLE_LINE2_DIST },\
/* 8*/  { DEF_DOUBLE_LINE3_OUT, DEF_DOUBLE_LINE3_IN, DEF_DOUBLE_LINE3_DIST },\
/* 9*/  { DEF_DOUBLE_LINE4_OUT, DEF_DOUBLE_LINE4_IN, DEF_DOUBLE_LINE4_DIST },\
/*10*/  { DEF_DOUBLE_LINE5_OUT, DEF_DOUBLE_LINE5_IN, DEF_DOUBLE_LINE5_DIST },\
/*11*/  { DEF_DOUBLE_LINE6_OUT, DEF_DOUBLE_LINE6_IN, DEF_DOUBLE_LINE6_DIST },\
/*12*/  { DEF_DOUBLE_LINE7_OUT, DEF_DOUBLE_LINE7_IN, DEF_DOUBLE_LINE7_DIST },\
/*13*/  { DEF_DOUBLE_LINE8_OUT, DEF_DOUBLE_LINE8_IN, DEF_DOUBLE_LINE8_DIST },\
/*14*/  { DEF_DOUBLE_LINE9_OUT, DEF_DOUBLE_LINE9_IN, DEF_DOUBLE_LINE9_DIST },\
/*15*/  { DEF_DOUBLE_LINE10_OUT,DEF_DOUBLE_LINE10_IN,DEF_DOUBLE_LINE10_DIST} \
    };

#define WW8_DECL_LINETAB_OFS_DOUBLE 5   // Beginn des DOUBLE_LINE Abschnitts in meiner Liste




struct WW8_XCHAR    // Hilfs-Konstrukt fuer WW8_DOPTYPOGRAPHY
{
    sal_Char A;
    sal_Char B;
};

struct WW8_DOPTYPOGRAPHY
{   /*
        Document Typography Info (DOPTYPOGRAPHY)
        These options are Far East only, and are accessible
        through the Typography tab of the Tools/Options dialog.
    */



    /* a c h t u n g :     es duerfen keine solchen Bitfelder ueber einen eingelesenes Byte-Array
                            gelegt werden!!
                            stattdessen ist ein aBits1 darueber zu legen, das mit & auszulesen ist
    GRUND: Compiler auf Intel und Sparc sortieren die Bits unterschiedlich
    */



    short fKerningPunct  : 1;       // true if we're kerning punctuation
    short iJustification : 2;       // Kinsoku method of justification:
                                                                //  0 = always expand
                                                                //  1 = compress punctuation
                                                                //  2 = compress punctuation and kana.
    short iLevelOfKinsoku: 2;       // Level of Kinsoku:
                                                                //  0 = Level 1
                                                                //  1 = Level 2
                                                                //  2 = Custom
    short f2on1          : 1;       // 2-page-on-1 feature is turned on.
    short                :10;       // reserved
    short cchFollowingPunct;        // length of rgxchFPunct
    short cchLeadingPunct;          // length of rgxchLPunct

    WW8_XCHAR rgxchFPunct[101]; // array of characters that should
                                                            // never appear at the start of a line
    WW8_XCHAR rgxchLPunct[51];  // array of characters that should
                                                            // never appear at the end of a line
};

struct WW8_DOGRID
{
    short xaGrid;       // x-coord of the upper left-hand corner of the grid
    short yaGrid;       // y-coord of the upper left-hand corner of the grid
    short dxaGrid;  // width of each grid square
    short dyaGrid;  // height of each grid square



    /* a c h t u n g :     es duerfen keine solchen Bitfelder ueber einen eingelesenes Byte-Array
                            gelegt werden!!
                            stattdessen ist ein aBits1 darueber zu legen, das mit & auszulesen ist
    GRUND: Compiler auf Intel und Sparc sortieren die Bits unterschiedlich
    */



    short dyGridDisplay:7;  // the number of grid squares (in the y direction)
                                                    // between each gridline drawn on the screen. 0 means
                                                    // don't display any gridlines in the y direction.
    short fTurnItOff   :1;  // suppress display of gridlines
    short dxGridDisplay:7;  // the number of grid squares (in the x direction)
                                                    // between each gridline drawn on the screen. 0 means
                                                    // don't display any gridlines in the y direction.
    short fFollowMargins:1; // if true, the grid will start at the left and top
                                                    // margins and ignore xaGrid and yaGrid.
};

struct WW8_PIC
{
    INT32 lcb;          // 0x0 number of bytes in the PIC structure plus size of following picture data which may be a Window's metafile, a bitmap, or the filename of a TIFF file.
    UINT16 cbHeader;    // 0x4 number of bytes in the PIC (to allow for future expansion).
    struct {
        INT16 mm;       // 0x6  int
        INT16 xExt;     // 0x8  int
        INT16 yExt;     // 0xa  int
        INT16 hMF;      // 0xc  int
    }MFP;
//  BYTE bm[14];        // 0xe  BITMAP(14 bytes)    Window's bitmap structure when PIC describes a BITMAP.
    BYTE rcWinMF[14];   // 0xe  rc (rectangle - 8 bytes) rect for window origin
                        //      and extents when  metafile is stored -- ignored if 0
    INT16 dxaGoal;      // 0x1c horizontal  measurement in twips of the  rectangle the picture should be imaged within.
    INT16 dyaGoal;      // 0x1e vertical  measurement in twips of the  rectangle the picture should be imaged within.
    UINT16 mx;          // 0x20 horizontal scaling factor supplied by user in .1% units.
    UINT16 my;          // 0x22 vertical scaling factor supplied by user in .1% units.
    INT16 dxaCropLeft;  // 0x24 the amount the picture has been cropped on the left in twips.
    INT16 dyaCropTop;   // 0x26 the amount the picture has been cropped on the top in twips.
    INT16 dxaCropRight; // 0x28 the amount the picture has been cropped on the right in twips.
    INT16 dyaCropBottom;// 0x2a the amount the picture has been cropped on the bottom in twips.
    INT16 brcl : 4;     // 000F Obsolete, superseded by brcTop, etc.  In
    INT16 fFrameEmpty : 1;  // 0010 picture consists of a single frame
    INT16 fBitmap : 1;      // 0020 ==1, when picture is just a bitmap
    INT16 fDrawHatch : 1;   // 0040 ==1, when picture is an active OLE object
    INT16 fError : 1;       // 0080 ==1, when picture is just an error message
    INT16 bpp : 8;      // FF00 bits per pixel, 0 = unknown
    WW8_BRC rgbrc[4];
//  BRC brcTop;         // 0x2e specification for border above picture
//  BRC brcLeft;        // 0x30 specification for border to the left
//  BRC brcBottom;      // 0x32 specification for border below picture
//  BRC brcRight;       // 0x34 specification for border to the right
    INT16 dxaOrigin;    // 0x36 horizontal offset of hand annotation origin
    INT16 dyaOrigin;    // 0x38 vertical offset of hand annotation origin
//  BYTE rgb[];         // 0x3a variable array of bytes containing Window's metafile, bitmap or TIFF file filename.
};

struct WW8_PIC_SHADOW
{
    SVBT32 lcb;         // 0x0 number of bytes in the PIC structure plus size of following picture data which may be a Window's metafile, a bitmap, or the filename of a TIFF file.
    SVBT16 cbHeader;    // 0x4 number of bytes in the PIC (to allow for future expansion).
    struct {
        SVBT16 mm;      // 0x6  int
        SVBT16 xExt;        // 0x8  int
        SVBT16 yExt;        // 0xa  int
        SVBT16 hMF;     // 0xc  int
    }MFP;
//  SVBT8 bm[14];       // 0xe  BITMAP(14 bytes)    Window's bitmap structure when PIC describes a BITMAP.
    SVBT8 rcWinMF[14];  // 0xe  rc (rectangle - 8 bytes) rect for window origin
                        //      and extents when  metafile is stored -- ignored if 0
    SVBT16 dxaGoal;     // 0x1c horizontal  measurement in twips of the  rectangle the picture should be imaged within.
    SVBT16 dyaGoal;     // 0x1e vertical  measurement in twips of the  rectangle the picture should be imaged within.
    SVBT16 mx;          // 0x20 horizontal scaling factor supplied by user in .1% units.
    SVBT16 my;          // 0x22 vertical scaling factor supplied by user in .1% units.
    SVBT16 dxaCropLeft; // 0x24 the amount the picture has been cropped on the left in twips.
    SVBT16 dyaCropTop;  // 0x26 the amount the picture has been cropped on the top in twips.
    SVBT16 dxaCropRight;    // 0x28 the amount the picture has been cropped on the right in twips.
    SVBT16 dyaCropBottom;// 0x2a    the amount the picture has been cropped on the bottom in twips.
    SVBT8 aBits1; //0x2c
    SVBT8 aBits2;
    WW8_BRC rgbrc[4];
//  BRC brcTop;         // 0x2e specification for border above picture
//  BRC brcLeft;        // 0x30 specification for border to the left
//  BRC brcBottom;      // 0x32 specification for border below picture
//  BRC brcRight;       // 0x34 specification for border to the right
    SVBT16 dxaOrigin;   // 0x36 horizontal offset of hand annotation origin
    SVBT16 dyaOrigin;   // 0x38 vertical offset of hand annotation origin
//  SVBT8 rgb[];            // 0x3a variable array of bytes containing Window's metafile, bitmap or TIFF file filename.
};


struct WW8_TBD
{
    SVBT8 aBits1;
//  BYTE jc : 3;        // 0x07 justification code: 0=left tab, 1=centered tab, 2=right tab, 3=decimal tab, 4=bar
//  BYTE tlc : 3;       // 0x38 tab leader code: 0=no leader, 1=dotted leader,
                        // 2=hyphenated leader, 3=single line leader, 4=heavy line leader
//  *   int :2  C0  reserved
};

struct WW8_TCell    // hiermit wird weitergearbeitet (entspricht weitestgehend dem Ver8-Format)
{
    BOOL bFirstMerged   : 1;// 0001 set to 1 when cell is first cell of a range of cells that have been merged.
    BOOL bMerged        : 1;// 0002 set to 1 when cell has been merged with preceding cell.
    BOOL bVertical      : 1;// set to 1 when cell has vertical text flow
    BOOL bBackward      : 1;// for a vertical table cell, text flow is bottom to top when 1 and is bottom to top when 0.
    BOOL bRotateFont    : 1;// set to 1 when cell has rotated characters (i.e. uses @font)
    BOOL bVertMerge     : 1;// set to 1 when cell is vertically merged with the cell(s) above and/or below. When cells are vertically merged, the display area of the merged cells are consolidated. The consolidated area is used to display the contents of the first vertically merged cell (the cell with fVertRestart set to 1), and all other vertically merged cells (those with fVertRestart set to 0) must be empty. Cells can only be merged vertically if their left and right boundaries are (nearly) identical (i.e. if corresponding entries in rgdxaCenter of the table rows differ by at most 3).
    BOOL bVertRestart   : 1;// set to 1 when the cell is the first of a set of vertically merged cells. The contents of a cell with fVertStart set to 1 are displayed in the consolidated area belonging to the entire set of vertically merged cells. Vertically merged cells with fVertRestart set to 0 must be empty.
    BYTE nVertAlign     : 2;// specifies the alignment of the cell contents relative to text flow (e.g. in a cell with bottom to top text flow and bottom vertical alignment, the text is shifted horizontally to match the cell's right boundary):
                                                    //          0 top
                                                    //          1 center
                                                    //          2 bottom
    UINT16 fUnused      : 7;// reserved - nicht loeschen: macht das UINT16 voll !!

    WW8_BRC rgbrc[4];               // border codes
//notational convenience for referring to brcTop, brcLeft, etc fields.
//  BRC brcTop;             // specification of the top border of a table cell
//  BRC brcLeft;            // specification of left border of table row
//  BRC brcBottom;          // specification of bottom border of table row
//  BRC brcRight;           // specification of right border of table row.
};
// cbTC (count of bytes of a TC) is 18(decimal), 12(hex).


struct WW8_TCellVer6    // wird aus der Datei gelesen
{
    SVBT8  aBits1Ver6;
    SVBT8  aBits2Ver6;
//  UINT16 fFirstMerged : 1;// 0001 set to 1 when cell is first cell of a range of cells that have been merged.
//  UINT16 fMerged : 1;     // 0002 set to 1 when cell has been merged with preceding cell.
//  UINT16 fUnused : 14;    // FFFC reserved
    WW8_BRCVer6 rgbrcVer6[4];
// notational convenience for referring to brcTop, brcLeft, etc fields:
//          BRC brcTop;             // specification of the top border of a table cell
//          BRC brcLeft;            // specification of left border of table row
//          BRC brcBottom;          // specification of bottom border of table row
//          BRC brcRight;           // specification of right border of table row.
};
// cbTC (count of bytes of a TC) is 10(decimal), A(hex).

struct WW8_TCellVer8    // wird aus der Datei gelesen
{
    SVBT16 aBits1Ver8;      // Dokumentation siehe oben unter WW8_TCell
    SVBT16 aUnused;         // reserve
    WW8_BRC rgbrcVer8[4];   // Dokumentation siehe oben unter WW8_TCell
};
// cbTC (count of bytes of a TC) is 20(decimal), 14(hex).


struct WW8_SHD              // struct SHD fehlt in der Beschreibung
{
private:
    UINT16 aBits;
//  UINT16 nFore : 5;       // 0x001f ForegroundColor
//  UINT16 nBack : 5;       // 0x03e0 BackgroundColor
//  UINT16 nStyle : 5;      // 0x7c00 Percentage and Style
//  UINT16 nDontKnow : 1;   // 0x8000 ???   ab Ver8: ebenfalls fuer Style

public:
    WW8_SHD(){ aBits = 0; }

    BYTE GetFore()  const                           { return (BYTE)( aBits        & 0x1f); }
    BYTE GetBack()  const                           { return (BYTE)((aBits >> 5 ) & 0x1f); }
    BYTE GetStyle(BOOL bVer67)  const { return (BYTE)((aBits >> 10) & ( bVer67?0x1f:0x3f ) ); }

    UINT16 GetValue()   const                       { return aBits; }

    void SetValue(   UINT16 nVal ){ aBits = nVal; }
    void SetWWValue( SVBT16 nVal ){ aBits = (UINT16)SVBT16ToShort( nVal ); }

    void SetFore( BYTE nVal ){ aBits = (aBits & 0xffe0) |  (nVal & 0x1f);     }
    void SetBack( BYTE nVal ){ aBits = (aBits & 0xfc1f) | ((nVal & 0x1f)<<5); }
    void SetStyle( BOOL bVer67, BYTE nVal ){
                                         aBits = (aBits & ( bVer67?0x83ff:0x03ff ) )
                                                                                      | ((nVal & ( bVer67?0x1f:0x2f ))<<10); }
};


struct WW8_ANLV
{
    SVBT8 nfc;          // 0        number format code, 0=Arabic, 1=Upper case Roman, 2=Lower case Roman
                        //          3=Upper case Letter, 4=Lower case letter, 5=Ordinal
    SVBT8 cbTextBefore; // 1        offset into anld.rgch limit of prefix text
    SVBT8 cbTextAfter;  // 2
    SVBT8 aBits1;
//  BYTE jc : 2;        // 3 : 0x03 justification code, 0=left, 1=center, 2=right, 3=left and right justify
//  BYTE fPrev : 1;     //     0x04 when ==1, include previous levels
//  BYTE fHang : 1;     //     0x08 when ==1, number will be displayed using a hanging indent
//  BYTE fSetBold : 1;  //     0x10 when ==1, boldness of number will be determined by anld.fBold.
//  BYTE fSetItalic : 1;//     0x20 when ==1, italicness of number will be determined by anld.fItalic
//  BYTE fSetSmallCaps : 1;//  0x40 when ==1, anld.fSmallCaps will determine whether number will be displayed in small caps or not.
//  BYTE fSetCaps : 1;  //     0x80 when ==1, anld.fCaps will determine whether number will be displayed capitalized or not
    SVBT8 aBits2;
//  BYTE fSetStrike : 1;// 4 : 0x01 when ==1, anld.fStrike will determine whether the number will be displayed using strikethrough or not.
//  BYTE fSetKul : 1;   //     0x02 when ==1, anld.kul will determine the underlining state of the autonumber.
//  BYTE fPrevSpace : 1;//     0x04 when ==1, autonumber will be displayed with a single prefixing space character
//  BYTE fBold : 1;     //     0x08 determines boldness of autonumber when anld.fSetBold == 1.
//  BYTE fItalic : 1;   //     0x10 determines italicness of autonumber when anld.fSetItalic == 1.
//  BYTE fSmallCaps : 1;//     0x20 determines whether autonumber will be displayed using small caps when anld.fSetSmallCaps == 1.
//  BYTE fCaps : 1;     //     0x40 determines whether autonumber will be displayed using caps when anld.fSetCaps == 1.
//  BYTE fStrike : 1;   //     0x80 determines whether autonumber will be displayed using caps when anld.fSetStrike == 1.
    SVBT8 aBits3;
//  BYTE kul : 3;       // 5 : 0x07 determines whether  autonumber will be displayed with underlining when anld.fSetKul == 1.
//  BYTE ico : 5;       //     0xF1 color of autonumber
    SVBT16 ftc;         // 6        font code of  autonumber
    SVBT16 hps;         // 8        font half point size (or 0=auto)
    SVBT16 iStartAt;    // 0x0a     starting value (0 to 65535)
    SVBT16 dxaIndent;   // 0x0c     *short?* *USHORT?* width of prefix text (same as indent)
    SVBT16 dxaSpace;    // 0x0e     minimum space between number and paragraph
};
// *cbANLV (count of bytes of ANLV) is 16 (decimal), 10(hex).

struct WW8_ANLD
{
    WW8_ANLV eAnlv;     // 0
    SVBT8 fNumber1;     // 0x10     number only 1 item per table cell
    SVBT8 fNumberAcross;    // 0x11     number across cells in table rows(instead of down)
    SVBT8 fRestartHdn;  // 0x12     restart heading number on section boundary
    SVBT8 fSpareX;      // 0x13     unused( should be 0)
    BYTE  rgchAnld[32]; // 0x14 characters displayed before/after autonumber
};


struct WW8_OLST
{
    WW8_ANLV rganlv[9]; // 0    an array of 9 ANLV structures (heading levels)
    SVBT8 fRestartHdr;  // 0x90 when ==1, restart heading on section break
    SVBT8 fSpareOlst2;  // 0x91 reserved
    SVBT8 fSpareOlst3;  // 0x92 reserved
    SVBT8 fSpareOlst4;  // 0x93 reserved
    BYTE rgch[64];      // 0x94 array of 64 chars       text before/after number
};
// cbOLST is 212(decimal), D4(hex).

struct WW8_FDOA
{
    SVBT32 fc;          // 0  FC pointing to drawing object data
    SVBT16 ctxbx;       // 4  count of textboxes in the drawing object
};

struct WW8_DO
{
    SVBT16 dok;             // 0    Drawn Object Kind, currently this is always 0
    SVBT16 cb;              // 2    size (count of bytes) of the entire DO
    SVBT8  bx;              // 4    x position relative to anchor CP
    SVBT8  by;              // 5    y position relative to anchor CP
    SVBT16 dhgt;                // 6    height of DO
    SVBT16 aBits1;
//  UINT16 fAnchorLock : 1; // 8    1 if the DO anchor is locked
//  BYTE[] rgdp;            // 0xa  variable length array of drawing primitives
};

struct WW8_DPHEAD
{
    SVBT16 dpk;         //  0   Drawn Primitive Kind  REVIEW davebu
  //        0=start of grouping, 1=line, 2=textbox, 3=rectangle,
  //        4=arc, 5=elipse, 6=polyline, 7=callout textbox,
  //        8=end of grouping, 9=sample primitve holding default values
    SVBT16 cb;          // 2    size (count of bytes) of this DP
    SVBT16 xa;          // 4    These 2 points describe the rectangle
    SVBT16 ya;          // 6    enclosing this DP relative to the origin of
    SVBT16 dxa;         // 8    the DO
    SVBT16 dya;         // 0xa
};


struct WW8_DP_LINETYPE
{
    SVBT32 lnpc;            // LiNe Property Color -- RGB color value
    SVBT16 lnpw;            // line property weight in twips
    SVBT16 lnps;            // line property style : 0=Solid, 1=Dashed
                            // 2=Dotted, 3=Dash Dot, 4=Dash Dot Dot, 5=Hollow
};

struct WW8_DP_SHADOW    // Schattierung!
{
    SVBT16 shdwpi;          // Shadow Property Intensity
    SVBT16 xaOffset;        // x offset of shadow
    SVBT16 yaOffset;        // y offset of shadow
};

struct WW8_DP_FILL
{
    SVBT32 dlpcFg;          // FiLl Property Color ForeGround -- RGB color value
    SVBT32 dlpcBg;          // Property Color BackGround -- RGB color value
    SVBT16 flpp;            // FiLl Property Pattern REVIEW davebu
};

struct WW8_DP_LINEEND
{
    SVBT16 aStartBits;
//  UINT16 eppsStart : 2;   // Start EndPoint Property Style
                            // 0=None, 1=Hollow, 2=Filled
//  UINT16 eppwStart : 2;   // Start EndPoint Property Weight
//  UINT16 epplStart : 2;   // Start EndPoint Property length
//  UINT16 dummyStart : 10; // Alignment
    SVBT16 aEndBits;
//  UINT16 eppsEnd : 2;     // End EndPoint Property Style
//  UINT16 eppwEnd : 2;     // End EndPoint Property Weight
//  UINT16 epplEnd : 2;     // End EndPoint Property length
//  UINT16 dummyEnd : 10;   // Alignment
};

struct WW8_DP_LINE
{
//  WW8_DPHEAD dphead;      // 0    Common header for a drawing primitive
    SVBT16 xaStart;         // starting point for line
    SVBT16 yaStart;         //
    SVBT16 xaEnd;           // ending point for line
    SVBT16 yaEnd;
    WW8_DP_LINETYPE aLnt;
    WW8_DP_LINEEND aEpp;
    WW8_DP_SHADOW aShd;
};

struct WW8_DP_TXTBOX
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_SHADOW aShd;
    SVBT16 aBits1;
//  UINT16 fRoundCorners : 1; //0x24    0001    1 if the textbox has rounded corners
//  UINT16 zaShape : 15;    // 0x24     000e    REVIEW davebu
    SVBT16 dzaInternalMargin; // 0x26   REVIEW davebu
};

struct WW8_DP_RECT
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_SHADOW aShd;
    SVBT16 aBits1;
//  UINT16 fRoundCorners : 1; // 0x24   0001    1 if the textbox has rounded corners
//  UINT16 zaShape : 15; // 0x24 000e   REVIEW davebu
};

struct WW8_DP_ARC
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_SHADOW aShd;
    SVBT8 fLeft;        // 0x24 00ff    REVIEW davebu
    SVBT8 fUp;          // 0x24 ff00    REVIEW davebu
//  UINT16 fLeft : 8;   // 0x24 00ff    REVIEW davebu
//  UINT16 fUp : 8;     // 0x24 ff00    REVIEW davebu
};

struct WW8_DP_ELIPSE
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_SHADOW aShd;
};

struct WW8_DP_POLYLINE
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_LINEEND aEpp;
    WW8_DP_SHADOW aShd;
    SVBT16 aBits1;
//  UINT16 fPolygon : 1; // 0x28  0001  1 if this is a polygon
//  UINT16 cpt : 15;    // 0x28   00fe  count of points
//  short xaFirst;      // 0x2a These are the endpoints of the first line.
//  short yaFirst;      // 0x2c
//  short xaEnd;        // 0x2e
//  short yaEnd;        // 0x30
//  short rgpta[];      // 0x32 An array of xa,ya pairs for the remaining points
};

struct WW8_DP_CALLOUT_TXTBOX
{
    SVBT16 flags;               // 0x0c REVIEW davebu flags
    SVBT16 dzaOffset;           // 0x0e REVIEW davebu
    SVBT16 dzaDescent;          // 0x10 REVIEW davebu
    SVBT16 dzaLength;           // 0x12 REVIEW davebu
    WW8_DPHEAD dpheadTxbx;      // 0x14 DPHEAD for a textbox
    WW8_DP_TXTBOX dptxbx;       // 0x20 DP for a textbox
    WW8_DPHEAD dpheadPolyLine;  // 0x4c DPHEAD for a Polyline
    WW8_DP_POLYLINE dpPolyLine; // 0x48 DP for a polyline
};

struct WW8_DP_DEFAULTS
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_LINEEND aEpp;
    WW8_DP_SHADOW aShd;
    SVBT16 dzaOffset;       // 0x2a REVIEW davebu
    SVBT16 dzaDescent;      // 0x2c REVIEW davebu
    SVBT16 dzaLength;       // 0x2e REVIEW davebu

    SVBT16 aBits3;
//  UINT16 fRoundCorners : 1;   // 0x30 0001    1 if the textbox has rounded corners
//  UINT16 zaShape : 15;        // 0x30 000fe   REVIEW davebu
    SVBT16 dzaInternalMargin;   // 0x32 REVIEW davebu
};


struct WW8_PCD
{
    SVBT8 aBits1;
//  BYTE fNoParaLast : 1;   // when 1, means that piece contains no end of paragraph marks.
//  BYTE fPaphNil : 1;      // used internally by Word
//  BYTE fCopied : 1;       // used internally by Word
//          *   int :5
    SVBT8 aBits2;           // fn int:8, used internally by Word
    SVBT32 fc;              // file offset of beginning of piece. The size of the
                            // ithpiece can be determined by subtracting rgcp[i] of
                            // the containing plcfpcd from its rgcp[i+1].
    SVBT16 prm;             // PRM contains either a single sprm or else an index number
                            // of the grpprl which contains the sprms that modify the
                            // properties of the piece.
};

struct WW8_PHE_Base
{
    BYTE aBits1;            //
//                              0   0   fSpare  int :1  0001    reserved
//                              fUnk    int :1  0002    phe entry is invalid
//                                                      when == 1
//                              fDiffLines  int :1  0004    when 1, total
//                               height of paragraph is known but lines in
//                               paragraph have different heights.
//                              *   int :5  00F8    reserved
    BYTE nlMac;             //  when fDiffLines is 0 is number of lines in
//                          //  paragraph
    SVBT16 dxaCol;          //  width of lines in paragraph
    SVBT16 dyl;
//                              4   4   dylLine int when fDiffLines is 0,
//                              is height of every line in paragraph.in pixels
//                              4   4 dylHeight uns when fDiffLines is 1,
//                              is the total height in pixels of the paragraph
};

/*
eigentlich muessten wir das jetzt in etwa *so* praezisieren:

            struct WW8_PHE_Ver6 : public WW8_PHE_Base
            {
                //  6 Bytes gross
            };
            struct WW8_PHE_Ver6 : public WW8_PHE_Base
            {
                SVBT16 a;
                SVBT16 b;
                SVBT16 c;   // 12 Byte gross
            };
*/

// AnnoTation Refernce Descriptor (ATRD)
struct WW8_ATRD                 // fuer die 8-Version
{
    SVBT16 xstUsrInitl[ 10 ];       // pascal-style String holding initials
                                    // of annotation author
    SVBT16 ibst;                    // index into GrpXstAtnOwners
    SVBT16 ak;                      // not used
    SVBT16 grfbmc;                  // not used
    SVBT32 ITagBkmk;                // when not -1, this tag identifies the
                                    // annotation bookmark that locates the
                                    // range of CPs in the main document which
                                    // this annotation references.
};

struct WW67_ATRD                // fuer die 6/7-Version
{
    sal_Char xstUsrInitl[ 10 ];     // pascal-style String holding initials
                                    // of annotation author
    SVBT16 ibst;                    // index into GrpXstAtnOwners
    SVBT16 ak;                      // not used
    SVBT16 grfbmc;                  // not used
    SVBT32 ITagBkmk;                // when not -1, this tag identifies the
                                    // annotation bookmark that locates the
                                    // range of CPs in the main document which
                                    // this annotation references.
};


#ifdef __WW8_NEEDS_PACK
#pragma pack()
#endif

/*************************************************************************
      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/ww8struc.hxx,v 1.2 2000-10-24 14:01:34 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.20  2000/09/18 16:05:02  willem.vandorp
      OpenOffice header added.

      Revision 1.19  2000/05/12 07:54:23  khz
      Changes for Unicode

      Revision 1.18  2000/02/11 14:40:36  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.17  1999/11/19 14:48:49  mib
      #70009#: export Solaris

      Revision 1.16  1999/06/16 17:54:56  JP
      Change interface of base class Writer, Export of W97 NumRules


      Rev 1.15   16 Jun 1999 19:54:56   JP
   Change interface of base class Writer, Export of W97 NumRules

      Rev 1.14   19 May 1999 11:12:56   JP
   WinWord97-ExportFilter

      Rev 1.13   11 May 1999 17:15:24   KHZ
   Task #66019# FontFamily: andere Bitreihenfolge auf Solaris beachten

      Rev 1.12   15 Feb 1999 21:36:50   KHZ
   Task #61381# Korrektur der Korrektur: jetzt include boxitem.hxx in ww8graf.cxx

      Rev 1.11   15 Feb 1999 20:54:44   KHZ
   Task #61381# Korrektur zur -r1.10: UEbernahme der defines statt der includes

      Rev 1.10   15 Feb 1999 18:19:44   HR
   <svx/eeitem.hxx> und <svx/boxitem.hxx> includen

      Rev 1.9   12 Feb 1999 16:51:38   KHZ
   Task #61381# Ersetzen von Sdr-Text-Objekten im Writer durch Rahmen (3)

      Rev 1.8   03 Dec 1998 10:39:52   JP
   Task #60063#: Kommentare als PostIts einlesen

      Rev 1.7   02 Dec 1998 15:34:28   JP
   Task #60063#: Kommentare als PostIts einlesen

      Rev 1.6   23 Oct 1998 15:37:02   KHZ
   Task #58199# jetzt kein GPF mehr bei nicht existenten Tabellenzellen :-)

      Rev 1.5   30 Jul 1998 14:28:34   KHZ
   Task #52607# Einrueckungen der Defines beseitigt

      Rev 1.4   09 Jul 1998 20:14:58   KHZ
   Tabellen: verbundene Zellen und Zellen-Hintergrundfarbe jetzt Ok.

      Rev 1.3   03 Jul 1998 16:14:54   KHZ
   ( Zwischenstand zur DaSi )

      Rev 1.2   30 Jun 1998 21:33:18   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.1   26 Jun 1998 20:50:10   KHZ
   Absatz-Attribute jetzt weitestgehend ok

      Rev 1.0   16 Jun 1998 10:57:08   KHZ
   Initial revision.

      Rev 1.1   10 Jun 1998 17:22:38   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:29:34   KHZ
   Initial revision.

*************************************************************************/

#endif
