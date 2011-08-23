/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <rsc/rscsfx.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <paratr.hxx>
#include <paratr.hxx>
#include <fmtpdsc.hxx>
#include "fmtlsplt.hxx"
#include "fmtornt.hxx"
#include "fmtfsize.hxx"
#include <unomid.h>
#include "unostyle.hxx"

#include <bf_xmloff/xmluconv.hxx>
#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/xmltkmap.hxx>
#include <bf_xmloff/i18nmap.hxx>

#include "xmlimp.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::binfilter::xmloff::token;

// ---------------------------------------------------------------------

sal_Bool SwFmtDrop::equalsXML( const SfxPoolItem& rItem ) const
{
    ASSERT( !this, "obsolete implementation!" );
    return sal_True;
}

// ---------------------------------------------------------------------

sal_Bool SwRegisterItem::importXML( const OUString& rValue, sal_uInt16,
                                    const SvXMLUnitConverter& rUnitConverter )
{
    ASSERT( !this, "obsolete implementation!" );
    return sal_False;
}

sal_Bool SwRegisterItem::exportXML(
        OUString& rValue, sal_uInt16,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    ASSERT( !this, "obsolete implementation!" );
    return sal_True;
}

// ---------------------------------------------------------------------

sal_Bool SwNumRuleItem::equalsXML( const SfxPoolItem& rItem ) const
{
    ASSERT( !this, "obsolete implementation!" );
    return sal_True;
}

// ---------------------------------------------------------------------

sal_Bool SwFmtPageDesc::importXML( const OUString& rValue, sal_uInt16 nMemberId,
                                    const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bRet = sal_False;

    if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
    {
        sal_Int32 nVal;
        bRet = rUnitConverter.convertNumber( nVal, rValue, 0, USHRT_MAX );
        if( bRet )
            SetNumOffset( (USHORT)nVal );
    }

    return bRet;
}

sal_Bool SwFmtPageDesc::exportXML(
        OUString& rValue, sal_uInt16 nMemberId,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( MID_PAGEDESC_PAGENUMOFFSET==nMemberId )
    {
        OUStringBuffer aOut;

        rUnitConverter.convertNumber( aOut, (sal_Int32)GetNumOffset() );
        rValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}
sal_Bool SwFmtPageDesc::equalsXML( const SfxPoolItem& rItem ) const
{
    // Everything except pDefinedIn must be the same.
    return ( nNumOffset == ((SwFmtPageDesc&)rItem).nNumOffset ) &&
           ( GetPageDesc() == ((SwFmtPageDesc&)rItem).GetPageDesc() );
}

// ---------------------------------------------------------------------

sal_Bool SwFmtCharFmt::exportXML( OUString& rValue, sal_uInt16,
                              const SvXMLUnitConverter& ) const
{
    ASSERT( !this, "obsolete implementation!" );
    return sal_True;
}

// ---------------------------------------------------------------------

sal_Bool SwFmtLayoutSplit::importXML( const OUString& rValue, sal_uInt16,
                                        const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bValue;
    sal_Bool bRet = rUnitConverter.convertBool( bValue, rValue );
    if( bRet )
        SetValue( bValue );

    return bRet;
}

sal_Bool SwFmtLayoutSplit::exportXML(
        OUString& rValue, sal_uInt16,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    rUnitConverter.convertBool( aOut, GetValue() );
    rValue = aOut.makeStringAndClear();

    return sal_True;
}

// ---------------------------------------------------------------------

static __FAR_DATA struct SvXMLEnumMapEntry aXMLTableAlignMap[] =
{
    { XML_LEFT,				HORI_LEFT			},
    { XML_LEFT,				HORI_LEFT_AND_WIDTH	},
    { XML_CENTER, 			HORI_CENTER			},
    { XML_RIGHT, 			HORI_RIGHT			},
    { XML_MARGINS, 			HORI_FULL			},
    { XML_MARGINS,			HORI_NONE			},
    { XML_TOKEN_INVALID, 0 }
};

sal_Bool SwFmtHoriOrient::importXML( const OUString& rValue, sal_uInt16,
                                       const SvXMLUnitConverter& rUnitConverter )
{
    sal_uInt16 nValue;
    sal_Bool bRet = rUnitConverter.convertEnum( nValue, rValue,
                                                aXMLTableAlignMap );
    if( bRet )
        SetHoriOrient( (SwHoriOrient)nValue );

    return bRet;
}

sal_Bool SwFmtHoriOrient::exportXML(
        OUString& rValue, sal_uInt16,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    rUnitConverter.convertEnum( aOut, GetHoriOrient(), aXMLTableAlignMap );
    rValue = aOut.makeStringAndClear();

    return sal_True;
}

// ---------------------------------------------------------------------

static __FAR_DATA SvXMLEnumMapEntry aXMLTableVAlignMap[] =
{
    { XML_TOP,					VERT_TOP		},
    { XML_MIDDLE,				VERT_CENTER		},
    { XML_BOTTOM, 				VERT_BOTTOM		},
    { XML_TOKEN_INVALID, 0 }
};

sal_Bool SwFmtVertOrient::importXML( const OUString& rValue, sal_uInt16,
                                       const SvXMLUnitConverter& rUnitConverter )
{
    sal_uInt16 nValue;
    sal_Bool bRet =
        rUnitConverter.convertEnum( nValue, rValue,aXMLTableVAlignMap );
    if( bRet )
        SetVertOrient( (SwVertOrient)nValue );

    return bRet;
}

sal_Bool SwFmtVertOrient::exportXML(
        OUString& rValue, sal_uInt16,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    rUnitConverter.convertEnum( aOut, GetVertOrient(), aXMLTableVAlignMap );
    rValue = aOut.makeStringAndClear();

    return sal_True;
}

// ---------------------------------------------------------------------

sal_Bool SwFmtFrmSize::importXML( const OUString& rValue, sal_uInt16 nMemberId,
                                  const SvXMLUnitConverter& rUnitConverter )
{
    sal_Bool bRet = sal_False;
    sal_Bool bSetHeight = sal_False;
    sal_Bool bSetWidth = sal_False;
    sal_Bool bSetSizeType = sal_False;
    SwFrmSize eSizeType = ATT_VAR_SIZE;
    sal_Int32 nMin = MINLAY;

    switch( nMemberId )
    {
    case MID_FRMSIZE_REL_WIDTH:
        {
            sal_Int32 nValue;
            bRet = rUnitConverter.convertPercent( nValue, rValue );
            if( bRet )
            {
                if( nValue < 1 )
                    nValue = 1;
                else if( nValue > 100 )
                    nValue = 100;

                SetWidthPercent( (sal_Int8)nValue );
            }
        }
        break;
    case MID_FRMSIZE_WIDTH:
        bSetWidth = sal_True;
        break;
    case MID_FRMSIZE_MIN_HEIGHT:
        eSizeType = ATT_MIN_SIZE;
        bSetHeight = sal_True;
        nMin = 1;
        bSetSizeType = sal_True;
        break;
    case MID_FRMSIZE_FIX_HEIGHT:
        eSizeType = ATT_FIX_SIZE;
        bSetHeight = sal_True;
        nMin = 1;
        bSetSizeType = sal_True;
        break;
    case MID_FRMSIZE_COL_WIDTH:
        eSizeType = ATT_FIX_SIZE;
        bSetWidth = sal_True;
        bSetSizeType = sal_True;
        break;
    case MID_FRMSIZE_REL_COL_WIDTH:
        {
            sal_Int32 nPos = rValue.indexOf( (sal_Unicode)'*' );
            if( -1L != nPos )
            {
                OUString sNum( rValue.copy( 0L, nPos ) );
                sal_Int32 nValue = rValue.toInt32();
                if( nValue < MINLAY )
                    nValue = MINLAY;
                else if( nValue > USHRT_MAX )
                    nValue = USHRT_MAX;

                SetWidth( (sal_uInt16)nValue );
                SetSizeType( ATT_VAR_SIZE );
                bRet = sal_True;
            }
        }
        break;
    }

    sal_Int32 nValue;
    if( bSetHeight || bSetWidth )
    {
        bRet = rUnitConverter.convertMeasure( nValue, rValue, nMin,
                                              USHRT_MAX );
        if( bRet )
        {
            if( bSetWidth )
                SetWidth( (sal_uInt16)nValue );
            if( bSetHeight )
                SetHeight( (sal_uInt16)nValue );
            if( bSetSizeType )
                SetSizeType( eSizeType );
        }
    }
    return bRet;
}

sal_Bool SwFmtFrmSize::exportXML(
        OUString& rValue, sal_uInt16 nMemberId,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Bool bOutHeight = sal_False;
    switch( nMemberId )
    {
    case MID_FRMSIZE_REL_WIDTH:
        if( GetWidthPercent() )
        {
            OUStringBuffer aOut;
            rUnitConverter.convertPercent( aOut, GetWidthPercent() );
            rValue = aOut.makeStringAndClear();
            bRet = sal_True;
        }
        break;
    case MID_FRMSIZE_MIN_HEIGHT:
        if( ATT_MIN_SIZE == GetSizeType() )
            bOutHeight = sal_True;
        break;
    case MID_FRMSIZE_FIX_HEIGHT:
        if( ATT_FIX_SIZE == GetSizeType() )
            bOutHeight = sal_True;
        break;
    }

    if( bOutHeight )
    {
        OUStringBuffer aOut;
        rUnitConverter.convertMeasure( aOut, GetHeight() );
        rValue = aOut.makeStringAndClear();
        bRet = sal_True;
    }

    return bRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
