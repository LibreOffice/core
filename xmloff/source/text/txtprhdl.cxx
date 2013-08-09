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

#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
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
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include "XMLAnchorTypePropHdl.hxx"
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include "XMLClipPropertyHandler.hxx"
#include "XMLTextColumnsPropertyHandler.hxx"
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include "txtprhdl.hxx"
// OD 2004-05-05 #i28701#
#include <com/sun/star/text/WrapInfluenceOnPosition.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

SvXMLEnumMapEntry const pXML_HoriPos_Enum[] =
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

SvXMLEnumMapEntry const pXML_HoriPosMirrored_Enum[] =
{
    { XML_FROM_INSIDE,      HoriOrientation::NONE   },
    { XML_INSIDE,           HoriOrientation::LEFT   },
    { XML_CENTER,           HoriOrientation::CENTER },
    { XML_OUTSIDE,          HoriOrientation::RIGHT  },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_HoriRel_Enum[] =
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

SvXMLEnumMapEntry const pXML_HoriRelFrame_Enum[] =
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

SvXMLEnumMapEntry const pXML_HoriMirror_Enum[] =
{
    { XML_FROM_LEFT,        sal_False   },
    { XML_FROM_INSIDE,      sal_True    },
    { XML_LEFT,             sal_False   },
    { XML_INSIDE,           sal_True    },
    { XML_CENTER,           sal_False   },
    { XML_RIGHT,            sal_False   },
    { XML_OUTSIDE,          sal_True    },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_VertPos_Enum[] =
{
    { XML_FROM_TOP,         VertOrientation::NONE       },
    { XML_TOP,              VertOrientation::TOP        },
    { XML_TOP,              VertOrientation::CHAR_TOP   },  // export only
    { XML_TOP,              VertOrientation::LINE_TOP   },  // export only
    { XML_MIDDLE,           VertOrientation::CENTER     },
    { XML_MIDDLE,           VertOrientation::CHAR_CENTER    },  // export only
    { XML_MIDDLE,           VertOrientation::LINE_CENTER    },  // export only
    { XML_BOTTOM,           VertOrientation::BOTTOM     },
    { XML_BOTTOM,           VertOrientation::CHAR_BOTTOM    },  // export only
    { XML_BOTTOM,           VertOrientation::LINE_BOTTOM    },  // export only
    { XML_BELOW,            VertOrientation::CHAR_BOTTOM    },  // import only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_VertPosAtChar_Enum[] =
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

SvXMLEnumMapEntry const pXML_VertRel_Enum[] =
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

SvXMLEnumMapEntry const pXML_VertRelPage_Enum[] =
{
    { XML_PAGE,         RelOrientation::FRAME   },
    { XML_PAGE_CONTENT, RelOrientation::PRINT_AREA  },
    { XML_PAGE,         RelOrientation::PAGE_FRAME  },
    { XML_PAGE_CONTENT, RelOrientation::PAGE_PRINT_AREA },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_VertRelFrame_Enum[] =
{
    { XML_FRAME,            RelOrientation::FRAME   },
    { XML_FRAME_CONTENT,    RelOrientation::PRINT_AREA  },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_VertRelAsChar_Enum[] =
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

SvXMLEnumMapEntry const pXML_RubyAdjust_Enum[] =
{
    { XML_LEFT,                 RubyAdjust_LEFT },
    { XML_CENTER,               RubyAdjust_CENTER },
    { XML_RIGHT,                RubyAdjust_RIGHT },
    { XML_DISTRIBUTE_LETTER,    RubyAdjust_BLOCK },
    { XML_DISTRIBUTE_SPACE,     RubyAdjust_INDENT_BLOCK },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_FontRelief_Enum[] =
{
    { XML_NONE,             FontRelief::NONE        },
    { XML_ENGRAVED,         FontRelief::ENGRAVED    },
    { XML_EMBOSSED,         FontRelief::EMBOSSED    },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_VerticalAlign_Enum[] =
{
    { XML_TOP,          ParagraphVertAlign::TOP     },
    { XML_MIDDLE,       ParagraphVertAlign::CENTER  },
    { XML_BOTTOM,       ParagraphVertAlign::BOTTOM  },
    { XML_BASELINE,     ParagraphVertAlign::BASELINE    },
    { XML_AUTO,         ParagraphVertAlign::AUTOMATIC   },
    { XML_TOKEN_INVALID, 0 }
};

// OD 2004-05-05 #i28701#
SvXMLEnumMapEntry const pXML_WrapInfluenceOnPosition_Enum[] =
{
    // Tokens have been renamed and <XML_ITERATIVE> has been added (#i35017#)
    { XML_ONCE_SUCCESSIVE, WrapInfluenceOnPosition::ONCE_SUCCESSIVE },
    { XML_ONCE_CONCURRENT, WrapInfluenceOnPosition::ONCE_CONCURRENT },
    { XML_ITERATIVE,       WrapInfluenceOnPosition::ITERATIVE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_FillStyle_Enum[] =
{
    { XML_NONE,     drawing::FillStyle_NONE },
    { XML_SOLID,    drawing::FillStyle_SOLID },
    { XML_BITMAP,   drawing::FillStyle_BITMAP },
    { XML_GRADIENT, drawing::FillStyle_GRADIENT },
    { XML_HATCH,    drawing::FillStyle_HATCH },
    { XML_TOKEN_INVALID, 0 }
};

class XMLDropCapPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLDropCapPropHdl_Impl ();

    virtual bool equals(
            const ::com::sun::star::uno::Any& r1,
            const ::com::sun::star::uno::Any& r2 ) const;

    /// TabStops will be imported/exported as XML-Elements. So the Import/Export-work must be done at another place.
    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

XMLDropCapPropHdl_Impl::~XMLDropCapPropHdl_Impl ()
{
}

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

sal_Bool XMLDropCapPropHdl_Impl::importXML(
        const OUString&,
           Any&,
        const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "drop caps are an element import property" );
    return sal_False;
}

sal_Bool XMLDropCapPropHdl_Impl::exportXML(
        OUString&,
        const Any&,
        const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "drop caps are an element export property" );
    return sal_False;
}

class XMLOpaquePropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLOpaquePropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLOpaquePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = sal_False;
    if( IsXMLToken( rStrImpValue, XML_FOREGROUND ) )
        bVal = sal_True;
    else if( !IsXMLToken( rStrImpValue, XML_BACKGROUND ) )
        bRet = sal_False;

    if( bRet )
        rValue.setValue( &bVal, ::getBooleanCppuType() );

    return bRet;
}

sal_Bool XMLOpaquePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *(sal_Bool *)rValue.getValue() )
           rStrExpValue = GetXMLToken( XML_FOREGROUND );
    else
           rStrExpValue = GetXMLToken( XML_BACKGROUND );

    return sal_True;
}

XMLOpaquePropHdl_Impl::~XMLOpaquePropHdl_Impl ()
{
}

class XMLContourModePropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLContourModePropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLContourModePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = sal_False;
    if( IsXMLToken( rStrImpValue, XML_OUTSIDE ) )
        bVal = sal_True;
    else if( ! IsXMLToken( rStrImpValue, XML_FULL ) )
        bRet = sal_False;

    if( bRet )
        rValue.setValue( &bVal, ::getBooleanCppuType() );

    return bRet;
}

sal_Bool XMLContourModePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *(sal_Bool *)rValue.getValue() )
           rStrExpValue = GetXMLToken( XML_OUTSIDE );
    else
           rStrExpValue = GetXMLToken( XML_FULL );

    return sal_True;
}

XMLContourModePropHdl_Impl::~XMLContourModePropHdl_Impl()
{
}

class XMLParagraphOnlyPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLParagraphOnlyPropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLParagraphOnlyPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = sal_False;

    if( ! IsXMLToken( rStrImpValue, XML_NO_LIMIT ) )
    {
        sal_Int32 nValue = 0;
        bRet = ::sax::Converter::convertNumber( nValue, rStrImpValue );
        bVal = 1 == nValue;
    }

    if( bRet )
        rValue.setValue( &bVal, ::getBooleanCppuType() );

    return bRet;
}

sal_Bool XMLParagraphOnlyPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *(sal_Bool *)rValue.getValue() )
           rStrExpValue = GetXMLToken( XML_1 );
    else
           rStrExpValue = GetXMLToken( XML_NO_LIMIT );

    return sal_True;
}

XMLParagraphOnlyPropHdl_Impl::~XMLParagraphOnlyPropHdl_Impl()
{
}

SvXMLEnumMapEntry const pXML_Wrap_Enum[] =
{
    { XML_NONE,         WrapTextMode_NONE },
    { XML_RUN_THROUGH,  WrapTextMode_THROUGHT },
    { XML_PARALLEL,     WrapTextMode_PARALLEL },
    { XML_DYNAMIC,      WrapTextMode_DYNAMIC },
    { XML_LEFT,         WrapTextMode_LEFT },
    { XML_RIGHT,        WrapTextMode_RIGHT },
    { XML_TOKEN_INVALID, 0 }
};

class XMLWrapPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLWrapPropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLWrapPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_uInt16 nWrap;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nWrap, rStrImpValue,
                                                pXML_Wrap_Enum );

    if( bRet )
        rValue <<= (WrapTextMode)nWrap;

    return bRet;
}

sal_Bool XMLWrapPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;
    WrapTextMode eVal;

    rValue >>= eVal;

    sal_Bool bRet = SvXMLUnitConverter::convertEnum( aOut, eVal, pXML_Wrap_Enum, XML_NONE );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

XMLWrapPropHdl_Impl::~XMLWrapPropHdl_Impl ()
{
}

class XMLFrameProtectPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sVal;
public:
    XMLFrameProtectPropHdl_Impl( enum XMLTokenEnum eVal ) :
           sVal( GetXMLToken(eVal) ) {}
    virtual ~XMLFrameProtectPropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLFrameProtectPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = sal_False;
    if( ! IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        bRet = sal_False;
        SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
        OUString aToken;
        while( aTokenEnum.getNextToken( aToken ) )
        {
            bRet = sal_True;
            if( aToken == sVal )
            {
                bVal = sal_True;
                break;
            }
        }
    }

    if( bRet )
        rValue.setValue( &bVal, ::getBooleanCppuType() );

    return bRet;
}

sal_Bool XMLFrameProtectPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *(sal_Bool *)rValue.getValue() )
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

    return sal_True;
}

XMLFrameProtectPropHdl_Impl::~XMLFrameProtectPropHdl_Impl()
{
}

SvXMLEnumMapEntry const pXML_Anchor_Enum[] =
{
    { XML_CHAR,         TextContentAnchorType_AT_CHARACTER },
    { XML_PAGE,         TextContentAnchorType_AT_PAGE },
    { XML_FRAME,        TextContentAnchorType_AT_FRAME },
    { XML_PARAGRAPH,        TextContentAnchorType_AT_PARAGRAPH },
    { XML_AS_CHAR,      TextContentAnchorType_AS_CHARACTER },
    { XML_TOKEN_INVALID, 0 }
};

sal_Bool XMLAnchorTypePropHdl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_uInt16 nAnchor;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nAnchor, rStrImpValue,
                                                pXML_Anchor_Enum );

    if( bRet )
        rValue <<= (TextContentAnchorType)nAnchor;

    return bRet;
}

sal_Bool XMLAnchorTypePropHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;
    TextContentAnchorType eVal;

    rValue >>= eVal;

    sal_Bool bRet = SvXMLUnitConverter::convertEnum( aOut, eVal, pXML_Anchor_Enum, XML_PARAGRAPH );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

XMLAnchorTypePropHdl::~XMLAnchorTypePropHdl()
{
}

sal_Bool XMLAnchorTypePropHdl::convert( const OUString& rStrImpValue,
                 TextContentAnchorType& rType )
{
    sal_uInt16 nAnchor;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nAnchor, rStrImpValue,
                                                pXML_Anchor_Enum );
    if( bRet )
        rType = (TextContentAnchorType)nAnchor;
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
        return sal_False;

    Sequence < TextColumn > aColumns1 = xColumns1->getColumns();
    Sequence < TextColumn > aColumns2 = xColumns2->getColumns();
    sal_Int32 nCount = aColumns1.getLength();
    if( aColumns2.getLength() != nCount )
        return sal_False;

    const TextColumn *pColumns1 = aColumns1.getArray();
    const TextColumn *pColumns2 = aColumns2.getArray();

    while( nCount-- )
    {
        if( pColumns1->Width != pColumns2->Width ||
             pColumns1->LeftMargin != pColumns2->LeftMargin ||
             pColumns1->RightMargin != pColumns2->RightMargin )
            return sal_False;

        pColumns1++;
        pColumns2++;
    }

    return sal_True;
}

sal_Bool XMLTextColumnsPropertyHandler::importXML(
        const OUString&,
           Any&,
        const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "columns are an element import property" );
    return sal_False;
}

sal_Bool XMLTextColumnsPropertyHandler::exportXML(
        OUString&,
        const Any&,
        const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "columns are an element export property" );
    return sal_False;
}

class XMLHoriMirrorPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLHoriMirrorPropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLHoriMirrorPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_uInt16 nHoriMirror;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nHoriMirror, rStrImpValue,
                                                pXML_HoriMirror_Enum );

    if( bRet )
    {
        sal_Bool bTmp = nHoriMirror != 0;
        rValue.setValue( &bTmp, ::getBooleanCppuType() );
    }

    return bRet;
}

sal_Bool XMLHoriMirrorPropHdl_Impl::exportXML(
        OUString&,
        const Any&,
        const SvXMLUnitConverter& ) const
{
    DBG_ASSERT( !this, "HorMirror properyt shouldn't be exported" );

    return sal_False;
}

XMLHoriMirrorPropHdl_Impl::~XMLHoriMirrorPropHdl_Impl ()
{
}

class XMLGrfMirrorPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sVal;
    sal_Bool bHori;

public:
    XMLGrfMirrorPropHdl_Impl( enum XMLTokenEnum eVal, sal_Bool bH ) :
           sVal( GetXMLToken( eVal ) ),
        bHori( bH ) {}
    virtual ~XMLGrfMirrorPropHdl_Impl ();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLGrfMirrorPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = sal_False;
    if( ! IsXMLToken( rStrImpValue, XML_NONE ) )
    {
        bRet = sal_False;
        SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
        OUString aToken;
        while( aTokenEnum.getNextToken( aToken ) )
        {
            bRet = sal_True;
            if( aToken == sVal ||
                 (bHori && IsXMLToken( aToken, XML_HORIZONTAL ) ) )
            {
                bVal = sal_True;
                break;
            }
        }
    }

    if( bRet )
        rValue.setValue( &bVal, ::getBooleanCppuType() );

    return bRet;
}

sal_Bool XMLGrfMirrorPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( *(sal_Bool *)rValue.getValue() )
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

    return sal_True;
}

XMLGrfMirrorPropHdl_Impl::~XMLGrfMirrorPropHdl_Impl()
{
}

SvXMLEnumMapEntry const pXML_Emphasize_Enum[] =
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
    virtual ~XMLTextEmphasizePropHdl_Impl();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLTextEmphasizePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_True;
    sal_uInt16 nVal = FontEmphasis::NONE;
    sal_Bool bBelow = sal_False;
    sal_Bool bHasPos = sal_False, bHasType = sal_False;
    OUString aToken;

    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    while( aTokenEnum.getNextToken( aToken ) )
    {
        if( !bHasPos && IsXMLToken( aToken, XML_ABOVE ) )
        {
            bBelow = sal_False;
            bHasPos = sal_True;
        }
        else if( !bHasPos && IsXMLToken( aToken, XML_BELOW ) )
        {
            bBelow = sal_True;
            bHasPos = sal_True;
        }
        else if( !bHasType &&
                  SvXMLUnitConverter::convertEnum( nVal, aToken,
                                             pXML_Emphasize_Enum ))
        {
            bHasType = sal_True;
        }
        else
        {
            bRet = sal_False;
            break;
        }
    }

    if( bRet )
    {
        if( FontEmphasis::NONE != nVal && bBelow )
            nVal += 10;
        rValue <<= (sal_Int16)nVal;
    }

    return bRet;
}

sal_Bool XMLTextEmphasizePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut( 15 );
    sal_Bool bRet = sal_True;
    sal_Int16 nType = sal_Int16();
    if( rValue >>= nType )
    {
        sal_Bool bBelow = sal_False;
        if( nType > 10 )
        {
            bBelow = sal_True;
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
                aOut.append( (sal_Unicode)' ' );
                aOut.append( GetXMLToken(ePos) );
            }
            rStrExpValue = aOut.makeStringAndClear();
        }
    }

    return bRet;
}

XMLTextEmphasizePropHdl_Impl::~XMLTextEmphasizePropHdl_Impl()
{
}

class XMLTextCombineCharPropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextCombineCharPropHdl_Impl() {}
    virtual ~XMLTextCombineCharPropHdl_Impl();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLTextCombineCharPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( !rStrImpValue.isEmpty() )
        rValue <<= rStrImpValue.copy( 0, 1 );
    else
        rValue <<= rStrImpValue;

    return sal_True;
}

sal_Bool XMLTextCombineCharPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    rValue >>= rStrExpValue;

    // #i114107# attribute of type "character": export only if length is 1
    return (1 == rStrExpValue.getLength())  ? sal_True : sal_False;
}

XMLTextCombineCharPropHdl_Impl::~XMLTextCombineCharPropHdl_Impl()
{
}

class XMLTextRelWidthHeightPropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextRelWidthHeightPropHdl_Impl() {}
    virtual ~XMLTextRelWidthHeightPropHdl_Impl();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLTextRelWidthHeightPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue;
    bool const bRet = ::sax::Converter::convertPercent( nValue, rStrImpValue );
    if( bRet )
        rValue <<= (sal_Int16)nValue;

    return bRet;
}

sal_Bool XMLTextRelWidthHeightPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();
    if( (rValue >>= nValue) && nValue > 0 )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

XMLTextRelWidthHeightPropHdl_Impl::~XMLTextRelWidthHeightPropHdl_Impl()
{
}

class XMLTextSyncWidthHeightPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sValue;

public:
    XMLTextSyncWidthHeightPropHdl_Impl( enum XMLTokenEnum eValue ) :
           sValue( GetXMLToken(eValue) )    {}
    virtual ~XMLTextSyncWidthHeightPropHdl_Impl();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLTextSyncWidthHeightPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bValue = (rStrImpValue == sValue );
    rValue.setValue( &bValue, ::getBooleanCppuType() );

    return sal_True;
}

sal_Bool XMLTextSyncWidthHeightPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    if( *(sal_Bool *)rValue.getValue() )
    {
        rStrExpValue = sValue;
        bRet = sal_True;
    }

    return bRet;
}

XMLTextSyncWidthHeightPropHdl_Impl::~XMLTextSyncWidthHeightPropHdl_Impl()
{
}

class XMLTextRotationAnglePropHdl_Impl : public XMLPropertyHandler
{

public:
    XMLTextRotationAnglePropHdl_Impl()  {}
    virtual ~XMLTextRotationAnglePropHdl_Impl();

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLTextRotationAnglePropHdl_Impl::importXML(
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
        else /* if nValalue <= 315 ) */
            nAngle = 2700;
        rValue <<= nAngle;
    }

    return bRet;
}

sal_Bool XMLTextRotationAnglePropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int16 nAngle = sal_Int16();
    sal_Bool bRet = ( rValue >>= nAngle );
    if( bRet )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertNumber( aOut, nAngle / 10 );
        rStrExpValue = aOut.makeStringAndClear();
    }
    OSL_ENSURE( bRet, "illegal rotation angle" );

    return bRet;
}

XMLTextRotationAnglePropHdl_Impl::~XMLTextRotationAnglePropHdl_Impl()
{
}
class XMLNumber8OneBasedHdl : public XMLPropertyHandler
{

public:
    XMLNumber8OneBasedHdl() {}
    virtual ~XMLNumber8OneBasedHdl() {};

    virtual sal_Bool importXML(
            const OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
    virtual sal_Bool exportXML(
            OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& ) const;
};

sal_Bool XMLNumber8OneBasedHdl::importXML(
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

sal_Bool XMLNumber8OneBasedHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Int8 nValue = sal_Int8();
    sal_Bool bRet = ( rValue >>= nValue );
    if( bRet )
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertNumber( aOut, nValue + 1 );
        rStrExpValue = aOut.makeStringAndClear();
    }
    return bRet;
}
class XMLTextPropertyHandlerFactory_Impl
{
public:
    const XMLPropertyHandler *GetPropertyHandler( sal_Int32 nType ) const;

    XMLTextPropertyHandlerFactory_Impl();
    ~XMLTextPropertyHandlerFactory_Impl();
};

const XMLPropertyHandler *XMLTextPropertyHandlerFactory_Impl::GetPropertyHandler
    ( sal_Int32 nType ) const
{
    const XMLPropertyHandler* pHdl = 0;
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
    case XML_TYPE_TEXT_VERTICAL_POS:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertPos_Enum, XML_TOKEN_INVALID );
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
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_VERTICAL, sal_False );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT:
        // XML_HORIZONTAL_ON_LEFT_PAGES is replaced by XML_HORIZONTAL_ON_EVEN. (#i49139#)
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_HORIZONTAL_ON_EVEN, sal_True );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT:
        // XML_HORIZONTAL_ON_RIGHT_PAGES is replaced by XML_HORIZONTAL_ON_ODD. (#i49139#)
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_HORIZONTAL_ON_ODD, sal_True );
        break;
    case XML_TYPE_TEXT_CLIP:
        pHdl = new XMLClipPropertyHandler( sal_False );
        break;
    case XML_TYPE_TEXT_CLIP11:
        pHdl = new XMLClipPropertyHandler( sal_True );
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
        pHdl = new XMLConstantsPropertyHandler( pXML_VerticalAlign_Enum, XML_TOKEN_INVALID );
        break;
    case XML_TYPE_TEXT_RUBY_POSITION:
        pHdl = new XMLNamedBoolPropertyHdl( ::xmloff::token::XML_ABOVE,
                                            ::xmloff::token::XML_BELOW );
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
    case XML_TYPE_FILLSTYLE:
        pHdl = new XMLConstantsPropertyHandler( pXML_FillStyle_Enum, XML_TOKEN_INVALID );
        break;
    }

    return pHdl;
}

XMLTextPropertyHandlerFactory_Impl::XMLTextPropertyHandlerFactory_Impl()
{
}

XMLTextPropertyHandlerFactory_Impl::~XMLTextPropertyHandlerFactory_Impl()
{
}

XMLTextPropertyHandlerFactory::XMLTextPropertyHandlerFactory() :
    XMLPropertyHandlerFactory(),
   pImpl( new XMLTextPropertyHandlerFactory_Impl )
{
}

XMLTextPropertyHandlerFactory::~XMLTextPropertyHandlerFactory()
{
    delete pImpl;
}

const XMLPropertyHandler *XMLTextPropertyHandlerFactory::GetPropertyHandler(
        sal_Int32 nType ) const
{
    const XMLPropertyHandler *pHdl =
        XMLPropertyHandlerFactory::GetPropertyHandler( nType );

    if( !pHdl )
    {
        const XMLPropertyHandler *pNewHdl = pImpl->GetPropertyHandler( nType );

        if( pNewHdl )
            PutHdlCache( nType, pNewHdl );

        pHdl = pNewHdl;
    }

    return pHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
