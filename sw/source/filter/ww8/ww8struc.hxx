/*************************************************************************
 *
 *  $RCSfile: ww8struc.hxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:46:37 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _WW8STRUC_HXX
#define _WW8STRUC_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#if defined  __BIGENDIAN || __ALIGNMENT4 > 2 || defined UNX
#   define __WW8_NEEDS_COPY
#endif

#ifdef SAL_W32
#   pragma pack(push, 2)
#elif defined(SAL_OS2)
#   pragma pack(2)
#endif

inline void Set_UInt8( BYTE *& p, UINT8 n )
{
    ByteToSVBT8( n, *(SVBT8*)p );
    p+= 1;
}

inline void Set_UInt16( BYTE *& p, UINT16 n )
{
    ShortToSVBT16( n, *(SVBT16*)p );
    p+= 2;
}

inline void Set_UInt32( BYTE *& p, UINT32 n )
{
    LongToSVBT32( n, *(SVBT32*)p );
    p+= 4;
}

typedef sal_Int16 WW8_PN;
typedef sal_Int32 WW8_FC;
typedef sal_Int32 WW8_CP;

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
public:
    enum Limits {STI_USER = 0x0FFE};
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
    String sFontname;// 0x6 bzw. 0x40 ab Ver8 zero terminated string that
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

class WW8_BRC      // Border Code
{
public:
    SVBT16 aBits1;
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
    WW8_BRC()
    {
        memset(aBits1, 0, sizeof(aBits1));
        memset(aBits2, 0, sizeof(aBits2));
    }
    short DetermineBorderProperties (bool bVer67, short *pSpace=0,
        BYTE *pCol=0, short *pIdx=0) const;
    bool IsEmpty(bool bVer67) const;
    bool IsZeroed(bool bVer67) const;
    bool IsBlank() const;
};

typedef WW8_BRC WW8_BRC5[5];        // 5 * Border Code

enum BRC_Sides
{
    WW8_TOP = 0, WW8_LEFT = 1, WW8_BOT = 2, WW8_RIGHT = 3, WW8_BETW = 4
};

class WW8_BordersSO         // for StarOffice-Border Code
{
public:
    USHORT mnOut;
    USHORT mnIn;
    USHORT mnDist;
public:
    enum eBorderCode
    {
        single0, single1, single2, single3, single4,
        double0, double1, double2, double3, double4, double5, double6,
        double7, double8, double9, double10,
        none
    };
    static const WW8_BordersSO &Get0x01LineMatch(eBorderCode nIdx);
};

/*
Document Typography Info (DOPTYPOGRAPHY) These options are Far East only,
and are accessible through the Typography tab of the Tools/Options dialog.
*/
class WW8DopTypography
{
public:
    void ReadFromMem(BYTE *&pData);
    void WriteToMem(BYTE *&pData) const;

    //Maps what I think is the language this is to affect to the OOo language
    USHORT GetConvertedLang() const;

    UINT16 fKerningPunct  : 1;  // true if we're kerning punctuation
    UINT16 iJustification : 2;  // Kinsoku method of justification:
                                //  0 = always expand
                                //  1 = compress punctuation
                                //  2 = compress punctuation and kana.
    UINT16 iLevelOfKinsoku : 2; // Level of Kinsoku:
                                //  0 = Level 1
                                //  1 = Level 2
                                //  2 = Custom
    UINT16 f2on1          : 1;  // 2-page-on-1 feature is turned on.
    UINT16 reserved1      : 4;  // in 97 its marked as reserved BUT
    UINT16 reserved2      : 6;  // reserved ?
    //we find that the following applies,
    //2 == Japanese
    //4 == Chinese (VR...
    //6 == Korean
    //8 == Chinese (Ta...
    //perhaps a bit field where the DOP can possibly relate to more than
    //one language at a time, nevertheless MS seems to have painted
    //themselves into a small corner with one DopTypography for the
    //full document, might not matter all that much though ?

    enum RuleLengths {nMaxFollowing = 101, nMaxLeading = 51};
    static const sal_Unicode * GetJapanNotBeginLevel1();
    static const sal_Unicode * GetJapanNotEndLevel1();

    INT16 cchFollowingPunct;    // length of rgxchFPunct
    INT16 cchLeadingPunct;      // length of rgxchLPunct

    // array of characters that should never appear at the start of a line
    sal_Unicode rgxchFPunct[nMaxFollowing];
    // array of characters that should never appear at the end of a line
    sal_Unicode rgxchLPunct[nMaxLeading];
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
//  WW8_BRC rgbrc[4];
//  BRC brcTop;         // 0x2e specification for border above picture
//  BRC brcLeft;        // 0x30 specification for border to the left
//  BRC brcBottom;      // 0x32 specification for border below picture
//  BRC brcRight;       // 0x34 specification for border to the right
//  SVBT16 dxaOrigin;   // 0x36 horizontal offset of hand annotation origin
//  SVBT16 dyaOrigin;   // 0x38 vertical offset of hand annotation origin
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
    BYTE bFirstMerged   : 1;// 0001 set to 1 when cell is first cell of a range of cells that have been merged.
    BYTE bMerged        : 1;// 0002 set to 1 when cell has been merged with preceding cell.
    BYTE bVertical      : 1;// set to 1 when cell has vertical text flow
    BYTE bBackward      : 1;// for a vertical table cell, text flow is bottom to top when 1 and is bottom to top when 0.
    BYTE bRotateFont    : 1;// set to 1 when cell has rotated characters (i.e. uses @font)
    BYTE bVertMerge     : 1;// set to 1 when cell is vertically merged with the cell(s) above and/or below. When cells are vertically merged, the display area of the merged cells are consolidated. The consolidated area is used to display the contents of the first vertically merged cell (the cell with fVertRestart set to 1), and all other vertically merged cells (those with fVertRestart set to 0) must be empty. Cells can only be merged vertically if their left and right boundaries are (nearly) identical (i.e. if corresponding entries in rgdxaCenter of the table rows differ by at most 3).
    BYTE bVertRestart   : 1;// set to 1 when the cell is the first of a set of vertically merged cells. The contents of a cell with fVertStart set to 1 are displayed in the consolidated area belonging to the entire set of vertically merged cells. Vertically merged cells with fVertRestart set to 0 must be empty.
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
    UINT16 maBits;
//  UINT16 nFore : 5;       // 0x001f ForegroundColor
//  UINT16 nBack : 5;       // 0x03e0 BackgroundColor
//  UINT16 nStyle : 5;      // 0x7c00 Percentage and Style
//  UINT16 nDontKnow : 1;   // 0x8000 ???   ab Ver8: ebenfalls fuer Style

public:
    WW8_SHD() : maBits(0) {}

    BYTE GetFore() const { return (BYTE)( maBits & 0x1f); }
    BYTE GetBack() const { return (BYTE)((maBits >> 5 ) & 0x1f); }
    BYTE GetStyle(bool bVer67)  const
        { return (BYTE)((maBits >> 10) & ( bVer67 ? 0x1f : 0x3f ) ); }

    UINT16 GetValue() const { return maBits; }

    void SetValue(UINT16 nVal) { maBits = nVal; }
    void SetWWValue(SVBT16 nVal) { maBits = SVBT16ToShort(nVal); }

    void SetFore(BYTE nVal)
    {
        maBits &= 0xffe0;
        maBits |= (nVal & 0x1f);
    }
    void SetBack(BYTE nVal)
    {
        maBits &= 0xfc1f;
        maBits |= (nVal & 0x1f) << 5;
    }
    void SetStyle(bool bVer67, BYTE nVal)
    {
        if (bVer67)
        {
            maBits &= 0x83ff;
            maBits |= (nVal & 0x1f) << 10;
        }
        else
        {
            maBits &= 0x03ff;
            maBits |= (nVal & 0x2f) << 10;
        }
    }
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

#if 0
struct ANLV
{
    ALNV();
    void ReadFromMem(const sal_uInt8 *&pData);
    sal_uInt8 nfc;
    sal_uInt8 cbTextBefore;
    sal_uInt8 cbTextAfter;
    sal_uInt8 jc : 2;
    sal_uInt8 fPrev : 1;
    sal_uInt8 fHang : 1;
    sal_uInt8 fSetBold : 1;
    sal_uInt8 fSetItalic : 1;
    sal_uInt8 fSetSmallCaps : 1;
    sal_uInt8 fSetCaps : 1;
    sal_uInt8 fSetStrike : 1;
    sal_uInt8 fSetKul : 1;
    sal_uInt8 fPrevSpace : 1;
    sal_uInt8 fBold : 1;
    sal_uInt8 fItalic : 1;
    sal_uInt8 fSmallCaps : 1;
    sal_uInt8 fCaps : 1;
    sal_uInt8 fStrike : 1;
    sal_uInt8 kul : 3;
    sal_uInt8 ico : 5;
    sal_uInt16 ftc;
    sal_uInt16 hps;
    sal_uInt16 iStartAt;
    sal_uInt16 dxaIndent;
    sal_uInt16 dxaSpace;
};

struct OLST
{
    OLST();
    void ReadFromMem(const sal_uInt8 *&pData, bool bVer67)
    ANLV rganlv[9];         // 0 an array of 9 ANLV structures (heading levels)
    sal_uInt8 fRestartHdr;  // when ==1, restart heading on section break
    sal_uInt8 fSpareOlst2;  // reserved
    sal_uInt8 fSpareOlst3;  // reserved
    sal_uInt8 fSpareOlst4;  // reserved
    sal_uInt16 rgxch[64];   // array of 64 chars text before/after number
#if 0
    sal_uInt16 rgxch[32];   // array of 32 chars text before/after number
    sal_uInt8 rgch[64];     // array of 64 chars text before/after number
#endif
};
#endif

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

struct WW8_TablePos
{
    INT16 nSp26;
    INT16 nSp27;
    INT16 nLeMgn;
    INT16 nRiMgn;
    INT16 nUpMgn;
    INT16 nLoMgn;
    BYTE nSp29;
    BYTE nSp37;
};

struct WW8_FSPA
{
public:
    long nSpId;     //Shape Identifier. Used in conjunction with the office art data (found via fcDggInfo in the FIB) to find the actual data for this shape.
    long nXaLeft;   //left of rectangle enclosing shape relative to the origin of the shape
    long nYaTop;        //top of rectangle enclosing shape relative to the origin of the shape
    long nXaRight;  //right of rectangle enclosing shape relative to the origin of the shape
    long nYaBottom;//bottom of the rectangle enclosing shape relative to the origin of the shape
    USHORT bHdr:1;
    //0001 1 in the undo doc when shape is from the header doc, 0 otherwise (undefined when not in the undo doc)
    USHORT nbx:2;
    //0006 x position of shape relative to anchor CP
    //0 relative to page margin
    //1 relative to top of page
    //2 relative to text (column for horizontal text; paragraph for vertical text)
    //3 reserved for future use
    USHORT nby:2;
    //0018 y position of shape relative to anchor CP
    //0 relative to page margin
    //1 relative to top of page
    //2 relative to text (paragraph for horizontal text; column for vertical text)
    USHORT nwr:4;
    //01E0 text wrapping mode
    //0 like 2, but doesn't require absolute object
    //1 no text next to shape
    //2 wrap around absolute object
    //3 wrap as if no object present
    //4 wrap tightly around object
    //5 wrap tightly, but allow holes
    //6-15 reserved for future use
    USHORT nwrk:4;
    //1E00 text wrapping mode type (valid only for wrapping modes 2 and 4
    //0 wrap both sides
    //1 wrap only on left
    //2 wrap only on right
    //3 wrap only on largest side
    USHORT bRcaSimple:1;
    //2000 when set, temporarily overrides bx, by, forcing the xaLeft, xaRight, yaTop, and yaBottom fields to all be page relative.
    USHORT bBelowText:1;
    //4000
    //1 shape is below text
    //0 shape is above text
    USHORT bAnchorLock:1;
    //8000  1 anchor is locked
    //      0 anchor is not locked
    long nTxbx; //count of textboxes in shape (undo doc only)
public:
    enum FSPAOrient {RelPgMargin, RelPageBorder, RelText};
};


struct WW8_FSPA_SHADOW  // alle Member an gleicher Position und Groesse,
{                                               // wegen:  pF = (WW8_FSPA*)pFS;
    SVBT32 nSpId;
    SVBT32 nXaLeft;
    SVBT32 nYaTop;
    SVBT32 nXaRight;
    SVBT32 nYaBottom;
    SVBT16 aBits1;
    SVBT32 nTxbx;
};

struct WW8_TXBXS
{
    SVBT32 cTxbx_iNextReuse;
    SVBT32 cReusable;
    SVBT16 fReusable;
    SVBT32 reserved;
    SVBT32 ShapeId;
    SVBT32 txidUndo;
};

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
#endif

struct SEPr
{
    SEPr();
    sal_uInt8 bkc;
    sal_uInt8 fTitlePage;
    sal_Int8 fAutoPgn;
    sal_uInt8 nfcPgn;
    sal_uInt8 fUnlocked;
    sal_uInt8 cnsPgn;
    sal_uInt8 fPgnRestart;
    sal_uInt8 fEndNote;
    sal_Int8 lnc;
    sal_Int8 grpfIhdt;
    sal_uInt16 nLnnMod;
    sal_Int32 dxaLnn;
    sal_Int16 dxaPgn;
    sal_Int16 dyaPgn;
    sal_Int8 fLBetween;
    sal_Int8 vjc;
    sal_uInt16 dmBinFirst;
    sal_uInt16 dmBinOther;
    sal_uInt16 dmPaperReq;
#if 0
    28  1C  brcTop                    BRC                   top page border

    32  20  brcLeft                   BRC                   left page border

    36  24  brcBottom                 BRC                   bottom page border

    40  28  brcRight                  BRC                   right page border
#endif
    sal_Int16 fPropRMark;
    sal_Int16 ibstPropRMark;
    sal_Int32 dttmPropRMark;        //DTTM
    sal_Int32 dxtCharSpace;
    sal_Int32 dyaLinePitch;
    sal_uInt16 clm;
    sal_Int16 reserved1;
    sal_uInt8 dmOrientPage;
    sal_uInt8 iHeadingPgn;
    sal_uInt16 pgnStart;
    sal_Int16 lnnMin;
    sal_uInt16 wTextFlow;
    sal_Int16 reserved2;
    sal_uInt16 pgbApplyTo:3;
    sal_uInt16 pgbPageDepth:2;
    sal_Int16 pgbOffsetFrom:3;
    sal_Int16 :8;
    sal_uInt32 xaPage;
    sal_uInt32 yaPage;
    sal_uInt32 xaPageNUp;
    sal_uInt32 yaPageNUp;
    sal_uInt32 dxaLeft;
    sal_uInt32 dxaRight;
    sal_Int32 dyaTop;
    sal_Int32 dyaBottom;
    sal_uInt32 dzaGutter;
    sal_uInt32 dyaHdrTop;
    sal_uInt32 dyaHdrBottom;
    sal_Int16 ccolM1;
    sal_Int8 fEvenlySpaced;
    sal_Int8 reserved3;
    sal_uInt8 fBiDi;
    sal_uInt8 fFacingCol;
    sal_uInt8 fRTLGutter;
    sal_uInt8 fRTLAlignment;
    sal_Int32 dxaColumns;
    sal_Int32 rgdxaColumnWidthSpacing[89];
    sal_Int32 dxaColumnWidth;
    sal_uInt8 dmOrientFirst;
    sal_uInt8 fLayout;
    sal_Int16 reserved4;
#if 0
    OLST olstAnm;       //currently unused
#endif
};

namespace wwUtility
{
    sal_uInt32 BGRToRGB(sal_uInt32 nColour);
    inline sal_uInt32 RGBToBGR(sal_uInt32 nColour) { return BGRToRGB(nColour); }
};

#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
