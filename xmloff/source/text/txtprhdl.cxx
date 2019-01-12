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

#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/fontenum.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/text/FontRelief.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/TextColumn.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include "XMLAnchorTypePropHdl.hxx"
#include <enummaps.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include <XMLClipPropertyHandler.hxx>
#include <XMLTextColumnsPropertyHandler.hxx>
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include "txtprhdl.hxx"
#include <com/sun/star/text/WrapInfluenceOnPosition.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <xmloff/EnumPropertyHdl.hxx>
#include <XMLFillBitmapSizePropertyHandler.hxx>
#include <XMLBitmapLogicalSizePropertyHandler.hxx>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <XMLBitmapRepeatOffsetPropertyHandler.hxx>
#include <vcl/graph.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;
using namespace ::com::sun::star::drawing;

static SvXMLEnumMapEntry<sal_uInt16> const pXML_HoriPos_Enum[] =
{
    { XML_FROM_LEFT,        HoriOrientation::NONE   },
    { XML_FROM_INSIDE,      HoriOrientation::NONE   },  // import only
    { XML_LEFT,             HoriOrientation::LEFT   },
    { XML_INSIDE,           HoriOrientation::LEFT   },  // import only
    { XML_CENTER,           HoriOrientation::CENTER },
    { XML_RIGHT,            HoriOrientation::RIGHT  },
    { XML_OUTSIDE,          HoriOrientation::RIGHT  },  // import only
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_HoriPosMirrored_Enum[] =
{
    { XML_FROM_INSIDE,      HoriOrientation::NONE   },
    { XML_INSIDE,           HoriOrientation::LEFT   },
    { XML_CENTER,           HoriOrientation::CENTER },
    { XML_OUTSIDE,          HoriOrientation::RIGHT  },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_HoriRel_Enum[] =
{
    { XML_PARAGRAPH,            RelOrientation::FRAME   },
    { XML_PARAGRAPH_CONTENT,    RelOrientation::PRINT_AREA  },
    { XML_PAGE,                 RelOrientation::PAGE_FRAME  },
    { XML_PAGE_CONTENT,         RelOrientation::PAGE_PRINT_AREA },
    { XML_PARAGRAPH_START_MARGIN,   RelOrientation::FRAME_LEFT  },
    { XML_PARAGRAPH_END_MARGIN, RelOrientation::FRAME_RIGHT },
    { XML_PAGE_START_MARGIN,    RelOrientation::PAGE_LEFT   },
    { XML_PAGE_END_MARGIN,      RelOrientation::PAGE_RIGHT  },
    { XML_CHAR,                 RelOrientation::CHAR    },
    { XML_FRAME,                RelOrientation::FRAME   },      // import only
    { XML_FRAME_CONTENT,        RelOrientation::PRINT_AREA  },  // import only
    { XML_FRAME_START_MARGIN,   RelOrientation::FRAME_LEFT  },  // import only
    { XML_FRAME_END_MARGIN,     RelOrientation::FRAME_RIGHT },  // import only
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_HoriRelFrame_Enum[] =
{
    { XML_FRAME,                RelOrientation::FRAME   },
    { XML_FRAME_CONTENT,        RelOrientation::PRINT_AREA  },
    { XML_PAGE,                 RelOrientation::PAGE_FRAME  },
    { XML_PAGE_CONTENT,         RelOrientation::PAGE_PRINT_AREA },
    { XML_FRAME_START_MARGIN,   RelOrientation::FRAME_LEFT  },
    { XML_FRAME_END_MARGIN,     RelOrientation::FRAME_RIGHT },
    { XML_PAGE_START_MARGIN,    RelOrientation::PAGE_LEFT   },
    { XML_PAGE_END_MARGIN,      RelOrientation::PAGE_RIGHT  },
    { XML_CHAR,                 RelOrientation::CHAR    },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<bool> const pXML_HoriMirror_Enum[] =
{
    { XML_FROM_LEFT,        false },
    { XML_FROM_INSIDE,      true },
    { XML_LEFT,             false },
    { XML_INSIDE,           true },
    { XML_CENTER,           false },
    { XML_RIGHT,            false },
    { XML_OUTSIDE,          true },
    { XML_TOKEN_INVALID,    false }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_VertPosAtChar_Enum[] =
{
    { XML_FROM_TOP,         VertOrientation::NONE       },
    { XML_TOP,              VertOrientation::TOP        },
    { XML_TOP,              VertOrientation::CHAR_TOP   },  // export only
    { XML_TOP,              VertOrientation::LINE_TOP   },  // export only
    { XML_MIDDLE,           VertOrientation::CENTER     },
    { XML_MIDDLE,           VertOrientation::CHAR_CENTER    },  // export only
    { XML_MIDDLE,           VertOrientation::LINE_CENTER    },  // export only
    { XML_BOTTOM,           VertOrientation::BOTTOM     },
    { XML_BELOW,            VertOrientation::CHAR_BOTTOM    },  // export only
    { XML_BOTTOM,           VertOrientation::LINE_BOTTOM    },  // export only
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_VertRel_Enum[] =
{
    { XML_PARAGRAPH,            RelOrientation::FRAME   },
    { XML_PARAGRAPH_CONTENT,    RelOrientation::PRINT_AREA  },
    { XML_CHAR,                 RelOrientation::CHAR    },
    // DVO, OD 17.09.2003 #i18732# - allow vertical alignment at page
    { XML_PAGE,                 RelOrientation::PAGE_FRAME  },
    { XML_PAGE_CONTENT,         RelOrientation::PAGE_PRINT_AREA },
    { XML_FRAME,                RelOrientation::FRAME   },      // import only
    { XML_FRAME_CONTENT,        RelOrientation::PRINT_AREA  },  // import only
    // OD 13.11.2003 #i22341# - new vertical alignment at top of line
    { XML_LINE,                 RelOrientation::TEXT_LINE },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_VertRelPage_Enum[] =
{
    { XML_PAGE,         RelOrientation::FRAME   },
    { XML_PAGE_CONTENT, RelOrientation::PRINT_AREA  },
    { XML_PAGE,         RelOrientation::PAGE_FRAME  },
    { XML_PAGE_CONTENT, RelOrientation::PAGE_PRINT_AREA },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_VertRelFrame_Enum[] =
{
    { XML_FRAME,            RelOrientation::FRAME   },
    { XML_FRAME_CONTENT,    RelOrientation::PRINT_AREA  },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_VertRelAsChar_Enum[] =
{
    { XML_BASELINE,     VertOrientation::TOP        },
    { XML_BASELINE,     VertOrientation::CENTER     },  // export only
    { XML_BASELINE,     VertOrientation::BOTTOM     },  // export only
    { XML_TEXT,         VertOrientation::CHAR_TOP   },
    { XML_TEXT,         VertOrientation::CHAR_CENTER    },  // export only
    { XML_TEXT,         VertOrientation::CHAR_BOTTOM    },  // export only
    { XML_LINE,         VertOrientation::LINE_TOP   },
    { XML_LINE,         VertOrientation::LINE_CENTER    },  // export only
    { XML_LINE,         VertOrientation::LINE_BOTTOM    },  // export only
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<RubyAdjust> const pXML_RubyAdjust_Enum[] =
{
    { XML_LEFT,                 RubyAdjust_LEFT },
    { XML_CENTER,               RubyAdjust_CENTER },
    { XML_RIGHT,                RubyAdjust_RIGHT },
    { XML_DISTRIBUTE_LETTER,    RubyAdjust_BLOCK },
    { XML_DISTRIBUTE_SPACE,     RubyAdjust_INDENT_BLOCK },
    { XML_TOKEN_INVALID,        RubyAdjust(0) }
};

static SvXMLEnumMapEntry<sal_Int16> const pXML_RubyPosition_Enum[] =
{
    { XML_ABOVE,                RubyPosition::ABOVE},
    { XML_BELOW,                RubyPosition::BELOW},
    { XML_INTER_CHARACTER,      RubyPosition::INTER_CHARACTER},
    { XML_TOKEN_INVALID,        0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_FontRelief_Enum[] =
{
    { XML_NONE,             FontRelief::NONE        },
    { XML_ENGRAVED,         FontRelief::ENGRAVED    },
    { XML_EMBOSSED,         FontRelief::EMBOSSED    },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<sal_uInt16> const pXML_ParaVerticalAlign_Enum[] =
{
    { XML_TOP,          ParagraphVertAlign::TOP     },
    { XML_MIDDLE,       ParagraphVertAlign::CENTER  },
    { XML_BOTTOM,       ParagraphVertAlign::BOTTOM  },
    { XML_BASELINE,     ParagraphVertAlign::BASELINE    },
    { XML_AUTO,         ParagraphVertAlign::AUTOMATIC   },
    { XML_TOKEN_INVALID, 0 }
};

// OD 2004-05-05 #i28701#
static SvXMLEnumMapEntry<sal_uInt16> const pXML_WrapInfluenceOnPosition_Enum[] =
{
    // Tokens have been renamed and <XML_ITERATIVE> has been added (#i35017#)
    { XML_ONCE_SUCCESSIVE, WrapInfluenceOnPosition::ONCE_SUCCESSIVE },
    { XML_ONCE_CONCURRENT, WrapInfluenceOnPosition::ONCE_CONCURRENT },
    { XML_ITERATIVE,       WrapInfluenceOnPosition::ITERATIVE },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry<drawing::TextVerticalAdjust> const pXML_VerticalAlign_Enum[] =
{
    { XML_TOP,          drawing::TextVerticalAdjust_TOP },
    { XML_MIDDLE,       drawing::TextVerticalAdjust_CENTER },
    { XML_BOTTOM,       drawing::TextVerticalAdjust_BOTTOM },
    { XML_JUSTIFY,      drawing::TextVerticalAdjust_BLOCK },
    { XML_TOKEN_INVALID, drawing::TextVerticalAdjust(0) }
};

class XMLDropCapPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual bool equals(
            const css::uno::Any& r1,
            const css::uno::Any& r2 ) const override;

    /// TabStops will be imported/exported as XML-Elements. So the Import/Export-work must be done at another place.
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLDropCapPropHdl_Impl::equals(
        const Any& r1,
        const Any& r2 ) const
{
    DropCapFormat aFormat1, aFormat2;
    r1 >>= aFormat1;
    r2 >>= aFormat2;

    return  (aFormat1.Lines <=1 && aFormat2.Lines <=1) ||
            (aFormat1.Lines == aFormat2.Lines &&
             aFormat1.Count == aFormat2.Count &&
             aFormat1.Distance == aFormat2.Distance);
}

bool XMLDropCapPropHdl_Impl::importXML(
        const OUString&,
           Any&,
        const SvXMLUnitConverter& ) const
{
    SAL_WARN( "xmloff", "drop caps are an element import property" );
    return false;
}

bool XMLDropCapPropHdl_Impl::exportXML(
        OUString&,
        const Any&,
        const SvXMLUnitConverter& ) const
{
    SAL_WARN( "xmloff", "drop caps are an element export property" );
    return false;
}

class XMLOpaquePropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLOpaquePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    bool bVal = false;
    if( IsXMLToken( rStrImpValue, XML_FOREGROUND ) )
        bVal = true;
    else if( !IsXMLToken( rStrImpValue, XML_BACKGROUND ) )
        bRet = false;

    if( bRet )
        rValue <<= bVal;

    return bRet;
}

bool XMLOpaquePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *o3tl::doAccess<bool>(rValue) )
           rStrExpValue = GetXMLToken( XML_FOREGROUND );
    else
           rStrExpValue = GetXMLToken( XML_BACKGROUND );

    return true;
}

class XMLContourModePropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLContourModePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    bool bVal = false;
    if( IsXMLToken( rStrImpValue, XML_OUTSIDE ) )
        bVal = true;
    else if( ! IsXMLToken( rStrImpValue, XML_FULL ) )
        bRet = false;

    if( bRet )
        rValue <<= bVal;

    return bRet;
}

bool XMLContourModePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *o3tl::doAccess<bool>(rValue) )
           rStrExpValue = GetXMLToken( XML_OUTSIDE );
    else
           rStrExpValue = GetXMLToken( XML_FULL );

    return true;
}

class XMLParagraphOnlyPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLParagraphOnlyPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    bool bVal = false;

    if( ! IsXMLToken( rStrImpValue, XML_NO_LIMIT ) )
    {
        sal_Int32 nValue = 0;
        bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
        bVal = 1 == nValue;
    }

    if( bRet )
        rValue <<= bVal;

    return bRet;
}

bool XMLParagraphOnlyPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *o3tl::doAccess<bool>(rValue) )
           rStrExpValue = GetXMLToken( XML_1 );
    else
           rStrExpValue = GetXMLToken( XML_NO_LIMIT );

    return true;
}

SvXMLEnumMapEntry<WrapTextMode> const pXML_Wrap_Enum[] =
{
    { XML_NONE,         WrapTextMode_NONE },
    { XML_RUN_THROUGH,  WrapTextMode_THROUGH },
    { XML_PARALLEL,     WrapTextMode_PARALLEL },
    { XML_DYNAMIC,      WrapTextMode_DYNAMIC },
    { XML_LEFT,         WrapTextMode_LEFT },
    { XML_RIGHT,        WrapTextMode_RIGHT },
    { XML_TOKEN_INVALID, WrapTextMode(0) }
};

class XMLWrapPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLWrapPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    WrapTextMode nWrap;
    bool bRet = SvXMLUnitConverter::convertEnum( nWrap, rStrImpValue,
                                                pXML_Wrap_Enum );

    if( bRet )
        rValue <<= nWrap;

    return bRet;
}

bool XMLWrapPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;
    WrapTextMode eVal;

    rValue >>= eVal;

    bool bRet = SvXMLUnitConverter::convertEnum( aOut, eVal, pXML_Wrap_Enum, XML_NONE );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

class XMLFrameProtectPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sVal;
public:
    explicit XMLFrameProtectPropHdl_Impl( enum XMLTokenEnum eVal ) :
           sVal( GetXMLToken(eVal) ) {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLFrameProtectPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    bool bVal = false;
    if( ! IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        bRet = false;
        SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
        OUString aToken;
        while( aTokenEnum.getNextToken( aToken ) )
        {
            bRet = true;
            if( aToken == sVal )
            {
                bVal = true;
                break;
            }
        }
    }

    if( bRet )
        rValue <<= bVal;

    return bRet;
}

bool XMLFrameProtectPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *o3tl::doAccess<bool>(rValue) )
    {
        if( rStrExpValue.isEmpty() ||
            IsXMLToken( rStrExpValue, XML_NONE ) )
        {
               rStrExpValue = sVal;
        }
        else
        {
            rStrExpValue = rStrExpValue + " " + sVal;
        }
    }
    else if( rStrExpValue.isEmpty() )
    {
           rStrExpValue = GetXMLToken( XML_NONE );
    }

    return true;
}

SvXMLEnumMapEntry<TextContentAnchorType> const pXML_Anchor_Enum[] =
{
    { XML_CHAR,          TextContentAnchorType_AT_CHARACTER },
    { XML_PAGE,          TextContentAnchorType_AT_PAGE },
    { XML_FRAME,         TextContentAnchorType_AT_FRAME },
    { XML_PARAGRAPH,     TextContentAnchorType_AT_PARAGRAPH },
    { XML_AS_CHAR,       TextContentAnchorType_AS_CHARACTER },
    { XML_TOKEN_INVALID, TextContentAnchorType(0) }
};

bool XMLAnchorTypePropHdl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    TextContentAnchorType nAnchor;
    bool bRet = SvXMLUnitConverter::convertEnum( nAnchor, rStrImpValue,
                                                pXML_Anchor_Enum );

    if( bRet )
        rValue <<= nAnchor;

    return bRet;
}

bool XMLAnchorTypePropHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;
    TextContentAnchorType eVal;

    rValue >>= eVal;

    bool bRet = SvXMLUnitConverter::convertEnum( aOut, eVal, pXML_Anchor_Enum, XML_PARAGRAPH );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

XMLAnchorTypePropHdl::~XMLAnchorTypePropHdl()
{
}

bool XMLAnchorTypePropHdl::convert( const OUString& rStrImpValue,
                 TextContentAnchorType& rType )
{
    TextContentAnchorType nAnchor;
    bool bRet = SvXMLUnitConverter::convertEnum( nAnchor, rStrImpValue,
                                                pXML_Anchor_Enum );
    if( bRet )
        rType = nAnchor;
    return bRet;
}

XMLTextColumnsPropertyHandler::~XMLTextColumnsPropertyHandler ()
{
}

bool XMLTextColumnsPropertyHandler::equals(
        const Any& r1,
        const Any& r2 ) const
{
    Reference < XTextColumns > xColumns1;
    r1 >>= xColumns1;

    Reference < XTextColumns > xColumns2;
    r2 >>= xColumns2;

    if( xColumns1->getColumnCount() != xColumns2->getColumnCount() ||
          xColumns1->getReferenceValue() != xColumns2->getReferenceValue() )
        return false;

    Sequence < TextColumn > aColumns1 = xColumns1->getColumns();
    Sequence < TextColumn > aColumns2 = xColumns2->getColumns();
    sal_Int32 nCount = aColumns1.getLength();
    if( aColumns2.getLength() != nCount )
        return false;

    const TextColumn *pColumns1 = aColumns1.getArray();
    const TextColumn *pColumns2 = aColumns2.getArray();

    while( nCount-- )
    {
        if( pColumns1->Width != pColumns2->Width ||
             pColumns1->LeftMargin != pColumns2->LeftMargin ||
             pColumns1->RightMargin != pColumns2->RightMargin )
            return false;

        pColumns1++;
        pColumns2++;
    }

    return true;
}

bool XMLTextColumnsPropertyHandler::importXML(
        const OUString&,
           Any&,
        const SvXMLUnitConverter& ) const
{
    SAL_WARN( "xmloff", "columns are an element import property" );
    return false;
}

bool XMLTextColumnsPropertyHandler::exportXML(
        OUString&,
        const Any&,
        const SvXMLUnitConverter& ) const
{
    SAL_WARN( "xmloff", "columns are an element export property" );
    return false;
}

class XMLHoriMirrorPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLHoriMirrorPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool nHoriMirror;
    bool bRet = SvXMLUnitConverter::convertEnum( nHoriMirror, rStrImpValue,
                                                pXML_HoriMirror_Enum );

    if( bRet )
    {
        rValue <<= nHoriMirror;
    }

    return bRet;
}

bool XMLHoriMirrorPropHdl_Impl::exportXML(
        OUString&,
        const Any&,
        const SvXMLUnitConverter& ) const
{
    SAL_WARN( "xmloff", "HorMirror property shouldn't be exported" );

    return false;
}

class XMLGrfMirrorPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sVal;
    bool const bHori;

public:
    XMLGrfMirrorPropHdl_Impl( enum XMLTokenEnum eVal, bool bH ) :
           sVal( GetXMLToken( eVal ) ),
        bHori( bH ) {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLGrfMirrorPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    bool bVal = false;
    if( ! IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        bRet = false;
        SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
        OUString aToken;
        while( aTokenEnum.getNextToken( aToken ) )
        {
            bRet = true;
            if( aToken == sVal ||
                 (bHori && IsXMLToken( aToken, XML_HORIZONTAL ) ) )
            {
                bVal = true;
                break;
            }
        }
    }

    if( bRet )
        rValue <<= bVal;

    return bRet;
}

bool XMLGrfMirrorPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *o3tl::doAccess<bool>(rValue) )
    {
        if( rStrExpValue.isEmpty() ||
            IsXMLToken( rStrExpValue, XML_NONE ) )
        {
               rStrExpValue = sVal;
        }
        else if( bHori &&
                 /* XML_HORIZONTAL_ON_LEFT_PAGES and XML_HORIZONTAL_ON_RIGHT_PAGES
                    are replaced by XML_HORIZONTAL_ON_EVEN and XML_HORIZONTAL_ON_ODD.
                    (#i49139#)
                 */
                 ( IsXMLToken( rStrExpValue, XML_HORIZONTAL_ON_EVEN ) ||
                   IsXMLToken( rStrExpValue, XML_HORIZONTAL_ON_ODD ) ))
        {
            rStrExpValue = GetXMLToken( XML_HORIZONTAL );
        }
        else
        {
            rStrExpValue = rStrExpValue + " " + sVal;
        }
    }
    else if( rStrExpValue.isEmpty() )
    {
           rStrExpValue = GetXMLToken( XML_NONE );
    }

    return true;
}

SvXMLEnumMapEntry<sal_uInt16> const pXML_Emphasize_Enum[] =
{
    { XML_NONE,     FontEmphasis::NONE },
    { XML_DOT,      FontEmphasis::DOT_ABOVE },
    { XML_CIRCLE,   FontEmphasis::CIRCLE_ABOVE },
    { XML_DISC,     FontEmphasis::DISK_ABOVE },
    { XML_ACCENT,   FontEmphasis::ACCENT_ABOVE },
    { XML_TOKEN_INVALID, 0 }
};
class XMLTextEmphasizePropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextEmphasizePropHdl_Impl() {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLTextEmphasizePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    sal_uInt16 nVal = FontEmphasis::NONE;
    bool bBelow = false;
    bool bHasPos = false, bHasType = false;
    OUString aToken;

    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    while( aTokenEnum.getNextToken( aToken ) )
    {
        if( !bHasPos && IsXMLToken( aToken, XML_ABOVE ) )
        {
            bBelow = false;
            bHasPos = true;
        }
        else if( !bHasPos && IsXMLToken( aToken, XML_BELOW ) )
        {
            bBelow = true;
            bHasPos = true;
        }
        else if( !bHasType &&
                  SvXMLUnitConverter::convertEnum( nVal, aToken,
                                             pXML_Emphasize_Enum ))
        {
            bHasType = true;
        }
        else
        {
            bRet = false;
            break;
        }
    }

    if( bRet )
    {
        if( FontEmphasis::NONE != nVal && bBelow )
            nVal += 10;
        rValue <<= static_cast<sal_Int16>(nVal);
    }

    return bRet;
}

bool XMLTextEmphasizePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut( 15 );
    bool bRet = true;
    sal_uInt16 nType = sal_uInt16();
    if( rValue >>= nType )
    {
        bool bBelow = false;
        if( nType > 10 )
        {
            bBelow = true;
            nType -= 10;
        }
        bRet = SvXMLUnitConverter::convertEnum( aOut, nType,
                                           pXML_Emphasize_Enum,
                                           XML_DOT );
        if( bRet )
        {
            if( nType != 0 )
            {
                enum XMLTokenEnum ePos = bBelow ? XML_BELOW : XML_ABOVE;
                aOut.append( ' ' );
                aOut.append( GetXMLToken(ePos) );
            }
            rStrExpValue = aOut.makeStringAndClear();
        }
    }

    return bRet;
}

class XMLTextCombineCharPropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextCombineCharPropHdl_Impl() {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLTextCombineCharPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( !rStrImpValue.isEmpty() )
        rValue <<= rStrImpValue.copy( 0, 1 );
    else
        rValue <<= rStrImpValue;

    return true;
}

bool XMLTextCombineCharPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    rValue >>= rStrExpValue;

    // #i114107# attribute of type "character": export only if length is 1
    return (1 == rStrExpValue.getLength());
}

class XMLTextRelWidthHeightPropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextRelWidthHeightPropHdl_Impl() {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLTextRelWidthHeightPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue;
    bool const bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
    if( bRet )
        rValue <<= static_cast<sal_Int16>(nValue);

    return bRet;
}

bool XMLTextRelWidthHeightPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int16 nValue = sal_Int16();
    if( (rValue >>= nValue) && nValue > 0 )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

class XMLTextSyncWidthHeightPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sValue;

public:
    explicit XMLTextSyncWidthHeightPropHdl_Impl( enum XMLTokenEnum eValue ) :
           sValue( GetXMLToken(eValue) )    {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLTextSyncWidthHeightPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    rValue <<= (rStrImpValue == sValue);

    return true;
}

bool XMLTextSyncWidthHeightPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    if( *o3tl::doAccess<bool>(rValue) )
    {
        rStrExpValue = sValue;
        bRet = true;
    }

    return bRet;
}

class XMLTextRotationAnglePropHdl_Impl : public XMLPropertyHandler
{

public:
    XMLTextRotationAnglePropHdl_Impl()  {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLTextRotationAnglePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue;
    bool const bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
    if( bRet )
    {
        nValue = (nValue % 360 );
        if( nValue < 0 )
            nValue = 360 + nValue;
        sal_Int16 nAngle;
        if( nValue < 45 || nValue > 315 )
            nAngle = 0;
        else if( nValue < 180 )
            nAngle = 900;
        else /* if nValue <= 315 ) */
            nAngle = 2700;
        rValue <<= nAngle;
    }

    return bRet;
}

bool XMLTextRotationAnglePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int16 nAngle = sal_Int16();
    bool bRet = ( rValue >>= nAngle );
    if( bRet )
    {
        rStrExpValue = OUString::number( nAngle / 10 );
    }
    OSL_ENSURE( bRet, "illegal rotation angle" );

    return bRet;
}

class XMLNumber8OneBasedHdl : public XMLPropertyHandler
{

public:
    XMLNumber8OneBasedHdl() {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& ) const override;
};

bool XMLNumber8OneBasedHdl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    bool const bRet = ::sax::Converter::convertNumber(nValue, rStrImpValue);
    if( bRet )
        rValue <<= static_cast<sal_Int8>( nValue - 1 );
    return bRet;
}

bool XMLNumber8OneBasedHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int8 nValue = sal_Int8();
    bool bRet = ( rValue >>= nValue );
    if( bRet )
    {
        rStrExpValue = OUString::number( nValue + 1 );
    }
    return bRet;
}

class XMLGraphicPropertyHandler : public XMLPropertyHandler
{
public:
    XMLGraphicPropertyHandler() {}

    virtual bool importXML(const OUString& , uno::Any& , const SvXMLUnitConverter& ) const override
    {
        SAL_WARN( "xmloff", "drop caps are an element import property" );
        return false;
    }

    virtual bool exportXML(OUString& , const uno::Any& , const SvXMLUnitConverter& ) const override
    {
        SAL_WARN( "xmloff", "drop caps are an element import property" );
        return false;
    }

    virtual bool equals(const css::uno::Any& rAny1, const css::uno::Any& rAny2) const override;
};

bool XMLGraphicPropertyHandler::equals(const Any& rAny1, const Any& rAny2) const
{
    uno::Reference<graphic::XGraphic> xGraphic1;
    uno::Reference<graphic::XGraphic> xGraphic2;
    rAny1 >>= xGraphic1;
    rAny2 >>= xGraphic2;
    Graphic aGraphic1(xGraphic1);
    Graphic aGraphic2(xGraphic2);

    return aGraphic1 == aGraphic2;
}

static const XMLPropertyHandler *GetPropertyHandler
    ( sal_Int32 nType )
{
    const XMLPropertyHandler* pHdl = nullptr;
    switch( nType )
    {
    case XML_TYPE_TEXT_DROPCAP:
        pHdl = new XMLDropCapPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_WRAP:
        pHdl = new XMLWrapPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_PARAGRAPH_ONLY:
        pHdl = new XMLParagraphOnlyPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_WRAP_OUTSIDE:
        pHdl = new XMLContourModePropHdl_Impl;
        break;
    case XML_TYPE_TEXT_OPAQUE:
        pHdl = new XMLOpaquePropHdl_Impl;
        break;
    case XML_TYPE_TEXT_PROTECT_CONTENT:
        pHdl = new XMLFrameProtectPropHdl_Impl( XML_CONTENT );
        break;
    case XML_TYPE_TEXT_PROTECT_SIZE:
        pHdl = new XMLFrameProtectPropHdl_Impl( XML_SIZE );
        break;
    case XML_TYPE_TEXT_PROTECT_POSITION:
        pHdl = new XMLFrameProtectPropHdl_Impl( XML_POSITION );
        break;
    case XML_TYPE_TEXT_ANCHOR_TYPE:
        pHdl = new XMLAnchorTypePropHdl;
        break;
    case XML_TYPE_TEXT_COLUMNS:
        pHdl = new XMLTextColumnsPropertyHandler;
        break;
    case XML_TYPE_TEXT_HORIZONTAL_POS:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriPos_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriPosMirrored_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_REL:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriRel_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_REL_FRAME:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriRelFrame_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_MIRROR:
        pHdl = new XMLHoriMirrorPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_VERTICAL_POS_AT_CHAR:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertPosAtChar_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRel_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL_PAGE:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRelPage_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL_FRAME:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRelFrame_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRelAsChar_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_MIRROR_VERTICAL:
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_VERTICAL, false );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT:
        // XML_HORIZONTAL_ON_LEFT_PAGES is replaced by XML_HORIZONTAL_ON_EVEN. (#i49139#)
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_HORIZONTAL_ON_EVEN, true );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT:
        // XML_HORIZONTAL_ON_RIGHT_PAGES is replaced by XML_HORIZONTAL_ON_ODD. (#i49139#)
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_HORIZONTAL_ON_ODD, true );
        break;
    case XML_TYPE_TEXT_CLIP:
        pHdl = new XMLClipPropertyHandler( false );
        break;
    case XML_TYPE_TEXT_CLIP11:
        pHdl = new XMLClipPropertyHandler( true );
        break;
    case XML_TYPE_TEXT_EMPHASIZE:
        pHdl = new XMLTextEmphasizePropHdl_Impl;
        break;
    case XML_TYPE_TEXT_COMBINE:
        pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_LINES ),
                                            GetXMLToken( XML_NONE ) );
        break;
    case XML_TYPE_TEXT_COMBINE_CHARACTERS:
        pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_LETTERS ),
                                            GetXMLToken( XML_NONE ) );
        break;
    case XML_TYPE_TEXT_COMBINECHAR:
        pHdl = new XMLTextCombineCharPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_AUTOSPACE:
        pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_IDEOGRAPH_ALPHA ),
                                            GetXMLToken( XML_NONE ) );
        break;
    case XML_TYPE_TEXT_PUNCTUATION_WRAP:
        pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_HANGING ),
                                            GetXMLToken( XML_SIMPLE ) );
        break;
    case XML_TYPE_TEXT_LINE_BREAK:
        pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_STRICT ),
                                            GetXMLToken( XML_NORMAL ) );
        break;
    case XML_TYPE_TEXT_REL_WIDTH_HEIGHT:
        pHdl = new XMLTextRelWidthHeightPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT:
        pHdl = new XMLTextSyncWidthHeightPropHdl_Impl( XML_SCALE );
        break;
    case XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT_MIN:
        pHdl = new XMLTextSyncWidthHeightPropHdl_Impl( XML_SCALE_MIN );
        break;
    case XML_TYPE_TEXT_RUBY_ADJUST:
        pHdl = new XMLConstantsPropertyHandler( pXML_RubyAdjust_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_FONT_RELIEF:
        pHdl = new XMLConstantsPropertyHandler( pXML_FontRelief_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_ROTATION_ANGLE:
        pHdl = new XMLTextRotationAnglePropHdl_Impl;
        break;
    case XML_TYPE_TEXT_ROTATION_SCALE:
        pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken( XML_FIXED ),
                                            GetXMLToken( XML_LINE_HEIGHT ) );
        break;
    case XML_TYPE_TEXT_VERTICAL_ALIGN:
        pHdl = new XMLConstantsPropertyHandler( pXML_ParaVerticalAlign_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_RUBY_POSITION:
        pHdl = new XMLConstantsPropertyHandler( pXML_RubyPosition_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_RUBY_IS_ABOVE:
        pHdl = new XMLNamedBoolPropertyHdl(::xmloff::token::XML_ABOVE, ::xmloff::token::XML_BELOW);
        break;
    // OD 2004-05-05 #i28701#
    case XML_TYPE_WRAP_INFLUENCE_ON_POSITION:
        pHdl = new XMLConstantsPropertyHandler( pXML_WrapInfluenceOnPosition_Enum,
                                                XML_TOKEN_INVALID );
        break;
    case XML_TYPE_BORDER_MODEL:
        pHdl = new XMLNamedBoolPropertyHdl( xmloff::token::XML_COLLAPSING,
                                            xmloff::token::XML_SEPARATING );
        break;
    case XML_TYPE_TEXT_LINE_MODE:
        pHdl = new XMLNamedBoolPropertyHdl(
                                    ::xmloff::token::XML_SKIP_WHITE_SPACE,
                                    ::xmloff::token::XML_CONTINUOUS);
        break;
    case XML_TYPE_TEXT_KEEP:
        pHdl = new XMLNamedBoolPropertyHdl(
                                    ::xmloff::token::XML_ALWAYS,
                                    ::xmloff::token::XML_AUTO);
        break;
    case XML_TYPE_TEXT_NKEEP:
        pHdl = new XMLNamedBoolPropertyHdl(
                                    ::xmloff::token::XML_AUTO,
                                    ::xmloff::token::XML_ALWAYS);
        break;
    case XML_TYPE_TEXT_NUMBER8_ONE_BASED:
        pHdl = new XMLNumber8OneBasedHdl();
        break;
    case XML_TYPE_VERTICAL_ALIGN:
        pHdl = new XMLConstantsPropertyHandler( pXML_VerticalAlign_Enum, XML_TOKEN_INVALID );
        break;

    case XML_SW_TYPE_FILLSTYLE:
        pHdl = new XMLEnumPropertyHdl( aXML_FillStyle_EnumMap);
        break;
    case XML_SW_TYPE_FILLBITMAPSIZE:
        pHdl = new XMLFillBitmapSizePropertyHandler();
        break;
    case XML_SW_TYPE_LOGICAL_SIZE:
        pHdl = new XMLBitmapLogicalSizePropertyHandler();
        break;
    case XML_SW_TYPE_BITMAP_REFPOINT:
        pHdl = new XMLEnumPropertyHdl( aXML_RefPoint_EnumMap);
        break;
    case XML_SW_TYPE_BITMAP_MODE:
        pHdl = new XMLEnumPropertyHdl( aXML_BitmapMode_EnumMap);
        break;
    case XML_SW_TYPE_BITMAPREPOFFSETX:
    case XML_SW_TYPE_BITMAPREPOFFSETY:
        pHdl = new XMLBitmapRepeatOffsetPropertyHandler(XML_SW_TYPE_BITMAPREPOFFSETX == nType);
        break;
    case XML_TYPE_GRAPHIC:
        pHdl = new XMLGraphicPropertyHandler;
        break;
    default:
    {
        OSL_ENSURE(false, "XMLPropertyHandler missing (!)");
        break;
    }
    }

    return pHdl;
}

XMLTextPropertyHandlerFactory::XMLTextPropertyHandlerFactory() :
    XMLPropertyHandlerFactory()
{
}

const XMLPropertyHandler *XMLTextPropertyHandlerFactory::GetPropertyHandler(
        sal_Int32 nType ) const
{
    const XMLPropertyHandler *pHdl =
        XMLPropertyHandlerFactory::GetPropertyHandler( nType );

    if( !pHdl )
    {
        const XMLPropertyHandler *pNewHdl = ::GetPropertyHandler( nType );

        if( pNewHdl )
            PutHdlCache( nType, pNewHdl );

        pHdl = pNewHdl;
    }

    return pHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
