/*************************************************************************
 *
 *  $RCSfile: controlpropertyhdl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:06:44 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#include "controlpropertyhdl.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWIDTH_HPP_
#include <com/sun/star/awt/FontWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTEMPHASISMARK_HPP_
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#endif
#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_FORMENUMS_HXX_
#include "formenums.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef _XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#include "XMLConstantsPropertyHandler.hxx"
#endif

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
                    m_pControlBorderStyleHandler = new OControlBorderStyleHandler;
                pHandler = m_pControlBorderStyleHandler;
                break;

            case XML_TYPE_CONTROL_BORDER_COLOR:
                if ( !m_pControlBorderColorHandler )
                    m_pControlBorderColorHandler = new OControlBorderColorHandler;
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
    sal_Bool OControlTextEmphasisHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        ::rtl::OUStringBuffer aReturn;
        sal_Bool bSuccess = sal_False;
        sal_Int16 nFontEmphasis;
        if (_rValue >>= nFontEmphasis)
        {
            // the type
            sal_Int16 nType = nFontEmphasis & ~(FontEmphasisMark::ABOVE | FontEmphasisMark::BELOW);
            // the position of the mark
            sal_Bool bBelow = 0 != (nFontEmphasis & FontEmphasisMark::BELOW);

            // convert
            if (bSuccess = _rUnitConverter.convertEnum(aReturn, nType, OEnumMapper::getEnumMap(OEnumMapper::epFontEmphasis), XML_NONE))
            {
                aReturn.append( (sal_Unicode)' ' );
                aReturn.append( GetXMLToken(bBelow ? XML_BELOW : XML_ABOVE) );

                _rStrExpValue = aReturn.makeStringAndClear();
            }
        }

        return bSuccess;
    }

    //---------------------------------------------------------------------
    sal_Bool OControlTextEmphasisHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
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
                if (_rUnitConverter.convertEnum(nEmphasis, sToken, OEnumMapper::getEnumMap(OEnumMapper::epFontEmphasis)))
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
    //= OControlBorderHandlerFactory
    //=====================================================================
    //---------------------------------------------------------------------
    const XMLPropertyHandler* OControlBorderHandlerFactory::createBorderHandler()
    {
        return new OControlBorderStyleHandler;
    }

    //---------------------------------------------------------------------
    const XMLPropertyHandler* OControlBorderHandlerFactory::createBorderColorHandler()
    {
        return new OControlBorderColorHandler;
    }

    //=====================================================================
    //= OControlBorderHandlerBase
    //=====================================================================
    //---------------------------------------------------------------------
    OControlBorderHandlerBase::OControlBorderHandlerBase()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderHandlerBase::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        ::rtl::OUString sToken;
        SvXMLTokenEnumerator aTokens(_rStrImpValue);

        sal_uInt16 nStyle = 1;
        Color aColor;

        sal_Bool bFoundStyle = sal_False;
        sal_Bool bFoundColor = sal_False;

        while   (   !( bFoundStyle && bFoundColor ) // did not yet find both aspects
                &&  aTokens.getNextToken(sToken)    // have a new token
                &&  (0 != sToken.getLength())       // really have a new token
                )
        {
            // is it a valid enum value?
            if ( !bFoundStyle )
                bFoundStyle = _rUnitConverter.convertEnum(nStyle, sToken, OEnumMapper::getEnumMap(OEnumMapper::epBorderWidth));
            // is it a color value?
            if ( !bFoundColor )
                bFoundColor = _rUnitConverter.convertColor( aColor, sToken );
        }

        if ( !bFoundStyle && !bFoundColor )
            return sal_False;

        // if we're here, the string could have had more or less than the requested 3 tokens, but we ignore this.
        // At least we have a valid style or a valid, which is everything we're interested in.
        pickOne( aColor, (sal_Int16)nStyle, _rValue );
        return sal_True;
    }

    //=====================================================================
    //= OControlBorderStyleHandler
    //=====================================================================
    //---------------------------------------------------------------------
    OControlBorderStyleHandler::OControlBorderStyleHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderStyleHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        sal_Bool bSuccess = sal_False;
        sal_Int16 nBorder = 0;

        ::rtl::OUStringBuffer aOut;
        bSuccess =  (_rValue >>= nBorder)
                &&  _rUnitConverter.convertEnum(aOut, nBorder, OEnumMapper::getEnumMap(OEnumMapper::epBorderWidth));

        if ( _rStrExpValue.getLength() )
            _rStrExpValue += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );
        _rStrExpValue += aOut.makeStringAndClear();
        return bSuccess;
    }

    //---------------------------------------------------------------------
    void OControlBorderStyleHandler::pickOne( const Color& _rColor, sal_Int16 _nStyle, Any& _rValue ) const
    {
        _rValue <<= _nStyle;
    }

    //=====================================================================
    //= OControlBorderColorHandler
    //=====================================================================
    //---------------------------------------------------------------------
    OControlBorderColorHandler::OControlBorderColorHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderColorHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        sal_Bool bSuccess = sal_False;
        sal_Int32 nBorderColor = 0;

        ::rtl::OUStringBuffer aOut;
        if ( _rValue >>= nBorderColor )
        {
            _rUnitConverter.convertColor( aOut, Color( nBorderColor ) );
            bSuccess = sal_True;
        }

        if ( _rStrExpValue.getLength() )
            _rStrExpValue += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );
        _rStrExpValue += aOut.makeStringAndClear();
        return bSuccess;
    }

    //---------------------------------------------------------------------
    void OControlBorderColorHandler::pickOne( const Color& _rColor, sal_Int16 _nStyle, Any& _rValue ) const
    {
        _rValue <<= (sal_Int32)_rColor.GetColor();
    }

    //=====================================================================
    //= OFontWidthHandler
    //=====================================================================
    //---------------------------------------------------------------------
    OFontWidthHandler::OFontWidthHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OFontWidthHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        sal_Bool bSuccess = sal_False;

        sal_Int32 nWidth = 0;
        if (bSuccess = _rUnitConverter.convertMeasure(nWidth, _rStrImpValue, MAP_POINT))
            _rValue <<= (sal_Int16)nWidth;

        return bSuccess;
    }

    //---------------------------------------------------------------------
    sal_Bool OFontWidthHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        sal_Int16 nWidth = 0;
        ::rtl::OUStringBuffer aResult;
        if (_rValue >>= nWidth)
            _rUnitConverter.convertMeasure(aResult, nWidth, MAP_POINT, MAP_POINT);
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
    sal_Bool ORotationAngleHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        sal_Bool bSucces = sal_False;

        double fValue;
        if (bSucces = _rUnitConverter.convertDouble(fValue, _rStrImpValue))
        {
            fValue *= 10;
            _rValue <<= (float)fValue;
        }

        return bSucces;
    }

    //---------------------------------------------------------------------
    sal_Bool ORotationAngleHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        float fAngle;
        sal_Bool bSuccess = sal_False;

        if (bSuccess = (_rValue >>= fAngle))
        {
            rtl::OUStringBuffer sValue;
            _rUnitConverter.convertDouble(sValue, ((double)fAngle) / 10);
            _rStrExpValue = sValue.makeStringAndClear();
        }

        return bSuccess;
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.10.312.1  2004/04/29 13:54:40  fs
 *  #i24694# control border color now exported as part of the fo:border attribute
 *
 *  Revision 1.10  2001/07/16 07:34:21  fs
 *  had uninitialized pointers
 *
 *  Revision 1.9  2001/06/29 21:07:14  dvo
 *  #86004# changes sXML_* strings to XML_* tokens
 *
 *  Revision 1.8  2001/06/15 12:35:28  dvo
 *  #86004# changes sXML_* string to XML_* tokens continued
 *
 *  Revision 1.7  2001/06/07 12:27:21  fs
 *  #86096# handler for FontEmphasis/FontRelief / substituted OEnumInt16Handler by XMLConstantsPropertyHandler
 *
 *  Revision 1.6  2001/05/28 15:03:48  fs
 *  #86712# hold the handler as pointer, not as member instance
 *
 *  Revision 1.5  2001/05/15 14:02:13  fs
 *  #86823# changed the handling for control borders
 *
 *  Revision 1.4  2001/02/26 10:28:04  aw
 *  Changed double import/export to use it's own conversion routines
 *  so iots more clear what type is used
 *
 *  Revision 1.3  2000/12/19 12:13:57  fs
 *  some changes ... now the exported styles are XSL conform
 *
 *  Revision 1.2  2000/12/19 08:43:00  fs
 *  no handling for the font with anymore - stored as measure
 *
 *  Revision 1.1  2000/12/18 15:16:04  fs
 *  initial checkin - property handlers / property handler factory
 *
 *
 *  Revision 1.0 14.12.00 10:53:10  fs
 ************************************************************************/

