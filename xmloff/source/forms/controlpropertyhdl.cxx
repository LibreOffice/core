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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <xmloff/controlpropertyhdl.hxx>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <xmloff/xmltypes.hxx>
#include "xmloff/NamedBoolPropertyHdl.hxx"
#include "formenums.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include "callbacks.hxx"
#include <xmloff/XMLConstantsPropertyHandler.hxx>

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::beans;
    using namespace ::xmloff::token;

    //=====================================================================
    //= OControlPropertyHandlerFactory
    //=====================================================================
    //---------------------------------------------------------------------
    OControlPropertyHandlerFactory::OControlPropertyHandlerFactory()
        :m_pTextAlignHandler(NULL)
        ,m_pControlBorderStyleHandler(NULL)
        ,m_pControlBorderColorHandler(NULL)
        ,m_pRotationAngleHandler(NULL)
        ,m_pFontWidthHandler(NULL)
        ,m_pFontEmphasisHandler(NULL)
        ,m_pFontReliefHandler(NULL)
    {
    }

    //---------------------------------------------------------------------
    OControlPropertyHandlerFactory::~OControlPropertyHandlerFactory()
    {
        delete m_pTextAlignHandler;
        delete m_pControlBorderStyleHandler;
        delete m_pControlBorderColorHandler;
        delete m_pRotationAngleHandler;
        delete m_pFontWidthHandler;
        delete m_pFontEmphasisHandler;
        delete m_pFontReliefHandler;
    }

    //---------------------------------------------------------------------
    const XMLPropertyHandler* OControlPropertyHandlerFactory::GetPropertyHandler(sal_Int32 _nType) const
    {
        const XMLPropertyHandler* pHandler = NULL;

        switch (_nType)
        {
            case XML_TYPE_TEXT_ALIGN:
                if (!m_pTextAlignHandler)
                    m_pTextAlignHandler = new XMLConstantsPropertyHandler(OEnumMapper::getEnumMap(OEnumMapper::epTextAlign), XML_TOKEN_INVALID );
                pHandler = m_pTextAlignHandler;
                break;

            case XML_TYPE_CONTROL_BORDER:
                if (!m_pControlBorderStyleHandler)
                    m_pControlBorderStyleHandler = new OControlBorderHandler( OControlBorderHandler::STYLE );
                pHandler = m_pControlBorderStyleHandler;
                break;

            case XML_TYPE_CONTROL_BORDER_COLOR:
                if ( !m_pControlBorderColorHandler )
                    m_pControlBorderColorHandler = new OControlBorderHandler( OControlBorderHandler::COLOR );
                pHandler = m_pControlBorderColorHandler;
                break;

            case XML_TYPE_ROTATION_ANGLE:
                if (!m_pRotationAngleHandler)
                    m_pRotationAngleHandler = new ORotationAngleHandler;
                pHandler = m_pRotationAngleHandler;
                break;

            case XML_TYPE_FONT_WIDTH:
                if (!m_pFontWidthHandler)
                    m_pFontWidthHandler = new OFontWidthHandler;
                pHandler = m_pFontWidthHandler;
                break;

            case XML_TYPE_CONTROL_TEXT_EMPHASIZE:
                if (!m_pFontEmphasisHandler)
                    m_pFontEmphasisHandler = new XMLConstantsPropertyHandler( OEnumMapper::getEnumMap(OEnumMapper::epFontEmphasis), XML_NONE );
                pHandler = m_pFontEmphasisHandler;
                break;

            case XML_TYPE_TEXT_FONT_RELIEF:
                if (!m_pFontReliefHandler)
                    m_pFontReliefHandler = new XMLConstantsPropertyHandler( OEnumMapper::getEnumMap(OEnumMapper::epFontRelief), XML_NONE );
                pHandler = m_pFontReliefHandler;
                break;
            case XML_TYPE_TEXT_LINE_MODE:
                pHandler = new XMLNamedBoolPropertyHdl(
                                            ::xmloff::token::XML_SKIP_WHITE_SPACE,
                                            ::xmloff::token::XML_CONTINUOUS);
                break;
        }

        if (!pHandler)
            pHandler = XMLPropertyHandlerFactory::GetPropertyHandler(_nType);
        return pHandler;
    }

    //=====================================================================
    //= OControlTextEmphasisHandler
    //=====================================================================
    OControlTextEmphasisHandler::OControlTextEmphasisHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OControlTextEmphasisHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        ::rtl::OUStringBuffer aReturn;
        sal_Bool bSuccess = sal_False;
        sal_Int16 nFontEmphasis = sal_Int16();
        if (_rValue >>= nFontEmphasis)
        {
            // the type
            sal_Int16 nType = nFontEmphasis & ~(FontEmphasisMark::ABOVE | FontEmphasisMark::BELOW);
            // the position of the mark
            sal_Bool bBelow = 0 != (nFontEmphasis & FontEmphasisMark::BELOW);

            // convert
            bSuccess = SvXMLUnitConverter::convertEnum(aReturn, nType, OEnumMapper::getEnumMap(OEnumMapper::epFontEmphasis), XML_NONE);
            if (bSuccess)
            {
                aReturn.append( (sal_Unicode)' ' );
                aReturn.append( GetXMLToken(bBelow ? XML_BELOW : XML_ABOVE) );

                _rStrExpValue = aReturn.makeStringAndClear();
            }
        }

        return bSuccess;
    }

    //---------------------------------------------------------------------
    sal_Bool OControlTextEmphasisHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        sal_Bool bSuccess = sal_True;
        sal_uInt16 nEmphasis = FontEmphasisMark::NONE;

        sal_Bool bBelow = sal_False;
        sal_Bool bHasPos = sal_False, bHasType = sal_False;

        ::rtl::OUString sToken;
        SvXMLTokenEnumerator aTokenEnum(_rStrImpValue);
        while (aTokenEnum.getNextToken(sToken))
        {
            if (!bHasPos)
            {
                if (IsXMLToken(sToken, XML_ABOVE))
                {
                    bBelow = sal_False;
                    bHasPos = sal_True;
                }
                else if (IsXMLToken(sToken, XML_BELOW))
                {
                    bBelow = sal_True;
                    bHasPos = sal_True;
                }
            }
            if (!bHasType)
            {
                if (SvXMLUnitConverter::convertEnum(nEmphasis, sToken, OEnumMapper::getEnumMap(OEnumMapper::epFontEmphasis)))
                {
                    bHasType = sal_True;
                }
                else
                {
                    bSuccess = sal_False;
                    break;
                }
            }
        }

        if (bSuccess)
        {
            nEmphasis |= bBelow ? FontEmphasisMark::BELOW : FontEmphasisMark::ABOVE;
            _rValue <<= (sal_Int16)nEmphasis;
        }

        return bSuccess;
    }

    //=====================================================================
    //= OControlBorderHandlerBase
    //=====================================================================
    //---------------------------------------------------------------------
    OControlBorderHandler::OControlBorderHandler( const OControlBorderHandler::BorderFacet _eFacet )
        :m_eFacet( _eFacet )
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        ::rtl::OUString sToken;
        SvXMLTokenEnumerator aTokens(_rStrImpValue);

        sal_uInt16 nStyle = 1;
        Color aColor;

        while   (   aTokens.getNextToken(sToken)    // have a new token
                &&  (0 != sToken.getLength())       // really have a new token
                )
        {
            // try interpreting the token as border style
            if ( m_eFacet == STYLE )
            {
                // is it a valid enum value?
                if ( SvXMLUnitConverter::convertEnum( nStyle, sToken, OEnumMapper::getEnumMap( OEnumMapper::epBorderWidth ) ) )
                {
                    _rValue <<= nStyle;
                    return sal_True;
                }
            }

            // try interpreting it as color value
            if ( m_eFacet == COLOR )
            {
                if ( SvXMLUnitConverter::convertColor( aColor, sToken ) )
                {
                    _rValue <<= (sal_Int32)aColor.GetColor();
                    return sal_True;
                }
            }
        }

        return sal_False;
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        sal_Bool bSuccess = sal_False;

        ::rtl::OUStringBuffer aOut;
        switch ( m_eFacet )
        {
        case STYLE:
        {
            sal_Int16 nBorder = 0;
            bSuccess =  (_rValue >>= nBorder)
                    &&  SvXMLUnitConverter::convertEnum( aOut, nBorder, OEnumMapper::getEnumMap( OEnumMapper::epBorderWidth ) );
        }
        break;
        case COLOR:
        {
            sal_Int32 nBorderColor = 0;
            if ( _rValue >>= nBorderColor )
            {
                SvXMLUnitConverter::convertColor( aOut, Color( nBorderColor ) );
                bSuccess = sal_True;
            }
        }
        break;
        }   // switch ( m_eFacet )

        if ( !bSuccess )
            return sal_False;

        if ( _rStrExpValue.getLength() )
            _rStrExpValue += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );
        _rStrExpValue += aOut.makeStringAndClear();

        return sal_True;
    }

    //=====================================================================
    //= OFontWidthHandler
    //=====================================================================
    //---------------------------------------------------------------------
    OFontWidthHandler::OFontWidthHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OFontWidthHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        sal_Int32 nWidth = 0;
        sal_Bool bSuccess = SvXMLUnitConverter::convertMeasure(nWidth, _rStrImpValue, MAP_POINT);
        if (bSuccess)
            _rValue <<= (sal_Int16)nWidth;

        return bSuccess;
    }

    //---------------------------------------------------------------------
    sal_Bool OFontWidthHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        sal_Int16 nWidth = 0;
        ::rtl::OUStringBuffer aResult;
        if (_rValue >>= nWidth)
            SvXMLUnitConverter::convertMeasure(aResult, nWidth, MAP_POINT, MAP_POINT);
        _rStrExpValue = aResult.makeStringAndClear();

        return _rStrExpValue.getLength() != 0;
    }

    //=====================================================================
    //= ORotationAngleHandler
    //=====================================================================
    //---------------------------------------------------------------------
    ORotationAngleHandler::ORotationAngleHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool ORotationAngleHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& ) const
    {
        double fValue;
        sal_Bool bSucces =
            SvXMLUnitConverter::convertDouble(fValue, _rStrImpValue);
        if (bSucces)
        {
            fValue *= 10;
            _rValue <<= (float)fValue;
        }

        return bSucces;
    }

    //---------------------------------------------------------------------
    sal_Bool ORotationAngleHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& ) const
    {
        float fAngle = 0;
        sal_Bool bSuccess = (_rValue >>= fAngle);

        if (bSuccess)
        {
            rtl::OUStringBuffer sValue;
            SvXMLUnitConverter::convertDouble(sValue, ((double)fAngle) / 10);
            _rStrExpValue = sValue.makeStringAndClear();
        }

        return bSuccess;
    }

    //=====================================================================
    //= ImageScaleModeHandler
    //=====================================================================
    //---------------------------------------------------------------------
    ImageScaleModeHandler::ImageScaleModeHandler()
        :XMLConstantsPropertyHandler( OEnumMapper::getEnumMap( OEnumMapper::epImageScaleMode ), XML_STRETCH )
    {
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
