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

#include <sal/config.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
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
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/orphitem.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>

#include <hintids.hxx>

#include "css1kywd.hxx"
#include "svxcss1.hxx"
#include "htmlnum.hxx"

using namespace ::com::sun::star;

/// type of functions to parse CSS1 properties
typedef void (*FnParseCSS1Prop)( const CSS1Expression *pExpr,
                                 SfxItemSet& rItemSet,
                                 SvxCSS1PropertyInfo& rPropInfo,
                                 const SvxCSS1Parser& rParser );

CSS1PropertyEnum const aFontSizeTable[] =
{
    { "xx-small",    0                   },
    { "x-small",     1                   },
    { "small",       2                   },
    { "medium",      3                   },
    { "large",       4                   },
    { "x-large",     5                   },
    { "xx-large",    6                   },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aFontWeightTable[] =
{
    { "extra-light", WEIGHT_NORMAL       }, // WEIGHT_ULTRALIGHT (OBS)
    { "light",       WEIGHT_NORMAL       }, // WEIGHT_LIGHT (OBSOLETE)
    { "demi-light",  WEIGHT_NORMAL       }, // WEIGHT_SEMILIGHT (OBS)
    { "medium",      WEIGHT_NORMAL       }, // WEIGHT_MEDIUM (OBS)
    { "normal",      WEIGHT_NORMAL       }, // WEIGHT_MEDIUM
    { "demi-bold",   WEIGHT_NORMAL       }, // WEIGHT_SEMIBOLD (OBS)
    { "bold",        WEIGHT_BOLD         }, // WEIGHT_BOLD (OBSOLETE)
    { "extra-bold",  WEIGHT_BOLD         }, // WEIGHT_ULTRABOLD (OBS)
    { "bolder",      WEIGHT_BOLD         },
    { "lighter",     WEIGHT_NORMAL       },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aFontStyleTable[] =
{
    { "normal",      ITALIC_NONE         },
    { "italic",      ITALIC_NORMAL       },
    { "oblique",     ITALIC_NORMAL       },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aFontVariantTable[] =
{
    { "normal",      sal_uInt16(SvxCaseMap::NotMapped)      },
    { "small-caps",  sal_uInt16(SvxCaseMap::SmallCaps)    },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aTextTransformTable[] =
{
    { "uppercase",  sal_uInt16(SvxCaseMap::Uppercase) },
    { "lowercase",  sal_uInt16(SvxCaseMap::Lowercase)   },
    { "capitalize", sal_uInt16(SvxCaseMap::Capitalize)     },
    { nullptr,                   0                     }
};

CSS1PropertyEnum const aDirectionTable[] =
{
    { "ltr",         sal_uInt16(SvxFrameDirection::Horizontal_LR_TB)  },
    { "rtl",         sal_uInt16(SvxFrameDirection::Horizontal_RL_TB)  },
    { "inherit",     sal_uInt16(SvxFrameDirection::Environment)       },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aBGRepeatTable[] =
{
    { "repeat",      GPOS_TILED                  },
    { "repeat-x",    GPOS_TILED                  },
    { "repeat-y",    GPOS_TILED                  },
    { "no-repeat",   GPOS_NONE                   },
    { nullptr,                    0                           }
};

CSS1PropertyEnum const aBGHoriPosTable[] =
{
    { "left",        GPOS_LT                 },
    { "center",      GPOS_MT                 },
    { "right",       GPOS_RT                 },
    { nullptr,                    0                       }
};

CSS1PropertyEnum const aBGVertPosTable[] =
{
    { "top",         GPOS_LT                 },
    { "middle",      GPOS_LM                 },
    { "bottom",      GPOS_LB                 },
    { nullptr,                    0                       }
};

CSS1PropertyEnum const aTextAlignTable[] =
{
    { "left",        sal_uInt16(SvxAdjust::Left)     },
    { "center",      sal_uInt16(SvxAdjust::Center)   },
    { "right",       sal_uInt16(SvxAdjust::Right)    },
    { "justify",     sal_uInt16(SvxAdjust::Block)    },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aBorderWidthTable[] =
{
    { "thin",        0   },  // DEF_LINE_WIDTH_0 / DEF_DOUBLE_LINE0
    { "medium",      1   },  // DEF_LINE_WIDTH_1 / DEF_DOUBLE_LINE1
    { "thick",       2   },  // DEF_LINE_WIDTH_2 / DEF_DOUBLE_LINE2
    { nullptr,                    0   }
};

namespace {

enum CSS1BorderStyle { CSS1_BS_NONE, CSS1_BS_SINGLE, CSS1_BS_DOUBLE, CSS1_BS_DOTTED, CSS1_BS_DASHED, CSS1_BS_GROOVE, CSS1_BS_RIDGE, CSS1_BS_INSET, CSS1_BS_OUTSET };

}

CSS1PropertyEnum const aBorderStyleTable[] =
{
    { "none",        CSS1_BS_NONE        },
    { "dotted",      CSS1_BS_DOTTED      },
    { "dashed",      CSS1_BS_DASHED      },
    { "solid",       CSS1_BS_SINGLE      },
    { "double",      CSS1_BS_DOUBLE      },
    { "groove",      CSS1_BS_GROOVE      },
    { "ridge",       CSS1_BS_RIDGE       },
    { "inset",       CSS1_BS_INSET       },
    { "outset",      CSS1_BS_OUTSET      },
    { nullptr,                    0                   }
};

CSS1PropertyEnum const aFloatTable[] =
{
    { "left",    sal_uInt16(SvxAdjust::Left)         },
    { "right",   sal_uInt16(SvxAdjust::Right)        },
    { "none",    sal_uInt16(SvxAdjust::End)          },
    { nullptr,                0                       }
};

CSS1PropertyEnum const aPositionTable[] =
{
    { "absolute",    SVX_CSS1_POS_ABSOLUTE   },
    { "relative",    SVX_CSS1_POS_RELATIVE   },
    { "static",      SVX_CSS1_POS_STATIC     },
    { nullptr,                    0                       }
};

// Feature: PrintExt
CSS1PropertyEnum const aSizeTable[] =
{
    { "auto",        SVX_CSS1_STYPE_AUTO         },
    { "landscape",   SVX_CSS1_STYPE_LANDSCAPE    },
    { "portrait",    SVX_CSS1_STYPE_PORTRAIT     },
    { nullptr,                    0                           }
};

CSS1PropertyEnum const aPageBreakTable[] =
{
    { "auto",        SVX_CSS1_PBREAK_AUTO        },
    { "always",      SVX_CSS1_PBREAK_ALWAYS      },
    { "avoid",       SVX_CSS1_PBREAK_AVOID       },
    { "left",        SVX_CSS1_PBREAK_LEFT        },
    { "right",       SVX_CSS1_PBREAK_RIGHT       },
    { nullptr,                    0                           }
};

CSS1PropertyEnum const aNumberStyleTable[] =
{
    { "decimal",          SVX_NUM_ARABIC              },
    { "lower-alpha",      SVX_NUM_CHARS_LOWER_LETTER  },
    { "lower-latin",      SVX_NUM_CHARS_LOWER_LETTER  },
    { "lower-roman",      SVX_NUM_ROMAN_LOWER         },
    { "upper-alpha",      SVX_NUM_CHARS_UPPER_LETTER  },
    { "upper-latin",      SVX_NUM_CHARS_UPPER_LETTER  },
    { "upper-roman",      SVX_NUM_ROMAN_UPPER         },
    { nullptr,            0                           }
};

CSS1PropertyEnum const aBulletStyleTable[] =
{
    { "circle",      HTML_BULLETCHAR_CIRCLE   },
    { "disc",        HTML_BULLETCHAR_DISC     },
    { "square",      HTML_BULLETCHAR_SQUARE   },
    { nullptr,       0                        }
};

sal_uInt16 const aBorderWidths[] =
{
    SvxBorderLineWidth::Hairline,
    SvxBorderLineWidth::VeryThin,
    SvxBorderLineWidth::Thin
};

#undef SBORDER_ENTRY
#undef DBORDER_ENTRY

namespace {

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
    sal_uInt16 nFormatSplit;

    // this looks a bit superfluous? TypedWhichId<SvxLRSpaceItem> nLRSpace{0};
    TypedWhichId<SvxULSpaceItem> nULSpace{0};
    sal_uInt16 nBox;
    sal_uInt16 nBrush;

    sal_uInt16 nLanguage;
    sal_uInt16 nLanguageCJK;
    sal_uInt16 nLanguageCTL;
    sal_uInt16 nDirection;
};

}

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

    void SetBorderLine( SvxBoxItemLine nLine, SvxBoxItem &rBoxItem ) const;
};

void SvxCSS1BorderInfo::SetBorderLine( SvxBoxItemLine nLine, SvxBoxItem &rBoxItem ) const
{
    if( CSS1_BS_NONE==eStyle || nAbsWidth==0 ||
        (nAbsWidth==USHRT_MAX && nNamedWidth==USHRT_MAX) )
    {
        rBoxItem.SetLine( nullptr, nLine );
        return;
    }

    ::editeng::SvxBorderLine aBorderLine( &aColor );

    // Line style double or single?
    switch ( eStyle )
    {
        case CSS1_BS_SINGLE:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::SOLID);
            break;
        case CSS1_BS_DOUBLE:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::DOUBLE);
            break;
        case CSS1_BS_DOTTED:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::DOTTED);
            break;
        case CSS1_BS_DASHED:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::DASHED);
            break;
        case CSS1_BS_GROOVE:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::ENGRAVED);
            break;
        case CSS1_BS_RIDGE:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::EMBOSSED);
            break;
        case CSS1_BS_INSET:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::INSET);
            break;
        case CSS1_BS_OUTSET:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::OUTSET);
            break;
        default:
            aBorderLine.SetBorderLineStyle(SvxBorderLineStyle::NONE);
            break;
    }

    // convert named width, if no absolute is given
    if( nAbsWidth==USHRT_MAX )
        aBorderLine.SetWidth( aBorderWidths[ nNamedWidth ] );
    else
        aBorderLine.SetWidth( nAbsWidth );

    rBoxItem.SetLine( &aBorderLine, nLine );
}

SvxCSS1PropertyInfo::SvxCSS1PropertyInfo()
{
    Clear();
}

SvxCSS1PropertyInfo::SvxCSS1PropertyInfo( const SvxCSS1PropertyInfo& rProp ) :
    m_aId( rProp.m_aId ),
    m_bTopMargin( rProp.m_bTopMargin ),
    m_bBottomMargin( rProp.m_bBottomMargin ),
    m_bLeftMargin( rProp.m_bLeftMargin ),
    m_bRightMargin( rProp.m_bRightMargin ),
    m_bTextIndent( rProp.m_bTextIndent ),
    m_bNumbering ( rProp.m_bNumbering ),
    m_bBullet ( rProp.m_bBullet ),
    m_eFloat( rProp.m_eFloat ),
    m_ePosition( rProp.m_ePosition ),
    m_nTopBorderDistance( rProp.m_nTopBorderDistance ),
    m_nBottomBorderDistance( rProp.m_nBottomBorderDistance ),
    m_nLeftBorderDistance( rProp.m_nLeftBorderDistance ),
    m_nRightBorderDistance( rProp.m_nRightBorderDistance ),
    m_nNumberingType ( rProp.m_nNumberingType ),
    m_cBulletChar( rProp.m_cBulletChar ),
    m_nColumnCount( rProp.m_nColumnCount ),
    m_nLeft( rProp.m_nLeft ),
    m_nTop( rProp.m_nTop ),
    m_nWidth( rProp.m_nWidth ),
    m_nHeight( rProp.m_nHeight ),
    m_nLeftMargin( rProp.m_nLeftMargin ),
    m_nRightMargin( rProp.m_nRightMargin ),
    m_eLeftType( rProp.m_eLeftType ),
    m_eTopType( rProp.m_eTopType ),
    m_eWidthType( rProp.m_eWidthType ),
    m_eHeightType( rProp.m_eHeightType ),
    m_eLeftMarginType( rProp.m_eLeftMarginType ),
    m_eRightMarginType( rProp.m_eRightMarginType ),
    m_eSizeType( rProp.m_eSizeType ),
    m_ePageBreakBefore( rProp.m_ePageBreakBefore ),
    m_ePageBreakAfter( rProp.m_ePageBreakAfter )
{
    for( size_t i=0; i<m_aBorderInfos.size(); ++i )
        if (rProp.m_aBorderInfos[i])
            m_aBorderInfos[i].reset( new SvxCSS1BorderInfo( *rProp.m_aBorderInfos[i] ) );
}

SvxCSS1PropertyInfo::~SvxCSS1PropertyInfo()
{
}

void SvxCSS1PropertyInfo::DestroyBorderInfos()
{
    for(auto & rp : m_aBorderInfos)
        rp.reset();
}

void SvxCSS1PropertyInfo::Clear()
{
    m_aId.clear();
    m_bTopMargin = m_bBottomMargin = false;
    m_bLeftMargin = m_bRightMargin = m_bTextIndent = false;
    m_bNumbering = m_bBullet = false;
    m_nLeftMargin = m_nRightMargin = 0;
    m_eFloat = SvxAdjust::End;

    m_ePosition = SVX_CSS1_POS_NONE;
    m_nTopBorderDistance = m_nBottomBorderDistance =
    m_nLeftBorderDistance = m_nRightBorderDistance = UNSET_BORDER_DISTANCE;

    m_nNumberingType = SVX_NUM_CHARS_UPPER_LETTER;
    m_cBulletChar = ' ';

    m_nColumnCount = 0;

    m_nLeft = m_nTop = m_nWidth = m_nHeight = 0;
    m_eLeftType = m_eTopType = m_eWidthType = m_eHeightType = SVX_CSS1_LTYPE_NONE;
    m_eLeftMarginType = SVX_CSS1_LTYPE_NONE;
    m_eRightMarginType = SVX_CSS1_LTYPE_NONE;

// Feature: PrintExt
    m_eSizeType = SVX_CSS1_STYPE_NONE;
    m_ePageBreakBefore = SVX_CSS1_PBREAK_NONE;
    m_ePageBreakAfter = SVX_CSS1_PBREAK_NONE;

    DestroyBorderInfos();
}

void SvxCSS1PropertyInfo::Merge( const SvxCSS1PropertyInfo& rProp )
{
    if( rProp.m_bTopMargin )
        m_bTopMargin = true;
    if( rProp.m_bBottomMargin )
        m_bBottomMargin = true;

    if( rProp.m_bLeftMargin )
    {
        m_bLeftMargin = true;
        m_nLeftMargin = rProp.m_nLeftMargin;
    }
    if( rProp.m_bRightMargin )
    {
        m_bRightMargin = true;
        m_nRightMargin = rProp.m_nRightMargin;
    }
    if( rProp.m_bTextIndent )
        m_bTextIndent = true;

    for( size_t i=0; i<m_aBorderInfos.size(); ++i )
    {
        if( rProp.m_aBorderInfos[i] )
            m_aBorderInfos[i].reset( new SvxCSS1BorderInfo( *rProp.m_aBorderInfos[i] ) );
    }

    if( UNSET_BORDER_DISTANCE != rProp.m_nTopBorderDistance )
        m_nTopBorderDistance = rProp.m_nTopBorderDistance;
    if( UNSET_BORDER_DISTANCE != rProp.m_nBottomBorderDistance )
        m_nBottomBorderDistance = rProp.m_nBottomBorderDistance;
    if( UNSET_BORDER_DISTANCE != rProp.m_nLeftBorderDistance )
        m_nLeftBorderDistance = rProp.m_nLeftBorderDistance;
    if( UNSET_BORDER_DISTANCE != rProp.m_nRightBorderDistance )
        m_nRightBorderDistance = rProp.m_nRightBorderDistance;

    m_nColumnCount = rProp.m_nColumnCount;

    if( rProp.m_eFloat != SvxAdjust::End )
        m_eFloat = rProp.m_eFloat;

    if( rProp.m_ePosition != SVX_CSS1_POS_NONE )
        m_ePosition = rProp.m_ePosition;

// Feature: PrintExt
    if( rProp.m_eSizeType != SVX_CSS1_STYPE_NONE )
    {
        m_eSizeType = rProp.m_eSizeType;
        m_nWidth = rProp.m_nWidth;
        m_nHeight = rProp.m_nHeight;
    }

    if( rProp.m_ePageBreakBefore != SVX_CSS1_PBREAK_NONE )
        m_ePageBreakBefore = rProp.m_ePageBreakBefore;

    if( rProp.m_ePageBreakAfter != SVX_CSS1_PBREAK_NONE )
        m_ePageBreakAfter = rProp.m_ePageBreakAfter;

    if( rProp.m_eLeftType != SVX_CSS1_LTYPE_NONE )
    {
        m_eLeftType = rProp.m_eLeftType;
        m_nLeft = rProp.m_nLeft;
    }

    if( rProp.m_eTopType != SVX_CSS1_LTYPE_NONE )
    {
        m_eTopType = rProp.m_eTopType;
        m_nTop = rProp.m_nTop;
    }

    if( rProp.m_eWidthType != SVX_CSS1_LTYPE_NONE )
    {
        m_eWidthType = rProp.m_eWidthType;
        m_nWidth = rProp.m_nWidth;
    }

    if( rProp.m_eHeightType != SVX_CSS1_LTYPE_NONE )
    {
        m_eHeightType = rProp.m_eHeightType;
        m_nHeight = rProp.m_nHeight;
    }
}

SvxCSS1BorderInfo *SvxCSS1PropertyInfo::GetBorderInfo( SvxBoxItemLine nLine, bool bCreate )
{
    sal_uInt16 nPos = 0;
    switch( nLine )
    {
    case SvxBoxItemLine::TOP:      nPos = 0;   break;
    case SvxBoxItemLine::BOTTOM:   nPos = 1;   break;
    case SvxBoxItemLine::LEFT:     nPos = 2;   break;
    case SvxBoxItemLine::RIGHT:    nPos = 3;   break;
    }

    if( !m_aBorderInfos[nPos] && bCreate )
        m_aBorderInfos[nPos].reset( new SvxCSS1BorderInfo );

    return m_aBorderInfos[nPos].get();
}

void SvxCSS1PropertyInfo::CopyBorderInfo( SvxBoxItemLine nSrcLine, SvxBoxItemLine nDstLine,
                                          sal_uInt16 nWhat )
{
    SvxCSS1BorderInfo *pSrcInfo = GetBorderInfo( nSrcLine, false );
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
        CopyBorderInfo( SvxBoxItemLine::BOTTOM, SvxBoxItemLine::TOP, nWhat );
        CopyBorderInfo( SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT, nWhat );
    }
    if( nCount<=1 )
    {
        CopyBorderInfo( SvxBoxItemLine::LEFT, SvxBoxItemLine::RIGHT, nWhat );
    }
}

void SvxCSS1PropertyInfo::SetBoxItem( SfxItemSet& rItemSet,
                                      sal_uInt16 nMinBorderDist,
                                      const SvxBoxItem *pDfltItem )
{
    bool bChg = m_nTopBorderDistance != UNSET_BORDER_DISTANCE ||
                m_nBottomBorderDistance != UNSET_BORDER_DISTANCE ||
                m_nLeftBorderDistance != UNSET_BORDER_DISTANCE ||
                m_nRightBorderDistance != UNSET_BORDER_DISTANCE;

    for( size_t i=0; !bChg && i<m_aBorderInfos.size(); ++i )
        bChg = m_aBorderInfos[i]!=nullptr;

    if( !bChg )
        return;

    std::shared_ptr<SvxBoxItem> aBoxItem(std::make_shared<SvxBoxItem>(aItemIds.nBox));
    if( pDfltItem )
        aBoxItem.reset(pDfltItem->Clone());

    SvxCSS1BorderInfo *pInfo = GetBorderInfo( SvxBoxItemLine::TOP, false );
    if( pInfo )
        pInfo->SetBorderLine( SvxBoxItemLine::TOP, *aBoxItem );

    pInfo = GetBorderInfo( SvxBoxItemLine::BOTTOM, false );
    if( pInfo )
        pInfo->SetBorderLine( SvxBoxItemLine::BOTTOM, *aBoxItem );

    pInfo = GetBorderInfo( SvxBoxItemLine::LEFT, false );
    if( pInfo )
        pInfo->SetBorderLine( SvxBoxItemLine::LEFT, *aBoxItem );

    pInfo = GetBorderInfo( SvxBoxItemLine::RIGHT, false );
    if( pInfo )
        pInfo->SetBorderLine( SvxBoxItemLine::RIGHT, *aBoxItem );

    for( size_t i=0; i<m_aBorderInfos.size(); ++i )
    {
        SvxBoxItemLine nLine = SvxBoxItemLine::TOP;
        sal_uInt16 nDist = 0;
        switch( i )
        {
        case 0: nLine = SvxBoxItemLine::TOP;
                nDist = m_nTopBorderDistance;
                m_nTopBorderDistance = UNSET_BORDER_DISTANCE;
                break;
        case 1: nLine = SvxBoxItemLine::BOTTOM;
                nDist = m_nBottomBorderDistance;
                m_nBottomBorderDistance = UNSET_BORDER_DISTANCE;
                break;
        case 2: nLine = SvxBoxItemLine::LEFT;
                nDist = m_nLeftBorderDistance;
                m_nLeftBorderDistance = UNSET_BORDER_DISTANCE;
                break;
        case 3: nLine = SvxBoxItemLine::RIGHT;
                nDist = m_nRightBorderDistance;
                m_nRightBorderDistance = UNSET_BORDER_DISTANCE;
                break;
        }

        if( aBoxItem->GetLine( nLine ) )
        {
            if( UNSET_BORDER_DISTANCE == nDist )
                nDist = aBoxItem->GetDistance( nLine );

            if( nDist < nMinBorderDist )
                nDist = nMinBorderDist;
        }
        else
        {
            nDist = 0U;
        }

        aBoxItem->SetDistance( nDist, nLine );
    }

    rItemSet.Put( *aBoxItem );

    DestroyBorderInfos();
}

SvxCSS1MapEntry::SvxCSS1MapEntry( SfxItemSet aItemSet,
                                  const SvxCSS1PropertyInfo& rProp ) :
    m_aItemSet(std::move( aItemSet )),
    m_aPropInfo( rProp )
{}

void SvxCSS1Parser::StyleParsed( const CSS1Selector * /*pSelector*/,
                                 SfxItemSet& /*rItemSet*/,
                                 SvxCSS1PropertyInfo& /*rPropInfo*/ )
{
    // you see nothing is happening here
}

void SvxCSS1Parser::SelectorParsed( std::unique_ptr<CSS1Selector> pSelector, bool bFirst )
{
    if( bFirst )
    {
        OSL_ENSURE( m_pSheetItemSet, "Where is the Item-Set for Style-Sheets?" );

        for (const std::unique_ptr<CSS1Selector> & rpSelection : m_Selectors)
        {
            StyleParsed(rpSelection.get(), *m_pSheetItemSet, *m_pSheetPropInfo);
        }
        m_pSheetItemSet->ClearItem();
        m_pSheetPropInfo->Clear();

        // prepare the next rule
        m_Selectors.clear();
    }

    m_Selectors.push_back(std::move(pSelector));
}

SvxCSS1Parser::SvxCSS1Parser( SfxItemPool& rPool, OUString aBaseURL,
                              sal_uInt16 const *pWhichIds, sal_uInt16 nWhichIds ) :
    m_sBaseURL(std::move( aBaseURL )),
    m_pItemSet(nullptr),
    m_pPropInfo( nullptr ),
    m_eDefaultEnc( RTL_TEXTENCODING_DONTKNOW ),
    m_bIgnoreFontFamily( false )
{
    // also initialize item IDs
    auto initTrueWhich = [&rPool, this](sal_uInt16 rWid)
    {
        rWid = rPool.GetTrueWhichIDFromSlotID(rWid, false);
        m_aWhichMap = m_aWhichMap.MergeRange(rWid, rWid);
        return rWid;
    };

    aItemIds.nFont = initTrueWhich( SID_ATTR_CHAR_FONT );
    aItemIds.nFontCJK = initTrueWhich( SID_ATTR_CHAR_CJK_FONT );
    aItemIds.nFontCTL = initTrueWhich( SID_ATTR_CHAR_CTL_FONT );
    aItemIds.nPosture = initTrueWhich( SID_ATTR_CHAR_POSTURE );
    aItemIds.nPostureCJK = initTrueWhich( SID_ATTR_CHAR_CJK_POSTURE );
    aItemIds.nPostureCTL = initTrueWhich( SID_ATTR_CHAR_CTL_POSTURE );
    aItemIds.nWeight = initTrueWhich( SID_ATTR_CHAR_WEIGHT );
    aItemIds.nWeightCJK = initTrueWhich( SID_ATTR_CHAR_CJK_WEIGHT );
    aItemIds.nWeightCTL = initTrueWhich( SID_ATTR_CHAR_CTL_WEIGHT );
    aItemIds.nFontHeight = initTrueWhich( SID_ATTR_CHAR_FONTHEIGHT );
    aItemIds.nFontHeightCJK = initTrueWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT );
    aItemIds.nFontHeightCTL = initTrueWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT );
    aItemIds.nUnderline = initTrueWhich( SID_ATTR_CHAR_UNDERLINE );
    aItemIds.nOverline = initTrueWhich( SID_ATTR_CHAR_OVERLINE );
    aItemIds.nCrossedOut = initTrueWhich( SID_ATTR_CHAR_STRIKEOUT );
    aItemIds.nColor = initTrueWhich( SID_ATTR_CHAR_COLOR );
    aItemIds.nKerning = initTrueWhich( SID_ATTR_CHAR_KERNING );
    aItemIds.nCaseMap = initTrueWhich( SID_ATTR_CHAR_CASEMAP );
    aItemIds.nBlink = initTrueWhich( SID_ATTR_FLASH );

    aItemIds.nLineSpacing = initTrueWhich( SID_ATTR_PARA_LINESPACE );
    aItemIds.nAdjust = initTrueWhich( SID_ATTR_PARA_ADJUST );
    aItemIds.nWidows = initTrueWhich( SID_ATTR_PARA_WIDOWS );
    aItemIds.nOrphans = initTrueWhich( SID_ATTR_PARA_ORPHANS );
    aItemIds.nFormatSplit = initTrueWhich( SID_ATTR_PARA_SPLIT );

    // every id that is used must be added
    m_aWhichMap = m_aWhichMap.MergeRange(RES_MARGIN_FIRSTLINE, RES_MARGIN_FIRSTLINE);
    m_aWhichMap = m_aWhichMap.MergeRange(RES_MARGIN_TEXTLEFT, RES_MARGIN_TEXTLEFT);
    m_aWhichMap = m_aWhichMap.MergeRange(RES_MARGIN_RIGHT, RES_MARGIN_RIGHT);
    aItemIds.nULSpace = TypedWhichId<SvxULSpaceItem>(initTrueWhich( SID_ATTR_ULSPACE ));
    aItemIds.nBox = initTrueWhich( SID_ATTR_BORDER_OUTER );
    aItemIds.nBrush = initTrueWhich( SID_ATTR_BRUSH );

    aItemIds.nLanguage = initTrueWhich( SID_ATTR_CHAR_LANGUAGE );
    aItemIds.nLanguageCJK = initTrueWhich( SID_ATTR_CHAR_CJK_LANGUAGE );
    aItemIds.nLanguageCTL = initTrueWhich( SID_ATTR_CHAR_CTL_LANGUAGE );
    aItemIds.nDirection = initTrueWhich( SID_ATTR_FRAMEDIRECTION );

    if( pWhichIds && nWhichIds )
        for (sal_uInt16 i = 0; i < nWhichIds; ++i)
            m_aWhichMap = m_aWhichMap.MergeRange(pWhichIds[i], pWhichIds[i]);

    m_pSheetItemSet.reset( new SfxItemSet( rPool, m_aWhichMap ) );
    m_pSheetPropInfo.reset( new SvxCSS1PropertyInfo );
}

SvxCSS1Parser::~SvxCSS1Parser()
{
    m_pSheetItemSet.reset();
    m_pSheetPropInfo.reset();
}

void SvxCSS1Parser::InsertId( const OUString& rId,
                              const SfxItemSet& rItemSet,
                              const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rId, rItemSet, rProp, m_Ids );
}

const SvxCSS1MapEntry* SvxCSS1Parser::GetId( const OUString& rId ) const
{
    CSS1Map::const_iterator itr = m_Ids.find(rId);
    return itr == m_Ids.end() ? nullptr : itr->second.get();
}

void SvxCSS1Parser::InsertClass( const OUString& rClass,
                                 const SfxItemSet& rItemSet,
                                 const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rClass, rItemSet, rProp, m_Classes );
}

const SvxCSS1MapEntry* SvxCSS1Parser::GetClass( const OUString& rClass ) const
{
    CSS1Map::const_iterator itr = m_Classes.find(rClass);
    return itr == m_Classes.end() ? nullptr : itr->second.get();
}

void SvxCSS1Parser::InsertPage( const OUString& rPage,
                                bool bPseudo,
                                const SfxItemSet& rItemSet,
                                const SvxCSS1PropertyInfo& rProp )
{
    OUString aKey( rPage );
    if( bPseudo )
        aKey = ":" + aKey;
    InsertMapEntry( aKey, rItemSet, rProp, m_Pages );
}

SvxCSS1MapEntry* SvxCSS1Parser::GetPage( const OUString& rPage, bool bPseudo )
{
    OUString aKey( rPage );
    if( bPseudo )
        aKey = ":" + aKey;

    CSS1Map::iterator itr = m_Pages.find(aKey);
    return itr == m_Pages.end() ? nullptr : itr->second.get();
}

void SvxCSS1Parser::InsertTag( const OUString& rTag,
                               const SfxItemSet& rItemSet,
                               const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rTag, rItemSet, rProp, m_Tags );
}

SvxCSS1MapEntry* SvxCSS1Parser::GetTag( const OUString& rTag )
{
    CSS1Map::iterator itr = m_Tags.find(rTag);
    return itr == m_Tags.end() ? nullptr : itr->second.get();
}

bool SvxCSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    m_pItemSet = m_pSheetItemSet.get();
    m_pPropInfo = m_pSheetPropInfo.get();

    CSS1Parser::ParseStyleSheet( rIn );

    for (const std::unique_ptr<CSS1Selector> & rpSelector : m_Selectors)
    {
        StyleParsed(rpSelector.get(), *m_pSheetItemSet, *m_pSheetPropInfo);
    }

    // and clean up a little bit
    m_Selectors.clear();
    m_pSheetItemSet->ClearItem();
    m_pSheetPropInfo->Clear();

    m_pItemSet = nullptr;
    m_pPropInfo = nullptr;

    return true;
}

void SvxCSS1Parser::ParseStyleOption( const OUString& rIn,
                                      SfxItemSet& rItemSet,
                                      SvxCSS1PropertyInfo& rPropInfo )
{
    m_pItemSet = &rItemSet;
    m_pPropInfo = &rPropInfo;

    CSS1Parser::ParseStyleOption( rIn );
    rItemSet.ClearItem( aItemIds.nDirection );

    m_pItemSet = nullptr;
    m_pPropInfo = nullptr;
}

bool SvxCSS1Parser::GetEnum( const CSS1PropertyEnum *pPropTable,
                          std::u16string_view rValue, sal_uInt16& rEnum )
{
    while( pPropTable->pName )
    {
        if( !o3tl::equalsIgnoreAsciiCase( rValue, pPropTable->pName ) )
            pPropTable++;
        else
            break;
    }

    if( pPropTable->pName )
        rEnum = pPropTable->nEnum;

    return (pPropTable->pName != nullptr);
}

void SvxCSS1Parser::PixelToTwip( tools::Long &rWidth, tools::Long &rHeight )
{
    rWidth = o3tl::convert(rWidth, o3tl::Length::px, o3tl::Length::twip);
    rHeight = o3tl::convert(rHeight, o3tl::Length::px, o3tl::Length::twip);
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
        return nullptr;
}

void SvxCSS1Parser::InsertMapEntry( const OUString& rKey,
                                    const SfxItemSet& rItemSet,
                                    const SvxCSS1PropertyInfo& rProp,
                                    CSS1Map& rMap )
{
    auto [itr,inserted] = rMap.insert(std::make_pair(rKey, nullptr));
    if (inserted)
        itr->second = std::make_unique<SvxCSS1MapEntry>(rItemSet, rProp);
    else
    {
        SvxCSS1MapEntry *const p = itr->second.get();
        MergeStyles( rItemSet, rProp,
                     p->GetItemSet(), p->GetPropertyInfo(), true );
    }
}

void SvxCSS1Parser::MergeStyles( const SfxItemSet& rSrcSet,
                                 const SvxCSS1PropertyInfo& rSrcInfo,
                                 SfxItemSet& rTargetSet,
                                 SvxCSS1PropertyInfo& rTargetInfo,
                                 bool bSmart )
{
    if( !bSmart )
    {
        rTargetSet.Put( rSrcSet );
    }
    else
    {
        // not sure if this is really necessary?
        SfxItemSet copy(rSrcSet);
        if (!rSrcInfo.m_bTextIndent)
        {
            copy.ClearItem(RES_MARGIN_FIRSTLINE);
        }
        if (!rSrcInfo.m_bLeftMargin)
        {
            copy.ClearItem(RES_MARGIN_TEXTLEFT);
        }
        if (!rSrcInfo.m_bRightMargin)
        {
            copy.ClearItem(RES_MARGIN_RIGHT);
        }

        SvxULSpaceItem aULSpace( rTargetSet.Get(aItemIds.nULSpace) );

        rTargetSet.Put(copy);

        if( rSrcInfo.m_bTopMargin || rSrcInfo.m_bBottomMargin )
        {
            const SvxULSpaceItem& rNewULSpace = rSrcSet.Get( aItemIds.nULSpace );

            if( rSrcInfo.m_bTopMargin )
                aULSpace.SetUpper( rNewULSpace.GetUpper() );
            if( rSrcInfo.m_bBottomMargin )
                aULSpace.SetLower( rNewULSpace.GetLower() );

            rTargetSet.Put( aULSpace );
        }
    }

    rTargetInfo.Merge( rSrcInfo );
}

void SvxCSS1Parser::SetDfltEncoding( rtl_TextEncoding eEnc )
{
    m_eDefaultEnc = eEnc;
}

static void ParseCSS1_font_size( const CSS1Expression *pExpr,
                                 SfxItemSet &rItemSet,
                                 SvxCSS1PropertyInfo& /*rPropInfo*/,
                                 const SvxCSS1Parser& rParser )
{
    assert(pExpr && "no expression");

    sal_uLong nHeight = 0;
    sal_uInt16 nPropHeight = 100;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nHeight = pExpr->GetULength();
        break;
    case CSS1_PIXLENGTH:
        {
            double fHeight = pExpr->GetNumber();
            if (fHeight < SAL_MAX_INT32/2.0 && fHeight > SAL_MIN_INT32/2.0)
            {
                tools::Long nPHeight = static_cast<tools::Long>(fHeight);
                tools::Long nPWidth = 0;
                SvxCSS1Parser::PixelToTwip(nPWidth, nPHeight);
                nHeight = static_cast<sal_uLong>(nPHeight);
            }
            else
            {
                SAL_WARN("sw.html", "out-of-size pxlength: " << fHeight);
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // only for drop caps!
        nPropHeight = o3tl::narrowing<sal_uInt16>(pExpr->GetNumber());
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
        rItemSet.Put( aFontHeight );
        aFontHeight.SetWhich( aItemIds.nFontHeightCJK );
        rItemSet.Put( aFontHeight );
        aFontHeight.SetWhich( aItemIds.nFontHeightCTL );
        rItemSet.Put( aFontHeight );
    }
}

static void ParseCSS1_font_family( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& /*rPropInfo*/,
                                   const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    OUStringBuffer aName;
    rtl_TextEncoding eEnc = rParser.GetDfltEncoding();
    const FontList *pFList = rParser.GetFontList();
    bool bFirst = true;
    bool bFound = false;
    while( pExpr && (bFirst || ','==pExpr->GetOp() || !pExpr->GetOp()) )
    {
        CSS1Token eType = pExpr->GetType();
        if( CSS1_IDENT==eType || CSS1_STRING==eType )
        {
            OUString aIdent( pExpr->GetString() );

            if( CSS1_IDENT==eType )
            {
                // Collect all following IDs and append them with a space
                const CSS1Expression *pNext = pExpr->GetNext();
                while( pNext && !pNext->GetOp() &&
                       CSS1_IDENT==pNext->GetType() )
                {
                    aIdent += " " + pNext->GetString();
                    pExpr = pNext;
                    pNext = pExpr->GetNext();
                }
            }
            if( !aIdent.isEmpty() )
            {
                if( !bFound && pFList )
                {
                    sal_Handle hFont = pFList->GetFirstFontMetric( aIdent );
                    if( nullptr != hFont )
                    {
                        const FontMetric& rFMetric = FontList::GetFontMetric( hFont );
                        if( RTL_TEXTENCODING_DONTKNOW != rFMetric.GetCharSet() )
                        {
                            bFound = true;
                            if( RTL_TEXTENCODING_SYMBOL == rFMetric.GetCharSet() )
                                eEnc = RTL_TEXTENCODING_SYMBOL;
                        }
                    }
                }
                if( !bFirst )
                    aName.append(";");
                aName.append(aIdent);
            }
        }

        pExpr = pExpr->GetNext();
        bFirst = false;
    }

    if( !aName.isEmpty() && !rParser.IsIgnoreFontFamily() )
    {
        SvxFontItem aFont( FAMILY_DONTKNOW, aName.makeStringAndClear(), OUString(), PITCH_DONTKNOW,
                            eEnc, aItemIds.nFont );
        rItemSet.Put( aFont );
        aFont.SetWhich( aItemIds.nFontCJK );
        rItemSet.Put( aFont );
        aFont.SetWhich( aItemIds.nFontCTL );
        rItemSet.Put( aFont );
    }
}

static void ParseCSS1_font_weight( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& /*rPropInfo*/,
                                   const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
    case CSS1_STRING:   // MS-IE, what else
        {
            sal_uInt16 nWeight;
            if( SvxCSS1Parser::GetEnum( aFontWeightTable, pExpr->GetString(),
                                        nWeight ) )
            {
                SvxWeightItem aWeight( static_cast<FontWeight>(nWeight), aItemIds.nWeight );
                rItemSet.Put( aWeight );
                aWeight.SetWhich( aItemIds.nWeightCJK );
                rItemSet.Put( aWeight );
                aWeight.SetWhich( aItemIds.nWeightCTL );
                rItemSet.Put( aWeight );
            }
        }
        break;
    case CSS1_NUMBER:
        {
            sal_uInt16 nWeight = o3tl::narrowing<sal_uInt16>(pExpr->GetNumber());
            SvxWeightItem aWeight( nWeight>400 ? WEIGHT_BOLD : WEIGHT_NORMAL,
                                   aItemIds.nWeight );
            rItemSet.Put( aWeight );
            aWeight.SetWhich( aItemIds.nWeightCJK );
            rItemSet.Put( aWeight );
            aWeight.SetWhich( aItemIds.nWeightCTL );
            rItemSet.Put( aWeight );
        }
        break;

    default:
        ;
    }
}

static void ParseCSS1_font_style( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& /*rPropInfo*/,
                                  const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    bool bPosture = false;
    bool bCaseMap = false;
    FontItalic eItalic = ITALIC_NONE;
    SvxCaseMap eCaseMap = SvxCaseMap::NotMapped;

    // normal | italic || small-caps | oblique || small-caps | small-caps
    // (only normal, italic and oblique are valid)

    // the value can have two values!
    for( int i=0; pExpr && i<2; ++i )
    {
        // also here MS-IE parser leaves traces
        if( (CSS1_IDENT==pExpr->GetType() || CSS1_STRING==pExpr->GetType()) &&
            !pExpr->GetOp() )
        {
            const OUString& rValue = pExpr->GetString();
            // first check if the value is italic or 'normal'
            sal_uInt16 nItalic;
            if( SvxCSS1Parser::GetEnum( aFontStyleTable, rValue, nItalic ) )
            {
                eItalic = static_cast<FontItalic>(nItalic);
                if( !bCaseMap && ITALIC_NONE==eItalic )
                {
                    // for 'normal' we must also exclude case-map
                    eCaseMap = SvxCaseMap::NotMapped;
                    bCaseMap = true;
                }
                bPosture = true;
            }
            else if( !bCaseMap &&
                     rValue.equalsIgnoreAsciiCase( "small-caps" ) )
            {
                eCaseMap = SvxCaseMap::SmallCaps;
                bCaseMap = true;
            }
        }

        // fetch next expression
        pExpr = pExpr->GetNext();
    }

    if( bPosture )
    {
        SvxPostureItem aPosture( eItalic, aItemIds.nPosture );
        rItemSet.Put( aPosture );
        aPosture.SetWhich( aItemIds.nPostureCJK );
        rItemSet.Put( aPosture );
        aPosture.SetWhich( aItemIds.nPostureCTL );
        rItemSet.Put( aPosture );
    }

    if( bCaseMap )
        rItemSet.Put( SvxCaseMapItem( eCaseMap, aItemIds.nCaseMap ) );
}

static void ParseCSS1_font_variant( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& /*rPropInfo*/,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    // normal | small-caps
    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            sal_uInt16 nCaseMap;
            if( SvxCSS1Parser::GetEnum( aFontVariantTable, pExpr->GetString(),
                                        nCaseMap ) )
            {
                rItemSet.Put( SvxCaseMapItem( static_cast<SvxCaseMap>(nCaseMap),
                                                aItemIds.nCaseMap ) );
            }
            break;
        }
    default:
        break;
    }
}

static void ParseCSS1_text_transform( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& /*rPropInfo*/,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    // none | capitalize | uppercase | lowercase

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        {
            sal_uInt16 nCaseMap;
            if( SvxCSS1Parser::GetEnum( aTextTransformTable, pExpr->GetString(),
                                        nCaseMap ) )
            {
                rItemSet.Put( SvxCaseMapItem( static_cast<SvxCaseMap>(nCaseMap),
                                                aItemIds.nCaseMap ) );
            }
            break;
        }
    default:
        break;
    }
}

static void ParseCSS1_color( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& /*rPropInfo*/,
                             const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
    case CSS1_RGB:
    case CSS1_HEXCOLOR:
    case CSS1_STRING:       // because MS-IE
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
    assert(pExpr && "no expression");

    if ( pExpr->GetType() == CSS1_NUMBER )
    {
        double columnCount = pExpr->GetNumber();
        if ( columnCount >= 2 )
        {
            rPropInfo.m_nColumnCount = columnCount;
        }
    }
}

static void ParseCSS1_direction( const CSS1Expression *pExpr,
                             SfxItemSet &rItemSet,
                             SvxCSS1PropertyInfo& /*rPropInfo*/,
                             const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

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
    OUString aURL;

    bool bColor = false, bTransparent = false;
    SvxGraphicPosition eRepeat = GPOS_TILED;
    SvxGraphicPosition ePos = GPOS_LT;
    bool bHori = false, bVert = false;

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
                // since we don't know any absolute position, we
                // only distinguish between 0 and !0. Therefore pixel
                // can be handled like all other units.

                bool nonZero = std::trunc(pExpr->GetNumber()) != 0.0;
                if( !bHori )
                {
                    ePos = nonZero ? GPOS_MM : GPOS_LT;
                    bHori = true;
                }
                else if( !bVert )
                {
                    MergeVert( ePos, (nonZero ? GPOS_LM : GPOS_LT) );
                    bVert = true;
                }
            }
            break;

        case CSS1_PERCENTAGE:
            {
                // the percentage is converted to an enum

                sal_uInt16 nPerc = o3tl::narrowing<sal_uInt16>(pExpr->GetNumber());
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
        case CSS1_STRING:       // because of MS-IE
            {
                sal_uInt16 nEnum;
                const OUString &rValue = pExpr->GetString();
                if( rValue.equalsIgnoreAsciiCase( "transparent" ) )
                {
                    bTransparent = true;
                }
                if( SvxCSS1Parser::GetEnum( aBGRepeatTable, rValue, nEnum ) )
                {
                    eRepeat = static_cast<SvxGraphicPosition>(nEnum);
                }
                else if( SvxCSS1Parser::GetEnum( aBGHoriPosTable, rValue, nEnum ) )
                {
                    // <position>, horizontal
                    MergeHori( ePos, static_cast<SvxGraphicPosition>(nEnum) );
                }
                else if( SvxCSS1Parser::GetEnum( aBGVertPosTable, rValue, nEnum ) )
                {
                    // <position>, vertical
                    MergeVert( ePos, static_cast<SvxGraphicPosition>(nEnum) );
                }
                else if( !bColor )
                {
                    // <color>
                    bColor = pExpr->GetColor( aColor );
                }
                // <scroll> we don't know
            }
            break;

        default:
            ;
        }

        pExpr = pExpr->GetNext();
    }

    // transparent beats everything
    if( bTransparent )
    {
        bColor = false;
        aURL.clear();
    }

    // repeat has priority over a position
    if( GPOS_NONE == eRepeat )
        eRepeat = ePos;

    if( !bTransparent && !bColor && aURL.isEmpty() )
        return;

    SvxBrushItem aBrushItem( aItemIds.nBrush );

    if( bTransparent )
        aBrushItem.SetColor( COL_TRANSPARENT);
    else if( bColor )
        aBrushItem.SetColor( aColor );

    if( !aURL.isEmpty() )
    {
        aBrushItem.SetGraphicLink( URIHelper::SmartRel2Abs( INetURLObject( rParser.GetBaseURL()), aURL, Link<OUString *, bool>(), false ) );
        aBrushItem.SetGraphicPos( eRepeat );
    }

    rItemSet.Put( aBrushItem );
}

static void ParseCSS1_background_color( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& /*rPropInfo*/,
                                  const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    Color aColor;

    bool bColor = false, bTransparent = false;

    switch( pExpr->GetType() )
    {
    case CSS1_RGB:
        bColor = pExpr->GetColor( aColor );
        break;
    case CSS1_IDENT:
    case CSS1_HEXCOLOR:
    case CSS1_STRING:       // because of MS-IE
        if( pExpr->GetString().equalsIgnoreAsciiCase( "transparent" ) )
        {
            bTransparent = true;
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
            aBrushItem.SetColor( COL_TRANSPARENT );
        else if( bColor )
            aBrushItem.SetColor( aColor);

        rItemSet.Put( aBrushItem );
    }
}

static void ParseCSS1_line_height( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& /*rPropInfo*/,
                                   const SvxCSS1Parser& )
{
    assert(pExpr && "no expression");

    sal_uInt16 nHeight = 0;
    sal_uInt16 nPropHeight = 0;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        nHeight = o3tl::narrowing<sal_uInt16>(pExpr->GetULength());
        break;
    case CSS1_PIXLENGTH:
        {
            double fHeight = pExpr->GetNumber();
            if (fHeight < SAL_MAX_INT32/2.0 && fHeight > SAL_MIN_INT32/2.0)
            {
                tools::Long nPHeight = static_cast<tools::Long>(fHeight);
                tools::Long nPWidth = 0;
                SvxCSS1Parser::PixelToTwip(nPWidth, nPHeight);
                nHeight = o3tl::narrowing<sal_uInt16>(nPHeight);
            }
        }
        break;
    case CSS1_PERCENTAGE:
        {
            nPropHeight = o3tl::narrowing<sal_uInt16>(pExpr->GetNumber());
        }
        break;
    case CSS1_NUMBER:
        {
            nPropHeight = o3tl::narrowing<sal_uInt16>(pExpr->GetNumber() * 100);
        }
        break;
    default:
        ;
    }

    if( nHeight )
    {
        if( nHeight < SvxCSS1Parser::GetMinFixLineSpace() )
            nHeight = SvxCSS1Parser::GetMinFixLineSpace();
        SvxLineSpacingItem aLSItem( nHeight, aItemIds.nLineSpacing );
        aLSItem.SetLineHeight( nHeight );
        // interpret <line-height> attribute as minimum line height
        aLSItem.SetLineSpaceRule( SvxLineSpaceRule::Min );
        aLSItem.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
        rItemSet.Put( aLSItem );
    }
    else if( nPropHeight )
    {
        SvxLineSpacingItem aLSItem( nPropHeight, aItemIds.nLineSpacing );
        aLSItem.SetLineSpaceRule( SvxLineSpaceRule::Auto );
        if( 100 == nPropHeight )
            aLSItem.SetInterLineSpaceRule( SvxInterLineSpaceRule::Off );
        else
            aLSItem.SetPropLineSpace( nPropHeight );
        rItemSet.Put( aLSItem );
    }

}

static void ParseCSS1_list_style_type( const CSS1Expression *pExpr,
                                       SfxItemSet & /*rItemSet*/,
                                       SvxCSS1PropertyInfo& rPropInfo,
                                       const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    if( pExpr->GetType() != CSS1_IDENT )
        return;

    const OUString& rValue = pExpr->GetString();

    // values are context-dependent, so fill both
    sal_uInt16 nEnum;
    if( SvxCSS1Parser::GetEnum( aNumberStyleTable, rValue, nEnum ) )
    {
        rPropInfo.m_bNumbering = true;
        rPropInfo.m_nNumberingType = static_cast<SvxNumType>(nEnum);
    }
    if( SvxCSS1Parser::GetEnum( aBulletStyleTable, rValue, nEnum ) )
    {
        rPropInfo.m_bBullet = true;
        rPropInfo.m_cBulletChar = nEnum;
    }
}

static void ParseCSS1_font( const CSS1Expression *pExpr,
                            SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo& rPropInfo,
                            const SvxCSS1Parser& rParser )
{
    OSL_ENSURE( pExpr, "no expression" );

    FontItalic eItalic = ITALIC_NONE;
    SvxCaseMap eCaseMap = SvxCaseMap::NotMapped;
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
            const OUString& rValue = pExpr->GetString();

            sal_uInt16 nEnum;

            if( SvxCSS1Parser::GetEnum( aFontStyleTable, rValue, nEnum ) )
            {
                eItalic = static_cast<FontItalic>(nEnum);
            }
            else if( SvxCSS1Parser::GetEnum( aFontVariantTable, rValue, nEnum ) )
            {
                eCaseMap = static_cast<SvxCaseMap>(nEnum);
            }
            else if( SvxCSS1Parser::GetEnum( aFontWeightTable, rValue, nEnum ) )
            {
                eWeight = static_cast<FontWeight>(nEnum);
            }
        }
        else
        {
            eWeight = o3tl::narrowing<sal_uInt16>(pExpr->GetNumber()) > 400 ? WEIGHT_BOLD
                                                       : WEIGHT_NORMAL;
        }

        pExpr = pExpr->GetNext();
    }

    if( !pExpr || pExpr->GetOp() )
        return;

    // Since "font" resets all values for which nothing is specified,
    // we do it here.
    SvxPostureItem aPosture( eItalic, aItemIds.nPosture );
    rItemSet.Put( aPosture );
    aPosture.SetWhich( aItemIds.nPostureCJK );
    rItemSet.Put( aPosture );
    aPosture.SetWhich( aItemIds.nPostureCTL );
    rItemSet.Put( aPosture );

    rItemSet.Put( SvxCaseMapItem( eCaseMap, aItemIds.nCaseMap ) );

    SvxWeightItem aWeight( eWeight, aItemIds.nWeight );
    rItemSet.Put( aWeight );
    aWeight.SetWhich( aItemIds.nWeightCJK );
    rItemSet.Put( aWeight );
    aWeight.SetWhich( aItemIds.nWeightCTL );
    rItemSet.Put( aWeight );

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
    assert(pExpr && "no expression");

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        rItemSet.Put( SvxKerningItem( static_cast<short>(pExpr->GetSLength()),
                                      aItemIds.nKerning ) );
        break;

    case CSS1_PIXLENGTH:
        {
            double fHeight = pExpr->GetNumber();
            if (fHeight < SAL_MAX_INT32/2.0 && fHeight > SAL_MIN_INT32/2.0)
            {
                tools::Long nPWidth = static_cast<tools::Long>(fHeight);
                tools::Long nPHeight = 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                rItemSet.Put( SvxKerningItem( static_cast<short>(nPWidth), aItemIds.nKerning ) );
            }
        }
        break;

    case CSS1_NUMBER:
        if( pExpr->GetNumber() == 0 )
        {
            // normally unnecessary, but we are tolerant
            rItemSet.Put( SvxKerningItem( short(0), aItemIds.nKerning ) );
        }
        break;

    case CSS1_IDENT:
    case CSS1_STRING: // As a precaution also MS-IE
        if( pExpr->GetString().equalsIgnoreAsciiCase( "normal" ) )
        {
            rItemSet.Put( SvxKerningItem( short(0), aItemIds.nKerning ) );
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

    bool bUnderline = false;
    bool bOverline = false;
    bool bCrossedOut = false;
    bool bBlink = false;
    bool bBlinkOn = false;
    FontLineStyle eUnderline  = LINESTYLE_NONE;
    FontLineStyle eOverline   = LINESTYLE_NONE;
    FontStrikeout eCrossedOut = STRIKEOUT_NONE;

    // the value can contain two values! And MS-IE also strings
    while( pExpr && (pExpr->GetType() == CSS1_IDENT ||
                     pExpr->GetType() == CSS1_STRING) && !pExpr->GetOp() )
    {
        OUString aValue = pExpr->GetString().toAsciiLowerCase();
        bool bKnown = false;

        switch( aValue[0] )
        {
        case 'n':
            if( aValue == "none" )
            {
                bUnderline = true;
                eUnderline = LINESTYLE_NONE;

                bOverline = true;
                eOverline = LINESTYLE_NONE;

                bCrossedOut = true;
                eCrossedOut = STRIKEOUT_NONE;

                bBlink = true;
                bBlinkOn = false;

                bKnown = true;
            }
            break;

        case 'u':
            if( aValue == "underline" )
            {
                bUnderline = true;
                eUnderline = LINESTYLE_SINGLE;

                bKnown = true;
            }
            break;

        case 'o':
            if( aValue == "overline" )
            {
                bOverline = true;
                eOverline = LINESTYLE_SINGLE;

                bKnown = true;
            }
            break;

        case 'l':
            if( aValue == "line-through" )
            {
                bCrossedOut = true;
                eCrossedOut = STRIKEOUT_SINGLE;

                bKnown = true;
            }
            break;

        case 'b':
            if( aValue == "blink" )
            {
                bBlink = true;
                bBlinkOn = true;

                bKnown = true;
            }
            break;
        }

        if( !bKnown )
        {
            bUnderline = true;
            eUnderline = LINESTYLE_SINGLE;
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
    assert(pExpr && "no expression");

    if( CSS1_IDENT==pExpr->GetType() ||
        CSS1_STRING==pExpr->GetType() ) // MS-IE, again
    {
        sal_uInt16 nAdjust;
        if( SvxCSS1Parser::GetEnum( aTextAlignTable, pExpr->GetString(),
                                    nAdjust ) )
        {
            rItemSet.Put( SvxAdjustItem( static_cast<SvxAdjust>(nAdjust),
                                         aItemIds.nAdjust ) );
        }
    }
}

static void ParseCSS1_text_indent( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    short nIndent = 0;
    bool bSet = false;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            double n = std::round(pExpr->GetNumber());
            SAL_WARN_IF(
                n < std::numeric_limits<short>::min() || n > std::numeric_limits<short>::max(),
                "sw.html", "clamping length " << n << " to short range");
            nIndent = static_cast<short>(
                std::clamp(
                    n, double(std::numeric_limits<short>::min()),
                    double(std::numeric_limits<short>::max())));
            bSet = true;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            double fWidth = pExpr->GetNumber();
            if (fWidth < SAL_MAX_INT32/2.0 && fWidth > SAL_MIN_INT32/2.0)
            {
                tools::Long nPWidth = static_cast<tools::Long>(fWidth);
                tools::Long nPHeight = 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                nIndent = static_cast<short>(nPWidth);
                bSet = true;
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // we aren't able
        break;
    default:
        ;
    }

    if( !bSet )
        return;

    SvxFirstLineIndentItem const firstLine(nIndent, RES_MARGIN_FIRSTLINE);
    rItemSet.Put(firstLine);
    rPropInfo.m_bTextIndent = true;
}

static void ParseCSS1_margin_left( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    tools::Long nLeft = 0;
    bool bSet = false;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            nLeft = pExpr->GetSLength();
            bSet = true;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            double fLeft = pExpr->GetNumber();
            if (fLeft < SAL_MAX_INT32/2.0 && fLeft > SAL_MIN_INT32/2.0)
            {
                nLeft = static_cast<tools::Long>(fLeft);
                tools::Long nPHeight = 0;
                SvxCSS1Parser::PixelToTwip( nLeft, nPHeight );
                bSet = true;
            }
            else
            {
                SAL_WARN("sw.html", "out-of-size pxlength: " << fLeft);
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // we aren't able
        break;
    default:
        ;
    }

    if (pExpr->GetString() == "auto")
    {
        rPropInfo.m_bLeftMargin = true;
        rPropInfo.m_eLeftMarginType = SVX_CSS1_LTYPE_AUTO;
    }

    if( !bSet )
        return;

    rPropInfo.m_nLeftMargin = nLeft;
    if( nLeft < 0 )
        nLeft = 0;

    // TODO: other things may need a SvxLeftMarginItem ? but they currently convert it anyway so they can convert that too.
    SvxTextLeftMarginItem const leftMargin(o3tl::narrowing<sal_uInt16>(nLeft), RES_MARGIN_TEXTLEFT);
    rItemSet.Put(leftMargin);
    rPropInfo.m_bLeftMargin = true;
}

static void ParseCSS1_margin_right( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    tools::Long nRight = 0;
    bool bSet = false;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            nRight = pExpr->GetSLength();
            bSet = true;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            double fRight = pExpr->GetNumber();
            if (fRight < SAL_MAX_INT32/2.0 && fRight > SAL_MIN_INT32/2.0)
            {
                nRight = static_cast<tools::Long>(fRight);
                tools::Long nPHeight = 0;
                SvxCSS1Parser::PixelToTwip( nRight, nPHeight );
                bSet = true;
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // we aren't able
        break;
    default:
        ;
    }

    if (pExpr->GetString() == "auto")
    {
        rPropInfo.m_bRightMargin = true;
        rPropInfo.m_eRightMarginType = SVX_CSS1_LTYPE_AUTO;
    }

    if( !bSet )
        return;

    rPropInfo.m_nRightMargin = nRight;
    if( nRight < 0 )
        nRight = 0;

    SvxRightMarginItem rightMargin(o3tl::narrowing<sal_uInt16>(nRight), RES_MARGIN_RIGHT);
    rItemSet.Put(rightMargin);
    rPropInfo.m_bRightMargin = true;
}

static void ParseCSS1_margin_top( const CSS1Expression *pExpr,
                                  SfxItemSet &rItemSet,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    sal_uInt16 nUpper = 0;
    bool bSet = false;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            tools::Long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            nUpper = o3tl::narrowing<sal_uInt16>(nTmp);
            bSet = true;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            double fHeight = pExpr->GetNumber();
            if (fHeight < SAL_MAX_INT32/2.0 && fHeight > SAL_MIN_INT32/2.0)
            {
                tools::Long nPWidth = 0;
                tools::Long nPHeight =  static_cast<tools::Long>(fHeight);
                if( nPHeight < 0 )
                    nPHeight = 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                nUpper = o3tl::narrowing<sal_uInt16>(nPHeight);
                bSet = true;
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // we aren't able
        break;
    default:
        ;
    }

    if( !bSet )
        return;

    if( const SvxULSpaceItem* pItem = rItemSet.GetItemIfSet( aItemIds.nULSpace, false ) )
    {
        SvxULSpaceItem aULItem( *pItem );
        aULItem.SetUpper( nUpper );
        rItemSet.Put( aULItem );
    }
    else
    {
        SvxULSpaceItem aULItem( aItemIds.nULSpace );
        aULItem.SetUpper( nUpper );
        rItemSet.Put( aULItem );
    }
    rPropInfo.m_bTopMargin = true;
}

static void ParseCSS1_margin_bottom( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    sal_uInt16 nLower = 0;
    bool bSet = false;
    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            tools::Long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            nLower = o3tl::narrowing<sal_uInt16>(nTmp);
            bSet = true;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            double fHeight = pExpr->GetNumber();
            if (fHeight < SAL_MAX_INT32/2.0 && fHeight > SAL_MIN_INT32/2.0)
            {
                tools::Long nPWidth = 0;
                tools::Long nPHeight =  static_cast<tools::Long>(fHeight);
                if( nPHeight < 0 )
                    nPHeight = 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                nLower = o3tl::narrowing<sal_uInt16>(nPHeight);
                bSet = true;
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // we aren't able
        break;
    default:
        ;
    }

    if( !bSet )
        return;

    if( const SvxULSpaceItem* pItem = rItemSet.GetItemIfSet( aItemIds.nULSpace, false ) )
    {
        SvxULSpaceItem aULItem( *pItem );
        aULItem.SetLower( nLower );
        rItemSet.Put( aULItem );
    }
    else
    {
        SvxULSpaceItem aULItem( aItemIds.nULSpace );
        aULItem.SetLower( nLower );
        rItemSet.Put( aULItem );
    }
    rPropInfo.m_bBottomMargin = true;
}

static void ParseCSS1_margin( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& /*rParser*/ )
{
    OSL_ENSURE( pExpr, "no expression" );

    tools::Long nMargins[4] = { 0, 0, 0, 0 };
    bool bSetMargins[4] = { false, false, false, false };

    for( int i=0; pExpr && i<4 && !pExpr->GetOp(); ++i )
    {
        bool bSetThis = false;
        tools::Long nMargin = 0;

        switch( pExpr->GetType() )
        {
        case CSS1_LENGTH:
            {
                nMargin = pExpr->GetSLength();
                bSetThis = true;
            }
            break;
        case CSS1_PIXLENGTH:
            {
                double fMargin = pExpr->GetNumber();
                if (fMargin < SAL_MAX_INT32/2.0 && fMargin > SAL_MIN_INT32/2.0)
                {
                    nMargin =  static_cast<tools::Long>(fMargin);
                    tools::Long nPWidth = 0;
                    SvxCSS1Parser::PixelToTwip( nPWidth, nMargin );
                    bSetThis = true;
                }
                else
                {
                    SAL_WARN("sw.html", "out-of-size pxlength: " << fMargin);
                }
            }
            break;
        case CSS1_PERCENTAGE:
            // we aren't able
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
                bSetMargins[2] = bSetMargins[3] = true;
                break;
            case 1:
                nMargins[1] = nMargins[3] = nMargin;    // right + left
                bSetMargins[1] = bSetMargins[3] = true;
                break;
            case 2:
                nMargins[2] = nMargin;  // bottom
                bSetMargins[2] = true;
                break;
            case 3:
                nMargins[3] = nMargin;  // left
                bSetMargins[3] = true;
                break;
            }
        }
        pExpr = pExpr->GetNext();
    }

    if( bSetMargins[3] || bSetMargins[1] )
    {
        if( bSetMargins[3] )
        {
            rPropInfo.m_bLeftMargin = true;
            rPropInfo.m_nLeftMargin = nMargins[3];
            if( nMargins[3] < 0 )
                nMargins[3] = 0;
        }
        if( bSetMargins[1] )
        {
            rPropInfo.m_bRightMargin = true;
            rPropInfo.m_nRightMargin = nMargins[1];
            if( nMargins[1] < 0 )
                nMargins[1] = 0;
        }

        if (bSetMargins[3])
        {
            SvxTextLeftMarginItem const leftMargin(o3tl::narrowing<sal_uInt16>(nMargins[3]), RES_MARGIN_TEXTLEFT);
            rItemSet.Put(leftMargin);
        }
        if (bSetMargins[1])
        {
            SvxRightMarginItem const rightMargin(o3tl::narrowing<sal_uInt16>(nMargins[1]), RES_MARGIN_RIGHT);
            rItemSet.Put(rightMargin);
        }
    }

    if( !(bSetMargins[0] || bSetMargins[2]) )
        return;

    if( nMargins[0] < 0 )
        nMargins[0] = 0;
    if( nMargins[2] < 0 )
        nMargins[2] = 0;

    if( const SvxULSpaceItem* pItem = rItemSet.GetItemIfSet( aItemIds.nULSpace, false ) )
    {
        SvxULSpaceItem aULItem( *pItem );
        if( bSetMargins[0] )
            aULItem.SetUpper( o3tl::narrowing<sal_uInt16>(nMargins[0]) );
        if( bSetMargins[2] )
            aULItem.SetLower( o3tl::narrowing<sal_uInt16>(nMargins[2]) );
        rItemSet.Put( aULItem );
    }
    else
    {
        SvxULSpaceItem aULItem( aItemIds.nULSpace );
        if( bSetMargins[0] )
            aULItem.SetUpper( o3tl::narrowing<sal_uInt16>(nMargins[0]) );
        if( bSetMargins[2] )
            aULItem.SetLower( o3tl::narrowing<sal_uInt16>(nMargins[2]) );
        rItemSet.Put( aULItem );
    }

    rPropInfo.m_bTopMargin |= bSetMargins[0];
    rPropInfo.m_bBottomMargin |= bSetMargins[2];
}

static bool ParseCSS1_padding_xxx( const CSS1Expression *pExpr,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   SvxBoxItemLine nWhichLine )
{
    assert(pExpr && "no expression");

    bool bSet = false;
    sal_uInt16 nDist = 0;

    switch( pExpr->GetType() )
    {
    case CSS1_LENGTH:
        {
            tools::Long nTmp = pExpr->GetSLength();
            if( nTmp < 0 )
                nTmp = 0;
            else if( nTmp > SvxCSS1PropertyInfo::UNSET_BORDER_DISTANCE-1 )
                nTmp = SvxCSS1PropertyInfo::UNSET_BORDER_DISTANCE-1;
            nDist = o3tl::narrowing<sal_uInt16>(nTmp);
            bSet = true;
        }
        break;
    case CSS1_PIXLENGTH:
        {
            double fWidth = pExpr->GetNumber();
            if (fWidth < SAL_MAX_INT32/2.0 && fWidth > SAL_MIN_INT32/2.0)
            {
                tools::Long nPWidth = static_cast<tools::Long>(fWidth);
                tools::Long nPHeight = 0;
                if( nPWidth < 0 )
                    nPWidth = 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                if( nPWidth > SvxCSS1PropertyInfo::UNSET_BORDER_DISTANCE-1 )
                    nPWidth = SvxCSS1PropertyInfo::UNSET_BORDER_DISTANCE-1;
                nDist = o3tl::narrowing<sal_uInt16>(nPWidth);
                bSet = true;
            }
        }
        break;
    case CSS1_PERCENTAGE:
        // we aren't able
        break;
    default:
        ;
    }

    if( bSet )
    {
        switch( nWhichLine )
        {
        case SvxBoxItemLine::TOP:      rPropInfo.m_nTopBorderDistance = nDist;   break;
        case SvxBoxItemLine::BOTTOM:   rPropInfo.m_nBottomBorderDistance = nDist;break;
        case SvxBoxItemLine::LEFT:     rPropInfo.m_nLeftBorderDistance = nDist;  break;
        case SvxBoxItemLine::RIGHT:    rPropInfo.m_nRightBorderDistance = nDist; break;
        }
    }

    return bSet;
}

static void ParseCSS1_padding_top( const CSS1Expression *pExpr,
                                   SfxItemSet & /*rItemSet*/,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_padding_xxx( pExpr, rPropInfo, SvxBoxItemLine::TOP );
}

static void ParseCSS1_padding_bottom( const CSS1Expression *pExpr,
                                      SfxItemSet & /*rItemSet*/,
                                      SvxCSS1PropertyInfo& rPropInfo,
                                      const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_padding_xxx( pExpr, rPropInfo, SvxBoxItemLine::BOTTOM );
}

static void ParseCSS1_padding_left( const CSS1Expression *pExpr,
                                    SfxItemSet & /*rItemSet*/,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_padding_xxx( pExpr, rPropInfo, SvxBoxItemLine::LEFT );
}

static void ParseCSS1_padding_right( const CSS1Expression *pExpr,
                                     SfxItemSet & /*rItemSet*/,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_padding_xxx( pExpr, rPropInfo, SvxBoxItemLine::RIGHT );
}

static void ParseCSS1_padding( const CSS1Expression *pExpr,
                               SfxItemSet & /*rItemSet*/,
                               SvxCSS1PropertyInfo& rPropInfo,
                               const SvxCSS1Parser& /*rParser*/ )
{
    int n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        SvxBoxItemLine nLine = n==0 || n==2 ? SvxBoxItemLine::BOTTOM : SvxBoxItemLine::LEFT;
        if( ParseCSS1_padding_xxx( pExpr, rPropInfo, nLine ) )
        {
            if( n==0 )
            {
                rPropInfo.m_nTopBorderDistance = rPropInfo.m_nBottomBorderDistance;
                rPropInfo.m_nLeftBorderDistance = rPropInfo.m_nTopBorderDistance;
            }
            if( n <= 1 )
                rPropInfo.m_nRightBorderDistance = rPropInfo.m_nLeftBorderDistance;
        }

        pExpr = pExpr->GetNext();
        n++;
    }
}

static void ParseCSS1_border_xxx( const CSS1Expression *pExpr,
                                  SfxItemSet & /*rItemSet*/,
                                  SvxCSS1PropertyInfo& rPropInfo,
                                  const SvxCSS1Parser& /*rParser*/,
                                  SvxBoxItemLine nWhichLine, bool bAll )
{
    assert(pExpr && "no expression");

    sal_uInt16 nWidth = USHRT_MAX;      // line thickness
    sal_uInt16 nNWidth = 1;             // named line thickness (and default)
    CSS1BorderStyle eStyle = CSS1_BS_NONE; // line style
    Color aColor;
    bool bColor = false;

    while( pExpr && !pExpr->GetOp() )
    {
        switch( pExpr->GetType() )
        {
        case CSS1_RGB:
        case CSS1_HEXCOLOR:
            if( pExpr->GetColor( aColor ) )
                bColor = true;
            break;

        case CSS1_IDENT:
            {
                const OUString& rValue = pExpr->GetString();
                sal_uInt16 nValue;
                if( SvxCSS1Parser::GetEnum( aBorderWidthTable, rValue, nValue ) )
                {
                    nNWidth = nValue;
                }
                else if( SvxCSS1Parser::GetEnum( aBorderStyleTable, rValue, nValue ) )
                {
                    eStyle = static_cast<CSS1BorderStyle>(nValue);
                }
                else if( pExpr->GetColor( aColor ) )
                {
                    bColor = true;
                }
            }
            break;

        case CSS1_LENGTH:
            nWidth = o3tl::narrowing<sal_uInt16>(pExpr->GetULength());
            break;

        case CSS1_PIXLENGTH:
            {
                // One Pixel becomes a hairline (is prettier)
                double fWidth = pExpr->GetNumber();
                if (fWidth > 1.0 && fWidth < SAL_MAX_INT32/2.0)
                {
                    bool bHori = nWhichLine == SvxBoxItemLine::TOP ||
                                 nWhichLine == SvxBoxItemLine::BOTTOM;

                    tools::Long nPWidth = bHori ? 0 : fWidth;
                    tools::Long nPHeight = bHori ? fWidth : 0;
                    SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                    nWidth = o3tl::narrowing<sal_uInt16>(bHori ? nPHeight : nPWidth);
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

    for( int i=0; i<4; ++i )
    {
        SvxBoxItemLine nLine = SvxBoxItemLine::TOP;
        switch( i )
        {
        case 0: nLine = SvxBoxItemLine::TOP; break;
        case 1: nLine = SvxBoxItemLine::BOTTOM; break;
        case 2: nLine = SvxBoxItemLine::LEFT; break;
        case 3: nLine = SvxBoxItemLine::RIGHT; break;
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
                                        SvxBoxItemLine nWhichLine )
{
    assert(pExpr && "no expression");

    sal_uInt16 nWidth = USHRT_MAX;      // line thickness
    sal_uInt16 nNWidth = 1;             // named line thickness (and default)

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
        nWidth = o3tl::narrowing<sal_uInt16>(pExpr->GetULength());
        break;

    case CSS1_PIXLENGTH:
        {
            double fLength = pExpr->GetNumber();
            if (fLength < SAL_MAX_INT32/2.0 && fLength > SAL_MIN_INT32/2.0)
            {
                tools::Long nWidthL = static_cast<tools::Long>(fLength);

                bool bHori = nWhichLine == SvxBoxItemLine::TOP ||
                             nWhichLine == SvxBoxItemLine::BOTTOM;

                tools::Long nPWidth = bHori ? 0 : nWidthL;
                tools::Long nPHeight = bHori ? nWidthL : 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                nWidth = o3tl::narrowing<sal_uInt16>(bHori ? nPHeight : nPWidth);
            }
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
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::TOP );
}

static void ParseCSS1_border_right_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::RIGHT );
}

static void ParseCSS1_border_bottom_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::BOTTOM );
}

static void ParseCSS1_border_left_width( const CSS1Expression *pExpr,
                                        SfxItemSet &rItemSet,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx_width( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::LEFT );
}

static void ParseCSS1_border_width( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    sal_uInt16 n=0;
    while( n<4 && pExpr && !pExpr->GetOp() )
    {
        SvxBoxItemLine nLine = n==0 || n==2 ? SvxBoxItemLine::BOTTOM : SvxBoxItemLine::LEFT;
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
        SvxBoxItemLine nLine = n==0 || n==2 ? SvxBoxItemLine::BOTTOM : SvxBoxItemLine::LEFT;
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
        SvxBoxItemLine nLine = n==0 || n==2 ? SvxBoxItemLine::BOTTOM : SvxBoxItemLine::LEFT;
        sal_uInt16 nValue = 0;
        if( CSS1_IDENT==pExpr->GetType() &&
            SvxCSS1Parser::GetEnum( aBorderStyleTable, pExpr->GetString(),
                                    nValue ) )
        {
            rPropInfo.GetBorderInfo( nLine )->eStyle = static_cast<CSS1BorderStyle>(nValue);
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
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::TOP, false );
}

static void ParseCSS1_border_right( const CSS1Expression *pExpr,
                                    SfxItemSet &rItemSet,
                                    SvxCSS1PropertyInfo& rPropInfo,
                                    const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::RIGHT, false );
}

static void ParseCSS1_border_bottom( const CSS1Expression *pExpr,
                                     SfxItemSet &rItemSet,
                                     SvxCSS1PropertyInfo& rPropInfo,
                                     const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::BOTTOM, false );
}

static void ParseCSS1_border_left( const CSS1Expression *pExpr,
                                   SfxItemSet &rItemSet,
                                   SvxCSS1PropertyInfo& rPropInfo,
                                   const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::LEFT, false );
}

static void ParseCSS1_border( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& rParser )
{
    ParseCSS1_border_xxx( pExpr, rItemSet, rPropInfo, rParser, SvxBoxItemLine::TOP, true );
}

static void ParseCSS1_float( const CSS1Expression *pExpr,
                             SfxItemSet & /*rItemSet*/,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    if( CSS1_IDENT==pExpr->GetType() )
    {
        sal_uInt16 nFloat;
        if( SvxCSS1Parser::GetEnum( aFloatTable, pExpr->GetString(), nFloat ) )
            rPropInfo.m_eFloat = static_cast<SvxAdjust>(nFloat);
    }
}

static void ParseCSS1_position( const CSS1Expression *pExpr,
                                SfxItemSet & /*rItemSet*/,
                                SvxCSS1PropertyInfo& rPropInfo,
                                const SvxCSS1Parser& /*rParser*/ )
{
    assert(pExpr && "no expression");

    if( CSS1_IDENT==pExpr->GetType() )
    {
        sal_uInt16 nPos;
        if( SvxCSS1Parser::GetEnum( aPositionTable, pExpr->GetString(), nPos ) )
            rPropInfo.m_ePosition = static_cast<SvxCSS1Position>(nPos);
    }
}

static void ParseCSS1_length( const CSS1Expression *pExpr,
                              tools::Long& rLength,
                              SvxCSS1LengthType& rLengthType,
                              bool bHori )
{
    switch( pExpr->GetType() )
    {
    case CSS1_IDENT:
        if( pExpr->GetString().equalsIgnoreAsciiCase( "auto" ) )
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
    case CSS1_NUMBER:       // because of Netscape and IE
        {
            double fLength = pExpr->GetNumber();
            if (fLength < SAL_MAX_INT32/2.0 && fLength > SAL_MIN_INT32/2.0)
            {
                tools::Long nWidthL = static_cast<tools::Long>(fLength);
                tools::Long nPWidth = bHori ? 0 : nWidthL;
                tools::Long nPHeight = bHori ? nWidthL : 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                rLength = (bHori ? nPHeight : nPWidth);
                rLengthType = SVX_CSS1_LTYPE_TWIP;
            }
        }
        break;

    case CSS1_PERCENTAGE:
        rLength = static_cast<tools::Long>(std::min(pExpr->GetNumber(), 100.0));
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
    ParseCSS1_length( pExpr, rPropInfo.m_nWidth, rPropInfo.m_eWidthType, true );
}

static void ParseCSS1_height( const CSS1Expression *pExpr,
                              SfxItemSet & /*rItemSet*/,
                              SvxCSS1PropertyInfo& rPropInfo,
                              const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.m_nHeight, rPropInfo.m_eHeightType, false );
}

static void ParseCSS1_left( const CSS1Expression *pExpr,
                             SfxItemSet & /*rItemSet*/,
                             SvxCSS1PropertyInfo& rPropInfo,
                             const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.m_nLeft, rPropInfo.m_eLeftType, true );
}

static void ParseCSS1_top( const CSS1Expression *pExpr,
                           SfxItemSet & /*rItemSet*/,
                           SvxCSS1PropertyInfo& rPropInfo,
                           const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_length( pExpr, rPropInfo.m_nTop, rPropInfo.m_eTopType, false );
}

// Feature: PrintExt
static void ParseCSS1_size( const CSS1Expression *pExpr,
                            SfxItemSet & /*rItemSet*/,
                            SvxCSS1PropertyInfo& rPropInfo,
                            const SvxCSS1Parser& /*rParser*/ )
{
    int n=0;
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
                    rPropInfo.m_eSizeType = static_cast<SvxCSS1SizeType>(nValue);
                }
            }
            break;

        case CSS1_LENGTH:
            rPropInfo.m_nHeight = pExpr->GetSLength();
            if( n==0 )
                rPropInfo.m_nWidth = rPropInfo.m_nHeight;
            rPropInfo.m_eSizeType = SVX_CSS1_STYPE_TWIP;
            break;

        case CSS1_PIXLENGTH:
        {
            double fHeight = pExpr->GetNumber();
            if (fHeight < SAL_MAX_INT32/2.0 && fHeight > SAL_MIN_INT32/2.0)
            {
                tools::Long nPHeight = static_cast<tools::Long>(fHeight);
                tools::Long nPWidth = n==0 ? nPHeight : 0;
                SvxCSS1Parser::PixelToTwip( nPWidth, nPHeight );
                rPropInfo.m_nHeight = nPHeight;
                if( n==0 )
                    rPropInfo.m_nWidth = nPWidth;
                rPropInfo.m_eSizeType = SVX_CSS1_STYPE_TWIP;
            }
            break;
        }
        default:
            ;
        }

        pExpr = pExpr->GetNext();
        n++;
    }
}

static void ParseCSS1_page_break_xxx( const CSS1Expression *pExpr,
                                      SvxCSS1PageBreak& rPBreak )
{
    if( CSS1_IDENT == pExpr->GetType() )
    {
        sal_uInt16 nValue;
        if( SvxCSS1Parser::GetEnum( aPageBreakTable, pExpr->GetString(),
                                    nValue ) )
        {
            rPBreak = static_cast<SvxCSS1PageBreak>(nValue);
        }
    }
}

static void ParseCSS1_page_break_before( const CSS1Expression *pExpr,
                                         SfxItemSet & /*rItemSet*/,
                                         SvxCSS1PropertyInfo& rPropInfo,
                                         const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_page_break_xxx( pExpr, rPropInfo.m_ePageBreakBefore );
}

static void ParseCSS1_page_break_after( const CSS1Expression *pExpr,
                                        SfxItemSet & /*rItemSet*/,
                                        SvxCSS1PropertyInfo& rPropInfo,
                                        const SvxCSS1Parser& /*rParser*/ )
{
    ParseCSS1_page_break_xxx( pExpr, rPropInfo.m_ePageBreakAfter );
}

static void ParseCSS1_page_break_inside( const CSS1Expression *pExpr,
                                         SfxItemSet &rItemSet,
                                         SvxCSS1PropertyInfo& /*rPropInfo*/,
                                         const SvxCSS1Parser& /*rParser*/ )
{
    SvxCSS1PageBreak eBreak(SVX_CSS1_PBREAK_NONE);
    ParseCSS1_page_break_xxx( pExpr, eBreak );

    bool bSetSplit = false, bSplit = true;
    switch( eBreak )
    {
    case SVX_CSS1_PBREAK_AUTO:
        bSetSplit = true;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bSplit = false;
        bSetSplit = true;
        break;
    default:
        ;
    }

    if( bSetSplit )
        rItemSet.Put( SvxFormatSplitItem( bSplit, aItemIds.nFormatSplit ) );
}

static void ParseCSS1_widows( const CSS1Expression *pExpr,
                              SfxItemSet &rItemSet,
                              SvxCSS1PropertyInfo& /*rPropInfo*/,
                              const SvxCSS1Parser& /*rParser*/ )
{
    if( CSS1_NUMBER == pExpr->GetType() )
    {
        sal_uInt8 nVal = pExpr->GetNumber() <= 255
                        ? static_cast<sal_uInt8>(pExpr->GetNumber())
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
                        ? static_cast<sal_uInt8>(pExpr->GetNumber())
                        : 255;
        SvxOrphansItem aOrphansItem( nVal, aItemIds.nOrphans );
        rItemSet.Put( aOrphansItem );
    }
}

static void ParseCSS1_so_language( const CSS1Expression *pExpr,
                               SfxItemSet &rItemSet,
                               SvxCSS1PropertyInfo& /*rPropInfo*/,
                               const SvxCSS1Parser& /*rParser*/ )
{
    if( CSS1_IDENT != pExpr->GetType() && CSS1_STRING != pExpr->GetType() )
        return;

    LanguageType eLang = LanguageTag::convertToLanguageTypeWithFallback( pExpr->GetString() );
    if( LANGUAGE_DONTKNOW != eLang )
    {
        SvxLanguageItem aLang( eLang, aItemIds.nLanguage );
        rItemSet.Put( aLang );
        aLang.SetWhich( aItemIds.nLanguageCJK );
        rItemSet.Put( aLang );
        aLang.SetWhich( aItemIds.nLanguageCTL );
        rItemSet.Put( aLang );
    }
}

static void ParseCSS1_visibility(const CSS1Expression* pExpr, SfxItemSet& /*rItemSet*/,
                                 SvxCSS1PropertyInfo& rPropInfo, const SvxCSS1Parser& /*rParser*/)
{
    if (pExpr->GetType() != CSS1_IDENT)
        return;

    rPropInfo.m_bVisible = pExpr->GetString() != "hidden";
}

static void ParseCSS1_white_space(const CSS1Expression* pExpr, SfxItemSet& /*rItemSet*/,
                                  SvxCSS1PropertyInfo& rPropInfo, const SvxCSS1Parser& /*rParser*/)
{
    if (pExpr->GetType() == CSS1_IDENT)
    {
        if (pExpr->GetString().equalsIgnoreAsciiCase("pre")
            || pExpr->GetString().equalsIgnoreAsciiCase("pre-wrap"))
        {
            rPropInfo.m_bPreserveSpace = true;
        }
    }
}

namespace {

// the assignment of property to parsing function
struct CSS1PropEntry
{
    std::string_view pName;
    FnParseCSS1Prop pFunc;
};

}

// the table with assignments
CSS1PropEntry constexpr aCSS1PropFnTab[] =
{
    { sCSS1_P_background, ParseCSS1_background },
    { sCSS1_P_background_color, ParseCSS1_background_color },
    { sCSS1_P_border, ParseCSS1_border },
    { sCSS1_P_border_bottom, ParseCSS1_border_bottom },
    { sCSS1_P_border_bottom_width, ParseCSS1_border_bottom_width },
    { sCSS1_P_border_color, ParseCSS1_border_color },
    { sCSS1_P_border_left, ParseCSS1_border_left },
    { sCSS1_P_border_left_width, ParseCSS1_border_left_width },
    { sCSS1_P_border_right, ParseCSS1_border_right },
    { sCSS1_P_border_right_width, ParseCSS1_border_right_width },
    { sCSS1_P_border_style, ParseCSS1_border_style },
    { sCSS1_P_border_top, ParseCSS1_border_top },
    { sCSS1_P_border_top_width, ParseCSS1_border_top_width },
    { sCSS1_P_border_width, ParseCSS1_border_width },
    { sCSS1_P_color, ParseCSS1_color },
    { sCSS1_P_column_count, ParseCSS1_column_count },
    { sCSS1_P_direction, ParseCSS1_direction },
    { sCSS1_P_float, ParseCSS1_float },
    { sCSS1_P_font, ParseCSS1_font },
    { sCSS1_P_font_family, ParseCSS1_font_family },
    { sCSS1_P_font_size, ParseCSS1_font_size },
    { sCSS1_P_font_style, ParseCSS1_font_style },
    { sCSS1_P_font_variant, ParseCSS1_font_variant },
    { sCSS1_P_font_weight, ParseCSS1_font_weight },
    { sCSS1_P_height, ParseCSS1_height },
    { sCSS1_P_left, ParseCSS1_left },
    { sCSS1_P_letter_spacing, ParseCSS1_letter_spacing },
    { sCSS1_P_line_height, ParseCSS1_line_height },
    { sCSS1_P_list_style_type, ParseCSS1_list_style_type },
    { sCSS1_P_margin, ParseCSS1_margin },
    { sCSS1_P_margin_bottom, ParseCSS1_margin_bottom },
    { sCSS1_P_margin_left, ParseCSS1_margin_left },
    { sCSS1_P_margin_right, ParseCSS1_margin_right },
    { sCSS1_P_margin_top, ParseCSS1_margin_top },
    { sCSS1_P_orphans, ParseCSS1_orphans },
    { sCSS1_P_padding, ParseCSS1_padding },
    { sCSS1_P_padding_bottom, ParseCSS1_padding_bottom },
    { sCSS1_P_padding_left, ParseCSS1_padding_left },
    { sCSS1_P_padding_right, ParseCSS1_padding_right },
    { sCSS1_P_padding_top, ParseCSS1_padding_top },
    { sCSS1_P_page_break_after, ParseCSS1_page_break_after },
    { sCSS1_P_page_break_before, ParseCSS1_page_break_before },
    { sCSS1_P_page_break_inside, ParseCSS1_page_break_inside },
    { sCSS1_P_position, ParseCSS1_position },
    { sCSS1_P_size, ParseCSS1_size },
    { sCSS1_P_so_language, ParseCSS1_so_language },
    { sCSS1_P_text_align, ParseCSS1_text_align },
    { sCSS1_P_text_decoration, ParseCSS1_text_decoration },
    { sCSS1_P_text_indent, ParseCSS1_text_indent },
    { sCSS1_P_text_transform, ParseCSS1_text_transform },
    { sCSS1_P_top, ParseCSS1_top },
    { sCSS1_P_visibility, ParseCSS1_visibility },
    { sCSS1_white_space, ParseCSS1_white_space },
    { sCSS1_P_widows, ParseCSS1_widows },
    { sCSS1_P_width, ParseCSS1_width },
};

static_assert(std::is_sorted(std::begin(aCSS1PropFnTab), std::end(aCSS1PropFnTab),
                             [](const auto& lhs, const auto& rhs) constexpr
                             { return lhs.pName < rhs.pName; }));

static bool CSS1PropEntryFindCompare(CSS1PropEntry const & lhs, OUString const & s)
{
    return s.compareToIgnoreAsciiCaseAscii(lhs.pName) > 0;
}

void SvxCSS1Parser::DeclarationParsed( const OUString& rProperty,
                                       std::unique_ptr<CSS1Expression> pExpr )
{
    assert(m_pItemSet && "DeclarationParsed() without ItemSet");

    auto it = std::lower_bound( std::begin(aCSS1PropFnTab), std::end(aCSS1PropFnTab), rProperty,
                                CSS1PropEntryFindCompare );
    if( it != std::end(aCSS1PropFnTab) && !CSS1PropEntryFindCompare(*it,rProperty)  )
    {
        it->pFunc( pExpr.get(), *m_pItemSet, *m_pPropInfo, *this );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
