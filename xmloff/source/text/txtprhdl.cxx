/*************************************************************************
 *
 *  $RCSfile: txtprhdl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: mib $ $Date: 2001-03-23 16:30:35 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_DROPCAPFORMAT_HPP_
#include <com/sun/star/style/DropCapFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCOLUMN_HPP_
#include <com/sun/star/text/TextColumn.hpp>
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

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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

using namespace ::rtl;
//using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
//using namespace ::com::sun::star::container;
//using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;

#define CONSTASCII_USTRINGPARAM_CMP(s) s, sizeof(s)-1

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriPos_Enum[] =
{
    { sXML_from_left,       HoriOrientation::NONE   },
    { sXML_from_inside,     HoriOrientation::NONE   },  // import only
    { sXML_left,            HoriOrientation::LEFT   },
    { sXML_inside,          HoriOrientation::LEFT   },  // import only
    { sXML_center,          HoriOrientation::CENTER },
    { sXML_right,           HoriOrientation::RIGHT  },
    { sXML_outside,         HoriOrientation::RIGHT  },  // import only
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriPosMirrored_Enum[] =
{
    { sXML_from_inside,     HoriOrientation::NONE   },
    { sXML_inside,          HoriOrientation::LEFT   },
    { sXML_center,          HoriOrientation::CENTER },
    { sXML_outside,         HoriOrientation::RIGHT  },
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriRel_Enum[] =
{
    { sXML_paragraph,           RelOrientation::FRAME   },
    { sXML_paragraph_content,   RelOrientation::PRINT_AREA  },
    { sXML_page,                RelOrientation::PAGE_FRAME  },
    { sXML_page_content,        RelOrientation::PAGE_PRINT_AREA },
    { sXML_paragraph_start_margin,  RelOrientation::FRAME_LEFT  },
    { sXML_paragraph_end_margin,    RelOrientation::FRAME_RIGHT },
    { sXML_page_start_margin,   RelOrientation::PAGE_LEFT   },
    { sXML_page_end_margin,     RelOrientation::PAGE_RIGHT  },
    { sXML_char,                RelOrientation::CHAR    },
    { sXML_frame,               RelOrientation::FRAME   },      // import only
    { sXML_frame_content,       RelOrientation::PRINT_AREA  },  // import only
    { sXML_frame_start_margin,  RelOrientation::FRAME_LEFT  },  // import only
    { sXML_frame_end_margin,    RelOrientation::FRAME_RIGHT },  // import only
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriRelFrame_Enum[] =
{
    { sXML_frame,               RelOrientation::FRAME   },
    { sXML_frame_content,       RelOrientation::PRINT_AREA  },
    { sXML_page,                RelOrientation::PAGE_FRAME  },
    { sXML_page_content,        RelOrientation::PAGE_PRINT_AREA },
    { sXML_frame_start_margin,  RelOrientation::FRAME_LEFT  },
    { sXML_frame_end_margin,    RelOrientation::FRAME_RIGHT },
    { sXML_page_start_margin,   RelOrientation::PAGE_LEFT   },
    { sXML_page_end_margin,     RelOrientation::PAGE_RIGHT  },
    { sXML_char,                RelOrientation::CHAR    },
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_HoriMirror_Enum[] =
{
    { sXML_from_left,       sal_False   },
    { sXML_from_inside,     sal_True    },
    { sXML_left,            sal_False   },
    { sXML_inside,          sal_True    },
    { sXML_center,          sal_False   },
    { sXML_right,           sal_False   },
    { sXML_outside,         sal_True    },
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertPos_Enum[] =
{
    { sXML_from_top,        VertOrientation::NONE       },
    { sXML_top,             VertOrientation::TOP        },
    { sXML_top,             VertOrientation::CHAR_TOP   },  // export only
    { sXML_top,             VertOrientation::LINE_TOP   },  // export only
    { sXML_middle,          VertOrientation::CENTER     },
    { sXML_middle,          VertOrientation::CHAR_CENTER    },  // export only
    { sXML_middle,          VertOrientation::LINE_CENTER    },  // export only
    { sXML_bottom,          VertOrientation::BOTTOM     },
    { sXML_bottom,          VertOrientation::CHAR_BOTTOM    },  // export only
    { sXML_bottom,          VertOrientation::LINE_BOTTOM    },  // export only
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRel_Enum[] =
{
    { sXML_paragraph,           RelOrientation::FRAME   },
    { sXML_paragraph_content,   RelOrientation::PRINT_AREA  },
    { sXML_char,                RelOrientation::CHAR    },
    { sXML_page,                RelOrientation::FRAME   },      // import only
    { sXML_page_content,        RelOrientation::PRINT_AREA  },  // import only
    { sXML_frame,               RelOrientation::FRAME   },      // import only
    { sXML_frame_content,       RelOrientation::PRINT_AREA  },  // import only
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRelPage_Enum[] =
{
    { sXML_page,            RelOrientation::FRAME   },
    { sXML_page_content,    RelOrientation::PRINT_AREA  },
    { sXML_page,            RelOrientation::PAGE_FRAME  },
    { sXML_page_content,    RelOrientation::PAGE_PRINT_AREA },
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRelFrame_Enum[] =
{
    { sXML_frame,           RelOrientation::FRAME   },
    { sXML_frame_content,   RelOrientation::PRINT_AREA  },
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VertRelAsChar_Enum[] =
{
    { sXML_baseline,        VertOrientation::TOP        },
    { sXML_baseline,        VertOrientation::CENTER     },  // export only
    { sXML_baseline,        VertOrientation::BOTTOM     },  // export only
    { sXML_char,            VertOrientation::CHAR_TOP   },
    { sXML_char,            VertOrientation::CHAR_CENTER    },  // export only
    { sXML_char,            VertOrientation::CHAR_BOTTOM    },  // export only
    { sXML_line,            VertOrientation::LINE_TOP   },
    { sXML_line,            VertOrientation::LINE_CENTER    },  // export only
    { sXML_line,            VertOrientation::LINE_BOTTOM    },  // export only
    { 0, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_RubyAdjust_Enum[] =
{
    { sXML_left,                RubyAdjust_LEFT },
    { sXML_center,              RubyAdjust_CENTER },
    { sXML_right,               RubyAdjust_RIGHT },
    { sXML_distribute_letter,   RubyAdjust_BLOCK },
    { sXML_distribute_space,    RubyAdjust_INDENT_BLOCK },
    { 0, 0 }
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

    return  (aFormat1.Lines <=1 && aFormat2.Lines <=1) ||
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
    if( rStrImpValue.equalsAsciiL(
            CONSTASCII_USTRINGPARAM_CMP( sXML_foreground ) ) )
        bVal = sal_True;
    else if( !rStrImpValue.equalsAsciiL(
            CONSTASCII_USTRINGPARAM_CMP( sXML_background ) ) )
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
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_foreground) );
    else
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_background) );

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
    if( rStrImpValue.equalsAsciiL(
            CONSTASCII_USTRINGPARAM_CMP( sXML_outside ) ) )
        bVal = sal_True;
    else if( !rStrImpValue.equalsAsciiL(
            CONSTASCII_USTRINGPARAM_CMP( sXML_full ) ) )
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
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_outside) );
    else
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_full) );

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

    if( !rStrImpValue.compareToAscii( sXML_no_limit ) == 0 )
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
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM("1") );
    else
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_no_limit) );

    return sal_True;
}

XMLParagraphOnlyPropHdl_Impl::~XMLParagraphOnlyPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_Wrap_Enum[] =
{
    { sXML_none,        WrapTextMode_NONE },
    { sXML_run_through, WrapTextMode_THROUGHT },
    { sXML_parallel,    WrapTextMode_PARALLEL },
    { sXML_dynamic,     WrapTextMode_DYNAMIC },
    { sXML_left,        WrapTextMode_LEFT },
    { sXML_right,       WrapTextMode_RIGHT },
    { 0, 0 }
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

    sal_Bool bRet = rUnitConverter.convertEnum( aOut, eVal, pXML_Wrap_Enum, sXML_none );

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
    XMLFrameProtectPropHdl_Impl( const sal_Char *pVal ) :
           sVal( OUString::createFromAscii(pVal) ) {}
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
    if( !rStrImpValue.equalsAsciiL( CONSTASCII_USTRINGPARAM_CMP( sXML_none ) ) )
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
            rStrExpValue.equalsAsciiL(CONSTASCII_USTRINGPARAM_CMP(sXML_none)) )
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
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_none) );
    }

    return sal_True;
}

XMLFrameProtectPropHdl_Impl::~XMLFrameProtectPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_Anchor_Enum[] =
{
    { sXML_char,        TextContentAnchorType_AT_CHARACTER },
    { sXML_page,    TextContentAnchorType_AT_PAGE },
    { sXML_frame,   TextContentAnchorType_AT_FRAME },
    { sXML_paragraph,       TextContentAnchorType_AT_PARAGRAPH },
    { sXML_as_char,     TextContentAnchorType_AS_CHARACTER },
    { 0, 0 }
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

    sal_Bool bRet = rUnitConverter.convertEnum( aOut, eVal, pXML_Anchor_Enum, sXML_paragraph );

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
    XMLGrfMirrorPropHdl_Impl( const sal_Char *pVal, sal_Bool bH ) :
           sVal( OUString::createFromAscii(pVal) ),
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
    if( !rStrImpValue.equalsAsciiL( CONSTASCII_USTRINGPARAM_CMP( sXML_none ) ) )
    {
        bRet = sal_False;
        SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
        OUString aToken;
        while( aTokenEnum.getNextToken( aToken ) )
        {
            bRet = sal_True;
            if( aToken == sVal ||
                 (bHori && aToken.equalsAsciiL(
                          CONSTASCII_USTRINGPARAM_CMP( sXML_horizontal ) ) ) )
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
            rStrExpValue.equalsAsciiL(CONSTASCII_USTRINGPARAM_CMP(sXML_none)) )
        {
               rStrExpValue = sVal;
        }
        else if( bHori &&
                 ( rStrExpValue.equalsAsciiL( CONSTASCII_USTRINGPARAM_CMP(sXML_horizontal_on_left_pages)) ||
                   rStrExpValue.equalsAsciiL( CONSTASCII_USTRINGPARAM_CMP(sXML_horizontal_on_right_pages)) ) )
        {
            rStrExpValue = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_horizontal) );
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
           rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM(sXML_none) );
    }

    return sal_True;
}

XMLGrfMirrorPropHdl_Impl::~XMLGrfMirrorPropHdl_Impl()
{
}

// ---------------------------------------------------------------------------

SvXMLEnumMapEntry __READONLY_DATA pXML_Emphasize_Enum[] =
{
    { sXML_none,    EMPHASISMARK_NONE },
    { sXML_dot,     EMPHASISMARK_DOT },
    { sXML_circle,  EMPHASISMARK_CIRCLE },
    { sXML_disc,    EMPHASISMARK_DISC },
    { sXML_accent,  EMPHASISMARK_ACCENT },
    { 0, 0 }
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
    sal_uInt16 nVal = EMPHASISMARK_NONE;
    sal_Bool bBelow = sal_False;
    sal_Bool bHasPos = sal_False, bHasType = sal_False;
    OUString aToken;

    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    while( aTokenEnum.getNextToken( aToken ) )
    {
        if( !bHasPos &&
            aToken.equalsAsciiL( sXML_above, sizeof(sXML_above)-1 ) )
        {
            bBelow = sal_False;
            bHasPos = sal_True;
        }
        else if( !bHasPos &&
                 aToken.equalsAsciiL( sXML_below, sizeof(sXML_below)-1 ) )
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
        if( EMPHASISMARK_NONE != nVal )
            nVal |= (bBelow ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE);
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
        bRet = rUnitConverter.convertEnum( aOut, nType & EMPHASISMARK_STYLE,
                                           pXML_Emphasize_Enum,
                                           sXML_dot );
        if( bRet )
        {
            const sal_Char *pPos = 0;
            switch( nType & ~EMPHASISMARK_STYLE )
            {
            case EMPHASISMARK_POS_ABOVE:
                pPos = sXML_above;
                break;
            case EMPHASISMARK_POS_BELOW:
                pPos = sXML_below;
                break;
            }
            if( pPos )
            {
                aOut.append( (sal_Unicode)' ' );
                aOut.appendAscii( pPos );
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
    XMLTextSyncWidthHeightPropHdl_Impl( const sal_Char *pValue ) :
           sValue( OUString::createFromAscii(pValue) )  {}
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
        pHdl = new XMLFrameProtectPropHdl_Impl( sXML_content );
        break;
    case XML_TYPE_TEXT_PROTECT_SIZE:
        pHdl = new XMLFrameProtectPropHdl_Impl( sXML_size );
        break;
    case XML_TYPE_TEXT_PROTECT_POSITION:
        pHdl = new XMLFrameProtectPropHdl_Impl( sXML_position );
        break;
    case XML_TYPE_TEXT_ANCHOR_TYPE:
        pHdl = new XMLAnchorTypePropHdl;
        break;
    case XML_TYPE_TEXT_COLUMNS:
        pHdl = new XMLTextColumnsPropertyHandler;
        break;
    case XML_TYPE_TEXT_HORIZONTAL_POS:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriPos_Enum, 0 );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriPosMirrored_Enum, 0 );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_REL:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriRel_Enum, 0 );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_REL_FRAME:
        pHdl = new XMLConstantsPropertyHandler( pXML_HoriRelFrame_Enum, 0 );
        break;
    case XML_TYPE_TEXT_HORIZONTAL_MIRROR:
        pHdl = new XMLHoriMirrorPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_VERTICAL_POS:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertPos_Enum, 0 );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRel_Enum, 0 );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL_PAGE:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRelPage_Enum, 0 );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL_FRAME:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRelFrame_Enum, 0 );
        break;
    case XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR:
        pHdl = new XMLConstantsPropertyHandler( pXML_VertRelAsChar_Enum, 0 );
        break;
    case XML_TYPE_TEXT_MIRROR_VERTICAL:
        pHdl = new XMLGrfMirrorPropHdl_Impl( sXML_vertical, sal_False );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT:
        pHdl = new XMLGrfMirrorPropHdl_Impl( sXML_horizontal_on_left_pages, sal_True );
        break;
    case XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT:
        pHdl = new XMLGrfMirrorPropHdl_Impl( sXML_horizontal_on_right_pages, sal_True );
        break;
    case XML_TYPE_TEXT_CLIP:
        pHdl = new XMLClipPropertyHandler;
        break;
    case XML_TYPE_TEXT_EMPHASIZE:
        pHdl = new XMLTextEmphasizePropHdl_Impl;
        break;
    case XML_TYPE_TEXT_COMBINE:
        pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_lines ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) ) );
        break;
    case XML_TYPE_TEXT_COMBINE_CHARACTERS:
        pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_letters ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) ) );
        break;
    case XML_TYPE_TEXT_COMBINECHAR:
        pHdl = new XMLTextCombineCharPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_AUTOSPACE:
        pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_ideograph_alpha ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_none ) ) );
        break;
    case XML_TYPE_TEXT_PUNCTUATION_WRAP:
        pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_hanging ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_simple ) ) );
        break;
    case XML_TYPE_TEXT_LINE_BREAK:
        pHdl = new XMLNamedBoolPropertyHdl(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_strict ) ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_normal ) ) );
        break;
    case XML_TYPE_TEXT_REL_WIDTH_HEIGHT:
        pHdl = new XMLTextRelWidthHeightPropHdl_Impl;
        break;
    case XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT:
        pHdl = new XMLTextSyncWidthHeightPropHdl_Impl( sXML_scale );
        break;
    case XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT_MIN:
        pHdl = new XMLTextSyncWidthHeightPropHdl_Impl( sXML_scale_min );
        break;
    case XML_TYPE_TEXT_RUBY_ADJUST:
        pHdl = new XMLConstantsPropertyHandler( pXML_RubyAdjust_Enum, 0 );
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


