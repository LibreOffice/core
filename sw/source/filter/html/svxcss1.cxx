/*************************************************************************
 *
 *  $RCSfile: svxcss1.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-08 10:42:05 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <stdlib.h>

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _HINTIDS_HXX
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#define ITEMID_FONT         SID_ATTR_CHAR_FONT
#define ITEMID_POSTURE      SID_ATTR_CHAR_POSTURE
#define ITEMID_WEIGHT       SID_ATTR_CHAR_WEIGHT
#define ITEMID_FONTHEIGHT   SID_ATTR_CHAR_FONTHEIGHT
#define ITEMID_UNDERLINE    SID_ATTR_CHAR_UNDERLINE
#define ITEMID_CROSSEDOUT   SID_ATTR_CHAR_STRIKEOUT
#define ITEMID_COLOR        SID_ATTR_CHAR_COLOR
#define ITEMID_KERNING      SID_ATTR_CHAR_KERNING
#define ITEMID_CASEMAP      SID_ATTR_CHAR_CASEMAP
#define ITEMID_BLINK        SID_ATTR_FLASH

#define ITEMID_LINESPACING  SID_ATTR_PARA_LINESPACE
#define ITEMID_ADJUST       SID_ATTR_PARA_ADJUST
#define ITEMID_WIDOWS       SID_ATTR_PARA_WIDOWS
#define ITEMID_ORPHANS      SID_ATTR_PARA_ORPHANS
#define ITEMID_FMTSPLIT     SID_ATTR_PARA_SPLIT

#define ITEMID_LRSPACE SID_ATTR_LRSPACE
#define ITEMID_ULSPACE SID_ATTR_ULSPACE
#define ITEMID_BOX SID_ATTR_BORDER_OUTER
#define ITEMID_BRUSH SID_ATTR_BRUSH
#endif

#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif

#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX //autogen
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_ORPHITEM_HXX //autogen
#include <svx/orphitem.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVPARSER_HXX //autogen
#include <svtools/svparser.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#include "css1kywd.hxx"
#include "svxcss1.hxx"

// die Funktionen zum Parsen einer CSS1-Property sind von folgendem Typ:
typedef void (*FnParseCSS1Prop)( const CSS1Expression *pExpr,
                                 SfxItemSet& rItemSet,
                                 SvxCSS1PropertyInfo& rPropInfo,
                                 const SvxCSS1Parser& rParser );

SV_IMPL_PTRARR( CSS1Selectors, CSS1Selector* )


/*  */

static CSS1PropertyEnum __READONLY_DATA aFontSizeTable[] =
{
    { sCSS1_PV_xx_small,    0                   },
    { sCSS1_PV_x_small,     1                   },
    { sCSS1_PV_small,       2                   },
    { sCSS1_PV_medium,      3                   },
    { sCSS1_PV_large,       4                   },
    { sCSS1_PV_x_large,     5                   },
    { sCSS1_PV_xx_large,    6                   },
    { 0,                    0                   }
};

static CSS1PropertyEnum __READONLY_DATA aFontFamilyTable[] =
{
    { sCSS1_PV_serif,       FAMILY_ROMAN        },
    { sCSS1_PV_sans_serif,  FAMILY_SWISS        },
    { sCSS1_PV_cursive,     FAMILY_SCRIPT       },
    { sCSS1_PV_fantasy,     FAMILY_DECORATIVE   },
    { sCSS1_PV_monospace,   FAMILY_MODERN       },
    { 0,                    0                   }
};

static CSS1PropertyEnum __READONLY_DATA aFontWeightTable[] =
{
    { sCSS1_PV_extra_light, WEIGHT_NORMAL       }, // WEIGHT_ULTRALIGHT (OBS)
    { sCSS1_PV_light,       WEIGHT_NORMAL       }, // WEIGHT_LIGHT (OBSOLETE)
    { sCSS1_PV_demi_light,  WEIGHT_NORMAL       }, // WEIGHT_SEMILIGHT (OBS)
    { sCSS1_PV_medium,      WEIGHT_NORMAL       }, // WEIGHT_MEDIUM (OBS)
    { sCSS1_PV_normal,      WEIGHT_NORMAL       }, // WEIGHT_MEDIUM
    { sCSS1_PV_demi_bold,   WEIGHT_NORMAL       }, // WEIGHT_SEMIBOLD (OBS)
    { sCSS1_PV_bold,        WEIGHT_BOLD         }, // WEIGHT_BOLD (OBSOLETE)
    { sCSS1_PV_extra_bold,  WEIGHT_BOLD         }, // WEIGHT_ULTRABOLD (OBS)
    { sCSS1_PV_bolder,      WEIGHT_BOLD         },
    { sCSS1_PV_lighter,     WEIGHT_NORMAL       },
    { 0,                    0                   }
};

static CSS1PropertyEnum __READONLY_DATA aFontStyleTable[] =
{
    { sCSS1_PV_normal,      ITALIC_NONE         },
    { sCSS1_PV_italic,      ITALIC_NORMAL       },
    { sCSS1_PV_oblique,     ITALIC_NORMAL       },
    { 0,                    0                   }
};

static CSS1PropertyEnum __READONLY_DATA aFontVariantTable[] =
{
    { sCSS1_PV_normal,      SVX_CASEMAP_NOT_MAPPED      },
    { sCSS1_PV_small_caps,  SVX_CASEMAP_KAPITAELCHEN    },
    { 0,                    0                   }
};

/*  */

static CSS1PropertyEnum __READONLY_DATA aBGRepeatTable[] =
{
    { sCSS1_PV_repeat,      GPOS_TILED                  },
    { sCSS1_PV_repeat_x,    GPOS_TILED                  },
    { sCSS1_PV_repeat_y,    GPOS_TILED                  },
    { sCSS1_PV_no_repeat,   GPOS_NONE                   },
    { 0,                    0                           }
};

static CSS1PropertyEnum __READONLY_DATA aBGHoriPosTable[] =
{
    { sCSS1_PV_left,        GPOS_LT                 },
    { sCSS1_PV_center,      GPOS_MT                 },
    { sCSS1_PV_right,       GPOS_RT                 },
    { 0,                    0                       }
};

static CSS1PropertyEnum __READONLY_DATA aBGVertPosTable[] =
{
    { sCSS1_PV_top,         GPOS_LT                 },
    { sCSS1_PV_middle,      GPOS_LM                 },
    { sCSS1_PV_bottom,      GPOS_LB                 },
    { 0,                    0                       }
};

/*  */

static CSS1PropertyEnum __READONLY_DATA aTextAlignTable[] =
{
    { sCSS1_PV_left,        SVX_ADJUST_LEFT     },
    { sCSS1_PV_center,      SVX_ADJUST_CENTER   },
    { sCSS1_PV_right,       SVX_ADJUST_RIGHT    },
    { sCSS1_PV_justify,     SVX_ADJUST_BLOCK    },
    { 0,                    0                   }
};

/*  */

static CSS1PropertyEnum __READONLY_DATA aBorderWidthTable[] =
{
    { sCSS1_PV_thin,        0   },  // DEF_LINE_WIDTH_0 / DEF_DOUBLE_LINE0
    { sCSS1_PV_medium,      1   },  // DEF_LINE_WIDTH_1 / DEF_DOUBLE_LINE1
    { sCSS1_PV_thick,       2   },  // DEF_LINE_WIDTH_2 / DEF_DOUBLE_LINE2
    { 0,                    0   }
};

enum CSS1BorderStyle { CSS1_BS_NONE, CSS1_BS_SINGLE, CSS1_BS_DOUBLE };

static CSS1PropertyEnum __READONLY_DATA aBorderStyleTable[] =
{
    { sCSS1_PV_none,        CSS1_BS_NONE        },
    { sCSS1_PV_dotted,      CSS1_BS_SINGLE      },
    { sCSS1_PV_dashed,      CSS1_BS_SINGLE      },
    { sCSS1_PV_solid,       CSS1_BS_SINGLE      },
    { sCSS1_PV_double,      CSS1_BS_DOUBLE      },
    { sCSS1_PV_groove,      CSS1_BS_SINGLE      },
    { sCSS1_PV_ridge,       CSS1_BS_SINGLE      },
    { sCSS1_PV_inset,       CSS1_BS_SINGLE      },
    { sCSS1_PV_outset,      CSS1_BS_SINGLE      },
    { 0,                    0                   }
};

static CSS1PropertyEnum __READONLY_DATA aFloatTable[] =
{
    { sCSS1_PV_left,    SVX_ADJUST_LEFT         },
    { sCSS1_PV_right,   SVX_ADJUST_RIGHT        },
    { sCSS1_PV_none,    SVX_ADJUST_END          },
    { 0,                0                       }
};

static CSS1PropertyEnum __READONLY_DATA aPositionTable[] =
{
    { sCSS1_PV_absolute,    SVX_CSS1_POS_ABSOLUTE   },
    { sCSS1_PV_relative,    SVX_CSS1_POS_RELATIVE   },
    { sCSS1_PV_static,      SVX_CSS1_POS_STATIC     },
    { 0,                    0                       }
};

// Feature: PrintExt
static CSS1PropertyEnum __READONLY_DATA aSizeTable[] =
{
    { sCSS1_PV_auto,        SVX_CSS1_STYPE_AUTO         },
    { sCSS1_PV_landscape,   SVX_CSS1_STYPE_LANDSCAPE    },
    { sCSS1_PV_portrait,    SVX_CSS1_STYPE_PORTRAIT     },
    { 0,                    0                           }
};

static CSS1PropertyEnum __READONLY_DATA aPageBreakTable[] =
{
    { sCSS1_PV_auto,        SVX_CSS1_PBREAK_AUTO        },
    { sCSS1_PV_always,      SVX_CSS1_PBREAK_ALWAYS      },
    { sCSS1_PV_avoid,       SVX_CSS1_PBREAK_AVOID       },
    { sCSS1_PV_left,        SVX_CSS1_PBREAK_LEFT        },
    { sCSS1_PV_right,       SVX_CSS1_PBREAK_RIGHT       },
    { 0,                    0                           }
};

// /Feature: PrintExt

/*  */

// Ein Eintrag besteht aus vier USHORTs. Der erste ist die Gesamtbreite,
// die anderen sind die 3 Einzelbreiten

#define SBORDER_ENTRY( n ) \
    DEF_LINE_WIDTH_##n, DEF_LINE_WIDTH_##n, 0, 0

#define DBORDER_ENTRY( n ) \
    DEF_DOUBLE_LINE##n##_OUT + DEF_DOUBLE_LINE##n##_IN + \
    DEF_DOUBLE_LINE##n##_DIST, \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_IN, \
    DEF_DOUBLE_LINE##n##_DIST

#define TDBORDER_ENTRY( n ) \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_IN, \
    DEF_DOUBLE_LINE##n##_DIST


static USHORT __READONLY_DATA aSBorderWidths[] =
{
    SBORDER_ENTRY( 0 ), SBORDER_ENTRY( 1 ), SBORDER_ENTRY( 2 ),
    SBORDER_ENTRY( 3 ), SBORDER_ENTRY( 4 )
};

static USHORT __READONLY_DATA aDBorderWidths[] =
{
    DBORDER_ENTRY( 0 ),
    DBORDER_ENTRY( 7 ),
    DBORDER_ENTRY( 1 ),
    DBORDER_ENTRY( 8 ),
    DBORDER_ENTRY( 4 ),
    DBORDER_ENTRY( 9 ),
    DBORDER_ENTRY( 3 ),
    DBORDER_ENTRY( 10 ),
    DBORDER_ENTRY( 2 ),
    DBORDER_ENTRY( 5 )
};

static USHORT __READONLY_DATA aTDBorderWidths[] =
{
    TDBORDER_ENTRY( 7 ), TDBORDER_ENTRY( 8 ), TDBORDER_ENTRY( 9 ),
    TDBORDER_ENTRY( 10 )
};

#undef SBORDER_ENTRY
#undef DBORDER_ENTRY

/*  */

struct SvxCSS1ItemIds
{
    USHORT nFont;
    USHORT nPosture;
    USHORT nWeight;
    USHORT nFontHeight;
    USHORT nUnderline;
    USHORT nCrossedOut;
    USHORT nColor;
    USHORT nKerning;
    USHORT nCaseMap;
    USHORT nBlink;

    USHORT nLineSpacing;
    USHORT nAdjust;
    USHORT nWidows;
    USHORT nOrphans;
    USHORT nFmtSplit;

    USHORT nLRSpace;
    USHORT nULSpace;
    USHORT nBox;
    USHORT nBrush;
};


static SvxCSS1ItemIds aItemIds;


/*  */

struct SvxCSS1BorderInfo
{
    Color aColor;
    USHORT nAbsWidth;
    USHORT nNamedWidth;
    CSS1BorderStyle eStyle;

    SvxCSS1BorderInfo() :
        aColor( COL_BLACK ), nAbsWidth( USHRT_MAX ),
        nNamedWidth( USHRT_MAX ), eStyle( CSS1_BS_NONE )
    {}

    SvxCSS1BorderInfo( const SvxCSS1BorderInfo& rInfo ) :
        aColor( rInfo.aColor ), nAbsWidth( rInfo.nAbsWidth ),
        nNamedWidth( rInfo.nNamedWidth ), eStyle( rInfo.eStyle )
    {}

    void SetBorderLine( USHORT nLine, SvxBoxItem &rBoxItem ) const;
};

void SvxCSS1BorderInfo::SetBorderLine( USHORT nLine, SvxBoxItem &rBoxItem ) const
{
    if( CSS1_BS_NONE==eStyle || nAbsWidth==0 ||
        (nAbsWidth==USHRT_MAX && nNamedWidth==USHRT_MAX) )
    {
        rBoxItem.SetLine( 0, nLine );
        return;
    }

    SvxBorderLine aBorderLine( &aColor );

    // Linien-Stil doppelt oder einfach?
    BOOL bDouble = eStyle == CSS1_BS_DOUBLE;

    // benannte Breite umrechnenen, wenn keine absolute gegeben ist
    if( nAbsWidth==USHRT_MAX )
    {
        const USHORT *aWidths = bDouble ? aDBorderWidths : aSBorderWidths;
        USHORT nNWidth = nNamedWidth * 4;
        aBorderLine.SetOutWidth( aWidths[nNWidth+1] );
        aBorderLine.SetInWidth( aWidths[nNWidth+2] );
        aBorderLine.SetDistance( aWidths[nNWidth+3] );
    }
    else
    {
        SvxCSS1Parser::SetBorderWidth( aBorderLine, nAbsWidth, bDouble );
    }

    rBoxItem.SetLine( &aBorderLine, nLine );
}


/*  */

SvxCSS1PropertyInfo::SvxCSS1PropertyInfo()
{
    for( USHORT i=0; i<4; i++ )
        aBorderInfos[i] = 0;

    Clear();
}

SvxCSS1PropertyInfo::SvxCSS1PropertyInfo( const SvxCSS1PropertyInfo& rProp ) :
    aId( rProp.aId ),
    bTopMargin( rProp.bTopMargin ), bBottomMargin( rProp.bBottomMargin ),
    bLeftMargin( rProp.bLeftMargin ), bRightMargin( rProp.bRightMargin ),
    bTextIndent( rProp.bTextIndent ),
    nLeftMargin( rProp.nLeftMargin ), nRightMargin( rProp.nRightMargin ),
    eFloat( rProp.eFloat ),
    ePosition( rProp.ePosition ),
    nTopBorderDistance( rProp.nTopBorderDistance ),
    nBottomBorderDistance( rProp.nBottomBorderDistance ),
    nLeftBorderDistance( rProp.nLeftBorderDistance ),
    nRightBorderDistance( rProp.nRightBorderDistance ),
    nLeft( rProp.nLeft ), nTop( rProp.nTop ),
    nWidth( rProp.nWidth ), nHeight( rProp.nHeight ),
    eLeftType( rProp.eLeftType ), eTopType( rProp.eTopType ),
    eWidthType( rProp.eWidthType ), eHeightType( rProp.eHeightType ),
// Feature: PrintExt
    eSizeType( rProp.eSizeType ),
    ePageBreakBefore( rProp.ePageBreakBefore ),
    ePageBreakAfter( rProp.ePageBreakAfter )
// /Feature: PrintExt
{
    for( USHORT i=0; i<4; i++ )
        aBorderInfos[i] = rProp.aBorderInfos[i]
                            ? new SvxCSS1BorderInfo( *rProp.aBorderInfos[i] )
                            : 0;
}

SvxCSS1PropertyInfo::~SvxCSS1PropertyInfo()
{
    DestroyBorderInfos();
}

void SvxCSS1PropertyInfo::DestroyBorderInfos()
{
    for( USHORT i=0; i<4; i++ )
    {
        delete aBorderInfos[i];
        aBorderInfos[i] = 0;
    }
}

void SvxCSS1PropertyInfo::Clear()
{
    aId.Erase();
    bTopMargin = bBottomMargin = FALSE;
    bLeftMargin = bRightMargin = bTextIndent = FALSE;
    nLeftMargin = nRightMargin = 0;
    eFloat = SVX_ADJUST_END;

    ePosition = SVX_CSS1_POS_NONE;
    nTopBorderDistance = nBottomBorderDistance =
    nLeftBorderDistance = nRightBorderDistance = USHRT_MAX;
    nLeft = nTop = nWidth = nHeight = 0;
    eLeftType = eTopType = eWidthType = eHeightType = SVX_CSS1_LTYPE_NONE;

// Feature: PrintExt
    eSizeType = SVX_CSS1_STYPE_NONE;
    ePageBreakBefore = SVX_CSS1_PBREAK_NONE;
    ePageBreakAfter = SVX_CSS1_PBREAK_NONE;

    DestroyBorderInfos();
}

void SvxCSS1PropertyInfo::Merge( const SvxCSS1PropertyInfo& rProp )
{
    if( rProp.bTopMargin )
        bTopMargin = TRUE;
    if( rProp.bBottomMargin )
        bBottomMargin = TRUE;

    if( rProp.bLeftMargin )
    {
        bLeftMargin = TRUE;
        nLeftMargin = rProp.nLeftMargin;
    }
    if( rProp.bRightMargin )
    {
        bRightMargin = TRUE;
        nRightMargin = rProp.nRightMargin;
    }
    if( rProp.bTextIndent )
        bTextIndent = TRUE;

    for( USHORT i=0; i<4; i++ )
    {
        if( rProp.aBorderInfos[i] )
        {
            if( aBorderInfos[i] )
                delete aBorderInfos[i];

            aBorderInfos[i] = new SvxCSS1BorderInfo( *rProp.aBorderInfos[i] );
        }
    }

    if( USHRT_MAX != rProp.nTopBorderDistance )
        nTopBorderDistance = rProp.nTopBorderDistance;
    if( USHRT_MAX != rProp.nBottomBorderDistance )
        nBottomBorderDistance = rProp.nBottomBorderDistance;
    if( USHRT_MAX != rProp.nLeftBorderDistance )
        nLeftBorderDistance = rProp.nLeftBorderDistance;
    if( USHRT_MAX != rProp.nRightBorderDistance )
        nRightBorderDistance = rProp.nRightBorderDistance;

    if( rProp.eFloat != SVX_ADJUST_END )
        eFloat = rProp.eFloat;

    if( rProp.ePosition != SVX_CSS1_POS_NONE )
        ePosition = rProp.ePosition;

// Feature: PrintExt
    if( rProp.eSizeType != SVX_CSS1_STYPE_NONE )
    {
        eSizeType = rProp.eSizeType;
        nWidth = rProp.nWidth;
        nHeight = rProp.nHeight;
    }

    if( rProp.ePageBreakBefore != SVX_CSS1_PBREAK_NONE )
        ePageBreakBefore = rProp.ePageBreakBefore;

    if( rProp.ePageBreakAfter != SVX_CSS1_PBREAK_NONE )
        ePageBreakAfter = rProp.ePageBreakAfter;

// /Feature: PrintExt

    if( rProp.eLeftType != SVX_CSS1_LTYPE_NONE )
    {
        eLeftType = rProp.eLeftType;
        nLeft = rProp.nLeft;
    }

    if( rProp.eTopType != SVX_CSS1_LTYPE_NONE )
    {
        eTopType = rProp.eTopType;
        nTop = rProp.nTop;
    }

    if( rProp.eWidthType != SVX_CSS1_LTYPE_NONE )
    {
        eWidthType = rProp.eWidthType;
        nWidth = rProp.nWidth;
    }

    if( rProp.eHeightType != SVX_CSS1_LTYPE_NONE )
    {
        eHeightType = rProp.eHeightType;
        nHeight = rProp.nHeight;
    }
}

SvxCSS1BorderInfo *SvxCSS1PropertyInfo::GetBorderInfo( USHORT nLine, BOOL bCreate )
{
    USHORT nPos = 0;
    switch( nLine )
    {
    case BOX_LINE_TOP:      nPos = 0;   break;
    case BOX_LINE_BOTTOM:   nPos = 1;   break;
    case BOX_LINE_LEFT:     nPos = 2;   break;
    case BOX_LINE_RIGHT:    nPos = 3;   break;
    }

    if( !aBorderInfos[nPos] && bCreate )
        aBorderInfos[nPos] = new SvxCSS1BorderInfo;

    return aBorderInfos[nPos];
}

void SvxCSS1PropertyInfo::CopyBorderInfo( USHORT nSrcLine, USHORT nDstLine,
                                          USHORT nWhat )
{
    SvxCSS1BorderInfo *pSrcInfo = GetBorderInfo( nSrcLine, FALSE );
    if( !pSrcInfo )
        return;

    SvxCSS1BorderInfo *pDstInfo = GetBorderInfo( nDstLine );
    if( (nWhat & SVX_CSS1_BORDERINFO_WIDTH) != 0 )
    {
        pDstInfo->nAbsWidth = pSrcInfo->nAbsWidth;
        pDstInfo->nNamedWidth = pSrcInfo->nNamedWidth;
    }

    if( (nWhat & SVX_CSS1_BORDERINFO_COLOR) != 0 )
        pDstInfo->aColor = pSrcInfo->aColor;

    if( (nWhat & SVX_CSS1_BORDERINFO_STYLE) != 0 )
        pDstInfo->eStyle = pSrcInfo->eStyle;
}

void SvxCSS1PropertyInfo::CopyBorderInfo( USHORT nCount, USHORT nWhat )
{
    if( nCount==0 )
    {
        CopyBorderInfo( BOX_LINE_BOTTOM, BOX_LINE_TOP, nWhat );
        CopyBorderInfo( BOX_LINE_TOP, BOX_LINE_LEFT, nWhat );
    }
    if( nCount<=1 )
    {
        CopyBorderInfo( BOX_LINE_LEFT, BOX_LINE_RIGHT, nWhat );
    }
}

void SvxCSS1PropertyInfo::SetBoxItem( SfxItemSet& rItemSet,
                                      USHORT nMinBorderDist,
                                      const SvxBoxItem *pDfltItem,
                                      BOOL bTable )
{
    BOOL bChg = nTopBorderDistance != USHRT_MAX ||
                nBottomBorderDistance != USHRT_MAX ||
                nLeftBorderDistance != USHRT_MAX ||
                nRightBorderDistance != USHRT_MAX;
    for( USHORT i=0; !bChg && i<4; i++ )
        bChg = aBorderInfos[i]!=0;

    if( !bChg )
        return;

    SvxBoxItem aBoxItem( aItemIds.nBox );
    if( pDfltItem )
        aBoxItem = *pDfltItem;

    SvxCSS1BorderInfo *pInfo = GetBorderInfo( BOX_LINE_TOP, FALSE );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_TOP, aBoxItem );

    pInfo = GetBorderInfo( BOX_LINE_BOTTOM, FALSE );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_BOTTOM, aBoxItem );

    pInfo = GetBorderInfo( BOX_LINE_LEFT, FALSE );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_LEFT, aBoxItem );

    pInfo = GetBorderInfo( BOX_LINE_RIGHT, FALSE );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_RIGHT, aBoxItem );

    for( i=0; i<4; i++ )
    {
        USHORT nLine, nDist;
        switch( i )
        {
        case 0: nLine = BOX_LINE_TOP;
                nDist = nTopBorderDistance;
                nTopBorderDistance = USHRT_MAX;
                break;
        case 1: nLine = BOX_LINE_BOTTOM;
                nDist = nBottomBorderDistance;
                nBottomBorderDistance = USHRT_MAX;
                break;
        case 2: nLine = BOX_LINE_LEFT;
                nDist = nLeftBorderDistance;
                nLeftBorderDistance = USHRT_MAX;
                break;
        case 3: nLine = BOX_LINE_RIGHT;
                nDist = nRightBorderDistance;
                nRightBorderDistance = USHRT_MAX;
                break;
        }

        if( aBoxItem.GetLine( nLine ) )
        {
            if( USHRT_MAX == nDist )
                nDist = aBoxItem.GetDistance( nLine );

            if( nDist < nMinBorderDist )
                nDist = nMinBorderDist;
        }
        else
        {
            if( USHRT_MAX == nDist )
                nDist = aBoxItem.GetDistance( nLine );

            if( !bTable )
                nDist = 0U;
            else if( nDist && nDist < nMinBorderDist )
                nDist = nMinBorderDist;
        }

        aBoxItem.SetDistance( nDist, nLine );
    }

    rItemSet.Put( aBoxItem );

    DestroyBorderInfos();
}


/*  */

SvxCSS1MapEntry::SvxCSS1MapEntry( const String& rKey, const SfxItemSet& rItemSet,
                                  const SvxCSS1PropertyInfo& rProp ) :
    aKey( rKey ),
    aItemSet( rItemSet ),
    aPropInfo( rProp )
{
    // TODO: ToUpperAscii
    aKey.ToUpperAscii();
}

#if defined( MAC ) || defined( ICC ) || defined( BLC )
BOOL operator==( const SvxCSS1MapEntry& rE1, const SvxCSS1MapEntry& rE2 )
{
    return  rE1.aKey==rE2.aKey;
}

BOOL operator<( const SvxCSS1MapEntry& rE1, const SvxCSS1MapEntry& rE2 )
{
    return  rE1.aKey<rE2.aKey;
}
#endif

SV_IMPL_OP_PTRARR_SORT( SvxCSS1Map, SvxCSS1MapEntryPtr )

/*  */

BOOL SvxCSS1Parser::StyleParsed( const CSS1Selector *pSelector,
                                 SfxItemSet& rItemSet,
                                 SvxCSS1PropertyInfo& rPropInfo )
{
    // wie man sieht passiert hier gar nichts
    return TRUE;
}

BOOL SvxCSS1Parser::SelectorParsed( const CSS1Selector *pSelector,
                                    BOOL bFirst )
{
    if( bFirst )
    {
        DBG_ASSERT( pSheetItemSet, "Wo ist der Item-Set fuer Style-Sheets?" );

        // Dieses ist der erste Selektor einer Rule, also muessen
        // die bisher geparsten Items auf die Styles verteilt werden
//      pSheetPropInfo->CreateBoxItem( *pSheetItemSet, GetDfltBorderDist() );
        for( USHORT i=0; i<aSelectors.Count(); i++ )
        {
            StyleParsed( aSelectors[i], *pSheetItemSet, *pSheetPropInfo );
        }
        pSheetItemSet->ClearItem();
        pSheetPropInfo->Clear();

        // und die naechste Rule vorbereiten
        if( aSelectors.Count() )
            aSelectors.DeleteAndDestroy( 0, aSelectors.Count() );
    }

    aSelectors.C40_INSERT( CSS1Selector, pSelector, aSelectors.Count() );

    return FALSE; // den Selektor haben wir gespeichert. Loeschen toedlich!
}


BOOL SvxCSS1Parser::DeclarationParsed( const String& rProperty,
                                       const CSS1Expression *pExpr )
{
    DBG_ASSERT( pExpr, "DeclarationParsed() ohne Expression" );

    if( !pExpr )
        return TRUE;

    ParseProperty( rProperty, pExpr );

    return TRUE;    // die Deklaration brauchen wir nicht mehr. Loeschen!
}

/*  */

SvxCSS1Parser::SvxCSS1Parser( SfxItemPool& rPool, USHORT nMinFixLineSp,
                              USHORT *pWhichIds, USHORT nWhichIds ) :
    CSS1Parser(),
    pSheetItemSet(0), pItemSet(0),
    nMinFixLineSpace( nMinFixLineSp ),
    pSearchEntry( 0 ), bIgnoreFontFamily( FALSE ),
    eDfltEnc( RTL_TEXTENCODING_DONTKNOW )
{
    // Item-Ids auch initialisieren
    aItemIds.nFont = rPool.GetTrueWhich( SID_ATTR_CHAR_FONT, FALSE );
    aItemIds.nPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_POSTURE, FALSE );
    aItemIds.nWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_WEIGHT, FALSE );
    aItemIds.nFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_FONTHEIGHT, FALSE );
    aItemIds.nUnderline = rPool.GetTrueWhich( SID_ATTR_CHAR_UNDERLINE, FALSE );
    aItemIds.nCrossedOut = rPool.GetTrueWhich( SID_ATTR_CHAR_STRIKEOUT, FALSE );
    aItemIds.nColor = rPool.GetTrueWhich( SID_ATTR_CHAR_COLOR, FALSE );
    aItemIds.nKerning = rPool.GetTrueWhich( SID_ATTR_CHAR_KERNING, FALSE );
    aItemIds.nCaseMap = rPool.GetTrueWhich( SID_ATTR_CHAR_CASEMAP, FALSE );
    aItemIds.nBlink = rPool.GetTrueWhich( SID_ATTR_FLASH, FALSE );

    aItemIds.nLineSpacing = rPool.GetTrueWhich( SID_ATTR_PARA_LINESPACE, FALSE );
    aItemIds.nAdjust = rPool.GetTrueWhich( SID_ATTR_PARA_ADJUST, FALSE );
    aItemIds.nWidows = rPool.GetTrueWhich( SID_ATTR_PARA_WIDOWS, FALSE );
    aItemIds.nOrphans = rPool.GetTrueWhich( SID_ATTR_PARA_ORPHANS, FALSE );
    aItemIds.nFmtSplit = rPool.GetTrueWhich( SID_ATTR_PARA_SPLIT, FALSE );

    aItemIds.nLRSpace = rPool.GetTrueWhich( SID_ATTR_LRSPACE, FALSE );
    aItemIds.nULSpace = rPool.GetTrueWhich( SID_ATTR_ULSPACE, FALSE );
    aItemIds.nBox = rPool.GetTrueWhich( SID_ATTR_BORDER_OUTER, FALSE );
    aItemIds.nBrush = rPool.GetTrueWhich( SID_ATTR_BRUSH, FALSE );

    aWhichMap.Insert( (USHORT)0, (USHORT)0 );
    SvParser::BuildWhichTbl( aWhichMap, (USHORT *)&aItemIds,
                             sizeof(aItemIds) / sizeof(USHORT) );
    if( pWhichIds && nWhichIds )
        SvParser::BuildWhichTbl( aWhichMap, pWhichIds, nWhichIds );

    pSheetItemSet = new SfxItemSet( rPool, aWhichMap.GetData() );
    pSheetPropInfo = new SvxCSS1PropertyInfo;
    pSearchEntry = new SvxCSS1MapEntry( rPool, aWhichMap.GetData() );
}

SvxCSS1Parser::~SvxCSS1Parser()
{
    delete pSheetItemSet;
    delete pSheetPropInfo;
    delete pSearchEntry;
}


/*  */

BOOL SvxCSS1Parser::ParseStyleSheet( const String& rIn )
{
    pItemSet = pSheetItemSet;
    pPropInfo = pSheetPropInfo;

    BOOL bSuccess = CSS1Parser::ParseStyleSheet( rIn );

    // die bisher geparsten Items auf die Styles verteilt werden
//  pSheetPropInfo->CreateBoxItem( *pSheetItemSet, GetDfltBorderDist() );
    for( USHORT i=0; i<aSelectors.Count(); i++ )
    {
        StyleParsed( aSelectors[i], *pSheetItemSet, *pSheetPropInfo );
    }

    // und etwas aufrauemen
    if( aSelectors.Count() )
        aSelectors.DeleteAndDestroy( 0, aSelectors.Count() );
    pSheetItemSet->ClearItem();
    pSheetPropInfo->Clear();

    pItemSet = 0;
    pPropInfo = 0;

    return bSuccess;
}

BOOL SvxCSS1Parser::ParseStyleOption( const String& rIn,
                                      SfxItemSet& rItemSet,
                                      SvxCSS1PropertyInfo& rPropInfo )
{
    pItemSet = &rItemSet;
    pPropInfo = &rPropInfo;

    BOOL bSuccess = CSS1Parser::ParseStyleOption( rIn );
//  pPropInfo->CreateBoxItem( *pItemSet, GetDfltBorderDist() );

    pItemSet = 0;
    pPropInfo = 0;

    return bSuccess;
}

/*  */

BOOL SvxCSS1Parser::GetEnum( const CSS1PropertyEnum *pPropTable,
                          const String &rValue, USHORT& rEnum )
{
    String aValue( rValue );
    aValue.ToLowerAscii();
    while( pPropTable->pName )
    {
        if( !rValue.EqualsIgnoreCaseAscii( pPropTable->pName ) )
            pPropTable++;
        else
            break;
    }

    if( pPropTable->pName )
        rEnum = pPropTable->nEnum;

    return (pPropTable->pName != 0);
}

void SvxCSS1Parser::PixelToTwip( long &rWidth, long &rHeight )
{
    if( Application::GetDefaultDevice() )
    {
        Size aTwipSz( rWidth, rHeight );
        aTwipSz = Application::GetDefaultDevice()->PixelToLogic( aTwipSz,
                                                          MapMode(MAP_TWIP) );

        rWidth = aTwipSz.Width();
        rHeight = aTwipSz.Height();
    }
}

void SvxCSS1Parser::SetBorderWidth( SvxBorderLine& aBorderLine, USHORT nWidth,
                                    BOOL bDouble, BOOL bTable )
{
    const USHORT *aWidths;
    USHORT nSize;
    if( !bDouble )
    {
        aWidths = aSBorderWidths;
        nSize = sizeof( aSBorderWidths );
    }
    else if( bTable )
    {
        aWidths = aTDBorderWidths;
        nSize = sizeof( aTDBorderWidths );
    }
    else
    {
        aWidths = aDBorderWidths;
        nSize = sizeof( aDBorderWidths );
    }

    USHORT i = (nSize / sizeof(USHORT)) - 4;
    while( i>0 &&
           nWidth <= ((aWidths[i] + aWidths[i-4]) / 2)  )
    {
        DBG_ASSERT( aWidths[i] > aWidths[i-4],
                "Linienbreiten sind nicht sortiert!" );
        i -= 4;
    }

    aBorderLine.SetOutWidth( aWidths[i+1] );
    aBorderLine.SetInWidth( aWidths[i+2] );
    aBorderLine.SetDistance( aWidths[i+3] );
}

ULONG SvxCSS1Parser::GetFontHeight( USHORT nSize ) const
{
    USHORT nHeight;

    switch( nSize )
    {
    case 0:     nHeight =  8*20;    break;
    case 1:     nHeight = 10*20;    break;
    case 2:     nHeight = 11*20;    break;
    case 3:     nHeight = 12*20;    break;
    case 4:     nHeight = 17*20;    break;
    case 5:     nHeight = 20*20;    break;
    case 6:
    default:    nHeight = 32*20;    break;
    }

    return nHeight;
}

const FontList *SvxCSS1Parser::GetFontList() const
{
        return 0;
}

SvxCSS1MapEntry *SvxCSS1Parser::GetMapEntry( const String& rKey,
                                             const SvxCSS1Map& rMap ) const
{
    pSearchEntry->SetKey( rKey );

    SvxCSS1MapEntry *pRet = 0;
    USHORT nPos;
    if( rMap.Seek_Entry( pSearchEntry, &nPos ) )
        pRet = rMap[nPos];

    return pRet;
}

void SvxCSS1Parser::InsertMapEntry( const String& rKey,
                                    const SfxItemSet& rItemSet,
                                    const SvxCSS1PropertyInfo& rProp,
                                    SvxCSS1Map& rMap )
{
    SvxCSS1MapEntry *pEntry = GetMapEntry( rKey, rMap );
    if( pEntry )
    {
        MergeStyles( rItemSet, rProp,
                     pEntry->GetItemSet(), pEntry->GetPropertyInfo(), TRUE );
    }
    else
    {
        rMap.Insert( new SvxCSS1MapEntry( rKey, rItemSet, rProp ) );
    }
}


void SvxCSS1Parser::MergeStyles( const SfxItemSet& rSrcSet,
                                 const SvxCSS1PropertyInfo& rSrcInfo,
                                 SfxItemSet& rTargetSet,
                                 SvxCSS1PropertyInfo& rTargetInfo,
                                 BOOL bSmart )
{
    if( !bSmart )
    {
        rTargetSet.Put( rSrcSet );
    }
    else
    {
        SvxLRSpaceItem aLRSpace( (const SvxLRSpaceItem&)rTargetSet.Get(aItemIds.nLRSpace) );
        SvxULSpaceItem aULSpace( (const SvxULSpaceItem&)rTargetSet.Get(aItemIds.nULSpace) );
        SvxBoxItem aBox( (const SvxBoxItem&)rTargetSet.Get(aItemIds.nBox) );

        rTargetSet.Put( rSrcSet );

        if( rSrcInfo.bLeftMargin || rSrcInfo.bRightMargin ||
            rSrcInfo.bTextIndent )
        {
            const SvxLRSpaceItem& rNewLRSpace =
                (const SvxLRSpaceItem&)rSrcSet.Get( aItemIds.nLRSpace );

            if( rSrcInfo.bLeftMargin )
                aLRSpace.SetLeft( rNewLRSpace.GetLeft() );
            if( rSrcInfo.bRightMargin )
                aLRSpace.SetRight( rNewLRSpace.GetRight() );
            if( rSrcInfo.bTextIndent )
                aLRSpace.SetTxtFirstLineOfst( rNewLRSpace.GetTxtFirstLineOfst() );

            rTargetSet.Put( aLRSpace );
        }

        if( rSrcInfo.bTopMargin || rSrcInfo.bBottomMargin )
        {
            const SvxULSpaceItem& rNewULSpace =
                (const SvxULSpaceItem&)rSrcSet.Get( aItemIds.nULSpace );

            if( rSrcInfo.bTopMargin )
                aULSpace.SetUpper( rNewULSpace.GetUpper() );
            if( rSrcInfo.bBottomMargin )
                aULSpace.SetLower( rNewULSpace.GetLower() );

            rTargetSet.Put( aULSpace );
        }
    }

    rTargetInfo.Merge( rSrcInfo );
}

void SvxCSS1Parser::SetDfltEncoding( rtl_TextEncoding eEnc )
{
    eDfltEnc = eEnc;
}

/*  */

static void ParseCSS1_font_size( const CSS1Expression *pExpr,
                                 SfxItemSet &rItemSet,
                                 SvxCSS1PropertyInfo& rPropInfo,
                                 const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    ULONG nHeight = 0;
    USHORT nPropHeight = 100;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nHeight = pExpr->GetULength();
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight = (long)pExpr->GetNumber();
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nHeight = (ULONG)nPHeight;
        }
        break;
//#ifdef PERCENTAGE_POSSIBLE
    case CSS1_PERCENTAGE:
        // nur fuer Drop-Caps!
        nPropHeight = (USHORT)pExpr->GetNumber();
        break;
//#endif
    case CSS1_IDENT:
        {
            USHORT nSize;
#ifdef PERCENTAGE_POSSIBLE
            const String& rValue = pExpr->GetString();
#endif
            if( SvxCSS1Parser::GetEnum( aFontSizeTable, pExpr->GetString(),
                                        nSize ) )
            {
                nHeight = rParser.GetFontHeight( nSize );
            }
#ifdef PERCENTAGE_POSSIBLE
            else if( rValue.EqualsIgnoreCaseAscii( sCSS1_PV_larger ) )
            {
                nPropHeight = 150;
            }
            else if( rValue.EqualsIgnoreCaseAscii( sCSS1_PV_smaller ) )
            {
                nPropHeight = 67;
            }
#endif
        }
        break;
    }

    if( nHeight || nPropHeight!=100 )
    {
        rItemSet.Put( SvxFontHeightItem( nHeight, nPropHeight,
                                         aItemIds.nFontHeight ) );
    }
}

/*  */

static void ParseCSS1_font_family( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    String aName, aStyleName, aDfltName;
    FontFamily eFamily = FAMILY_DONTKNOW;
    FontPitch ePitch = PITCH_DONTKNOW;
    rtl_TextEncoding eEnc = rParser.GetDfltEncoding();
    const FontList *pFList = rParser.GetFontList();
    BOOL bFirst = TRUE;
    BOOL bFound = FALSE;
    while( pExpr && (bFirst || ','==pExpr->GetOp() || !pExpr->GetOp()) )
    {
        CSS1Token eType = pExpr->GetType();
        if( CSS1_IDENT==eType || CSS1_STRING==eType )
        {
            String aIdent( pExpr->GetString() );

            if( CSS1_IDENT==eType )
            {
                // Alle nachfolgenden id's sammeln und mit einem
                // Space getrennt hintendranhaengen
                const CSS1Expression *pNext = pExpr->GetNext();
                while( pNext && !pNext->GetOp() &&
                       CSS1_IDENT==pNext->GetType() )
                {
                    (aIdent += ' ') += pNext->GetString();
                    pExpr = pNext;
                    pNext = pExpr->GetNext();
                }
            }
            if( aIdent.Len() )
            {
                if( !bFound && pFList )
                {
                    sal_Handle hFont = pFList->GetFirstFontInfo( aIdent );
                    if( 0 != hFont )
                    {
                        const FontInfo& rFInfo = pFList->GetFontInfo( hFont );
                        if( RTL_TEXTENCODING_DONTKNOW != rFInfo.GetCharSet() )
                        {
                            bFound = TRUE;
                            if( RTL_TEXTENCODING_SYMBOL == rFInfo.GetCharSet() )
                                eEnc = RTL_TEXTENCODING_SYMBOL;
                        }
                    }
                }
                if( !bFirst )
                    aName += ';';
                aName += aIdent;
            }
        }

        pExpr = pExpr->GetNext();
        bFirst = FALSE;
    }

    if( aName.Len() && !rParser.IsIgnoreFontFamily() )
        rItemSet.Put( SvxFontItem( eFamily, aName, aStyleName, ePitch,
                                   eEnc, aItemIds.nFont ) );
}

/*  */

static void ParseCSS1_font_weight( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
    case CSS1_STRING:   // MS-IE, was sonst
        {
            USHORT nWeight;
            if( SvxCSS1Parser::GetEnum( aFontWeightTable, pExpr->GetString(),
                                        nWeight ) )
            {
                rItemSet.Put( SvxWeightItem( (FontWeight)nWeight,
                                             aItemIds.nWeight ) );
            }
        }
        break;
    case CSS1_NUMBER:
        {
            USHORT nWeight = (USHORT)pExpr->GetNumber();
            rItemSet.Put( SvxWeightItem( nWeight>400 ? WEIGHT_BOLD
                                                     : WEIGHT_NORMAL,
                                         aItemIds.nWeight ) );
        }
        break;
    }
}

/*  */

static void ParseCSS1_font_style( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    BOOL bPosture = FALSE;
    BOOL bCaseMap = FALSE;
    FontItalic eItalic;
    SvxCaseMap eCaseMap;

    // normal | italic || small-caps | oblique || small-caps | small-caps
    // (wobei nor noch normal | italic und oblique zulaessig sind

    // der Wert kann zwei Werte enthalten!
    for( USHORT i=0; pExpr && i<2; i++ )
    {
        // Auch hier hinterlaesst MS-IEs Parser seine Spuren
        if( (CSS1_IDENT==pExpr->GetType() || CSS1_STRING==pExpr->GetType()) &&
            !pExpr->GetOp() )
        {
            const String& rValue = pExpr->GetString();
            // erstmal pruefen, ob es ein Italic-Wert oder 'normal' ist
            USHORT nItalic;
            if( SvxCSS1Parser::GetEnum( aFontStyleTable, rValue, nItalic ) )
            {
                eItalic = (FontItalic)nItalic;
                if( !bCaseMap && ITALIC_NONE==eItalic )
                {
                    // fuer 'normal' muessen wir auch die case-map aussch.
                    eCaseMap = SVX_CASEMAP_NOT_MAPPED;
                    bCaseMap = TRUE;
                }
                bPosture = TRUE;
            }
            else if( !bCaseMap &&
                     rValue.EqualsIgnoreCaseAscii(sCSS1_PV_small_caps) )
            {
                eCaseMap = SVX_CASEMAP_KAPITAELCHEN;
                bCaseMap = TRUE;
            }
        }

        // den naechsten Ausdruck holen
        pExpr = pExpr->GetNext();
    }

    if( bPosture )
        rItemSet.Put( SvxPostureItem( eItalic, aItemIds.nPosture ) );

    if( bCaseMap )
        rItemSet.Put( SvxCaseMapItem( eCaseMap, aItemIds.nCaseMap ) );
}

/*  */

static void ParseCSS1_font_variant( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    // normal | small-caps

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            USHORT nCaseMap;
            if( SvxCSS1Parser::GetEnum( aFontVariantTable, pExpr->GetString(),
                                        nCaseMap ) )
            {
                rItemSet.Put( SvxCaseMapItem( (SvxCaseMap)nCaseMap,
                                                aItemIds.nCaseMap ) );
            }
        }
    }
}

/*  */

static void ParseCSS1_color( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
    case CSS1_RGB:
    case CSS1_HEXCOLOR:
    case CSS1_STRING:       // Wegen MS-IE
        {
            Color aColor;
            if( pExpr->GetColor( aColor ) )
                rItemSet.Put( SvxColorItem( aColor, aItemIds.nColor ) );
        }
        break;

    }
}

/*  */

static void MergeHori( SvxGraphicPosition& ePos, SvxGraphicPosition eHori )
{
    DBG_ASSERT( GPOS_LT==eHori || GPOS_MT==eHori || GPOS_RT==eHori,
                "vertikale Position nicht oben" );

    switch( ePos )
    {
    case GPOS_LT:
    case GPOS_MT:
    case GPOS_RT:
        ePos = eHori;
        break;

    case GPOS_LM:
    case GPOS_MM:
    case GPOS_RM:
        ePos = GPOS_LT==eHori ? GPOS_LM : (GPOS_MT==eHori ? GPOS_MM : GPOS_RM);
        break;

    case GPOS_LB:
    case GPOS_MB:
    case GPOS_RB:
        ePos = GPOS_LT==eHori ? GPOS_LB : (GPOS_MT==eHori ? GPOS_MB : GPOS_RB);
        break;
    }
}

static void MergeVert( SvxGraphicPosition& ePos, SvxGraphicPosition eVert )
{
    DBG_ASSERT( GPOS_LT==eVert || GPOS_LM==eVert || GPOS_LB==eVert,
                "horizontale Position nicht links" );

    switch( ePos )
    {
    case GPOS_LT:
    case GPOS_LM:
    case GPOS_LB:
        ePos = eVert;
        break;

    case GPOS_MT:
    case GPOS_MM:
    case GPOS_MB:
        ePos = GPOS_LT==eVert ? GPOS_MT : (GPOS_LM==eVert ? GPOS_MM : GPOS_MB);
        break;

    case GPOS_RT:
    case GPOS_RM:
    case GPOS_RB:
        ePos = GPOS_LT==eVert ? GPOS_RT : (GPOS_LM==eVert ? GPOS_RM : GPOS_RB);
        break;
    }
}

static void ParseCSS1_background( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    Color aColor;
    String aURL;

    BOOL bColor = FALSE, bTransparent = FALSE;
    SvxGraphicPosition eRepeat = GPOS_TILED;
    SvxGraphicPosition ePos = GPOS_LT;
    BOOL bHori = FALSE, bVert = FALSE;

    while( pExpr && !pExpr->GetOp() )
    {
        switch( pExpr->GetType() )
        {
        case CSS1_URL:
            pExpr->GetURL( aURL );
            break;

        case CSS1_RGB:
            bColor = pExpr->GetColor( aColor );
            break;

        case CSS1_LENGTH:
        case CSS1_PIXLENGTH:
            {
                // da wir keine absolute Positionierung koennen,
                // unterscheiden wir nur zwischen  0 und !0. Deshalb
                // koennen Pixel auch wie alle anderen Einheiten behandelt
                // werden.

                ULONG nLength = (ULONG)pExpr->GetNumber();
                if( !bHori )
                {
                    ePos = nLength ? GPOS_MM : GPOS_LT;
                    bHori = TRUE;
                }
                else if( !bVert )
                {
                    MergeVert( ePos, (nLength ? GPOS_LM : GPOS_LT) );
                    bVert = TRUE;
                }
            }
            break;

        case CSS1_PERCENTAGE:
            {
                // die %-Angabe wird auf den enum abgebildet

                USHORT nPerc = (USHORT)pExpr->GetNumber();
                if( !bHori )
                {
                    ePos = nPerc < 25 ? GPOS_LT
                                      : (nPerc < 75 ? GPOS_MM
                                                    : GPOS_RB);
                }
                else if( !bVert )
                {
                    SvxGraphicPosition eVert =
                        nPerc < 25 ? GPOS_LT: (nPerc < 75 ? GPOS_LM
                                                          : GPOS_LB);
                    MergeVert( ePos, eVert );
                }
            }
            break;

        case CSS1_IDENT:
        case CSS1_HEXCOLOR:
        case CSS1_STRING:       // Wegen MS-IE
            {
                USHORT nEnum;
                const String &rValue = pExpr->GetString();
                if( rValue.EqualsIgnoreCaseAscii( sCSS1_PV_transparent ) )
                {
                    bTransparent = TRUE;
                }
                if( SvxCSS1Parser::GetEnum( aBGRepeatTable, rValue, nEnum ) )
                {
                    eRepeat = (SvxGraphicPosition)nEnum;
                }
                else if( SvxCSS1Parser::GetEnum( aBGHoriPosTable, rValue, nEnum ) )
                {
                    // <position>, horizontal
                    MergeHori( ePos, (SvxGraphicPosition)nEnum );
                }
                else if( SvxCSS1Parser::GetEnum( aBGVertPosTable, rValue, nEnum ) )
                {
                    // <position>, vertikal
                    MergeVert( ePos, (SvxGraphicPosition)nEnum );
                }
                else if( !bColor )
                {
                    // <color>
                    bColor = pExpr->GetColor( aColor );
                }
                // <scroll> kennen wir nicht
            }
            break;
        }

        pExpr = pExpr->GetNext();
    }

    // transparent schlaegt alles
    if( bTransparent )
    {
        bColor = FALSE;
        aURL.Erase();
    }

    // repeat hat prio gegenueber einer Position
    if( GPOS_NONE == eRepeat )
        eRepeat = ePos;

    if( bTransparent || bColor || aURL.Len() )
    {
        SvxBrushItem aBrushItem( aItemIds.nBrush );

        if( bTransparent )
            aBrushItem.SetColor( Color(COL_TRANSPARENT));
        else if( bColor )
            aBrushItem.SetColor( aColor );

        if( aURL.Len() )
        {
            aBrushItem.SetGraphicLink( INetURLObject::RelToAbs( aURL ) );
            aBrushItem.SetGraphicPos( eRepeat );
        }

        rItemSet.Put( aBrushItem );
    }
}

static void ParseCSS1_background_color( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    Color aColor;

    BOOL bColor = FALSE, bTransparent = FALSE;

    switch( pExpr->GetType() )
    {
    case CSS1_RGB:
        bColor = pExpr->GetColor( aColor );
        break;
    case CSS1_IDENT:
    case CSS1_HEXCOLOR:
    case CSS1_STRING:       // Wegen MS-IE
        if( pExpr->GetString().EqualsIgnoreCaseAscii( sCSS1_PV_transparent ) )
        {
            bTransparent = TRUE;
        }
        else
        {
            // <color>
            bColor = pExpr->GetColor( aColor );
        }
        break;
    }

    if( bTransparent || bColor )
    {
        SvxBrushItem aBrushItem( aItemIds.nBrush );

        if( bTransparent )
            aBrushItem.SetColor( Color(COL_TRANSPARENT) );
        else if( bColor )
            aBrushItem.SetColor( aColor);

        rItemSet.Put( aBrushItem );
    }
}

/*  */

static void ParseCSS1_line_height( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    USHORT nHeight = 0;
    BYTE nPropHeight = 0;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nHeight = (USHORT)pExpr->GetULength();
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight = (long)pExpr->GetNumber();
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nHeight = (USHORT)nPHeight;
        }
        break;
    case CSS1_PERCENTAGE:
        {
            USHORT nPHeight = (USHORT)pExpr->GetNumber();
            nPropHeight = nPHeight <= 200 ? (BYTE)nPHeight : 200;
        }
        break;
    case CSS1_NUMBER:
        {
            USHORT nPHeight = (USHORT)(pExpr->GetNumber() * 100);
            nPropHeight = nPHeight <= 200 ? (BYTE)nPHeight : 200;
        }
        break;
    }

    if( nHeight )
    {
        if( nHeight < rParser.GetMinFixLineSpace() )
            nHeight = rParser.GetMinFixLineSpace();
        SvxLineSpacingItem aLSItem( nHeight, aItemIds.nLineSpacing );
        aLSItem.SetLineHeight( nHeight );
        aLSItem.GetLineSpaceRule() = SVX_LINE_SPACE_FIX;
        aLSItem.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        rItemSet.Put( aLSItem );
    }
    else if( nPropHeight )
    {
        SvxLineSpacingItem aLSItem( nPropHeight, aItemIds.nLineSpacing );
        aLSItem.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
        if( 100 == nPropHeight )
            aLSItem.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        else
            aLSItem.SetPropLineSpace( nPropHeight );
        rItemSet.Put( aLSItem );
    }

}

/*  */

static void ParseCSS1_font( const CSS1Expression *pExpr,
                            SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo& rPropInfo,
                            const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    FontItalic eItalic = ITALIC_NONE;
    SvxCaseMap eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    FontWeight eWeight = WEIGHT_NORMAL;

    // [ <font-style> || <font-variant> || <font-weight> ] ?
    while( pExpr && !pExpr->GetOp() &&
           (CSS1_IDENT==pExpr->GetType() ||
            CSS1_STRING==pExpr->GetType() ||
            CSS1_NUMBER==pExpr->GetType()) )
    {
        if( CSS1_IDENT==pExpr->GetType() ||
            CSS1_STRING==pExpr->GetType() )
        {
            const String& rValue = pExpr->GetString();

            USHORT nEnum;

            if( SvxCSS1Parser::GetEnum( aFontStyleTable, rValue, nEnum ) )
            {
                eItalic = (FontItalic)nEnum;
            }
            else if( SvxCSS1Parser::GetEnum( aFontVariantTable, rValue, nEnum ) )
            {
                eCaseMap = (SvxCaseMap)nEnum;
            }
            else if( SvxCSS1Parser::GetEnum( aFontWeightTable, rValue, nEnum ) )
            {
                eWeight = (FontWeight)nEnum;
            }
        }
        else
        {
            eWeight = (USHORT)pExpr->GetNumber() > 400 ? WEIGHT_BOLD
                                                       : WEIGHT_NORMAL;
        }

        pExpr = pExpr->GetNext();
    }

    if( !pExpr || pExpr->GetOp() )
        return;

    // Da "font" alle Werte zurecksetzt, fuer die nichts angegeben ist,
    // tun wir das hier.
    rItemSet.Put( SvxPostureItem( eItalic, aItemIds.nPosture ) );
    rItemSet.Put( SvxCaseMapItem( eCaseMap, aItemIds.nCaseMap ) );
    rItemSet.Put( SvxWeightItem( eWeight, aItemIds.nWeight ) );


    // font-size
    CSS1Expression aExpr( pExpr->GetType(), pExpr->GetString(),
                          pExpr->GetNumber() );
    ParseCSS1_font_size( &aExpr, rItemSet, rPropInfo, rParser );
    pExpr = pExpr->GetNext();

    if( !pExpr )
        return;

    // [ '/' line-height ]?
    if( '/' == pExpr->GetOp() )
    {
        // '/' line-height
        aExpr.Set( pExpr->GetType(), pExpr->GetString(), pExpr->GetNumber() );
        ParseCSS1_line_height( &aExpr, rItemSet, rPropInfo, rParser );

        pExpr = pExpr->GetNext();
    }

    if( !pExpr || pExpr->GetOp() )
        return;

    // font-family
    ParseCSS1_font_family( pExpr, rItemSet, rPropInfo, rParser );
}

/*  */

static void ParseCSS1_letter_spacing( const CSS1Expression *pExpr,
                                      SfxItemSet &rItemSet,
                                      SvxCSS1PropertyInfo& rPropInfo,
                                      const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        rItemSet.Put( SvxKerningItem( (short)pExpr->GetSLength(),
                                      aItemIds.nKerning ) );
        break;

    case CSS1_PIXLENGTH:
        {
            long nPWidth = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            rItemSet.Put( SvxKerningItem( (short)nPWidth, aItemIds.nKerning ) );
        }
        break;

    case CSS1_NUMBER:
        if( pExpr->GetNumber() == 0 )
        {
            // eigentlich unnoetig, aber wir sind ja tollerant
            rItemSet.Put( SvxKerningItem( (short)0, aItemIds.nKerning ) );
        }
        break;

    case CSS1_IDENT:
    case CSS1_STRING: // Vorschtshalber auch MS-IE
        if( pExpr->GetString().EqualsIgnoreCaseAscii(sCSS1_PV_normal) )
        {
            rItemSet.Put( SvxKerningItem( (short)0, aItemIds.nKerning ) );
        }
        break;
    }
}

/*  */

static void ParseCSS1_text_decoration( const CSS1Expression *pExpr,
                                       SfxItemSet &rItemSet,
                                       SvxCSS1PropertyInfo& rPropInfo,
                                       const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    BOOL bUnderline = FALSE;
    BOOL bCrossedOut = FALSE;
    BOOL bBlink = FALSE;
    BOOL bBlinkOn;
    FontUnderline eUnderline;
    FontStrikeout eCrossedOut;

    // der Wert kann zwei Werte enthalten! Und MS-IE auch Strings
    while( pExpr && (pExpr->GetType() == CSS1_IDENT ||
                     pExpr->GetType() == CSS1_STRING) && !pExpr->GetOp() )
    {
        String aValue = pExpr->GetString();
        aValue.ToLowerAscii();
        BOOL bKnown = FALSE;

        switch( aValue.GetChar( 0 ) )
        {
        case 'n':
            if( aValue.EqualsAscii( sCSS1_PV_none ) )
            {
                bUnderline = TRUE;
                eUnderline = UNDERLINE_NONE;

                bCrossedOut = TRUE;
                eCrossedOut = STRIKEOUT_NONE;

                bBlink = TRUE;
                bBlinkOn = FALSE;

                bKnown = TRUE;
            }
            break;

        case 'u':
            if( aValue.EqualsAscii( sCSS1_PV_underline ) )
            {
                bUnderline = TRUE;
                eUnderline = UNDERLINE_SINGLE;

                bKnown = TRUE;
            }
            break;

        case 'l':
            if( aValue.EqualsAscii( sCSS1_PV_line_through ) )
            {
                bCrossedOut = TRUE;
                eCrossedOut = STRIKEOUT_SINGLE;

                bKnown = TRUE;
            }
            break;

        case 'b':
            if( aValue.EqualsAscii( sCSS1_PV_blink ) )
            {
                bBlink = TRUE;
                bBlinkOn = TRUE;

                bKnown = TRUE;
            }
            break;
        }

        if( !bKnown )
        {
            bUnderline = TRUE;
            eUnderline = UNDERLINE_SINGLE;
        }

        pExpr = pExpr->GetNext();
    }

    if( bUnderline )
        rItemSet.Put( SvxUnderlineItem( eUnderline, aItemIds.nUnderline ) );

    if( bCrossedOut )
        rItemSet.Put( SvxCrossedOutItem( eCrossedOut, aItemIds.nCrossedOut ) );

    if( bBlink )
        rItemSet.Put( SvxBlinkItem( bBlinkOn, aItemIds.nBlink ) );
}

/*  */

static void ParseCSS1_text_align( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    if( CSS1_IDENT==pExpr->GetType() ||
        CSS1_STRING==pExpr->GetType() ) // MS-IE, mal wieder
    {
        USHORT nAdjust;
        if( SvxCSS1Parser::GetEnum( aTextAlignTable, pExpr->GetString(),
                                    nAdjust ) )
        {
            rItemSet.Put( SvxAdjustItem( (SvxAdjust)nAdjust,
                                         aItemIds.nAdjust ) );
        }
    }
}

/*  */

static void ParseCSS1_text_indent( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    short nIndent = 0;
    BOOL bSet = FALSE;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nIndent = (short)pExpr->GetSLength();
        bSet = TRUE;
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nIndent = (short)nPWidth;
            bSet = TRUE;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    }

    if( bSet )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, FALSE,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            aLRItem.SetTxtFirstLineOfst( nIndent );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            aLRItem.SetTxtFirstLineOfst( nIndent );
            rItemSet.Put( aLRItem );
        }
        rPropInfo.bTextIndent = TRUE;
    }
}

/*  */

static void ParseCSS1_margin_left( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    long nLeft = 0;
    BOOL bSet = FALSE;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            nLeft = pExpr->GetSLength();
            bSet = TRUE;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            nLeft = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nLeft, nPHeight );
            bSet = TRUE;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    }

    if( bSet )
    {
        rPropInfo.nLeftMargin = nLeft;
        if( nLeft < 0 )
            nLeft = 0;
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, FALSE,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            aLRItem.SetTxtLeft( (USHORT)nLeft );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            aLRItem.SetTxtLeft( (USHORT)nLeft );
            rItemSet.Put( aLRItem );
        }
        rPropInfo.bLeftMargin = TRUE;
    }
}

/*  */

static void ParseCSS1_margin_right( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    long nRight = 0;
    BOOL bSet = FALSE;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            nRight = pExpr->GetSLength();
            bSet = TRUE;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            nRight = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nRight, nPHeight );
            bSet = TRUE;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    }

    if( bSet )
    {
        rPropInfo.nRightMargin = nRight;
        if( nRight < 0 )
            nRight = 0;
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, FALSE,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            aLRItem.SetRight( (USHORT)nRight );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            aLRItem.SetRight( (USHORT)nRight );
            rItemSet.Put( aLRItem );
        }
        rPropInfo.bRightMargin = TRUE;
    }
}

/*  */

static void ParseCSS1_margin_top( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    USHORT nUpper = 0;
    BOOL bSet = FALSE;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            nUpper = (USHORT)nTmp;
            bSet = TRUE;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight =  (long)pExpr->GetNumber();
            if( nPHeight < 0 )
                nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nUpper = (USHORT)nPHeight;
            bSet = TRUE;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    }

    if( bSet )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nULSpace, FALSE,
                                                   &pItem ) )
        {
            SvxULSpaceItem aULItem( *((const SvxULSpaceItem*)pItem) );
            aULItem.SetUpper( nUpper );
            rItemSet.Put( aULItem );
        }
        else
        {
            SvxULSpaceItem aULItem( aItemIds.nULSpace );
            aULItem.SetUpper( nUpper );
            rItemSet.Put( aULItem );
        }
        rPropInfo.bTopMargin = TRUE;
    }
}

/*  */

static void ParseCSS1_margin_bottom( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    USHORT nLower = 0;
    BOOL bSet = FALSE;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            nLower = (USHORT)nTmp;
            bSet = TRUE;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight =  (long)pExpr->GetNumber();
            if( nPHeight < 0 )
                nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nLower = (USHORT)nPHeight;
            bSet = TRUE;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    }

    if( bSet )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nULSpace, FALSE,
                                                   &pItem ) )
        {
            SvxULSpaceItem aULItem( *((const SvxULSpaceItem*)pItem) );
            aULItem.SetLower( nLower );
            rItemSet.Put( aULItem );
        }
        else
        {
            SvxULSpaceItem aULItem( aItemIds.nULSpace );
            aULItem.SetLower( nLower );
            rItemSet.Put( aULItem );
        }
        rPropInfo.bBottomMargin = TRUE;
    }
}

/*  */

static void ParseCSS1_margin( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    long nMargins[4] = { 0, 0, 0, 0 };
    BOOL bSetMargins[4] = { FALSE, FALSE, FALSE, FALSE };

    for( USHORT i=0; pExpr && i<4 && !pExpr->GetOp(); i++ )
    {
        BOOL bSetThis = FALSE;
        long nMargin;

        switch( pExpr->GetType() )
        {
        case CSS1_LENGTH:
            {
                nMargin = pExpr->GetSLength();
                bSetThis = TRUE;
            }
            break;
        case CSS1_PIXLENGTH:
            {
                long nPWidth = 0;
                nMargin =  (long)pExpr->GetNumber();
                SvxCSS1Parser::PixelToTwip( nPWidth, nMargin );
                bSetThis = TRUE;
            }
            break;
        case CSS1_PERCENTAGE:
            // koennen wir nicht
            break;
        }

        if( bSetThis )
        {
            // 0 = top
            // 1 = right
            // 2 = bottom
            // 3 = left
            switch( i )
            {
            case 0:
                nMargins[0] = nMargins[1] =nMargins[2] = nMargins[3] = nMargin;
                bSetMargins[0] = bSetMargins[1] =
                bSetMargins[2] = bSetMargins[3] = TRUE;
                break;
            case 1:
                nMargins[1] = nMargins[3] = nMargin;    // right + left
                bSetMargins[1] = bSetMargins[3] = TRUE;
                break;
            case 2:
                nMargins[2] = nMargin;  // bottom
                bSetMargins[2] = TRUE;
                break;
            case 3:
                nMargins[3] = nMargin;  // left
                bSetMargins[3] = TRUE;
                break;
            }
        }
        pExpr = pExpr->GetNext();
    }

    if( bSetMargins[3] || bSetMargins[1] )
    {
        if( bSetMargins[3] )
        {
            rPropInfo.bLeftMargin = TRUE;
            rPropInfo.nLeftMargin = nMargins[3];
            if( nMargins[3] < 0 )
                nMargins[3] = 0;
        }
        if( bSetMargins[1] )
        {
            rPropInfo.bRightMargin = TRUE;
            rPropInfo.nRightMargin = nMargins[1];
            if( nMargins[1] < 0 )
                nMargins[1] = 0;
        }

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, FALSE,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            if( bSetMargins[3] )
                aLRItem.SetLeft( (USHORT)nMargins[3] );
            if( bSetMargins[1] )
                aLRItem.SetRight( (USHORT)nMargins[1] );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            if( bSetMargins[3] )
                aLRItem.SetLeft( (USHORT)nMargins[3] );
            if( bSetMargins[1] )
                aLRItem.SetRight( (USHORT)nMargins[1] );
            rItemSet.Put( aLRItem );
        }
    }

    if( bSetMargins[0] || bSetMargins[2] )
    {
        if( nMargins[0] < 0 )
            nMargins[0] = 0;
        if( nMargins[2] < 0 )
            nMargins[2] = 0;

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nULSpace, FALSE,
                                                   &pItem ) )
        {
            SvxULSpaceItem aULItem( *((const SvxULSpaceItem*)pItem) );
            if( bSetMargins[0] )
                aULItem.SetUpper( (USHORT)nMargins[0] );
            if( bSetMargins[2] )
                aULItem.SetLower( (USHORT)nMargins[2] );
            rItemSet.Put( aULItem );
        }
        else
        {
            SvxULSpaceItem aULItem( aItemIds.nULSpace );
            if( bSetMargins[0] )
                aULItem.SetUpper( (USHORT)nMargins[0] );
            if( bSetMargins[2] )
                aULItem.SetLower( (USHORT)nMargins[2] );
            rItemSet.Put( aULItem );
        }

        rPropInfo.bTopMargin |= bSetMargins[0];
        rPropInfo.bBottomMargin |= bSetMargins[2];
    }
}

/*  */

static BOOL ParseCSS1_padding_xxx( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser,
                                   USHORT nWhichLine )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    BOOL bSet = FALSE;
    USHORT nDist = 0;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            else if( nTmp > USHRT_MAX-1 )
                nTmp = USHRT_MAX-1;
            nDist = (USHORT)nTmp;
            bSet = TRUE;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = (long)pExpr->GetNumber();
            long nPHeight = 0;
            if( nPWidth < 0 )
                nPWidth = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            if( nPWidth > USHRT_MAX-1 )
                nPWidth = USHRT_MAX-1;
            nDist = (USHORT)nPWidth;
            bSet = TRUE;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    }

    if( bSet )
    {
        switch( nWhichLine )
        {
        case BOX_LINE_TOP:      rPropInfo.nTopBorderDistance = nDist;   break;
        case BOX_LINE_BOTTOM:   rPropInfo.nBottomBorderDistance = nDist;break;
        case BOX_LINE_LEFT:     rPropInfo.nLeftBorderDistance = nDist;  break;
        case BOX_LINE_RIGHT:    rPropInfo.nRightBorderDistance = nDist; break;
        }
    }

    return bSet;
}

/*  */

static void ParseCSS1_padding_top( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    ParseCSS1_padding_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_TOP );
}

static void ParseCSS1_padding_bottom( const CSS1Expression *pExpr,
                                      SfxItemSet &rItemSet,
                                      SvxCSS1PropertyInfo& rPropInfo,
                                      const SvxCSS1Parser& rParser )
{
    ParseCSS1_padding_xxx( pExpr, rItemSet, rPropInfo, rParser,
                           BOX_LINE_BOTTOM );
}

static void ParseCSS1_padding_left( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    ParseCSS1_padding_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_LEFT );
}

static void ParseCSS1_padding_right( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& rParser )
{
    ParseCSS1_padding_xxx( pExpr, rItemSet, rPropInfo, rParser,
                           BOX_LINE_RIGHT );
}

static void ParseCSS1_padding( const CSS1Expression *pExpr,
                               SfxItemSet &rItemSet,
                               SvxCSS1PropertyInfo& rPropInfo,
                               const SvxCSS1Parser& rParser )
{
    USHORT n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        USHORT nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        if( ParseCSS1_padding_xxx( pExpr, rItemSet, rPropInfo, rParser,
                                   nLine ) )
        {
            if( n==0 )
            {
                rPropInfo.nTopBorderDistance = rPropInfo.nBottomBorderDistance;
                rPropInfo.nLeftBorderDistance = rPropInfo.nTopBorderDistance;
            }
            if( n <= 1 )
                rPropInfo.nRightBorderDistance = rPropInfo.nLeftBorderDistance;
        }

        pExpr = pExpr->GetNext();
        n++;
    }
}

/*  */

static void ParseCSS1_border_xxx( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser,
                                  USHORT nWhichLine, BOOL bAll )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    USHORT nWidth = USHRT_MAX;      // die Linien-Dicke
    USHORT nNWidth = 1;             // benannte Linien-Dicke (und default)
    CSS1BorderStyle eStyle = CSS1_BS_NONE; // Linien-Style
    Color aColor;
    BOOL bColor = FALSE;

    while( pExpr && !pExpr->GetOp() )
    {
        switch( pExpr->GetType() )
        {
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
            if( pExpr->GetColor( aColor ) )
                bColor = TRUE;
            break;

        case CSS1_IDENT:
            {
                const String& rValue = pExpr->GetString();
                USHORT nValue;
                if( SvxCSS1Parser::GetEnum( aBorderWidthTable, rValue, nValue ) )
                {
                    nNWidth = nValue;
                }
                else if( SvxCSS1Parser::GetEnum( aBorderStyleTable, rValue, nValue ) )
                {
                    eStyle = (CSS1BorderStyle)nValue;
                }
                else if( pExpr->GetColor( aColor ) )
                {
                    bColor = TRUE;
                }
            }
            break;

        case CSS1_LENGTH:
            nWidth = (USHORT)pExpr->GetULength();
            break;

        case CSS1_PIXLENGTH:
            {
                BOOL bHori = nWhichLine == BOX_LINE_TOP ||
                             nWhichLine == BOX_LINE_BOTTOM;
                // Ein Pixel wird zur Haarlinie (ist huebscher)
                long nWidthL = (long)pExpr->GetNumber();
                if( nWidthL > 1 )
                {
                    long nPWidth = bHori ? 0 : nWidthL;
                    long nPHeight = bHori ? nWidthL : 0;
                    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                    nWidth = (USHORT)(bHori ? nPHeight : nPWidth);
                }
                else
                    nWidth = 1;
            }
            break;
        }

        pExpr = pExpr->GetNext();
    }

    for( USHORT i=0; i<4; i++ )
    {
        USHORT nLine;
        switch( i )
        {
        case 0: nLine = BOX_LINE_TOP; break;
        case 1: nLine = BOX_LINE_BOTTOM; break;
        case 2: nLine = BOX_LINE_LEFT; break;
        case 3: nLine = BOX_LINE_RIGHT; break;
        }

        if( bAll || nLine == nWhichLine )
        {
            SvxCSS1BorderInfo *pInfo = rPropInfo.GetBorderInfo( nLine );
            pInfo->eStyle = eStyle;
            pInfo->nAbsWidth = nWidth;
            pInfo->nNamedWidth = nNWidth;
            if( bColor )
                pInfo->aColor = aColor;
        }
    }
}

static void ParseCSS1_border_xxx_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser,
                                        USHORT nWhichLine )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    USHORT nWidth = USHRT_MAX;      // die Linien-Dicke
    USHORT nNWidth = 1;             // benannte Linien-Dicke (und default)

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            USHORT nValue;
            if( SvxCSS1Parser::GetEnum( aBorderWidthTable, pExpr->GetString(), nValue ) )
            {
                nNWidth = nValue;
            }
        }
        break;

    case CSS1_LENGTH:
        nWidth = (USHORT)pExpr->GetULength();
        break;

    case CSS1_PIXLENGTH:
        {
            BOOL bHori = nWhichLine == BOX_LINE_TOP ||
                         nWhichLine == BOX_LINE_BOTTOM;
            long nWidthL = (long)pExpr->GetNumber();
            long nPWidth = bHori ? 0 : nWidthL;
            long nPHeight = bHori ? nWidthL : 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nWidth = (USHORT)(bHori ? nPHeight : nPWidth);
        }
        break;
    }

    SvxCSS1BorderInfo *pInfo = rPropInfo.GetBorderInfo( nWhichLine );
    pInfo->nAbsWidth = nWidth;
    pInfo->nNamedWidth = nNWidth;
}

/*  */

static void ParseCSS1_border_top_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_TOP );
}

static void ParseCSS1_border_right_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_RIGHT );
}

static void ParseCSS1_border_bottom_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_BOTTOM );
}

static void ParseCSS1_border_left_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_LEFT );
}

static void ParseCSS1_border_width( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    USHORT n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        USHORT nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, nLine );
        rPropInfo.CopyBorderInfo( n, SVX_CSS1_BORDERINFO_WIDTH );

        pExpr = pExpr->GetNext();
        n++;
    }
}

static void ParseCSS1_border_color( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    USHORT n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        USHORT nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        Color aColor;
        switch( pExpr->GetType() )
        {
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
        case CSS1_IDENT:
            if( pExpr->GetColor( aColor ) )
                rPropInfo.GetBorderInfo( nLine )->aColor = aColor;
            break;
        }
        rPropInfo.CopyBorderInfo( n, SVX_CSS1_BORDERINFO_COLOR );

        pExpr = pExpr->GetNext();
        n++;
    }
}

static void ParseCSS1_border_style( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    USHORT n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        USHORT nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        USHORT nValue;
        if( CSS1_IDENT==pExpr->GetType() &&
            SvxCSS1Parser::GetEnum( aBorderStyleTable, pExpr->GetString(),
                                    nValue ) )
        {
            rPropInfo.GetBorderInfo( nLine )->eStyle = (CSS1BorderStyle)nValue;
        }
        rPropInfo.CopyBorderInfo( n, SVX_CSS1_BORDERINFO_STYLE );

        pExpr = pExpr->GetNext();
        n++;
    }
}


static void ParseCSS1_border_top( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_TOP, FALSE );
}

static void ParseCSS1_border_right( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_RIGHT, FALSE );
}

static void ParseCSS1_border_bottom( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_BOTTOM, FALSE );
}

static void ParseCSS1_border_left( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_LEFT, FALSE );
}

static void ParseCSS1_border( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, 0, TRUE );
}

/*  */

static void ParseCSS1_float( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    if( CSS1_IDENT==pExpr->GetType() )
    {
        USHORT nFloat;
        if( SvxCSS1Parser::GetEnum( aFloatTable, pExpr->GetString(), nFloat ) )
            rPropInfo.eFloat = (SvxAdjust)nFloat;
    }
}


/*  */

static void ParseCSS1_position( const CSS1Expression *pExpr,
                                SfxItemSet &rItemSet,
                                SvxCSS1PropertyInfo& rPropInfo,
                                const SvxCSS1Parser& rParser )
{
    DBG_ASSERT( pExpr, "kein Ausdruck" );

    if( CSS1_IDENT==pExpr->GetType() )
    {
        USHORT nPos;
        if( SvxCSS1Parser::GetEnum( aPositionTable, pExpr->GetString(), nPos ) )
            rPropInfo.ePosition = (SvxCSS1Position)nPos;
    }
}

/*  */

static void ParseCSS1_length( const CSS1Expression *pExpr,
                              long& rLength,
                              SvxCSS1LengthType& rLengthType,
                              BOOL bHori )
{
    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        if( pExpr->GetString().EqualsIgnoreCaseAscii( sCSS1_PV_auto ) )
        {
            rLength = 0;
            rLengthType = SVX_CSS1_LTYPE_AUTO;
        }
        break;

    case CSS1_LENGTH:
        rLength = pExpr->GetSLength();
        rLengthType = SVX_CSS1_LTYPE_TWIP;
        break;

    case CSS1_PIXLENGTH:
    case CSS1_NUMBER:       // wegen Netscape und IE
        {
            long nWidthL = (long)pExpr->GetNumber();
            long nPWidth = bHori ? 0 : nWidthL;
            long nPHeight = bHori ? nWidthL : 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            rLength = (bHori ? nPHeight : nPWidth);
            rLengthType = SVX_CSS1_LTYPE_TWIP;
        }
        break;

    case CSS1_PERCENTAGE:
        rLength = (long)pExpr->GetNumber();
        if( rLength > 100 )
            rLength = 100;
        rLengthType = SVX_CSS1_LTYPE_PERCENTAGE;
        break;
    }
}

/*  */

static void ParseCSS1_width( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& rParser )
{
    ParseCSS1_length( pExpr, rPropInfo.nWidth, rPropInfo.eWidthType, TRUE );
}

static void ParseCSS1_height( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& rParser )
{
    ParseCSS1_length( pExpr, rPropInfo.nHeight, rPropInfo.eHeightType, FALSE );
}

static void ParseCSS1_left( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& rParser )
{
    ParseCSS1_length( pExpr, rPropInfo.nLeft, rPropInfo.eLeftType, TRUE );
}

static void ParseCSS1_top( const CSS1Expression *pExpr,
                           SfxItemSet &rItemSet,
                           SvxCSS1PropertyInfo& rPropInfo,
                           const SvxCSS1Parser& rParser )
{
    ParseCSS1_length( pExpr, rPropInfo.nTop, rPropInfo.eTopType, FALSE );
}

/*  */

// Feature: PrintExt
static void ParseCSS1_size( const CSS1Expression *pExpr,
                            SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo& rPropInfo,
                            const SvxCSS1Parser& rParser )
{
    USHORT n=0;
    while( n<2 && pExpr && !pExpr->GetOp() )
    {
        switch( pExpr->GetType() )
        {
        case CSS1_IDENT:
            {
                USHORT nValue;
                if( SvxCSS1Parser::GetEnum( aSizeTable, pExpr->GetString(),
                                            nValue ) )
                {
                    rPropInfo.eSizeType = (SvxCSS1SizeType)nValue;
                }
            }
            break;

        case CSS1_LENGTH:
            rPropInfo.nHeight = pExpr->GetSLength();
            if( n==0 )
                rPropInfo.nWidth = rPropInfo.nHeight;
            rPropInfo.eSizeType = SVX_CSS1_STYPE_TWIP;
            break;

        case CSS1_PIXLENGTH:
            {
                long nPHeight = (long)pExpr->GetNumber();
                long nPWidth = n==0 ? nPHeight : 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                rPropInfo.nHeight = nPHeight;
                if( n==0 )
                    rPropInfo.nWidth = nPWidth;
                rPropInfo.eSizeType = SVX_CSS1_STYPE_TWIP;
            }
            break;
        }

        pExpr = pExpr->GetNext();
        n++;
    }
}

// /Feature: PrintExt

/*  */

// Feature: PrintExt

static void ParseCSS1_page_break_xxx( const CSS1Expression *pExpr,
                                      SvxCSS1PageBreak& rPBreak )
{
    if( CSS1_IDENT == pExpr->GetType() )
    {
        USHORT nValue;
        if( SvxCSS1Parser::GetEnum( aPageBreakTable, pExpr->GetString(),
                                    nValue ) )
        {
            rPBreak = (SvxCSS1PageBreak)nValue;
        }
    }
}

static void ParseCSS1_page_break_before( const CSS1Expression *pExpr,
                                         SfxItemSet &rItemSet,
                                         SvxCSS1PropertyInfo& rPropInfo,
                                         const SvxCSS1Parser& rParser )
{
    ParseCSS1_page_break_xxx( pExpr, rPropInfo.ePageBreakBefore );
}

static void ParseCSS1_page_break_after( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_page_break_xxx( pExpr, rPropInfo.ePageBreakAfter );
}

static void ParseCSS1_page_break_inside( const CSS1Expression *pExpr,
                                         SfxItemSet &rItemSet,
                                         SvxCSS1PropertyInfo& rPropInfo,
                                         const SvxCSS1Parser& rParser )
{
    SvxCSS1PageBreak eBreak;
    ParseCSS1_page_break_xxx( pExpr, eBreak );

    BOOL bSetSplit = FALSE, bSplit = TRUE;
    switch( eBreak )
    {
    case SVX_CSS1_PBREAK_AUTO:
        bSetSplit = TRUE;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bSplit = FALSE;
        bSetSplit = TRUE;
        break;
    }

    if( bSetSplit )
        rItemSet.Put( SvxFmtSplitItem( bSplit, aItemIds.nFmtSplit ) );
}

static void ParseCSS1_widows( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& rParser )
{
    if( CSS1_NUMBER == pExpr->GetType() )
    {
        BYTE nVal = pExpr->GetNumber() <= 255
                        ? (BYTE)pExpr->GetNumber()
                        : 255;
        SvxWidowsItem aWidowsItem( nVal, aItemIds.nWidows );
        rItemSet.Put( aWidowsItem );
    }
}

static void ParseCSS1_orphans( const CSS1Expression *pExpr,
                               SfxItemSet &rItemSet,
                               SvxCSS1PropertyInfo& rPropInfo,
                               const SvxCSS1Parser& rParser )
{
    if( CSS1_NUMBER == pExpr->GetType() )
    {
        BYTE nVal = pExpr->GetNumber() <= 255
                        ? (BYTE)pExpr->GetNumber()
                        : 255;
        SvxOrphansItem aOrphansItem( nVal, aItemIds.nOrphans );
        rItemSet.Put( aOrphansItem );
    }
}
// /Feature: PrintExt

/*  */

// die Zuordung Property zu parsender Funktion
struct CSS1PropEntry
{
    union
    {
        const sal_Char  *sName;
        String          *pName;
    };
    FnParseCSS1Prop pFunc;
};

#define CSS1_PROP_ENTRY(p) \
    {   sCSS1_P_##p, ParseCSS1_##p }


// die Tabelle mit den Zuordnungen
static CSS1PropEntry __FAR_DATA aCSS1PropFnTab[] =
{
    CSS1_PROP_ENTRY(background),
    CSS1_PROP_ENTRY(background_color),
    CSS1_PROP_ENTRY(border_top_width),
    CSS1_PROP_ENTRY(border_right_width),
    CSS1_PROP_ENTRY(border_bottom_width),
    CSS1_PROP_ENTRY(border_left_width),
    CSS1_PROP_ENTRY(border_width),
    CSS1_PROP_ENTRY(border_color),
    CSS1_PROP_ENTRY(border_style),
    CSS1_PROP_ENTRY(border_top),
    CSS1_PROP_ENTRY(border_right),
    CSS1_PROP_ENTRY(border_bottom),
    CSS1_PROP_ENTRY(border_left),
    CSS1_PROP_ENTRY(border),
    CSS1_PROP_ENTRY(color),
    CSS1_PROP_ENTRY(float),
    CSS1_PROP_ENTRY(font_size),
    CSS1_PROP_ENTRY(font_family),
    CSS1_PROP_ENTRY(font_style),
    CSS1_PROP_ENTRY(font_variant),
    CSS1_PROP_ENTRY(font_weight),
    CSS1_PROP_ENTRY(letter_spacing),
    CSS1_PROP_ENTRY(line_height),
    CSS1_PROP_ENTRY(font),
    CSS1_PROP_ENTRY(text_align),
    CSS1_PROP_ENTRY(text_decoration),
    CSS1_PROP_ENTRY(text_indent),
    CSS1_PROP_ENTRY(margin_left),
    CSS1_PROP_ENTRY(margin_right),
    CSS1_PROP_ENTRY(margin_top),
    CSS1_PROP_ENTRY(margin_bottom),
    CSS1_PROP_ENTRY(margin),
    CSS1_PROP_ENTRY(padding_top),
    CSS1_PROP_ENTRY(padding_bottom),
    CSS1_PROP_ENTRY(padding_left),
    CSS1_PROP_ENTRY(padding_right),
    CSS1_PROP_ENTRY(padding),
    CSS1_PROP_ENTRY(position),
    CSS1_PROP_ENTRY(left),
    CSS1_PROP_ENTRY(top),
    CSS1_PROP_ENTRY(width),
    CSS1_PROP_ENTRY(height),
// Feature: PrintExt
    CSS1_PROP_ENTRY(size),
    CSS1_PROP_ENTRY(page_break_before),
    CSS1_PROP_ENTRY(page_break_after),
    CSS1_PROP_ENTRY(page_break_inside),
    CSS1_PROP_ENTRY(widows),
    CSS1_PROP_ENTRY(orphans)
// /Feature: PrintExt
};

/*  */

static int __FAR_DATA bSortedPropFns = FALSE;

extern "C"
{
static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC )
 _Optlink
#endif
    CSS1PropEntryCompare( const void *pFirst, const void *pSecond)
{
    int nRet;
    if( ((CSS1PropEntry*)pFirst)->pFunc )
    {
        if( ((CSS1PropEntry*)pSecond)->pFunc )
            nRet = strcmp( ((CSS1PropEntry*)pFirst)->sName ,
                    ((CSS1PropEntry*)pSecond)->sName );
        else
            nRet = -1 * ((CSS1PropEntry*)pSecond)->pName->CompareToAscii(
                            ((CSS1PropEntry*)pFirst)->sName );
    }
    else
    {
        if( ((CSS1PropEntry*)pSecond)->pFunc )
            nRet = ((CSS1PropEntry*)pFirst)->pName->CompareToAscii(
                        ((CSS1PropEntry*)pSecond)->sName );
        else
            nRet = ((CSS1PropEntry*)pFirst)->pName->CompareTo(
                        *((CSS1PropEntry*)pSecond)->pName );
    }

    return nRet;
}
}

void SvxCSS1Parser::ParseProperty( const String& rProperty,
                                   const CSS1Expression *pExpr )
{
    DBG_ASSERT( pItemSet, "DeclarationParsed() ohne ItemSet" );

    if( !bSortedPropFns )
    {
        qsort( (void*) aCSS1PropFnTab,
                sizeof( aCSS1PropFnTab ) / sizeof( CSS1PropEntry ),
                sizeof( CSS1PropEntry ),
                CSS1PropEntryCompare );
        bSortedPropFns = TRUE;
    }

    String aTmp( rProperty );
    aTmp.ToLowerAscii();

    CSS1PropEntry aSrch;
    aSrch.pName = &aTmp;
    aSrch.pFunc = 0;

    void* pFound;
    if( 0 != ( pFound = bsearch( (char *) &aSrch,
                        (void*) aCSS1PropFnTab,
                        sizeof( aCSS1PropFnTab ) / sizeof( CSS1PropEntry ),
                        sizeof( CSS1PropEntry ),
                        CSS1PropEntryCompare )))
    {
        (((CSS1PropEntry*)pFound)->pFunc)( pExpr, *pItemSet, *pPropInfo, *this );
    }
}
