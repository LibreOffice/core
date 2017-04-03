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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8STRUC_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8STRUC_HXX

#include <sal/config.h>

#include <osl/endian.h>
#include <rtl/ustring.hxx>

#include <editeng/borderline.hxx>
#include <filter/msfilter/util.hxx>

#ifdef SAL_W32
#   pragma pack(push, 2)
#endif

class WW8Export;

inline void Set_UInt8( sal_uInt8 *& p, sal_uInt8 n )
{
    *p = n;
    p+= 1;
}

inline void Set_UInt16( sal_uInt8 *& p, sal_uInt16 n )
{
    ShortToSVBT16( n, *reinterpret_cast<SVBT16*>(p) );
    p+= 2;
}

inline void Set_UInt32( sal_uInt8 *& p, sal_uInt32 n )
{
    UInt32ToSVBT32( n, *reinterpret_cast<SVBT32*>(p) );
    p+= 4;
}

struct Word2CHPX
{
    sal_uInt16 fBold:1;
    sal_uInt16 fItalic:1;
    sal_uInt16 fRMarkDel:1;
    sal_uInt16 fOutline:1;
    sal_uInt16 fFieldVanish:1;
    sal_uInt16 fSmallCaps:1;
    sal_uInt16 fCaps:1;
    sal_uInt16 fVanish:1;
    sal_uInt16 fRMark:1;
    sal_uInt16 fSpec:1;
    sal_uInt16 fStrike:1;
    sal_uInt16 fObj:1;
    sal_uInt16 fBoldBi:1;
    sal_uInt16 fItalicBi:1;
    sal_uInt16 fBiDi:1;
    sal_uInt16 fDiacUSico:1;
    sal_uInt16 fsIco:1;
    sal_uInt16 fsFtc:1;
    sal_uInt16 fsHps:1;
    sal_uInt16 fsKul:1;
    sal_uInt16 fsPos:1;
    sal_uInt16 fsSpace:1;
    sal_uInt16 fsLid:1;
    sal_uInt16 fsIcoBi:1;
    sal_uInt16 fsFtcBi:1;
    sal_uInt16 fsHpsBi:1;
    sal_uInt16 fsLidBi:1;

    sal_uInt16 ftc;
    sal_uInt16 hps;
    sal_uInt8 qpsSpace:6;
    sal_uInt8 fSysVanish:1;
    sal_uInt8 fNumRun:1;
    sal_uInt8 ico:5;
    sal_uInt8 kul:3;
    sal_uInt8 hpsPos;
    sal_uInt8 icoBi;
    sal_uInt16 lid;
    sal_uInt16 ftcBi;
    sal_uInt16 hpsBi;
    sal_uInt16 lidBi;
    sal_uInt32 fcPic;

    Word2CHPX()
    {
        fBold = 0;
        fItalic = 0;
        fRMarkDel = 0;
        fOutline = 0;
        fFieldVanish = 0;
        fSmallCaps = 0;
        fCaps = 0;
        fVanish = 0;
        fRMark = 0;
        fSpec = 0;
        fStrike = 0;
        fObj = 0;
        fBoldBi = 0;
        fItalicBi = 0;
        fBiDi = 0;
        fDiacUSico = 0;
        fsIco = 0;
        fsFtc = 0;
        fsHps = 0;
        fsKul = 0;
        fsPos = 0;
        fsSpace = 0;
        fsLid = 0;
        fsIcoBi = 0;
        fsFtcBi = 0;
        fsHpsBi = 0;
        fsLidBi = 0;

        ftc = 0;
        hps = 0;
        qpsSpace = 0;
        fSysVanish = 0;
        fNumRun = 0;
        ico = 0;
        kul = 0;
        hpsPos = 0;
        icoBi = 0;
        lid = 0;
        ftcBi = 0;
        hpsBi = 0;
        lidBi = 0;
        fcPic = 0;
    }
};

typedef sal_Int16 WW8_PN;
typedef sal_Int32 WW8_FC;
typedef sal_Int32 WW8_CP;

const WW8_FC WW8_FC_MAX = SAL_MAX_INT32;
const WW8_CP WW8_CP_MAX = SAL_MAX_INT32;

/** STD - STyle Definition

    The STD contains the entire definition of a style.
    It has two parts, a fixed-length base (cbSTDBase bytes long)
    and a variable length remainder holding the name, and the upx and upe
    arrays (a upx and upe for each type stored in the style, std.cupx)
    Note that new fields can be added to the BASE of the STD without
    invalidating the file format, because the STSHI contains the length
    that is stored in the file.  When reading STDs from an older version,
    new fields will be zero.
*/
struct WW8_STD
{
    // Base part of STD:
    sal_uInt16  sti : 12;          // invariant style identifier
    sal_uInt16  fScratch : 1;      // spare field for any temporary use,
                                                         // always reset back to zero!
    sal_uInt16  fInvalHeight : 1;  // PHEs of all text with this style are wrong
    sal_uInt16  fHasUpe : 1;       // UPEs have been generated
    sal_uInt16  fMassCopy : 1;     // std has been mass-copied; if unused at
                                                         // save time, style should be deleted
    sal_uInt16  sgc : 4;           // style type code
    sal_uInt16  istdBase : 12;     // base style
    sal_uInt16  cupx : 4;          // # of UPXs (and UPEs)
    sal_uInt16  istdNext : 12;     // next style
    sal_uInt16  bchUpe;            // offset to end of upx's, start of upe's
    // new:
    // from Ver8 on there are two more fields:
  sal_uInt16    fAutoRedef : 1;    /* auto redefine style when appropriate */
  sal_uInt16    fHidden : 1;       /* hidden from UI? */
  sal_uInt16    : 14;              /* unused bits */

    // Variable length part of STD:
    //  sal_uInt8   stzName[2];        /* sub-names are separated by chDelimStyle
    // char grupx[];
            // the UPEs are not stored on the file; they are a cache of the based-on
        // chain
    // char grupe[];
};

static_assert(sizeof (WW8_STD) == 10, "this has to match the msword size");

/** base for reading AND working on (will have different subclasses */
struct WW8_FFN_BASE     // Font Descriptor
{
    // ab Ver6
    sal_uInt8    cbFfnM1;        //  0x0     total length of FFN - 1.

    sal_uInt8    prg: 2;         //  0x1:03  pitch request
    sal_uInt8    fTrueType : 1;  //  0x1:04  when 1, font is a TrueType font
    sal_uInt8    _reserved1 : 1; //  0x1:08  reserved
    sal_uInt8    ff : 3;         //  0x1:70  font family id
    sal_uInt8    _reserved2 : 1; //  0x1:80  reserved

    short wWeight;          //  0x2     base weight of font
    sal_uInt8    chs;            //  0x4     character set identifier
    sal_uInt8    ibszAlt;        //  0x5     index into ffn.szFfn to the name of the alternate font
};

static_assert(sizeof (WW8_FFN_BASE) == 6, "this has to match the msword size");

/** This is what we use in the Parser (and Dumper)
*/
struct WW8_FFN : public WW8_FFN_BASE
{
    // ab Ver8 als Unicode
    OUString sFontname;// 0x6 or 0x40 resp. from Ver8 on zero terminated string that
                                        // records name of font.
                                        // Maximal size of szFfn is 65 characters.
                                        // Attention: This array can be smaller!!!
                                        // Possibly followed by a second sz which records the
                                        // name of an alternate font to use if the first named
                                        // font does not exist on this system.
};

struct WW8_BRCVer6  // BoRder Code (WW6 version)
{
    SVBT16 aBits1;
//  sal_uInt16 dxpLineWidth : 3;// 0007 When dxpLineWidth is 0, 1, 2, 3, 4, or 5, this field is the width of
                            //      a single line of border in units of 0.75 points
                            //      Must be nonzero when brcType is nonzero.
                            //      6 == dotted, 7 == dashed.
//  sal_uInt16 brcType : 2;     // 0018 border type code: 0 == none, 1 == single, 2 == thick, 3 == double
//  sal_uInt16 fShadow : 1;     // 0020 when 1, border is drawn with shadow. Must be 0 when BRC is a substructure of the TC
//  sal_uInt16 ico : 5;         // 07C0 color code (see chp.ico)
//  sal_uInt16 dxpSpace : 5;    // F800 width of space to maintain between border and text within border.
                            //      Must be 0 when BRC is a substructure of the TC.  Stored in points for Windows.
    WW8_BRCVer6()
    {
        memset(aBits1, 0, sizeof(aBits1));
    }

    sal_uInt8 dxpLineWidth() const
        { return aBits1[0] & 0x07; }
    sal_uInt8 brcType() const
        { return (aBits1[0] & 0x18) >> 3; }
    bool fShadow() const
        { return !!(aBits1[0] & 0x20); }
    sal_uInt8 ico() const
        { return ((aBits1[0] & 0xc0) >> 6) | ((aBits1[1] & 0x07) << 2); }
    sal_uInt8 dxpSpace() const
        { return aBits1[1] >> 3; }

    WW8_BRCVer6(sal_uInt8 _dxpLineWidth, sal_uInt8 _brcType, sal_uInt8 _ico,
        sal_uInt8 _dxpSpace, bool _fShadow)
    {
        assert(_dxpSpace < 0x20);
        assert(_brcType <= 3);
        assert(_ico < 32);
        aBits1[0] = _dxpLineWidth | (_brcType << 3) | ((sal_uInt8)_fShadow << 5)
            | ((_ico << 6) & 0xc0);
        aBits1[1] = (_ico >> 2) | (_dxpSpace << 3);
    }
};

struct WW8_BRC  // BoRder Code (WW8 version)
// Documented at http://msdn.microsoft.com/en-us/library/dd952599.aspx
{
    SVBT16 aBits1;
    SVBT16 aBits2;
//  sal_uInt8 dptLineWidth;
//  sal_uInt8 brcType;
//  sal_uInt8 ico;
//  sal_uInt8 dptSpace : 5
//  bool fShadow : 1;
//  bool fFrame : 1;
//  bool fReserved : 1;
    WW8_BRC()
    {
        memset(aBits1, 0, sizeof(aBits1));
        memset(aBits2, 0, sizeof(aBits2));
    }

    sal_uInt8 dptLineWidth() const // border line width (1/8pt)
        { return aBits1[0]; }
    sal_uInt8 brcType() const      // border type (eg single, double, dotted)
        { return aBits1[1]; }
    sal_uInt8 ico() const          // colour index, 1-17 or 0=auto
        { return aBits2[0]; }
    sal_uInt8 dptSpace() const     // space between text & border (pt)
        { return aBits2[1] & 0x1f; }
    bool fShadow() const           // shadow effect
        { return !!(aBits2[1] & 0x20); }
    bool fFrame() const            // 3D frame effect
        { return !!(aBits2[1] & 0x40); }
    bool isNil() const             // nil = no border
        { return aBits1[0] == 0xff && aBits1[1] == 0xff; }

    WW8_BRC(sal_uInt8 _dptLineWidth, sal_uInt8 _brcType, sal_uInt8 _ico,
        sal_uInt8 _dptSpace, bool _fShadow, bool _fFrame)
    {
        assert(_dptSpace < 0x20);
        aBits1[0] = _dptLineWidth;
        aBits1[1] = _brcType;
        aBits2[0] = _ico;
        aBits2[1] = _dptSpace | ((sal_uInt8)_fShadow << 5)
            | ((sal_uInt8)_fFrame << 6);
    }
    // Convert BRC from WW6 to WW8 format
    explicit WW8_BRC(const WW8_BRCVer6& brcVer6);

    // Returns LO border width in twips=1/20pt, taking into account brcType
    short DetermineBorderProperties(short *pSpace=nullptr) const;
};

typedef WW8_BRC WW8_BRC5[5];        // 5 * Border Code

struct WW8_BRCVer9  // BoRder Code (WW9 version)
// Documented at http://msdn.microsoft.com/en-us/library/dd907496.aspx
{
    SVBT32 aBits1; // border colour (RGB)
    SVBT32 aBits2;
//  sal_uInt8 dptLineWidth;   // border line width (1/8pt)
//  sal_uInt8 brcType;        // border type (eg single, double, dotted)
//  sal_uInt8 dptSpace : 5;   // space between text & border (pt)
//  bool fShadow : 1;         // border has shadow effect
//  bool fFrame : 1;          // border has 3D effect
//  sal_uInt16 fReserved : 9; // unused
    WW8_BRCVer9()
    {
        memset(aBits1, 0, sizeof(aBits1));
        memset(aBits2, 0, sizeof(aBits2));
    }

    sal_uInt32 cv() const          // colour value (BGR)
        { return SVBT32ToUInt32(aBits1); }
    sal_uInt8 dptLineWidth() const // border line width (1/8pt)
        { return aBits2[0]; }
    sal_uInt8 brcType() const      // border type (eg single, double, dotted)
        { return aBits2[1]; }
    sal_uInt8 dptSpace() const     // space between text & border (pt)
        { return aBits2[2] & 0x1f; }
    bool fShadow() const           // shadow effect
        { return !!(aBits2[2] & 0x20); }
    bool fFrame() const            // 3D frame effect
        { return !!(aBits2[2] & 0x40); }
    bool isNil() const             // nil = no border
        { return SVBT32ToUInt32(aBits2) == 0xffffffff; }

    WW8_BRCVer9(sal_uInt32 _cv, sal_uInt8 _dptLineWidth, sal_uInt8 _brcType,
        sal_uInt8 _dptSpace, bool _fShadow, bool _fFrame)
    {
        assert(_dptSpace < 0x20);
        UInt32ToSVBT32(_cv, aBits1);
        aBits2[0] = _dptLineWidth;
        aBits2[1] = _brcType;
        aBits2[2] = _dptSpace | ((sal_uInt8)_fShadow << 5)
            | ((sal_uInt8)_fFrame << 6);
        aBits2[3] = 0;
    }
    // Convert BRC from WW8 to WW9 format
    explicit WW8_BRCVer9(const WW8_BRC& brcVer8);

    // Returns LO border width in twips=1/20pt, taking into account brcType
    short DetermineBorderProperties(short *pSpace=nullptr) const;
};

typedef WW8_BRCVer9 WW8_BRCVer9_5[5];        // 5 * Border Code

enum BRC_Sides
{
    WW8_TOP = 0, WW8_LEFT = 1, WW8_BOT = 2, WW8_RIGHT = 3, WW8_BETW = 4
};

/*
Document Typography Info (DOPTYPOGRAPHY) These options are Far East only,
and are accessible through the Typography tab of the Tools/Options dialog.
*/
class WW8DopTypography
{
public:
    void ReadFromMem(sal_uInt8 *&pData);
    void WriteToMem(sal_uInt8 *&pData) const;

    //Maps what I think is the language this is to affect to the OOo language
    sal_uInt16 GetConvertedLang() const;

    sal_uInt16 fKerningPunct  : 1;  // true if we're kerning punctuation
    sal_uInt16 iJustification : 2;  // Kinsoku method of justification:
                                //  0 = always expand
                                //  1 = compress punctuation
                                //  2 = compress punctuation and kana.
    sal_uInt16 iLevelOfKinsoku : 2; // Level of Kinsoku:
                                //  0 = Level 1
                                //  1 = Level 2
                                //  2 = Custom
    sal_uInt16 f2on1          : 1;  // 2-page-on-1 feature is turned on.
    sal_uInt16 reserved1      : 4;  // in 97 its marked as reserved BUT
    sal_uInt16 reserved2      : 6;  // reserved ?
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

    sal_Int16 cchFollowingPunct;    // length of rgxchFPunct
    sal_Int16 cchLeadingPunct;      // length of rgxchLPunct

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

    /* attention: you must not put bit fields on top of such a byte array read from a file!
       instead put an aBits1 on it and read it out with &.
       reason: compilers on Intel and Sparc sort the bits differently
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

static_assert(sizeof (WW8_DOGRID) == 10, "this has to match the msword size");

struct WW8_PIC
{
    sal_Int32 lcb;          // 0x0 number of bytes in the PIC structure plus size of following picture data which may be a Window's metafile, a bitmap, or the filename of a TIFF file.
    sal_uInt16 cbHeader;    // 0x4 number of bytes in the PIC (to allow for future expansion).
    struct {
        sal_Int16 mm;       // 0x6  int
        sal_Int16 xExt;     // 0x8  int
        sal_Int16 yExt;     // 0xa  int
        sal_Int16 hMF;      // 0xc  int
    }MFP;
//  sal_uInt8 bm[14];        // 0xe  BITMAP(14 bytes)    Window's bitmap structure when PIC describes a BITMAP.
    sal_uInt8 rcWinMF[14];   // 0xe  rc (rectangle - 8 bytes) rect for window origin
                        //      and extents when  metafile is stored -- ignored if 0
    sal_Int16 dxaGoal;      // 0x1c horizontal  measurement in twips of the  rectangle the picture should be imaged within.
    sal_Int16 dyaGoal;      // 0x1e vertical  measurement in twips of the  rectangle the picture should be imaged within.
    sal_uInt16 mx;          // 0x20 horizontal scaling factor supplied by user in .1% units.
    sal_uInt16 my;          // 0x22 vertical scaling factor supplied by user in .1% units.
    sal_Int16 dxaCropLeft;  // 0x24 the amount the picture has been cropped on the left in twips.
    sal_Int16 dyaCropTop;   // 0x26 the amount the picture has been cropped on the top in twips.
    sal_Int16 dxaCropRight; // 0x28 the amount the picture has been cropped on the right in twips.
    sal_Int16 dyaCropBottom;// 0x2a the amount the picture has been cropped on the bottom in twips.
    sal_Int16 brcl : 4;     // 000F Obsolete, superseded by brcTop, etc.  In
    sal_Int16 fFrameEmpty : 1;  // 0010 picture consists of a single frame
    sal_Int16 fBitmap : 1;      // 0020 ==1, when picture is just a bitmap
    sal_Int16 fDrawHatch : 1;   // 0040 ==1, when picture is an active OLE object
    sal_Int16 fError : 1;       // 0080 ==1, when picture is just an error message
    sal_Int16 bpp : 8;      // FF00 bits per pixel, 0 = unknown
    WW8_BRC rgbrc[4];
//  BRC brcTop;         // 0x2e specification for border above picture
//  BRC brcLeft;        // 0x30 specification for border to the left
//  BRC brcBottom;      // 0x32 specification for border below picture
//  BRC brcRight;       // 0x34 specification for border to the right
    sal_Int16 dxaOrigin;    // 0x36 horizontal offset of hand annotation origin
    sal_Int16 dyaOrigin;    // 0x38 vertical offset of hand annotation origin
//  sal_uInt8 rgb[];         // 0x3a variable array of bytes containing Window's metafile, bitmap or TIFF file filename.
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
//  sal_uInt8 bm[14];       // 0xe  BITMAP(14 bytes)    Window's bitmap structure when PIC describes a BITMAP.
    sal_uInt8 rcWinMF[14];  // 0xe  rc (rectangle - 8 bytes) rect for window origin
                        //      and extents when  metafile is stored -- ignored if 0
    SVBT16 dxaGoal;     // 0x1c horizontal  measurement in twips of the  rectangle the picture should be imaged within.
    SVBT16 dyaGoal;     // 0x1e vertical  measurement in twips of the  rectangle the picture should be imaged within.
    SVBT16 mx;          // 0x20 horizontal scaling factor supplied by user in .1% units.
    SVBT16 my;          // 0x22 vertical scaling factor supplied by user in .1% units.
    SVBT16 dxaCropLeft; // 0x24 the amount the picture has been cropped on the left in twips.
    SVBT16 dyaCropTop;  // 0x26 the amount the picture has been cropped on the top in twips.
    SVBT16 dxaCropRight;    // 0x28 the amount the picture has been cropped on the right in twips.
    SVBT16 dyaCropBottom;// 0x2a    the amount the picture has been cropped on the bottom in twips.
    sal_uInt8 aBits1; //0x2c
    sal_uInt8 aBits2;
//  WW8_BRC rgbrc[4];
//  BRC brcTop;         // 0x2e specification for border above picture
//  BRC brcLeft;        // 0x30 specification for border to the left
//  BRC brcBottom;      // 0x32 specification for border below picture
//  BRC brcRight;       // 0x34 specification for border to the right
//  SVBT16 dxaOrigin;   // 0x36 horizontal offset of hand annotation origin
//  SVBT16 dyaOrigin;   // 0x38 vertical offset of hand annotation origin
//  sal_uInt8 rgb[];            // 0x3a variable array of bytes containing Window's metafile, bitmap or TIFF file filename.
};

static_assert(sizeof (WW8_PIC_SHADOW) == 0x2E, "this has to match the msword size");
    // "0x2E": cf. SwWW8ImplReader::PicRead pDataStream->Read call

struct WW8_TBD
{
    sal_uInt8 aBits1;
//  sal_uInt8 jc : 3;        // 0x07 justification code: 0=left tab, 1=centered tab, 2=right tab, 3=decimal tab, 4=bar
//  sal_uInt8 tlc : 3;       // 0x38 tab leader code: 0=no leader, 1=dotted leader,
                        // 2=hyphenated leader, 3=single line leader, 4=heavy line leader
//  *   int :2  C0  reserved
};

struct WW8_TCell    // this is the base for further work (corresponds mostly to the Ver8 format)
{
    // The single-bit fields should ideally be bool, but probably need to keep
    // them as sal_uInt8 to make them combine with the following two-bit
    // nVertAlign:
    sal_uInt8 bFirstMerged   : 1;// 0001 set to 1 when cell is first cell of a range of cells that have been merged.
    sal_uInt8 bMerged        : 1;// 0002 set to 1 when cell has been merged with preceding cell.
    sal_uInt8 bVertical      : 1;// set to 1 when cell has vertical text flow
    sal_uInt8 bBackward      : 1;// for a vertical table cell, text flow is bottom to top when 1 and is bottom to top when 0.
    sal_uInt8 bRotateFont    : 1;// set to 1 when cell has rotated characters (i.e. uses @font)
    sal_uInt8 bVertMerge     : 1;// set to 1 when cell is vertically merged with the cell(s) above and/or below. When cells are vertically merged, the display area of the merged cells are consolidated. The consolidated area is used to display the contents of the first vertically merged cell (the cell with fVertRestart set to 1), and all other vertically merged cells (those with fVertRestart set to 0) must be empty. Cells can only be merged vertically if their left and right boundaries are (nearly) identical (i.e. if corresponding entries in rgdxaCenter of the table rows differ by at most 3).
    sal_uInt8 bVertRestart   : 1;// set to 1 when the cell is the first of a set of vertically merged cells. The contents of a cell with fVertStart set to 1 are displayed in the consolidated area belonging to the entire set of vertically merged cells. Vertically merged cells with fVertRestart set to 0 must be empty.
    sal_uInt8 nVertAlign     : 2;// specifies the alignment of the cell contents relative to text flow (e.g. in a cell with bottom to top text flow and bottom vertical alignment, the text is shifted horizontally to match the cell's right boundary):
                                                    //          0 top
                                                    //          1 center
                                                    //          2 bottom
    sal_uInt16 fUnused      : 7;// reserved - do not remove, fills up the sal_uInt16!

    WW8_BRCVer9 rgbrc[4];   // border codes
//notational convenience for referring to brcTop, brcLeft, etc fields.
//  BRC brcTop;             // specification of the top border of a table cell
//  BRC brcLeft;            // specification of left border of table row
//  BRC brcBottom;          // specification of bottom border of table row
//  BRC brcRight;           // specification of right border of table row.
};
// cbTC (count of bytes of a TC) is 18(decimal), 12(hex).

struct WW8_TCellVer6    // read from file
{
    sal_uInt8  aBits1Ver6;
    sal_uInt8  aBits2Ver6;
//  sal_uInt16 fFirstMerged : 1;// 0001 set to 1 when cell is first cell of a range of cells that have been merged.
//  sal_uInt16 fMerged : 1;     // 0002 set to 1 when cell has been merged with preceding cell.
//  sal_uInt16 fUnused : 14;    // FFFC reserved
    WW8_BRCVer6 rgbrcVer6[4];
// notational convenience for referring to brcTop, brcLeft, etc fields:
//          BRC brcTop;             // specification of the top border of a table cell
//          BRC brcLeft;            // specification of left border of table row
//          BRC brcBottom;          // specification of bottom border of table row
//          BRC brcRight;           // specification of right border of table row.
};
// cbTC (count of bytes of a TC) is 10(decimal), A(hex).

struct WW8_TCellVer8    // read from file
{
    SVBT16 aBits1Ver8;      // Documentation: see above at WW8_TCell
    SVBT16 aUnused;         // reserve
    WW8_BRC rgbrcVer8[4];   // Documentation: see above at WW8_TCell
};
// cbTC (count of bytes of a TC) is 20(decimal), 14(hex).

struct WW8_SHD              // struct SHD is missing from the description
{
private:
    sal_uInt16 maBits;
//  sal_uInt16 nFore : 5;       // 0x001f ForegroundColor
//  sal_uInt16 nBack : 5;       // 0x03e0 BackgroundColor
//  sal_uInt16 nStyle : 5;      // 0x7c00 Percentage and Style
//  sal_uInt16 nDontKnow : 1;   // 0x8000 ???   from Ver8: also for Style

public:
    WW8_SHD() : maBits(0) {}

    sal_uInt8 GetFore() const { return (sal_uInt8)( maBits & 0x1f); }
    sal_uInt8 GetBack() const { return (sal_uInt8)((maBits >> 5 ) & 0x1f); }
    sal_uInt8 GetStyle(bool bVer67)  const
        { return (sal_uInt8)((maBits >> 10) & ( bVer67 ? 0x1f : 0x3f ) ); }

    sal_uInt16 GetValue() const { return maBits; }

    void SetWWValue(SVBT16 const nVal) { maBits = SVBT16ToShort(nVal); }

    void SetFore(sal_uInt8 nVal)
    {
        maBits &= 0xffe0;
        maBits |= (nVal & 0x1f);
    }
    void SetBack(sal_uInt8 nVal)
    {
        maBits &= 0xfc1f;
        maBits |= (nVal & 0x1f) << 5;
    }
    void SetStyle(sal_uInt8 nVal)
    {
        maBits &= 0x03ff;
        maBits |= (nVal & 0x3f) << 10;
    }
};

struct WW8_ANLV
{
    sal_uInt8 nfc;          // 0        number format code, 0=Arabic, 1=Upper case Roman, 2=Lower case Roman
                        //          3=Upper case Letter, 4=Lower case letter, 5=Ordinal
    sal_uInt8 cbTextBefore; // 1        offset into anld.rgch limit of prefix text
    sal_uInt8 cbTextAfter;  // 2
    sal_uInt8 aBits1;
//  sal_uInt8 jc : 2;        // 3 : 0x03 justification code, 0=left, 1=center, 2=right, 3=left and right justify
//  sal_uInt8 fPrev : 1;     //     0x04 when ==1, include previous levels
//  sal_uInt8 fHang : 1;     //     0x08 when ==1, number will be displayed using a hanging indent
//  sal_uInt8 fSetBold : 1;  //     0x10 when ==1, boldness of number will be determined by anld.fBold.
//  sal_uInt8 fSetItalic : 1;//     0x20 when ==1, italicness of number will be determined by anld.fItalic
//  sal_uInt8 fSetSmallCaps : 1;//  0x40 when ==1, anld.fSmallCaps will determine whether number will be displayed in small caps or not.
//  sal_uInt8 fSetCaps : 1;  //     0x80 when ==1, anld.fCaps will determine whether number will be displayed capitalized or not
    sal_uInt8 aBits2;
//  sal_uInt8 fSetStrike : 1;// 4 : 0x01 when ==1, anld.fStrike will determine whether the number will be displayed using strikethrough or not.
//  sal_uInt8 fSetKul : 1;   //     0x02 when ==1, anld.kul will determine the underlining state of the autonumber.
//  sal_uInt8 fPrevSpace : 1;//     0x04 when ==1, autonumber will be displayed with a single prefixing space character
//  sal_uInt8 fBold : 1;     //     0x08 determines boldness of autonumber when anld.fSetBold == 1.
//  sal_uInt8 fItalic : 1;   //     0x10 determines italicness of autonumber when anld.fSetItalic == 1.
//  sal_uInt8 fSmallCaps : 1;//     0x20 determines whether autonumber will be displayed using small caps when anld.fSetSmallCaps == 1.
//  sal_uInt8 fCaps : 1;     //     0x40 determines whether autonumber will be displayed using caps when anld.fSetCaps == 1.
//  sal_uInt8 fStrike : 1;   //     0x80 determines whether autonumber will be displayed using caps when anld.fSetStrike == 1.
    sal_uInt8 aBits3;
//  sal_uInt8 kul : 3;       // 5 : 0x07 determines whether  autonumber will be displayed with underlining when anld.fSetKul == 1.
//  sal_uInt8 ico : 5;       //     0xF1 color of autonumber
    SVBT16 ftc;         // 6        font code of  autonumber
    SVBT16 hps;         // 8        font half point size (or 0=auto)
    SVBT16 iStartAt;    // 0x0a     starting value (0 to 65535)
    SVBT16 dxaIndent;   // 0x0c     *short?* *sal_uInt16?* width of prefix text (same as indent)
    SVBT16 dxaSpace;    // 0x0e     minimum space between number and paragraph
};
// *cbANLV (count of bytes of ANLV) is 16 (decimal), 10(hex).

struct WW8_ANLD
{
    WW8_ANLV eAnlv;     // 0
    sal_uInt8 fNumber1;     // 0x10     number only 1 item per table cell
    sal_uInt8 fNumberAcross;    // 0x11     number across cells in table rows(instead of down)
    sal_uInt8 fRestartHdn;  // 0x12     restart heading number on section boundary
    sal_uInt8 fSpareX;      // 0x13     unused( should be 0)
    sal_uInt8 rgchAnld[32]; // 0x14 characters displayed before/after autonumber
};

struct WW8_OLST
{
    WW8_ANLV rganlv[9]; // 0    an array of 9 ANLV structures (heading levels)
    sal_uInt8 fRestartHdr;  // 0x90 when ==1, restart heading on section break
    sal_uInt8 fSpareOlst2;  // 0x91 reserved
    sal_uInt8 fSpareOlst3;  // 0x92 reserved
    sal_uInt8 fSpareOlst4;  // 0x93 reserved
    sal_uInt8 rgch[64];      // 0x94 array of 64 chars       text before/after number
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
    sal_uInt8  bx;              // 4    x position relative to anchor CP
    sal_uInt8  by;              // 5    y position relative to anchor CP

    /*
     bx and by above are apparently better described by this info from the rtf standard...

    \dobxpage   The drawing object is page relative in the x-direction.
    \dobxcolumn The drawing object is column relative in the x-direction.
    \dobxmargin The drawing object is margin relative in the x-direction.

    \dobypage   The drawing object is page relative in the y-direction.
    \dobypara   The drawing object is paragraph relative in the y-direction.
    \dobymargin The drawing object is margin relative in the y-direction.

    */

    SVBT16 dhgt;                // 6    height of DO
    SVBT16 aBits1;
//  sal_uInt16 fAnchorLock : 1; // 8    1 if the DO anchor is locked
//  sal_uInt8[] rgdp;            // 0xa  variable length array of drawing primitives
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

struct WW8_DP_SHADOW    // shading!
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
//  sal_uInt16 eppsStart : 2;   // Start EndPoint Property Style
                            // 0=None, 1=Hollow, 2=Filled
//  sal_uInt16 eppwStart : 2;   // Start EndPoint Property Weight
//  sal_uInt16 epplStart : 2;   // Start EndPoint Property length
//  sal_uInt16 dummyStart : 10; // Alignment
    SVBT16 aEndBits;
//  sal_uInt16 eppsEnd : 2;     // End EndPoint Property Style
//  sal_uInt16 eppwEnd : 2;     // End EndPoint Property Weight
//  sal_uInt16 epplEnd : 2;     // End EndPoint Property length
//  sal_uInt16 dummyEnd : 10;   // Alignment
};

struct WW8_DP_LINE
{
//  WW8_DPHEAD dphead;      // 0    Common header for a drawing primitive
    SVBT16 xaStart;         // starting point for line
    SVBT16 yaStart;
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
//  sal_uInt16 fRoundCorners : 1; //0x24    0001    1 if the textbox has rounded corners
//  sal_uInt16 zaShape : 15;    // 0x24     000e    REVIEW davebu
    SVBT16 dzaInternalMargin; // 0x26   REVIEW davebu
};

struct WW8_DP_RECT
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_SHADOW aShd;
    SVBT16 aBits1;
//  sal_uInt16 fRoundCorners : 1; // 0x24   0001    1 if the textbox has rounded corners
//  sal_uInt16 zaShape : 15; // 0x24 000e   REVIEW davebu
};

struct WW8_DP_ARC
{
    WW8_DP_LINETYPE aLnt;
    WW8_DP_FILL aFill;
    WW8_DP_SHADOW aShd;
    sal_uInt8 fLeft;        // 0x24 00ff    REVIEW davebu
    sal_uInt8 fUp;          // 0x24 ff00    REVIEW davebu
//  sal_uInt16 fLeft : 8;   // 0x24 00ff    REVIEW davebu
//  sal_uInt16 fUp : 8;     // 0x24 ff00    REVIEW davebu
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
//  sal_uInt16 fPolygon : 1; // 0x28  0001  1 if this is a polygon
//  sal_uInt16 cpt : 15;    // 0x28   00fe  count of points
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
    sal_uInt8 aBits1;
//  sal_uInt8 fNoParaLast : 1;   // when 1, means that piece contains no end of paragraph marks.
//  sal_uInt8 fPaphNil : 1;      // used internally by Word
//  sal_uInt8 fCopied : 1;       // used internally by Word
//          *   int :5
    sal_uInt8 aBits2;           // fn int:8, used internally by Word
    SVBT32 fc;              // file offset of beginning of piece. The size of the
                            // ithpiece can be determined by subtracting rgcp[i] of
                            // the containing plcfpcd from its rgcp[i+1].
    SVBT16 prm;             // PRM contains either a single sprm or else an index number
                            // of the grpprl which contains the sprms that modify the
                            // properties of the piece.
};

// AnnoTation References Descriptor (ATRD)
struct WW8_ATRD                 // for version 8
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

struct WW8_ATRDEXTRA
{
    // ---  Extended bit since Word 2002 ---

    SVBT32 dttm;
    SVBT16 bf;
    SVBT32 cDepth;
    SVBT32 diatrdParent;
    SVBT32 Discussitem;
};

struct WW67_ATRD                // for versions 6/7
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
    sal_Int16 nSp26;
    sal_Int16 nSp27;
    sal_Int16 nLeMgn;
    sal_Int16 nRiMgn;
    sal_Int16 nUpMgn;
    sal_Int16 nLoMgn;
    sal_uInt8 nSp29;
    sal_uInt8 nSp37;
    bool bNoFly;
};

struct WW8_FSPA
{
public:
    sal_Int32 nSpId;     //Shape Identifier. Used in conjunction with the office art data (found via fcDggInfo in the FIB) to find the actual data for this shape.
    sal_Int32 nXaLeft;   //left of rectangle enclosing shape relative to the origin of the shape
    sal_Int32 nYaTop;        //top of rectangle enclosing shape relative to the origin of the shape
    sal_Int32 nXaRight;  //right of rectangle enclosing shape relative to the origin of the shape
    sal_Int32 nYaBottom;//bottom of the rectangle enclosing shape relative to the origin of the shape
    sal_uInt16 bHdr:1;
    //0001 1 in the undo doc when shape is from the header doc, 0 otherwise (undefined when not in the undo doc)
    sal_uInt16 nbx:2;
    //0006 x position of shape relative to anchor CP
    //0 relative to page margin
    //1 relative to top of page
    //2 relative to text (column for horizontal text; paragraph for vertical text)
    //3 reserved for future use
    sal_uInt16 nby:2;
    //0018 y position of shape relative to anchor CP
    //0 relative to page margin
    //1 relative to top of page
    //2 relative to text (paragraph for horizontal text; column for vertical text)
    sal_uInt16 nwr:4;
    //01E0 text wrapping mode
    //0 like 2, but doesn't require absolute object
    //1 no text next to shape
    //2 wrap around absolute object
    //3 wrap as if no object present
    //4 wrap tightly around object
    //5 wrap tightly, but allow holes
    //6-15 reserved for future use
    sal_uInt16 nwrk:4;
    //1E00 text wrapping mode type (valid only for wrapping modes 2 and 4
    //0 wrap both sides
    //1 wrap only on left
    //2 wrap only on right
    //3 wrap only on largest side
    sal_uInt16 bRcaSimple:1;
    //2000 when set, temporarily overrides bx, by, forcing the xaLeft, xaRight, yaTop, and yaBottom fields to all be page relative.
    sal_uInt16 bBelowText:1;
    //4000
    //1 shape is below text
    //0 shape is above text
    sal_uInt16 bAnchorLock:1;
    //8000  1 anchor is locked
    //      0 anchor is not locked
    sal_Int32 nTxbx; //count of textboxes in shape (undo doc only)
public:
    enum FSPAOrient {RelPgMargin, RelPageBorder, RelText};
};

struct WW8_FSPA_SHADOW  // all members at same position and size
{                                               // due to:  pF = (WW8_FSPA*)pFS;
    SVBT32 nSpId;
    SVBT32 nXaLeft;
    SVBT32 nYaTop;
    SVBT32 nXaRight;
    SVBT32 nYaBottom;
    SVBT16 aBits1;
    SVBT32 nTxbx;
};

static_assert(sizeof (WW8_FSPA_SHADOW) == 26, "this has to match the msword size");
    // "26": cf. WW8ScannerBase ctor case 8 creation of pMainFdoa and pHdFtFdoa

struct WW8_TXBXS
{
    SVBT32 cTxbx_iNextReuse;
    SVBT32 cReusable;
    SVBT16 fReusable;
    SVBT32 reserved;
    SVBT32 ShapeId;
    SVBT32 txidUndo;
};

struct WW8_STRINGID
{
    // M.M. This is the extra data stored in the SttbfFnm
    // For now I only need the String Id
    SVBT16 nStringId;
    SVBT16 reserved1;
    SVBT16 reserved2;
    SVBT16 reserved3;
};

struct WW8_WKB
{
    // M.M. This is the WkbPLCF struct
    // For now I only need the Link Id
    SVBT16 reserved1;
    SVBT16 reserved2;
    SVBT16 reserved3;
    SVBT16 nLinkId;
    SVBT16 reserved4;
    SVBT16 reserved5;
};

#ifdef SAL_W32
#   pragma pack(pop)
#endif

// Maximum number of columns according the WW8 specification
static const sal_uInt8 MAX_NO_OF_SEP_COLUMNS = 44;

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
/*
    28  1C  brcTop                    BRC                   top page border

    32  20  brcLeft                   BRC                   left page border

    36  24  brcBottom                 BRC                   bottom page border

    40  28  brcRight                  BRC                   right page border
*/
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
    sal_Int16 ccolM1;   // have to be less than MAX_NO_OF_SEP_COLUMNS according the WW8 specification
    sal_Int8 fEvenlySpaced;
    sal_Int8 reserved3;
    sal_uInt8 fBiDi;
    sal_uInt8 fFacingCol;
    sal_uInt8 fRTLGutter;
    sal_uInt8 fRTLAlignment;
    sal_Int32 dxaColumns;

    // Fixed array - two entries for each SEP column to store width of column and spacing to next column.
    // At odd index values [1,3,5,...] the column widths are stored.
    // At even index values [2,4,6,...] the spacings to the next columns are stored.
    // Value at index 0 is initialized with 0 and used for easier interation on the array
    sal_Int32 rgdxaColumnWidthSpacing[MAX_NO_OF_SEP_COLUMNS*2 + 1];

    sal_Int32 dxaColumnWidth;
    sal_uInt8 dmOrientFirst;
    sal_uInt8 fLayout;
    sal_Int16 reserved4;
};

namespace wwUtility
{
    inline sal_uInt32 RGBToBGR(sal_uInt32 nColour) { return msfilter::util::BGRToRGB(nColour); }
}

/// [MS-OSHARED] FactoidType: one smart tag type.
class MSOFactoidType
{
public:
    MSOFactoidType();
    void Read(SvStream& rStream);
    void Write(WW8Export& rExport);

    sal_uInt32 m_nId;
    OUString m_aUri;
    OUString m_aTag;
};

/// [MS-OSHARED] PropertyBagStore: smart tag types and string store.
class MSOPropertyBagStore
{
public:
    void Read(SvStream& rStream);
    void Write(WW8Export& rExport);

    std::vector<MSOFactoidType> m_aFactoidTypes;
    std::vector<OUString> m_aStringTable;
};

/// [MS-OSHARED] Property: stores information about one smart-tag key/value.
class MSOProperty
{
public:
    MSOProperty();
    void Read(SvStream& rStream);
    void Write(SvStream& rStream);

    /// Index into MSOPropertyBagStore::m_aStringTable.
    sal_uInt32 m_nKey;
    /// Index into MSOPropertyBagStore::m_aStringTable.
    sal_uInt32 m_nValue;
};

/// [MS-OSHARED] PropertyBag: stores information about one smart tag.
class MSOPropertyBag
{
public:
    MSOPropertyBag();
    void Read(SvStream& rStream);
    void Write(WW8Export& rExport);

    /// Matches MSOFactoidType::m_nId in MSOPropertyBagStore::m_aFactoidTypes.
    sal_uInt16 m_nId;
    std::vector<MSOProperty> m_aProperties;
};

/// [MS-DOC] SmartTagData: stores information about all smart tags in the document.
class WW8SmartTagData
{
public:
    void Read(SvStream& rStream, WW8_FC fcFactoidData, sal_uInt32 lcbFactoidData);
    void Write(WW8Export& rExport);

    MSOPropertyBagStore m_aPropBagStore;
    std::vector<MSOPropertyBag> m_aPropBags;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
