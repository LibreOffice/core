/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_DROPCAPFORMAT_HPP_
#include <com/sun/star/style/DropCapFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FONTRELIEF_HPP_
#include <com/sun/star/text/FontRelief.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RUBYADJUST_HPP_
#include <com/sun/star/text/RubyAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FONTEMPHASIS_HPP_
#include <com/sun/star/text/FontEmphasis.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PARAGRAPHVERTALIGN_HPP_
#include <com/sun/star/text/ParagraphVertAlign.hpp>
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLANCHORTYPEPROPHDL_HXX
#include "XMLAnchorTypePropHdl.hxx"
#endif
#ifndef _XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#include "XMLConstantsPropertyHandler.hxx"
#endif
#ifndef _XMLOFF_XMLCLIPPROPERTYHANDLER_HXX
#include "XMLClipPropertyHandler.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTCOLUMNSPPROPERTYHANDLER_HXX
#include "XMLTextColumnsPropertyHandler.hxx"
#endif
#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include "NamedBoolPropertyHdl.hxx"
#endif
#ifndef _XMLOFF_TXTPRHDL_HXX
#include "txtprhdl.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
//using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
//using namespace ::com::sun::star::container;
//using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;


#define CONSTASCII_USTRINGPARAM_CMP(s) s, sizeof(s)-1

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriPos_Enum[] =
{
    { XML_FROM_LEFT,		HoriOrientation::NONE	},
    { XML_FROM_INSIDE,		HoriOrientation::NONE	},	// import only
    { XML_LEFT,		    	HoriOrientation::LEFT	},
    { XML_INSIDE,			HoriOrientation::LEFT	},	// import only
    { XML_CENTER,			HoriOrientation::CENTER	},
    { XML_RIGHT,			HoriOrientation::RIGHT	},
    { XML_OUTSIDE,			HoriOrientation::RIGHT	},	// import only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriPosMirrored_Enum[] =
{
    { XML_FROM_INSIDE,		HoriOrientation::NONE	},
    { XML_INSIDE,			HoriOrientation::LEFT	},
    { XML_CENTER,			HoriOrientation::CENTER	},
    { XML_OUTSIDE,			HoriOrientation::RIGHT	},
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriRel_Enum[] =
{
    { XML_PARAGRAPH,			RelOrientation::FRAME	},
    { XML_PARAGRAPH_CONTENT,	RelOrientation::PRINT_AREA	},
    { XML_PAGE,				    RelOrientation::PAGE_FRAME	},
    { XML_PAGE_CONTENT,		    RelOrientation::PAGE_PRINT_AREA	},
    { XML_PARAGRAPH_START_MARGIN,	RelOrientation::FRAME_LEFT	},
    { XML_PARAGRAPH_END_MARGIN,	RelOrientation::FRAME_RIGHT	},
    { XML_PAGE_START_MARGIN,	RelOrientation::PAGE_LEFT	},
    { XML_PAGE_END_MARGIN,		RelOrientation::PAGE_RIGHT	},
    { XML_CHAR,				    RelOrientation::CHAR	},
    { XML_FRAME,				RelOrientation::FRAME	},		// import only
    { XML_FRAME_CONTENT,		RelOrientation::PRINT_AREA	},	// import only
    { XML_FRAME_START_MARGIN,	RelOrientation::FRAME_LEFT	},	// import only
    { XML_FRAME_END_MARGIN, 	RelOrientation::FRAME_RIGHT	},	// import only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriRelFrame_Enum[] =
{
    { XML_FRAME,				RelOrientation::FRAME	},
    { XML_FRAME_CONTENT,		RelOrientation::PRINT_AREA	},
    { XML_PAGE,				    RelOrientation::PAGE_FRAME	},
    { XML_PAGE_CONTENT,		    RelOrientation::PAGE_PRINT_AREA	},
    { XML_FRAME_START_MARGIN,	RelOrientation::FRAME_LEFT	},
    { XML_FRAME_END_MARGIN,	    RelOrientation::FRAME_RIGHT	},
    { XML_PAGE_START_MARGIN,	RelOrientation::PAGE_LEFT	},
    { XML_PAGE_END_MARGIN,		RelOrientation::PAGE_RIGHT	},
    { XML_CHAR,				    RelOrientation::CHAR	},
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriMirror_Enum[] =
{
    { XML_FROM_LEFT,		sal_False	},
    { XML_FROM_INSIDE,		sal_True	},
    { XML_LEFT,		    	sal_False	},
    { XML_INSIDE,			sal_True	},
    { XML_CENTER,			sal_False	},
    { XML_RIGHT,			sal_False	},
    { XML_OUTSIDE,			sal_True	},
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertPos_Enum[] =
{
    { XML_FROM_TOP,		    VertOrientation::NONE		},
    { XML_TOP,				VertOrientation::TOP		},
    { XML_TOP,				VertOrientation::CHAR_TOP	},	// export only
    { XML_TOP,				VertOrientation::LINE_TOP	},	// export only
    { XML_MIDDLE,			VertOrientation::CENTER		},
    { XML_MIDDLE,			VertOrientation::CHAR_CENTER	},	// export only
    { XML_MIDDLE,			VertOrientation::LINE_CENTER	},	// export only
    { XML_BOTTOM,			VertOrientation::BOTTOM		},
    { XML_BOTTOM,			VertOrientation::CHAR_BOTTOM	},	// export only
    { XML_BOTTOM,			VertOrientation::LINE_BOTTOM	},	// export only
    { XML_BELOW,			VertOrientation::CHAR_BOTTOM	},	// import only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertPosAtChar_Enum[] =
{
    { XML_FROM_TOP,		    VertOrientation::NONE		},
    { XML_TOP,				VertOrientation::TOP		},
    { XML_TOP,				VertOrientation::CHAR_TOP	},	// export only
    { XML_TOP,				VertOrientation::LINE_TOP	},	// export only
    { XML_MIDDLE,			VertOrientation::CENTER		},
    { XML_MIDDLE,			VertOrientation::CHAR_CENTER	},	// export only
    { XML_MIDDLE,			VertOrientation::LINE_CENTER	},	// export only
    { XML_BOTTOM,			VertOrientation::BOTTOM		},
    { XML_BELOW,			VertOrientation::CHAR_BOTTOM	},	// export only
    { XML_BOTTOM,			VertOrientation::LINE_BOTTOM	},	// export only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRel_Enum[] =
{
    { XML_PARAGRAPH,			RelOrientation::FRAME	},
    { XML_PARAGRAPH_CONTENT,	RelOrientation::PRINT_AREA	},
    { XML_CHAR,				    RelOrientation::CHAR	},
    { XML_PAGE,			    	RelOrientation::FRAME	},		// import only
    { XML_PAGE_CONTENT, 		RelOrientation::PRINT_AREA	},	// import only
    { XML_FRAME,				RelOrientation::FRAME	},		// import only
    { XML_FRAME_CONTENT,		RelOrientation::PRINT_AREA	},	// import only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRelPage_Enum[] =
{
    { XML_PAGE,			RelOrientation::FRAME	},
    { XML_PAGE_CONTENT,	RelOrientation::PRINT_AREA	},
    { XML_PAGE,			RelOrientation::PAGE_FRAME	},
    { XML_PAGE_CONTENT,	RelOrientation::PAGE_PRINT_AREA	},
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRelFrame_Enum[] =
{
    { XML_FRAME,			RelOrientation::FRAME	},
    { XML_FRAME_CONTENT,	RelOrientation::PRINT_AREA	},
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRelAsChar_Enum[] =
{
    { XML_BASELINE,		VertOrientation::TOP		},
    { XML_BASELINE,		VertOrientation::CENTER		},	// export only
    { XML_BASELINE,		VertOrientation::BOTTOM		},	// export only
    { XML_TEXT,			VertOrientation::CHAR_TOP	},
    { XML_TEXT,			VertOrientation::CHAR_CENTER	},	// export only
    { XML_TEXT,			VertOrientation::CHAR_BOTTOM	},	// export only
    { XML_LINE,			VertOrientation::LINE_TOP	},
    { XML_LINE,			VertOrientation::LINE_CENTER	},	// export only
    { XML_LINE,			VertOrientation::LINE_BOTTOM	},	// export only
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_RubyAdjust_Enum[] =
{
    { XML_LEFT,				    RubyAdjust_LEFT },
    { XML_CENTER,				RubyAdjust_CENTER },
    { XML_RIGHT,				RubyAdjust_RIGHT },
    { XML_DISTRIBUTE_LETTER,	RubyAdjust_BLOCK },
    { XML_DISTRIBUTE_SPACE,	    RubyAdjust_INDENT_BLOCK },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_FontRelief_Enum[] =
{
    { XML_NONE,				FontRelief::NONE		},
    { XML_ENGRAVED,			FontRelief::ENGRAVED	},
    { XML_EMBOSSED,			FontRelief::EMBOSSED	},
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VerticalAlign_Enum[] =
{
    { XML_TOP,			ParagraphVertAlign::TOP		},
    { XML_MIDDLE,		ParagraphVertAlign::CENTER	},
    { XML_BOTTOM,		ParagraphVertAlign::BOTTOM	},
    { XML_BASELINE,	    ParagraphVertAlign::BASELINE	},
    { XML_AUTO,		    ParagraphVertAlign::AUTOMATIC	},
    { XML_TOKEN_INVALID, 0 }
};

// ---------------------------------------------------------------------------

class XMLDropCapPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLDropCapPropHdl_Impl ();

    virtual sal_Bool equals(
            const ::com::sun::star::uno::Any& r1,
            const ::com::sun::star::uno::Any& r2 ) const;

    /// TabStops will be imported/exported as XML-Elements. So the Import/Export-work must be done at another place.
    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

XMLDropCapPropHdl_Impl::~XMLDropCapPropHdl_Impl ()
{
}

sal_Bool XMLDropCapPropHdl_Impl::equals(
        const Any& r1,
        const Any& r2 ) const
{
    DropCapFormat aFormat1, aFormat2;
    r1 >>= aFormat1;
    r2 >>= aFormat2;

    return	(aFormat1.Lines <=1 && aFormat2.Lines <=1) ||
            (aFormat1.Lines == aFormat2.Lines &&
             aFormat1.Count == aFormat2.Count &&
             aFormat1.Distance == aFormat2.Distance);
}

sal_Bool XMLDropCapPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "drop caps are an element import property" );
    return sal_False;
}

sal_Bool XMLDropCapPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "drop caps are an element export property" );
    return sal_False;
}

// ---------------------------------------------------------------------------

class XMLOpaquePropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLOpaquePropHdl_Impl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLOpaquePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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
        const SvXMLUnitConverter& rUnitConverter ) const
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

// ---------------------------------------------------------------------------

class XMLContourModePropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLContourModePropHdl_Impl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLContourModePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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
        const SvXMLUnitConverter& rUnitConverter ) const
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

// ---------------------------------------------------------------------------

class XMLParagraphOnlyPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLParagraphOnlyPropHdl_Impl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLParagraphOnlyPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_True;
    sal_Bool bVal = sal_False;

    if( ! IsXMLToken( rStrImpValue, XML_NO_LIMIT ) )
    {
        sal_Int32 nValue = 0;
        bRet = rUnitConverter.convertNumber( nValue, rStrImpValue );
        bVal = 1 == nValue;
    }

    if( bRet )
        rValue.setValue( &bVal, ::getBooleanCppuType() );

    return bRet;
}

sal_Bool XMLParagraphOnlyPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_Wrap_Enum[] =
{
    { XML_NONE,		    WrapTextMode_NONE },
    { XML_RUN_THROUGH,	WrapTextMode_THROUGHT },
    { XML_PARALLEL,	    WrapTextMode_PARALLEL },
    { XML_DYNAMIC,		WrapTextMode_DYNAMIC },
    { XML_LEFT,		    WrapTextMode_LEFT },
    { XML_RIGHT,		WrapTextMode_RIGHT },
    { XML_TOKEN_INVALID, 0 }
};

class XMLWrapPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLWrapPropHdl_Impl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLWrapPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_uInt16 nWrap;
    sal_Bool bRet = rUnitConverter.convertEnum( nWrap, rStrImpValue,
                                                pXML_Wrap_Enum );

    if( bRet )
        rValue <<= (WrapTextMode)nWrap;

    return bRet;
}

sal_Bool XMLWrapPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;
    WrapTextMode eVal;

    rValue >>= eVal;

    sal_Bool bRet = rUnitConverter.convertEnum( aOut, eVal, pXML_Wrap_Enum, XML_NONE );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

XMLWrapPropHdl_Impl::~XMLWrapPropHdl_Impl ()
{
}

// ---------------------------------------------------------------------------

class XMLFrameProtectPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sVal;
public:
    XMLFrameProtectPropHdl_Impl( enum XMLTokenEnum eVal ) :
           sVal( GetXMLToken(eVal) ) {}
    virtual ~XMLFrameProtectPropHdl_Impl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLFrameProtectPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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
        const SvXMLUnitConverter& rUnitConverter ) const
{
    if( *(sal_Bool *)rValue.getValue() )
    {
        if( !rStrExpValue.getLength() ||
            IsXMLToken( rStrExpValue, XML_NONE ) )
        {
               rStrExpValue = sVal;
        }
        else
        {
            OUStringBuffer aOut( rStrExpValue.getLength() + 1 +
                                 sVal.getLength() );
            aOut.append( rStrExpValue );
            aOut.append( (sal_Unicode)' ' );
            aOut.append( sVal );
            rStrExpValue = aOut.makeStringAndClear();
        }
    }
    else if( !rStrExpValue.getLength() )
    {
           rStrExpValue = GetXMLToken( XML_NONE );
    }

    return sal_True;
}

XMLFrameProtectPropHdl_Impl::~XMLFrameProtectPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_Anchor_Enum[] =
{
    { XML_CHAR,		    TextContentAnchorType_AT_CHARACTER },
    { XML_PAGE,	        TextContentAnchorType_AT_PAGE },
    { XML_FRAME,	    TextContentAnchorType_AT_FRAME },
    { XML_PARAGRAPH,		TextContentAnchorType_AT_PARAGRAPH },
    { XML_AS_CHAR,	    TextContentAnchorType_AS_CHARACTER },
    { XML_TOKEN_INVALID, 0 }
};


sal_Bool XMLAnchorTypePropHdl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_uInt16 nAnchor;
    sal_Bool bRet = rUnitConverter.convertEnum( nAnchor, rStrImpValue,
                                                pXML_Anchor_Enum );

    if( bRet )
        rValue <<= (TextContentAnchorType)nAnchor;

    return bRet;
}

sal_Bool XMLAnchorTypePropHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;
    TextContentAnchorType eVal;

    rValue >>= eVal;

    sal_Bool bRet = rUnitConverter.convertEnum( aOut, eVal, pXML_Anchor_Enum, XML_PARAGRAPH );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

XMLAnchorTypePropHdl::~XMLAnchorTypePropHdl()
{
}

sal_Bool XMLAnchorTypePropHdl::convert( const OUString& rStrImpValue,
                 const SvXMLUnitConverter& rUnitConverter,
                 TextContentAnchorType& rType )
{
    sal_uInt16 nAnchor;
    sal_Bool bRet = rUnitConverter.convertEnum( nAnchor, rStrImpValue,
                                                pXML_Anchor_Enum );
    if( bRet )
        rType = (TextContentAnchorType)nAnchor;
    return bRet;
}

// ---------------------------------------------------------------------------


XMLTextColumnsPropertyHandler::~XMLTextColumnsPropertyHandler ()
{
}

sal_Bool XMLTextColumnsPropertyHandler::equals(
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
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "columns are an element import property" );
    return sal_False;
}

sal_Bool XMLTextColumnsPropertyHandler::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "columns are an element export property" );
    return sal_False;
}

// ---------------------------------------------------------------------------

class XMLHoriMirrorPropHdl_Impl : public XMLPropertyHandler
{
public:
    virtual ~XMLHoriMirrorPropHdl_Impl ();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLHoriMirrorPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_uInt16 nHoriMirror;
    sal_Bool bRet = rUnitConverter.convertEnum( nHoriMirror, rStrImpValue,
                                                pXML_HoriMirror_Enum );

    if( bRet )
    {
        sal_Bool bTmp = nHoriMirror != 0;
        rValue.setValue( &bTmp, ::getBooleanCppuType() );
    }

    return bRet;
}

sal_Bool XMLHoriMirrorPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    DBG_ASSERT( !this, "HorMirror properyt shouldn't be exported" );

    return sal_False;
}

XMLHoriMirrorPropHdl_Impl::~XMLHoriMirrorPropHdl_Impl ()
{
}

// ---------------------------------------------------------------------------

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
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLGrfMirrorPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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
        const SvXMLUnitConverter& rUnitConverter ) const
{
    if( *(sal_Bool *)rValue.getValue() )
    {
        if( !rStrExpValue.getLength() ||
            IsXMLToken( rStrExpValue, XML_NONE ) )
        {
               rStrExpValue = sVal;
        }
        else if( bHori &&
                 ( IsXMLToken( rStrExpValue, XML_HORIZONTAL_ON_LEFT_PAGES ) ||
                   IsXMLToken( rStrExpValue, XML_HORIZONTAL_ON_RIGHT_PAGES ) ))
        {
            rStrExpValue = GetXMLToken( XML_HORIZONTAL );
        }
        else
        {
            OUStringBuffer aOut( rStrExpValue.getLength() + 1 +
                                 sVal.getLength() );
            aOut.append( rStrExpValue );
            aOut.append( (sal_Unicode)' ' );
            aOut.append( sVal );
            rStrExpValue = aOut.makeStringAndClear();
        }
    }
    else if( !rStrExpValue.getLength() )
    {
           rStrExpValue = GetXMLToken( XML_NONE );
    }

    return sal_True;
}

XMLGrfMirrorPropHdl_Impl::~XMLGrfMirrorPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_Emphasize_Enum[] =
{
    { XML_NONE,	    FontEmphasis::NONE },
    { XML_DOT,		FontEmphasis::DOT_ABOVE },
    { XML_CIRCLE,	FontEmphasis::CIRCLE_ABOVE },
    { XML_DISC,	    FontEmphasis::DISK_ABOVE },
    { XML_ACCENT,	FontEmphasis::ACCENT_ABOVE },
    { XML_TOKEN_INVALID, 0 }
};
class XMLTextEmphasizePropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextEmphasizePropHdl_Impl() {}
    virtual ~XMLTextEmphasizePropHdl_Impl();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLTextEmphasizePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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
                  rUnitConverter.convertEnum( nVal, aToken,
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
        const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut( 15 );
    sal_Bool bRet = sal_True;
    sal_Int16 nType;
    if( rValue >>= nType )
    {
        sal_Bool bBelow = sal_False;
        if( nType > 10 )
        {
            bBelow = sal_True;
            nType -= 10;
        }
        bRet = rUnitConverter.convertEnum( aOut, nType,
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


// ---------------------------------------------------------------------------

class XMLTextCombineCharPropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextCombineCharPropHdl_Impl() {}
    virtual ~XMLTextCombineCharPropHdl_Impl();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLTextCombineCharPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    if( rStrImpValue.getLength() )
        rValue <<= rStrImpValue.copy( 0, 1 );
    else
        rValue <<= rStrImpValue;

    return sal_True;
}

sal_Bool XMLTextCombineCharPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    rValue >>= rStrExpValue;

    return sal_True;
}

XMLTextCombineCharPropHdl_Impl::~XMLTextCombineCharPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

class XMLTextRelWidthHeightPropHdl_Impl : public XMLPropertyHandler
{
public:
    XMLTextRelWidthHeightPropHdl_Impl() {}
    virtual ~XMLTextRelWidthHeightPropHdl_Impl();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLTextRelWidthHeightPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet;
    sal_Int32 nValue;
    bRet = rUnitConverter.convertPercent( nValue, rStrImpValue );
    if( bRet )
        rValue <<= (sal_Int16)nValue;

    return bRet;
}

sal_Bool XMLTextRelWidthHeightPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    if( (rValue >>= nValue) && nValue > 0 )
    {
        OUStringBuffer aOut;
         rUnitConverter.convertPercent( aOut, nValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

XMLTextRelWidthHeightPropHdl_Impl::~XMLTextRelWidthHeightPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

class XMLTextSyncWidthHeightPropHdl_Impl : public XMLPropertyHandler
{
    const OUString sValue;

public:
    XMLTextSyncWidthHeightPropHdl_Impl( enum XMLTokenEnum eValue ) :
           sValue( GetXMLToken(eValue) )	{}
    virtual ~XMLTextSyncWidthHeightPropHdl_Impl();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLTextSyncWidthHeightPropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bValue = (rStrImpValue == sValue );
    rValue.setValue( &bValue, ::getBooleanCppuType() );

    return sal_True;
}

sal_Bool XMLTextSyncWidthHeightPropHdl_Impl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
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

// ---------------------------------------------------------------------------

class XMLTextRotationAnglePropHdl_Impl : public XMLPropertyHandler
{

public:
    XMLTextRotationAnglePropHdl_Impl()	{}
    virtual ~XMLTextRotationAnglePropHdl_Impl();

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLTextRotationAnglePropHdl_Impl::importXML(
        const OUString& rStrImpValue,
           Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int32 nValue;
    sal_Bool bRet = rUnitConverter.convertNumber( nValue, rStrImpValue );
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
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nAngle;
    sal_Bool bRet = ( rValue >>= nAngle );
    if( bRet )
    {
        OUStringBuffer aOut;
        rUnitConverter.convertNumber( aOut, nAngle / 10 );
        rStrExpValue = aOut.makeStringAndClear();
    }
    OSL_ENSURE( bRet, "illegal rotation angle" );

    return bRet;
}

XMLTextRotationAnglePropHdl_Impl::~XMLTextRotationAnglePropHdl_Impl()
{
}
// ---------------------------------------------------------------------------
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
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_HORIZONTAL_ON_LEFT_PAGES, sal_True );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT:
        pHdl = new XMLGrfMirrorPropHdl_Impl( XML_HORIZONTAL_ON_RIGHT_PAGES, sal_True );
        break;
    case XML_TYPE_TEXT_CLIP:
        pHdl = new XMLClipPropertyHandler;
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
        pHdl = new XMLNamedBoolPropertyHdl( ::binfilter::xmloff::token::XML_ABOVE,
                                            ::binfilter::xmloff::token::XML_BELOW );
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

// ----------------------------------------------------------------------------

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


}//end of namespace binfilter
