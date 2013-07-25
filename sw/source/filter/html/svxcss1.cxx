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

#include <stdlib.h>

#include <svx/svxids.hrc>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/urihelper.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/orphitem.hxx>
#include <svtools/svparser.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include "css1kywd.hxx"
#include "svxcss1.hxx"

#include <memory>


using namespace ::com::sun::star;

// die Funktionen zum Parsen einer CSS1-Property sind von folgendem Typ:
typedef void (*FnParseCSS1Prop)( const CSS1Expression *pExpr,
                                 SfxItemSet& rItemSet,
                                 SvxCSS1PropertyInfo& rPropInfo,
                                 const SvxCSS1Parser& rParser );

static CSS1PropertyEnum const aFontSizeTable[] =
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

static CSS1PropertyEnum const aFontFamilyTable[] =
{
    { sCSS1_PV_serif,       FAMILY_ROMAN        },
    { sCSS1_PV_sans_serif,  FAMILY_SWISS        },
    { sCSS1_PV_cursive,     FAMILY_SCRIPT       },
    { sCSS1_PV_fantasy,     FAMILY_DECORATIVE   },
    { sCSS1_PV_monospace,   FAMILY_MODERN       },
    { 0,                    0                   }
};

static CSS1PropertyEnum const aFontWeightTable[] =
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

static CSS1PropertyEnum const aFontStyleTable[] =
{
    { sCSS1_PV_normal,      ITALIC_NONE         },
    { sCSS1_PV_italic,      ITALIC_NORMAL       },
    { sCSS1_PV_oblique,     ITALIC_NORMAL       },
    { 0,                    0                   }
};

static CSS1PropertyEnum const aFontVariantTable[] =
{
    { sCSS1_PV_normal,      SVX_CASEMAP_NOT_MAPPED      },
    { sCSS1_PV_small_caps,  SVX_CASEMAP_KAPITAELCHEN    },
    { 0,                    0                   }
};

static CSS1PropertyEnum const aTextTransformTable[] =
{
    { sCSS1_PV_uppercase,  SVX_CASEMAP_VERSALIEN },
    { sCSS1_PV_lowercase,  SVX_CASEMAP_GEMEINE   },
    { sCSS1_PV_capitalize, SVX_CASEMAP_TITEL     },
    { 0,                   0                     }
};

static CSS1PropertyEnum const aDirectionTable[] =
{
    { sCSS1_PV_ltr,         FRMDIR_HORI_LEFT_TOP        },
    { sCSS1_PV_rtl,         FRMDIR_HORI_RIGHT_TOP       },
    { sCSS1_PV_inherit,     FRMDIR_ENVIRONMENT          },
    { 0,                    0                   }
};

static CSS1PropertyEnum const aBGRepeatTable[] =
{
    { sCSS1_PV_repeat,      GPOS_TILED                  },
    { sCSS1_PV_repeat_x,    GPOS_TILED                  },
    { sCSS1_PV_repeat_y,    GPOS_TILED                  },
    { sCSS1_PV_no_repeat,   GPOS_NONE                   },
    { 0,                    0                           }
};

static CSS1PropertyEnum const aBGHoriPosTable[] =
{
    { sCSS1_PV_left,        GPOS_LT                 },
    { sCSS1_PV_center,      GPOS_MT                 },
    { sCSS1_PV_right,       GPOS_RT                 },
    { 0,                    0                       }
};

static CSS1PropertyEnum const aBGVertPosTable[] =
{
    { sCSS1_PV_top,         GPOS_LT                 },
    { sCSS1_PV_middle,      GPOS_LM                 },
    { sCSS1_PV_bottom,      GPOS_LB                 },
    { 0,                    0                       }
};

static CSS1PropertyEnum const aTextAlignTable[] =
{
    { sCSS1_PV_left,        SVX_ADJUST_LEFT     },
    { sCSS1_PV_center,      SVX_ADJUST_CENTER   },
    { sCSS1_PV_right,       SVX_ADJUST_RIGHT    },
    { sCSS1_PV_justify,     SVX_ADJUST_BLOCK    },
    { 0,                    0                   }
};

static CSS1PropertyEnum const aBorderWidthTable[] =
{
    { sCSS1_PV_thin,        0   },  // DEF_LINE_WIDTH_0 / DEF_DOUBLE_LINE0
    { sCSS1_PV_medium,      1   },  // DEF_LINE_WIDTH_1 / DEF_DOUBLE_LINE1
    { sCSS1_PV_thick,       2   },  // DEF_LINE_WIDTH_2 / DEF_DOUBLE_LINE2
    { 0,                    0   }
};

enum CSS1BorderStyle { CSS1_BS_NONE, CSS1_BS_SINGLE, CSS1_BS_DOUBLE, CSS1_BS_DOTTED, CSS1_BS_DASHED, CSS1_BS_GROOVE, CSS1_BS_RIDGE, CSS1_BS_INSET, CSS1_BS_OUTSET };

static CSS1PropertyEnum const aBorderStyleTable[] =
{
    { sCSS1_PV_none,        CSS1_BS_NONE        },
    { sCSS1_PV_dotted,      CSS1_BS_DOTTED      },
    { sCSS1_PV_dashed,      CSS1_BS_DASHED      },
    { sCSS1_PV_solid,       CSS1_BS_SINGLE      },
    { sCSS1_PV_double,      CSS1_BS_DOUBLE      },
    { sCSS1_PV_groove,      CSS1_BS_GROOVE      },
    { sCSS1_PV_ridge,       CSS1_BS_RIDGE       },
    { sCSS1_PV_inset,       CSS1_BS_INSET       },
    { sCSS1_PV_outset,      CSS1_BS_OUTSET      },
    { 0,                    0                   }
};

static CSS1PropertyEnum const aFloatTable[] =
{
    { sCSS1_PV_left,    SVX_ADJUST_LEFT         },
    { sCSS1_PV_right,   SVX_ADJUST_RIGHT        },
    { sCSS1_PV_none,    SVX_ADJUST_END          },
    { 0,                0                       }
};

static CSS1PropertyEnum const aPositionTable[] =
{
    { sCSS1_PV_absolute,    SVX_CSS1_POS_ABSOLUTE   },
    { sCSS1_PV_relative,    SVX_CSS1_POS_RELATIVE   },
    { sCSS1_PV_static,      SVX_CSS1_POS_STATIC     },
    { 0,                    0                       }
};

// Feature: PrintExt
static CSS1PropertyEnum const aSizeTable[] =
{
    { sCSS1_PV_auto,        SVX_CSS1_STYPE_AUTO         },
    { sCSS1_PV_landscape,   SVX_CSS1_STYPE_LANDSCAPE    },
    { sCSS1_PV_portrait,    SVX_CSS1_STYPE_PORTRAIT     },
    { 0,                    0                           }
};

static CSS1PropertyEnum const aPageBreakTable[] =
{
    { sCSS1_PV_auto,        SVX_CSS1_PBREAK_AUTO        },
    { sCSS1_PV_always,      SVX_CSS1_PBREAK_ALWAYS      },
    { sCSS1_PV_avoid,       SVX_CSS1_PBREAK_AVOID       },
    { sCSS1_PV_left,        SVX_CSS1_PBREAK_LEFT        },
    { sCSS1_PV_right,       SVX_CSS1_PBREAK_RIGHT       },
    { 0,                    0                           }
};

// /Feature: PrintExt

static sal_uInt16 const aBorderWidths[] =
{
    DEF_LINE_WIDTH_0,
    DEF_LINE_WIDTH_5,
    DEF_LINE_WIDTH_1
};

#undef SBORDER_ENTRY
#undef DBORDER_ENTRY

struct SvxCSS1ItemIds
{
    sal_uInt16 nFont;
    sal_uInt16 nFontCJK;
    sal_uInt16 nFontCTL;
    sal_uInt16 nPosture;
    sal_uInt16 nPostureCJK;
    sal_uInt16 nPostureCTL;
    sal_uInt16 nWeight;
    sal_uInt16 nWeightCJK;
    sal_uInt16 nWeightCTL;
    sal_uInt16 nFontHeight;
    sal_uInt16 nFontHeightCJK;
    sal_uInt16 nFontHeightCTL;
    sal_uInt16 nUnderline;
    sal_uInt16 nOverline;
    sal_uInt16 nCrossedOut;
    sal_uInt16 nColor;
    sal_uInt16 nKerning;
    sal_uInt16 nCaseMap;
    sal_uInt16 nBlink;

    sal_uInt16 nLineSpacing;
    sal_uInt16 nAdjust;
    sal_uInt16 nWidows;
    sal_uInt16 nOrphans;
    sal_uInt16 nFmtSplit;

    sal_uInt16 nLRSpace;
    sal_uInt16 nULSpace;
    sal_uInt16 nBox;
    sal_uInt16 nBrush;

    sal_uInt16 nLanguage;
    sal_uInt16 nLanguageCJK;
    sal_uInt16 nLanguageCTL;
    sal_uInt16 nDirection;
};


static SvxCSS1ItemIds aItemIds;



struct SvxCSS1BorderInfo
{
    Color aColor;
    sal_uInt16 nAbsWidth;
    sal_uInt16 nNamedWidth;
    CSS1BorderStyle eStyle;

    SvxCSS1BorderInfo() :
        aColor( COL_BLACK ), nAbsWidth( USHRT_MAX ),
        nNamedWidth( USHRT_MAX ), eStyle( CSS1_BS_NONE )
    {}

    SvxCSS1BorderInfo( const SvxCSS1BorderInfo& rInfo ) :
        aColor( rInfo.aColor ), nAbsWidth( rInfo.nAbsWidth ),
        nNamedWidth( rInfo.nNamedWidth ), eStyle( rInfo.eStyle )
    {}

    void SetBorderLine( sal_uInt16 nLine, SvxBoxItem &rBoxItem ) const;
};

void SvxCSS1BorderInfo::SetBorderLine( sal_uInt16 nLine, SvxBoxItem &rBoxItem ) const
{
    if( CSS1_BS_NONE==eStyle || nAbsWidth==0 ||
        (nAbsWidth==USHRT_MAX && nNamedWidth==USHRT_MAX) )
    {
        rBoxItem.SetLine( 0, nLine );
        return;
    }

    ::editeng::SvxBorderLine aBorderLine( &aColor );

    // Linien-Stil doppelt oder einfach?
    switch ( eStyle )
    {
        case CSS1_BS_SINGLE:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::SOLID);
            break;
        case CSS1_BS_DOUBLE:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOUBLE);
            break;
        case CSS1_BS_DOTTED:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DOTTED);
            break;
        case CSS1_BS_DASHED:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::DASHED);
            break;
        case CSS1_BS_GROOVE:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::ENGRAVED);
            break;
        case CSS1_BS_RIDGE:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::EMBOSSED);
            break;
        case CSS1_BS_INSET:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::INSET);
            break;
        case CSS1_BS_OUTSET:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::OUTSET);
            break;
        default:
            aBorderLine.SetBorderLineStyle(table::BorderLineStyle::NONE);
            break;
    }

    // benannte Breite umrechnenen, wenn keine absolute gegeben ist
    if( nAbsWidth==USHRT_MAX )
        aBorderLine.SetWidth( aBorderWidths[ nNamedWidth ] );
    else
        aBorderLine.SetWidth( nAbsWidth );

    rBoxItem.SetLine( &aBorderLine, nLine );
}

SvxCSS1PropertyInfo::SvxCSS1PropertyInfo()
{
    for( sal_uInt16 i=0; i<4; i++ )
        aBorderInfos[i] = 0;

    Clear();
}

SvxCSS1PropertyInfo::SvxCSS1PropertyInfo( const SvxCSS1PropertyInfo& rProp ) :
    aId( rProp.aId ),
    bTopMargin( rProp.bTopMargin ),
    bBottomMargin( rProp.bBottomMargin ),
    bLeftMargin( rProp.bLeftMargin ),
    bRightMargin( rProp.bRightMargin ),
    bTextIndent( rProp.bTextIndent ),
    eFloat( rProp.eFloat ),
    ePosition( rProp.ePosition ),
    nTopBorderDistance( rProp.nTopBorderDistance ),
    nBottomBorderDistance( rProp.nBottomBorderDistance ),
    nLeftBorderDistance( rProp.nLeftBorderDistance ),
    nRightBorderDistance( rProp.nRightBorderDistance ),
    nColumnCount( rProp.nColumnCount ),
    nLeft( rProp.nLeft ),
    nTop( rProp.nTop ),
    nWidth( rProp.nWidth ),
    nHeight( rProp.nHeight ),
    nLeftMargin( rProp.nLeftMargin ),
    nRightMargin( rProp.nRightMargin ),
    eLeftType( rProp.eLeftType ),
    eTopType( rProp.eTopType ),
    eWidthType( rProp.eWidthType ),
    eHeightType( rProp.eHeightType ),
// Feature: PrintExt
    eSizeType( rProp.eSizeType ),
    ePageBreakBefore( rProp.ePageBreakBefore ),
    ePageBreakAfter( rProp.ePageBreakAfter )
// /Feature: PrintExt
{
    for( sal_uInt16 i=0; i<4; i++ )
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
    for( sal_uInt16 i=0; i<4; i++ )
    {
        delete aBorderInfos[i];
        aBorderInfos[i] = 0;
    }
}

void SvxCSS1PropertyInfo::Clear()
{
    aId.Erase();
    bTopMargin = bBottomMargin = sal_False;
    bLeftMargin = bRightMargin = bTextIndent = sal_False;
    nLeftMargin = nRightMargin = 0;
    eFloat = SVX_ADJUST_END;

    ePosition = SVX_CSS1_POS_NONE;
    nTopBorderDistance = nBottomBorderDistance =
    nLeftBorderDistance = nRightBorderDistance = USHRT_MAX;

    nColumnCount = 0;

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
        bTopMargin = sal_True;
    if( rProp.bBottomMargin )
        bBottomMargin = sal_True;

    if( rProp.bLeftMargin )
    {
        bLeftMargin = sal_True;
        nLeftMargin = rProp.nLeftMargin;
    }
    if( rProp.bRightMargin )
    {
        bRightMargin = sal_True;
        nRightMargin = rProp.nRightMargin;
    }
    if( rProp.bTextIndent )
        bTextIndent = sal_True;

    for( sal_uInt16 i=0; i<4; i++ )
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

    nColumnCount = rProp.nColumnCount;

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

SvxCSS1BorderInfo *SvxCSS1PropertyInfo::GetBorderInfo( sal_uInt16 nLine, sal_Bool bCreate )
{
    sal_uInt16 nPos = 0;
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

void SvxCSS1PropertyInfo::CopyBorderInfo( sal_uInt16 nSrcLine, sal_uInt16 nDstLine,
                                          sal_uInt16 nWhat )
{
    SvxCSS1BorderInfo *pSrcInfo = GetBorderInfo( nSrcLine, sal_False );
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

void SvxCSS1PropertyInfo::CopyBorderInfo( sal_uInt16 nCount, sal_uInt16 nWhat )
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
                                      sal_uInt16 nMinBorderDist,
                                      const SvxBoxItem *pDfltItem,
                                      bool bTable )
{
    bool bChg = nTopBorderDistance != USHRT_MAX ||
                nBottomBorderDistance != USHRT_MAX ||
                nLeftBorderDistance != USHRT_MAX ||
                nRightBorderDistance != USHRT_MAX;
    sal_uInt16 i;

    for( i = 0; !bChg && i < 4; i++ )
        bChg = aBorderInfos[i]!=0;

    if( !bChg )
        return;

    SvxBoxItem aBoxItem( aItemIds.nBox );
    if( pDfltItem )
        aBoxItem = *pDfltItem;

    SvxCSS1BorderInfo *pInfo = GetBorderInfo( BOX_LINE_TOP, sal_False );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_TOP, aBoxItem );

    pInfo = GetBorderInfo( BOX_LINE_BOTTOM, sal_False );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_BOTTOM, aBoxItem );

    pInfo = GetBorderInfo( BOX_LINE_LEFT, sal_False );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_LEFT, aBoxItem );

    pInfo = GetBorderInfo( BOX_LINE_RIGHT, sal_False );
    if( pInfo )
        pInfo->SetBorderLine( BOX_LINE_RIGHT, aBoxItem );

    for( i=0; i<4; i++ )
    {
        sal_uInt16 nLine = BOX_LINE_TOP, nDist = 0;
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

SvxCSS1MapEntry::SvxCSS1MapEntry( const String& rKey, const SfxItemSet& rItemSet,
                                  const SvxCSS1PropertyInfo& rProp ) :
    aKey( rKey ),
    aItemSet( rItemSet ),
    aPropInfo( rProp )
{
    // TODO: ToUpperAscii
    aKey.ToUpperAscii();
}

sal_Bool SvxCSS1Parser::StyleParsed( const CSS1Selector * /*pSelector*/,
                                 SfxItemSet& /*rItemSet*/,
                                 SvxCSS1PropertyInfo& /*rPropInfo*/ )
{
    // wie man sieht passiert hier gar nichts
    return sal_True;
}

bool SvxCSS1Parser::SelectorParsed( CSS1Selector *pSelector, bool bFirst )
{
    if( bFirst )
    {
        OSL_ENSURE( pSheetItemSet, "Where is the Item-Set for Style-Sheets?" );

        for (size_t i = 0; i < aSelectors.size(); ++i)
        {
            StyleParsed( &aSelectors[i], *pSheetItemSet, *pSheetPropInfo );
        }
        pSheetItemSet->ClearItem();
        pSheetPropInfo->Clear();

        // und die naechste Rule vorbereiten
        aSelectors.clear();
    }

    aSelectors.push_back(pSelector);

    return false; // den Selektor haben wir gespeichert. Loeschen toedlich!
}


sal_Bool SvxCSS1Parser::DeclarationParsed( const OUString& rProperty,
                                       const CSS1Expression *pExpr )
{
    OSL_ENSURE( pExpr, "DeclarationParsed() without Expression" );

    if( !pExpr )
        return sal_True;

    ParseProperty( rProperty, pExpr );

    return sal_True;    // die Deklaration brauchen wir nicht mehr. Loeschen!
}


SvxCSS1Parser::SvxCSS1Parser( SfxItemPool& rPool, const OUString& rBaseURL, sal_uInt16 nMinFixLineSp,
                              sal_uInt16 *pWhichIds, sal_uInt16 nWhichIds ) :
    CSS1Parser(),
    sBaseURL( rBaseURL ),
    pSheetItemSet(0),
    pItemSet(0),
    pSearchEntry( 0 ),
    pPropInfo( 0 ),
    nMinFixLineSpace( nMinFixLineSp ),
    eDfltEnc( RTL_TEXTENCODING_DONTKNOW ),
    nScriptFlags( CSS1_SCRIPT_ALL ),
    bIgnoreFontFamily( sal_False )
{
    // Item-Ids auch initialisieren
    aItemIds.nFont = rPool.GetTrueWhich( SID_ATTR_CHAR_FONT, sal_False );
    aItemIds.nFontCJK = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONT, sal_False );
    aItemIds.nFontCTL = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONT, sal_False );
    aItemIds.nPosture = rPool.GetTrueWhich( SID_ATTR_CHAR_POSTURE, sal_False );
    aItemIds.nPostureCJK = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_POSTURE, sal_False );
    aItemIds.nPostureCTL = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_POSTURE, sal_False );
    aItemIds.nWeight = rPool.GetTrueWhich( SID_ATTR_CHAR_WEIGHT, sal_False );
    aItemIds.nWeightCJK = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_WEIGHT, sal_False );
    aItemIds.nWeightCTL = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_WEIGHT, sal_False );
    aItemIds.nFontHeight = rPool.GetTrueWhich( SID_ATTR_CHAR_FONTHEIGHT, sal_False );
    aItemIds.nFontHeightCJK = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT, sal_False );
    aItemIds.nFontHeightCTL = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT, sal_False );
    aItemIds.nUnderline = rPool.GetTrueWhich( SID_ATTR_CHAR_UNDERLINE, sal_False );
    aItemIds.nOverline = rPool.GetTrueWhich( SID_ATTR_CHAR_OVERLINE, sal_False );
    aItemIds.nCrossedOut = rPool.GetTrueWhich( SID_ATTR_CHAR_STRIKEOUT, sal_False );
    aItemIds.nColor = rPool.GetTrueWhich( SID_ATTR_CHAR_COLOR, sal_False );
    aItemIds.nKerning = rPool.GetTrueWhich( SID_ATTR_CHAR_KERNING, sal_False );
    aItemIds.nCaseMap = rPool.GetTrueWhich( SID_ATTR_CHAR_CASEMAP, sal_False );
    aItemIds.nBlink = rPool.GetTrueWhich( SID_ATTR_FLASH, sal_False );

    aItemIds.nLineSpacing = rPool.GetTrueWhich( SID_ATTR_PARA_LINESPACE, sal_False );
    aItemIds.nAdjust = rPool.GetTrueWhich( SID_ATTR_PARA_ADJUST, sal_False );
    aItemIds.nWidows = rPool.GetTrueWhich( SID_ATTR_PARA_WIDOWS, sal_False );
    aItemIds.nOrphans = rPool.GetTrueWhich( SID_ATTR_PARA_ORPHANS, sal_False );
    aItemIds.nFmtSplit = rPool.GetTrueWhich( SID_ATTR_PARA_SPLIT, sal_False );

    aItemIds.nLRSpace = rPool.GetTrueWhich( SID_ATTR_LRSPACE, sal_False );
    aItemIds.nULSpace = rPool.GetTrueWhich( SID_ATTR_ULSPACE, sal_False );
    aItemIds.nBox = rPool.GetTrueWhich( SID_ATTR_BORDER_OUTER, sal_False );
    aItemIds.nBrush = rPool.GetTrueWhich( SID_ATTR_BRUSH, sal_False );

    aItemIds.nLanguage = rPool.GetTrueWhich( SID_ATTR_CHAR_LANGUAGE, sal_False );
    aItemIds.nLanguageCJK = rPool.GetTrueWhich( SID_ATTR_CHAR_CJK_LANGUAGE, sal_False );
    aItemIds.nLanguageCTL = rPool.GetTrueWhich( SID_ATTR_CHAR_CTL_LANGUAGE, sal_False );
    aItemIds.nDirection = rPool.GetTrueWhich( SID_ATTR_FRAMEDIRECTION, sal_False );

    aWhichMap.insert( aWhichMap.begin(), 0 );
    SvParser::BuildWhichTbl( aWhichMap, (sal_uInt16 *)&aItemIds,
                             sizeof(aItemIds) / sizeof(sal_uInt16) );
    if( pWhichIds && nWhichIds )
        SvParser::BuildWhichTbl( aWhichMap, pWhichIds, nWhichIds );

    pSheetItemSet = new SfxItemSet( rPool, &aWhichMap[0] );
    pSheetPropInfo = new SvxCSS1PropertyInfo;
    pSearchEntry = new SvxCSS1MapEntry( rPool, &aWhichMap[0] );
}

SvxCSS1Parser::~SvxCSS1Parser()
{
    delete pSheetItemSet;
    delete pSheetPropInfo;
    delete pSearchEntry;
}

void SvxCSS1Parser::InsertId( const OUString& rId,
                              const SfxItemSet& rItemSet,
                              const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rId, rItemSet, rProp, aIds );
}

const SvxCSS1MapEntry* SvxCSS1Parser::GetId( const OUString& rId ) const
{
    CSS1Map::const_iterator itr = aIds.find(rId);
    return itr == aIds.end() ? NULL : itr->second;
}

void SvxCSS1Parser::InsertClass( const OUString& rClass,
                                 const SfxItemSet& rItemSet,
                                 const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rClass, rItemSet, rProp, aClasses );
}

const SvxCSS1MapEntry* SvxCSS1Parser::GetClass( const OUString& rClass ) const
{
    CSS1Map::const_iterator itr = aClasses.find(rClass);
    return itr == aClasses.end() ? NULL : itr->second;
}

void SvxCSS1Parser::InsertPage( const OUString& rPage,
                                sal_Bool bPseudo,
                                const SfxItemSet& rItemSet,
                                const SvxCSS1PropertyInfo& rProp )
{
    OUString aKey( rPage );
    if( bPseudo )
        aKey = ":" + aKey;
    InsertMapEntry( aKey, rItemSet, rProp, aPages );
}

SvxCSS1MapEntry* SvxCSS1Parser::GetPage( const OUString& rPage, bool bPseudo )
{
    OUString aKey( rPage );
    if( bPseudo )
        aKey = ":" + aKey;

    CSS1Map::iterator itr = aPages.find(aKey);
    return itr == aPages.end() ? NULL : itr->second;
}

void SvxCSS1Parser::InsertTag( const OUString& rTag,
                               const SfxItemSet& rItemSet,
                               const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rTag, rItemSet, rProp, aTags );
}

SvxCSS1MapEntry* SvxCSS1Parser::GetTag( const OUString& rTag )
{
    CSS1Map::iterator itr = aTags.find(rTag);
    return itr == aTags.end() ? NULL : itr->second;
}

sal_Bool SvxCSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    pItemSet = pSheetItemSet;
    pPropInfo = pSheetPropInfo;

    sal_Bool bSuccess = CSS1Parser::ParseStyleSheet( rIn );

    for (size_t i = 0; i < aSelectors.size(); ++i)
    {
        StyleParsed( &aSelectors[i], *pSheetItemSet, *pSheetPropInfo );
    }

    // und etwas aufrauemen
    aSelectors.clear();
    pSheetItemSet->ClearItem();
    pSheetPropInfo->Clear();

    pItemSet = 0;
    pPropInfo = 0;

    return bSuccess;
}

sal_Bool SvxCSS1Parser::ParseStyleOption( const OUString& rIn,
                                      SfxItemSet& rItemSet,
                                      SvxCSS1PropertyInfo& rPropInfo )
{
    pItemSet = &rItemSet;
    pPropInfo = &rPropInfo;

    sal_Bool bSuccess = CSS1Parser::ParseStyleOption( rIn );
    rItemSet.ClearItem( aItemIds.nDirection );

    pItemSet = 0;
    pPropInfo = 0;

    return bSuccess;
}


sal_Bool SvxCSS1Parser::GetEnum( const CSS1PropertyEnum *pPropTable,
                          const OUString &rValue, sal_uInt16& rEnum )
{
    OUString aValue( rValue.toAsciiLowerCase() );

    while( pPropTable->pName )
    {
        if( !rValue.equalsIgnoreAsciiCaseAscii( pPropTable->pName ) )
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

sal_uInt32 SvxCSS1Parser::GetFontHeight( sal_uInt16 nSize ) const
{
    sal_uInt16 nHeight;

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

void SvxCSS1Parser::InsertMapEntry( const OUString& rKey,
                                    const SfxItemSet& rItemSet,
                                    const SvxCSS1PropertyInfo& rProp,
                                    CSS1Map& rMap )
{
    CSS1Map::iterator itr = rMap.find(rKey);
    if (itr == rMap.end())
    {
        std::auto_ptr<SvxCSS1MapEntry> p(new SvxCSS1MapEntry(rKey, rItemSet, rProp));
        rMap.insert(rKey, p);
    }
    else
    {
        SvxCSS1MapEntry* p = itr->second;
        MergeStyles( rItemSet, rProp,
                     p->GetItemSet(), p->GetPropertyInfo(), sal_True );
    }
}

void SvxCSS1Parser::MergeStyles( const SfxItemSet& rSrcSet,
                                 const SvxCSS1PropertyInfo& rSrcInfo,
                                 SfxItemSet& rTargetSet,
                                 SvxCSS1PropertyInfo& rTargetInfo,
                                 sal_Bool bSmart )
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


static void ParseCSS1_font_size( const CSS1Expression *pExpr,
                                 SfxItemSet &rItemSet,
                                 SvxCSS1PropertyInfo& /*rPropInfo*/,
                                 const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uLong nHeight = 0;
    sal_uInt16 nPropHeight = 100;

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
            nHeight = (sal_uLong)nPHeight;
        }
        break;
    case CSS1_PERCENTAGE:
        // nur fuer Drop-Caps!
        nPropHeight = (sal_uInt16)pExpr->GetNumber();
        break;
    case CSS1_IDENT:
        {
            sal_uInt16 nSize;

            if( SvxCSS1Parser::GetEnum( aFontSizeTable, pExpr->GetString(),
                                        nSize ) )
            {
                nHeight = rParser.GetFontHeight( nSize );
            }
        }
        break;

    default:
        ;
    }

    if( nHeight || nPropHeight!=100 )
    {
        SvxFontHeightItem aFontHeight( nHeight, nPropHeight,
                                       aItemIds.nFontHeight );
        if( rParser.IsSetWesternProps() )
            rItemSet.Put( aFontHeight );
        if( rParser.IsSetCJKProps() )
        {
            aFontHeight.SetWhich( aItemIds.nFontHeightCJK );
            rItemSet.Put( aFontHeight );
        }
        if( rParser.IsSetCTLProps() )
        {
            aFontHeight.SetWhich( aItemIds.nFontHeightCTL );
            rItemSet.Put( aFontHeight );
        }
    }
}



static void ParseCSS1_font_family( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& /*rPropInfo*/,
                                   const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    String aName, aStyleName;
    FontFamily eFamily = FAMILY_DONTKNOW;
    FontPitch ePitch = PITCH_DONTKNOW;
    rtl_TextEncoding eEnc = rParser.GetDfltEncoding();
    const FontList *pFList = rParser.GetFontList();
    sal_Bool bFirst = sal_True;
    sal_Bool bFound = sal_False;
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
                            bFound = sal_True;
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
        bFirst = sal_False;
    }

    if( aName.Len() && !rParser.IsIgnoreFontFamily() )
    {
        SvxFontItem aFont( eFamily, aName, aStyleName, ePitch,
                            eEnc, aItemIds.nFont );
        if( rParser.IsSetWesternProps() )
            rItemSet.Put( aFont );
        if( rParser.IsSetCJKProps() )
        {
            aFont.SetWhich( aItemIds.nFontCJK );
            rItemSet.Put( aFont );
        }
        if( rParser.IsSetCTLProps() )
        {
            aFont.SetWhich( aItemIds.nFontCTL );
            rItemSet.Put( aFont );
        }
    }
}


static void ParseCSS1_font_weight( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& /*rPropInfo*/,
                                   const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
    case CSS1_STRING:   // MS-IE, was sonst
        {
            sal_uInt16 nWeight;
            if( SvxCSS1Parser::GetEnum( aFontWeightTable, pExpr->GetString(),
                                        nWeight ) )
            {
                SvxWeightItem aWeight( (FontWeight)nWeight, aItemIds.nWeight );
                if( rParser.IsSetWesternProps() )
                    rItemSet.Put( aWeight );
                if( rParser.IsSetCJKProps() )
                {
                    aWeight.SetWhich( aItemIds.nWeightCJK );
                    rItemSet.Put( aWeight );
                }
                if( rParser.IsSetCTLProps() )
                {
                    aWeight.SetWhich( aItemIds.nWeightCTL );
                    rItemSet.Put( aWeight );
                }
            }
        }
        break;
    case CSS1_NUMBER:
        {
            sal_uInt16 nWeight = (sal_uInt16)pExpr->GetNumber();
            SvxWeightItem aWeight( nWeight>400 ? WEIGHT_BOLD : WEIGHT_NORMAL,
                                   aItemIds.nWeight );
            if( rParser.IsSetWesternProps() )
                rItemSet.Put( aWeight );
            if( rParser.IsSetCJKProps() )
            {
                aWeight.SetWhich( aItemIds.nWeightCJK );
                rItemSet.Put( aWeight );
            }
            if( rParser.IsSetCTLProps() )
            {
                aWeight.SetWhich( aItemIds.nWeightCTL );
                rItemSet.Put( aWeight );
            }
        }
        break;

    default:
        ;
    }
}


static void ParseCSS1_font_style( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& /*rPropInfo*/,
                                  const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_Bool bPosture = sal_False;
    sal_Bool bCaseMap = sal_False;
    FontItalic eItalic = ITALIC_NONE;
    SvxCaseMap eCaseMap = SVX_CASEMAP_NOT_MAPPED;

    // normal | italic || small-caps | oblique || small-caps | small-caps
    // (wobei nor noch normal | italic und oblique zulaessig sind

    // der Wert kann zwei Werte enthalten!
    for( sal_uInt16 i=0; pExpr && i<2; i++ )
    {
        // Auch hier hinterlaesst MS-IEs Parser seine Spuren
        if( (CSS1_IDENT==pExpr->GetType() || CSS1_STRING==pExpr->GetType()) &&
            !pExpr->GetOp() )
        {
            const String& rValue = pExpr->GetString();
            // erstmal pruefen, ob es ein Italic-Wert oder 'normal' ist
            sal_uInt16 nItalic;
            if( SvxCSS1Parser::GetEnum( aFontStyleTable, rValue, nItalic ) )
            {
                eItalic = (FontItalic)nItalic;
                if( !bCaseMap && ITALIC_NONE==eItalic )
                {
                    // fuer 'normal' muessen wir auch die case-map aussch.
                    eCaseMap = SVX_CASEMAP_NOT_MAPPED;
                    bCaseMap = sal_True;
                }
                bPosture = sal_True;
            }
            else if( !bCaseMap &&
                     rValue.EqualsIgnoreCaseAscii(sCSS1_PV_small_caps) )
            {
                eCaseMap = SVX_CASEMAP_KAPITAELCHEN;
                bCaseMap = sal_True;
            }
        }

        // den naechsten Ausdruck holen
        pExpr = pExpr->GetNext();
    }

    if( bPosture )
    {
        SvxPostureItem aPosture( eItalic, aItemIds.nPosture );
        if( rParser.IsSetWesternProps() )
            rItemSet.Put( aPosture );
        if( rParser.IsSetCJKProps() )
        {
            aPosture.SetWhich( aItemIds.nPostureCJK );
            rItemSet.Put( aPosture );
        }
        if( rParser.IsSetCTLProps() )
        {
            aPosture.SetWhich( aItemIds.nPostureCTL );
            rItemSet.Put( aPosture );
        }
    }

    if( bCaseMap )
        rItemSet.Put( SvxCaseMapItem( eCaseMap, aItemIds.nCaseMap ) );
}


static void ParseCSS1_font_variant( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& /*rPropInfo*/,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    // normal | small-caps

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            sal_uInt16 nCaseMap;
            if( SvxCSS1Parser::GetEnum( aFontVariantTable, pExpr->GetString(),
                                        nCaseMap ) )
            {
                rItemSet.Put( SvxCaseMapItem( (SvxCaseMap)nCaseMap,
                                                aItemIds.nCaseMap ) );
            }
        }
    default:
        ;
    }
}

static void ParseCSS1_text_transform( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& /*rPropInfo*/,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    // none | capitalize | uppercase | lowercase

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            sal_uInt16 nCaseMap;
            if( SvxCSS1Parser::GetEnum( aTextTransformTable, pExpr->GetString(),
                                        nCaseMap ) )
            {
                rItemSet.Put( SvxCaseMapItem( (SvxCaseMap)nCaseMap,
                                                aItemIds.nCaseMap ) );
            }
        }
    default:
        ;
    }
}


static void ParseCSS1_color( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& /*rPropInfo*/,
                             const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

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
    default:
        ;
    }
}


static void ParseCSS1_column_count( const CSS1Expression *pExpr,
                             SfxItemSet& /*rItemSet*/,
                             SvxCSS1PropertyInfo &rPropInfo,
                             const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    if ( pExpr->GetType() == CSS1_NUMBER )
    {
        double columnCount = pExpr->GetNumber();
        if ( columnCount >= 2 )
        {
            rPropInfo.nColumnCount = columnCount;
        }
    }
}


static void ParseCSS1_direction( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& /*rPropInfo*/,
                             const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uInt16 nDir;
    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
    case CSS1_STRING:
        if( SvxCSS1Parser::GetEnum( aDirectionTable, pExpr->GetString(),
                                        nDir ) )
        {
            rItemSet.Put( SvxFrameDirectionItem(
                       static_cast < SvxFrameDirection >( nDir ),
                       aItemIds.nDirection ) );
        }
        break;
    default:
        ;
    }
}


static void MergeHori( SvxGraphicPosition& ePos, SvxGraphicPosition eHori )
{
    OSL_ENSURE( GPOS_LT==eHori || GPOS_MT==eHori || GPOS_RT==eHori,
                "vertical position not at the top" );

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

    default:
        ;
    }
}

static void MergeVert( SvxGraphicPosition& ePos, SvxGraphicPosition eVert )
{
    OSL_ENSURE( GPOS_LT==eVert || GPOS_LM==eVert || GPOS_LB==eVert,
                "horizontal position not on the left side" );

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

    default:
        ;
    }
}

static void ParseCSS1_background( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& /*rPropInfo*/,
                                  const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    Color aColor;
    String aURL;

    sal_Bool bColor = sal_False, bTransparent = sal_False;
    SvxGraphicPosition eRepeat = GPOS_TILED;
    SvxGraphicPosition ePos = GPOS_LT;
    sal_Bool bHori = sal_False, bVert = sal_False;

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

                sal_uLong nLength = (sal_uLong)pExpr->GetNumber();
                if( !bHori )
                {
                    ePos = nLength ? GPOS_MM : GPOS_LT;
                    bHori = sal_True;
                }
                else if( !bVert )
                {
                    MergeVert( ePos, (nLength ? GPOS_LM : GPOS_LT) );
                    bVert = sal_True;
                }
            }
            break;

        case CSS1_PERCENTAGE:
            {
                // die %-Angabe wird auf den enum abgebildet

                sal_uInt16 nPerc = (sal_uInt16)pExpr->GetNumber();
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
                sal_uInt16 nEnum;
                const String &rValue = pExpr->GetString();
                if( rValue.EqualsIgnoreCaseAscii( sCSS1_PV_transparent ) )
                {
                    bTransparent = sal_True;
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

        default:
            ;
        }

        pExpr = pExpr->GetNext();
    }

    // transparent schlaegt alles
    if( bTransparent )
    {
        bColor = sal_False;
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
            aBrushItem.SetGraphicLink( URIHelper::SmartRel2Abs( INetURLObject( rParser.GetBaseURL()), aURL, Link(), false ) );
            aBrushItem.SetGraphicPos( eRepeat );
        }

        rItemSet.Put( aBrushItem );
    }
}

static void ParseCSS1_background_color( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& /*rPropInfo*/,
                                  const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    Color aColor;

    sal_Bool bColor = sal_False, bTransparent = sal_False;

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
            bTransparent = sal_True;
        }
        else
        {
            // <color>
            bColor = pExpr->GetColor( aColor );
        }
        break;
    default:
        ;
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


static void ParseCSS1_line_height( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& /*rPropInfo*/,
                                   const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uInt16 nHeight = 0;
    sal_uInt8 nPropHeight = 0;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nHeight = (sal_uInt16)pExpr->GetULength();
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight = (long)pExpr->GetNumber();
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nHeight = (sal_uInt16)nPHeight;
        }
        break;
    case CSS1_PERCENTAGE:
        {
            sal_uInt16 nPHeight = (sal_uInt16)pExpr->GetNumber();
            nPropHeight = nPHeight <= 200 ? (sal_uInt8)nPHeight : 200;
        }
        break;
    case CSS1_NUMBER:
        {
            sal_uInt16 nPHeight = (sal_uInt16)(pExpr->GetNumber() * 100);
            nPropHeight = nPHeight <= 200 ? (sal_uInt8)nPHeight : 200;
        }
        break;
    default:
        ;
    }

    if( nHeight )
    {
        if( nHeight < rParser.GetMinFixLineSpace() )
            nHeight = rParser.GetMinFixLineSpace();
        SvxLineSpacingItem aLSItem( nHeight, aItemIds.nLineSpacing );
        aLSItem.SetLineHeight( nHeight );
        // interpret <line-height> attribute as minimum line height
        aLSItem.GetLineSpaceRule() = SVX_LINE_SPACE_MIN;
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


static void ParseCSS1_font( const CSS1Expression *pExpr,
                            SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo& rPropInfo,
                            const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

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

            sal_uInt16 nEnum;

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
            eWeight = (sal_uInt16)pExpr->GetNumber() > 400 ? WEIGHT_BOLD
                                                       : WEIGHT_NORMAL;
        }

        pExpr = pExpr->GetNext();
    }

    if( !pExpr || pExpr->GetOp() )
        return;

    // Da "font" alle Werte zurecksetzt, fuer die nichts angegeben ist,
    // tun wir das hier.
    SvxPostureItem aPosture( eItalic, aItemIds.nPosture );
    if( rParser.IsSetWesternProps() )
        rItemSet.Put( aPosture );
    if( rParser.IsSetCJKProps() )
    {
        aPosture.SetWhich( aItemIds.nPostureCJK );
        rItemSet.Put( aPosture );
    }
    if( rParser.IsSetCTLProps() )
    {
        aPosture.SetWhich( aItemIds.nPostureCTL );
        rItemSet.Put( aPosture );
    }

    rItemSet.Put( SvxCaseMapItem( eCaseMap, aItemIds.nCaseMap ) );

    SvxWeightItem aWeight( eWeight, aItemIds.nWeight );
    if( rParser.IsSetWesternProps() )
        rItemSet.Put( aWeight );
    if( rParser.IsSetCJKProps() )
    {
        aWeight.SetWhich( aItemIds.nWeightCJK );
        rItemSet.Put( aWeight );
    }
    if( rParser.IsSetCTLProps() )
    {
        aWeight.SetWhich( aItemIds.nWeightCTL );
        rItemSet.Put( aWeight );
    }


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


static void ParseCSS1_letter_spacing( const CSS1Expression *pExpr,
                                      SfxItemSet &rItemSet,
                                      SvxCSS1PropertyInfo& /*rPropInfo*/,
                                      const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

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
    default:
        ;
    }
}


static void ParseCSS1_text_decoration( const CSS1Expression *pExpr,
                                       SfxItemSet &rItemSet,
                                       SvxCSS1PropertyInfo& /*rPropInfo*/,
                                       const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_Bool bUnderline = sal_False;
    sal_Bool bOverline = sal_False;
    sal_Bool bCrossedOut = sal_False;
    sal_Bool bBlink = sal_False;
    sal_Bool bBlinkOn = sal_False;
    FontUnderline eUnderline  = UNDERLINE_NONE;
    FontUnderline eOverline   = UNDERLINE_NONE;
    FontStrikeout eCrossedOut = STRIKEOUT_NONE;

    // der Wert kann zwei Werte enthalten! Und MS-IE auch Strings
    while( pExpr && (pExpr->GetType() == CSS1_IDENT ||
                     pExpr->GetType() == CSS1_STRING) && !pExpr->GetOp() )
    {
        String aValue = pExpr->GetString();
        aValue.ToLowerAscii();
        sal_Bool bKnown = sal_False;

        switch( aValue.GetChar( 0 ) )
        {
        case 'n':
            if( aValue.EqualsAscii( sCSS1_PV_none ) )
            {
                bUnderline = sal_True;
                eUnderline = UNDERLINE_NONE;

                bOverline = sal_True;
                eOverline = UNDERLINE_NONE;

                bCrossedOut = sal_True;
                eCrossedOut = STRIKEOUT_NONE;

                bBlink = sal_True;
                bBlinkOn = sal_False;

                bKnown = sal_True;
            }
            break;

        case 'u':
            if( aValue.EqualsAscii( sCSS1_PV_underline ) )
            {
                bUnderline = sal_True;
                eUnderline = UNDERLINE_SINGLE;

                bKnown = sal_True;
            }
            break;

        case 'o':
            if( aValue.EqualsAscii( sCSS1_PV_overline ) )
            {
                bOverline = sal_True;
                eOverline = UNDERLINE_SINGLE;

                bKnown = sal_True;
            }
            break;

        case 'l':
            if( aValue.EqualsAscii( sCSS1_PV_line_through ) )
            {
                bCrossedOut = sal_True;
                eCrossedOut = STRIKEOUT_SINGLE;

                bKnown = sal_True;
            }
            break;

        case 'b':
            if( aValue.EqualsAscii( sCSS1_PV_blink ) )
            {
                bBlink = sal_True;
                bBlinkOn = sal_True;

                bKnown = sal_True;
            }
            break;
        }

        if( !bKnown )
        {
            bUnderline = sal_True;
            eUnderline = UNDERLINE_SINGLE;
        }

        pExpr = pExpr->GetNext();
    }

    if( bUnderline )
        rItemSet.Put( SvxUnderlineItem( eUnderline, aItemIds.nUnderline ) );

    if( bOverline )
        rItemSet.Put( SvxOverlineItem( eOverline, aItemIds.nOverline ) );

    if( bCrossedOut )
        rItemSet.Put( SvxCrossedOutItem( eCrossedOut, aItemIds.nCrossedOut ) );

    if( bBlink )
        rItemSet.Put( SvxBlinkItem( bBlinkOn, aItemIds.nBlink ) );
}


static void ParseCSS1_text_align( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& /*rPropInfo*/,
                                  const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    if( CSS1_IDENT==pExpr->GetType() ||
        CSS1_STRING==pExpr->GetType() ) // MS-IE, mal wieder
    {
        sal_uInt16 nAdjust;
        if( SvxCSS1Parser::GetEnum( aTextAlignTable, pExpr->GetString(),
                                    nAdjust ) )
        {
            rItemSet.Put( SvxAdjustItem( (SvxAdjust)nAdjust,
                                         aItemIds.nAdjust ) );
        }
    }
}


static void ParseCSS1_text_indent( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    short nIndent = 0;
    sal_Bool bSet = sal_False;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nIndent = (short)pExpr->GetSLength();
        bSet = sal_True;
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nIndent = (short)nPWidth;
            bSet = sal_True;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    default:
        ;
    }

    if( bSet )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, sal_False,
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
        rPropInfo.bTextIndent = sal_True;
    }
}


static void ParseCSS1_margin_left( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    long nLeft = 0;
    sal_Bool bSet = sal_False;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            nLeft = pExpr->GetSLength();
            bSet = sal_True;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            nLeft = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nLeft, nPHeight );
            bSet = sal_True;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    default:
        ;
    }

    if( bSet )
    {
        rPropInfo.nLeftMargin = nLeft;
        if( nLeft < 0 )
            nLeft = 0;
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, sal_False,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            aLRItem.SetTxtLeft( (sal_uInt16)nLeft );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            aLRItem.SetTxtLeft( (sal_uInt16)nLeft );
            rItemSet.Put( aLRItem );
        }
        rPropInfo.bLeftMargin = sal_True;
    }
}


static void ParseCSS1_margin_right( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    long nRight = 0;
    sal_Bool bSet = sal_False;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            nRight = pExpr->GetSLength();
            bSet = sal_True;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            nRight = (long)pExpr->GetNumber();
            long nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nRight, nPHeight );
            bSet = sal_True;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    default:
        ;
    }

    if( bSet )
    {
        rPropInfo.nRightMargin = nRight;
        if( nRight < 0 )
            nRight = 0;
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, sal_False,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            aLRItem.SetRight( (sal_uInt16)nRight );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            aLRItem.SetRight( (sal_uInt16)nRight );
            rItemSet.Put( aLRItem );
        }
        rPropInfo.bRightMargin = sal_True;
    }
}


static void ParseCSS1_margin_top( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uInt16 nUpper = 0;
    sal_Bool bSet = sal_False;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            nUpper = (sal_uInt16)nTmp;
            bSet = sal_True;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight =  (long)pExpr->GetNumber();
            if( nPHeight < 0 )
                nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nUpper = (sal_uInt16)nPHeight;
            bSet = sal_True;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    default:
        ;
    }

    if( bSet )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nULSpace, sal_False,
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
        rPropInfo.bTopMargin = sal_True;
    }
}


static void ParseCSS1_margin_bottom( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uInt16 nLower = 0;
    sal_Bool bSet = sal_False;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            nLower = (sal_uInt16)nTmp;
            bSet = sal_True;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            long nPWidth = 0;
            long nPHeight =  (long)pExpr->GetNumber();
            if( nPHeight < 0 )
                nPHeight = 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nLower = (sal_uInt16)nPHeight;
            bSet = sal_True;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    default:
        ;
    }

    if( bSet )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nULSpace, sal_False,
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
        rPropInfo.bBottomMargin = sal_True;
    }
}


static void ParseCSS1_margin( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    long nMargins[4] = { 0, 0, 0, 0 };
    sal_Bool bSetMargins[4] = { sal_False, sal_False, sal_False, sal_False };

    for( sal_uInt16 i=0; pExpr && i<4 && !pExpr->GetOp(); i++ )
    {
        sal_Bool bSetThis = sal_False;
        long nMargin = 0;

        switch( pExpr->GetType() )
        {
        case CSS1_LENGTH:
            {
                nMargin = pExpr->GetSLength();
                bSetThis = sal_True;
            }
            break;
        case CSS1_PIXLENGTH:
            {
                long nPWidth = 0;
                nMargin =  (long)pExpr->GetNumber();
                SvxCSS1Parser::PixelToTwip( nPWidth, nMargin );
                bSetThis = sal_True;
            }
            break;
        case CSS1_PERCENTAGE:
            // koennen wir nicht
            break;
        default:
            ;
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
                bSetMargins[2] = bSetMargins[3] = sal_True;
                break;
            case 1:
                nMargins[1] = nMargins[3] = nMargin;    // right + left
                bSetMargins[1] = bSetMargins[3] = sal_True;
                break;
            case 2:
                nMargins[2] = nMargin;  // bottom
                bSetMargins[2] = sal_True;
                break;
            case 3:
                nMargins[3] = nMargin;  // left
                bSetMargins[3] = sal_True;
                break;
            }
        }
        pExpr = pExpr->GetNext();
    }

    if( bSetMargins[3] || bSetMargins[1] )
    {
        if( bSetMargins[3] )
        {
            rPropInfo.bLeftMargin = sal_True;
            rPropInfo.nLeftMargin = nMargins[3];
            if( nMargins[3] < 0 )
                nMargins[3] = 0;
        }
        if( bSetMargins[1] )
        {
            rPropInfo.bRightMargin = sal_True;
            rPropInfo.nRightMargin = nMargins[1];
            if( nMargins[1] < 0 )
                nMargins[1] = 0;
        }

        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nLRSpace, sal_False,
                                                   &pItem ) )
        {
            SvxLRSpaceItem aLRItem( *((const SvxLRSpaceItem*)pItem) );
            if( bSetMargins[3] )
                aLRItem.SetLeft( (sal_uInt16)nMargins[3] );
            if( bSetMargins[1] )
                aLRItem.SetRight( (sal_uInt16)nMargins[1] );
            rItemSet.Put( aLRItem );
        }
        else
        {
            SvxLRSpaceItem aLRItem( aItemIds.nLRSpace );
            if( bSetMargins[3] )
                aLRItem.SetLeft( (sal_uInt16)nMargins[3] );
            if( bSetMargins[1] )
                aLRItem.SetRight( (sal_uInt16)nMargins[1] );
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
        if( SFX_ITEM_SET == rItemSet.GetItemState( aItemIds.nULSpace, sal_False,
                                                   &pItem ) )
        {
            SvxULSpaceItem aULItem( *((const SvxULSpaceItem*)pItem) );
            if( bSetMargins[0] )
                aULItem.SetUpper( (sal_uInt16)nMargins[0] );
            if( bSetMargins[2] )
                aULItem.SetLower( (sal_uInt16)nMargins[2] );
            rItemSet.Put( aULItem );
        }
        else
        {
            SvxULSpaceItem aULItem( aItemIds.nULSpace );
            if( bSetMargins[0] )
                aULItem.SetUpper( (sal_uInt16)nMargins[0] );
            if( bSetMargins[2] )
                aULItem.SetLower( (sal_uInt16)nMargins[2] );
            rItemSet.Put( aULItem );
        }

        rPropInfo.bTopMargin |= bSetMargins[0];
        rPropInfo.bBottomMargin |= bSetMargins[2];
    }
}


static sal_Bool ParseCSS1_padding_xxx( const CSS1Expression *pExpr,
                                   SfxItemSet & /*rItemSet*/,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& /*rParser*/,
                                   sal_uInt16 nWhichLine )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_Bool bSet = sal_False;
    sal_uInt16 nDist = 0;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            else if( nTmp > USHRT_MAX-1 )
                nTmp = USHRT_MAX-1;
            nDist = (sal_uInt16)nTmp;
            bSet = sal_True;
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
            nDist = (sal_uInt16)nPWidth;
            bSet = sal_True;
        }
        break;
    case CSS1_PERCENTAGE:
        // koennen wir nicht
        break;
    default:
        ;
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
    sal_uInt16 n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        sal_uInt16 nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
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


static void ParseCSS1_border_xxx( const CSS1Expression *pExpr,
                                  SfxItemSet & /*rItemSet*/,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& /*rParser*/,
                                  sal_uInt16 nWhichLine, sal_Bool bAll )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uInt16 nWidth = USHRT_MAX;      // die Linien-Dicke
    sal_uInt16 nNWidth = 1;             // benannte Linien-Dicke (und default)
    CSS1BorderStyle eStyle = CSS1_BS_NONE; // Linien-Style
    Color aColor;
    sal_Bool bColor = sal_False;

    while( pExpr && !pExpr->GetOp() )
    {
        switch( pExpr->GetType() )
        {
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
            if( pExpr->GetColor( aColor ) )
                bColor = sal_True;
            break;

        case CSS1_IDENT:
            {
                const String& rValue = pExpr->GetString();
                sal_uInt16 nValue;
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
                    bColor = sal_True;
                }
            }
            break;

        case CSS1_LENGTH:
            nWidth = (sal_uInt16)pExpr->GetULength();
            break;

        case CSS1_PIXLENGTH:
            {
                sal_Bool bHori = nWhichLine == BOX_LINE_TOP ||
                             nWhichLine == BOX_LINE_BOTTOM;
                // Ein Pixel wird zur Haarlinie (ist huebscher)
                long nWidthL = (long)pExpr->GetNumber();
                if( nWidthL > 1 )
                {
                    long nPWidth = bHori ? 0 : nWidthL;
                    long nPHeight = bHori ? nWidthL : 0;
                    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                    nWidth = (sal_uInt16)(bHori ? nPHeight : nPWidth);
                }
                else
                    nWidth = 1;
            }
            break;

        default:
            ;
        }

        pExpr = pExpr->GetNext();
    }

    for( sal_uInt16 i=0; i<4; i++ )
    {
        sal_uInt16 nLine = 0;
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
                                        SfxItemSet & /*rItemSet*/,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& /*rParser*/,
                                        sal_uInt16 nWhichLine )
{
    OSL_ENSURE( pExpr, "no expression" );

    sal_uInt16 nWidth = USHRT_MAX;      // die Linien-Dicke
    sal_uInt16 nNWidth = 1;             // benannte Linien-Dicke (und default)

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            sal_uInt16 nValue;
            if( SvxCSS1Parser::GetEnum( aBorderWidthTable, pExpr->GetString(), nValue ) )
            {
                nNWidth = nValue;
            }
        }
        break;

    case CSS1_LENGTH:
        nWidth = (sal_uInt16)pExpr->GetULength();
        break;

    case CSS1_PIXLENGTH:
        {
            sal_Bool bHori = nWhichLine == BOX_LINE_TOP ||
                         nWhichLine == BOX_LINE_BOTTOM;
            long nWidthL = (long)pExpr->GetNumber();
            long nPWidth = bHori ? 0 : nWidthL;
            long nPHeight = bHori ? nWidthL : 0;
            SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
            nWidth = (sal_uInt16)(bHori ? nPHeight : nPWidth);
        }
        break;

    default:
        ;
    }

    SvxCSS1BorderInfo *pInfo = rPropInfo.GetBorderInfo( nWhichLine );
    pInfo->nAbsWidth = nWidth;
    pInfo->nNamedWidth = nNWidth;
}


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
    sal_uInt16 n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        sal_uInt16 nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, nLine );
        rPropInfo.CopyBorderInfo( n, SVX_CSS1_BORDERINFO_WIDTH );

        pExpr = pExpr->GetNext();
        n++;
    }
}

static void ParseCSS1_border_color( const CSS1Expression *pExpr,
                                    SfxItemSet & /*rItemSet*/,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    sal_uInt16 n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        sal_uInt16 nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        Color aColor;
        switch( pExpr->GetType() )
        {
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
        case CSS1_IDENT:
            if( pExpr->GetColor( aColor ) )
                rPropInfo.GetBorderInfo( nLine )->aColor = aColor;
            break;
        default:
            ;
        }
        rPropInfo.CopyBorderInfo( n, SVX_CSS1_BORDERINFO_COLOR );

        pExpr = pExpr->GetNext();
        n++;
    }
}

static void ParseCSS1_border_style( const CSS1Expression *pExpr,
                                    SfxItemSet & /*rItemSet*/,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    sal_uInt16 n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        sal_uInt16 nLine = n==0 || n==2 ? BOX_LINE_BOTTOM : BOX_LINE_LEFT;
        sal_uInt16 nValue;
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
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_TOP, sal_False );
}

static void ParseCSS1_border_right( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_RIGHT, sal_False );
}

static void ParseCSS1_border_bottom( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_BOTTOM, sal_False );
}

static void ParseCSS1_border_left( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, BOX_LINE_LEFT, sal_False );
}

static void ParseCSS1_border( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, 0, sal_True );
}


static void ParseCSS1_float( const CSS1Expression *pExpr,
                             SfxItemSet & /*rItemSet*/,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    if( CSS1_IDENT==pExpr->GetType() )
    {
        sal_uInt16 nFloat;
        if( SvxCSS1Parser::GetEnum( aFloatTable, pExpr->GetString(), nFloat ) )
            rPropInfo.eFloat = (SvxAdjust)nFloat;
    }
}



static void ParseCSS1_position( const CSS1Expression *pExpr,
                                SfxItemSet & /*rItemSet*/,
                                SvxCSS1PropertyInfo& rPropInfo,
                                const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    if( CSS1_IDENT==pExpr->GetType() )
    {
        sal_uInt16 nPos;
        if( SvxCSS1Parser::GetEnum( aPositionTable, pExpr->GetString(), nPos ) )
            rPropInfo.ePosition = (SvxCSS1Position)nPos;
    }
}


static void ParseCSS1_length( const CSS1Expression *pExpr,
                              long& rLength,
                              SvxCSS1LengthType& rLengthType,
                              sal_Bool bHori )
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

    default:
        ;
    }
}


static void ParseCSS1_width( const CSS1Expression *pExpr,
                             SfxItemSet & /*rItemSet*/,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.nWidth, rPropInfo.eWidthType, sal_True );
}

static void ParseCSS1_height( const CSS1Expression *pExpr,
                              SfxItemSet & /*rItemSet*/,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.nHeight, rPropInfo.eHeightType, sal_False );
}

static void ParseCSS1_left( const CSS1Expression *pExpr,
                             SfxItemSet & /*rItemSet*/,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.nLeft, rPropInfo.eLeftType, sal_True );
}

static void ParseCSS1_top( const CSS1Expression *pExpr,
                           SfxItemSet & /*rItemSet*/,
                           SvxCSS1PropertyInfo& rPropInfo,
                           const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.nTop, rPropInfo.eTopType, sal_False );
}


// Feature: PrintExt
static void ParseCSS1_size( const CSS1Expression *pExpr,
                            SfxItemSet & /*rItemSet*/,
                            SvxCSS1PropertyInfo& rPropInfo,
                            const SvxCSS1Parser& /*rParser*/ )
{
    sal_uInt16 n=0;
    while( n<2 && pExpr && !pExpr->GetOp() )
    {
        switch( pExpr->GetType() )
        {
        case CSS1_IDENT:
            {
                sal_uInt16 nValue;
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

        default:
            ;
        }

        pExpr = pExpr->GetNext();
        n++;
    }
}

// /Feature: PrintExt


// Feature: PrintExt

static void ParseCSS1_page_break_xxx( const CSS1Expression *pExpr,
                                      SvxCSS1PageBreak& rPBreak )
{
    if( CSS1_IDENT == pExpr->GetType() )
    {
        sal_uInt16 nValue;
        if( SvxCSS1Parser::GetEnum( aPageBreakTable, pExpr->GetString(),
                                    nValue ) )
        {
            rPBreak = (SvxCSS1PageBreak)nValue;
        }
    }
}

static void ParseCSS1_page_break_before( const CSS1Expression *pExpr,
                                         SfxItemSet & /*rItemSet*/,
                                         SvxCSS1PropertyInfo& rPropInfo,
                                         const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_page_break_xxx( pExpr, rPropInfo.ePageBreakBefore );
}

static void ParseCSS1_page_break_after( const CSS1Expression *pExpr,
                                        SfxItemSet & /*rItemSet*/,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_page_break_xxx( pExpr, rPropInfo.ePageBreakAfter );
}

static void ParseCSS1_page_break_inside( const CSS1Expression *pExpr,
                                         SfxItemSet &rItemSet,
                                         SvxCSS1PropertyInfo& /*rPropInfo*/,
                                         const SvxCSS1Parser& /*rParser*/ )
{
    SvxCSS1PageBreak eBreak(SVX_CSS1_PBREAK_NONE);
    ParseCSS1_page_break_xxx( pExpr, eBreak );

    sal_Bool bSetSplit = sal_False, bSplit = sal_True;
    switch( eBreak )
    {
    case SVX_CSS1_PBREAK_AUTO:
        bSetSplit = sal_True;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bSplit = sal_False;
        bSetSplit = sal_True;
        break;
    default:
        ;
    }

    if( bSetSplit )
        rItemSet.Put( SvxFmtSplitItem( bSplit, aItemIds.nFmtSplit ) );
}

static void ParseCSS1_widows( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& /*rPropInfo*/,
                              const SvxCSS1Parser& /*rParser*/ )
{
    if( CSS1_NUMBER == pExpr->GetType() )
    {
        sal_uInt8 nVal = pExpr->GetNumber() <= 255
                        ? (sal_uInt8)pExpr->GetNumber()
                        : 255;
        SvxWidowsItem aWidowsItem( nVal, aItemIds.nWidows );
        rItemSet.Put( aWidowsItem );
    }
}

static void ParseCSS1_orphans( const CSS1Expression *pExpr,
                               SfxItemSet &rItemSet,
                               SvxCSS1PropertyInfo& /*rPropInfo*/,
                               const SvxCSS1Parser& /*rParser*/ )
{
    if( CSS1_NUMBER == pExpr->GetType() )
    {
        sal_uInt8 nVal = pExpr->GetNumber() <= 255
                        ? (sal_uInt8)pExpr->GetNumber()
                        : 255;
        SvxOrphansItem aOrphansItem( nVal, aItemIds.nOrphans );
        rItemSet.Put( aOrphansItem );
    }
}
// /Feature: PrintExt

static void ParseCSS1_so_language( const CSS1Expression *pExpr,
                               SfxItemSet &rItemSet,
                               SvxCSS1PropertyInfo& /*rPropInfo*/,
                               const SvxCSS1Parser& rParser )
{
    if( CSS1_IDENT == pExpr->GetType() ||
        CSS1_STRING == pExpr->GetType() )
    {
        LanguageType eLang = LanguageTag( pExpr->GetString() ).getLanguageType();
        if( LANGUAGE_DONTKNOW != eLang )
        {
            SvxLanguageItem aLang( eLang, aItemIds.nLanguage );
            if( rParser.IsSetWesternProps() )
                rItemSet.Put( aLang );
            if( rParser.IsSetCJKProps() )
            {
                aLang.SetWhich( aItemIds.nLanguageCJK );
                rItemSet.Put( aLang );
            }
            if( rParser.IsSetCTLProps() )
            {
                aLang.SetWhich( aItemIds.nLanguageCTL );
                rItemSet.Put( aLang );
            }
        }
    }
}


// die Zuordung Property zu parsender Funktion
struct CSS1PropEntry
{
    union
    {
        const sal_Char  *sName;
        OUString          *pName;
    };
    FnParseCSS1Prop pFunc;
};

#define CSS1_PROP_ENTRY(p) \
    {   { sCSS1_P_##p }, ParseCSS1_##p }


// die Tabelle mit den Zuordnungen
static CSS1PropEntry aCSS1PropFnTab[] =
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
    CSS1_PROP_ENTRY(column_count),
    CSS1_PROP_ENTRY(direction),
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
    CSS1_PROP_ENTRY(text_transform),
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
    CSS1_PROP_ENTRY(orphans),
// /Feature: PrintExt
    CSS1_PROP_ENTRY(so_language)
};


static int bSortedPropFns = sal_False;

extern "C"
{
static int SAL_CALL CSS1PropEntryCompare( const void *pFirst, const void *pSecond)
{
    int nRet;
    if( ((CSS1PropEntry*)pFirst)->pFunc )
    {
        if( ((CSS1PropEntry*)pSecond)->pFunc )
            nRet = strcmp( ((CSS1PropEntry*)pFirst)->sName ,
                    ((CSS1PropEntry*)pSecond)->sName );
        else
            nRet = -1 * ((CSS1PropEntry*)pSecond)->pName->compareToAscii(
                            ((CSS1PropEntry*)pFirst)->sName );
    }
    else
    {
        if( ((CSS1PropEntry*)pSecond)->pFunc )
            nRet = ((CSS1PropEntry*)pFirst)->pName->compareToAscii(
                        ((CSS1PropEntry*)pSecond)->sName );
        else
            nRet = ((CSS1PropEntry*)pFirst)->pName->compareTo(
                        *((CSS1PropEntry*)pSecond)->pName );
    }

    return nRet;
}
}

void SvxCSS1Parser::ParseProperty( const OUString& rProperty,
                                   const CSS1Expression *pExpr )
{
    OSL_ENSURE( pItemSet, "DeclarationParsed() without ItemSet" );

    if( !bSortedPropFns )
    {
        qsort( (void*) aCSS1PropFnTab,
                sizeof( aCSS1PropFnTab ) / sizeof( CSS1PropEntry ),
                sizeof( CSS1PropEntry ),
                CSS1PropEntryCompare );
        bSortedPropFns = sal_True;
    }

    OUString aTmp( rProperty.toAsciiLowerCase() );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
