/*************************************************************************
 *
 *  $RCSfile: controlpropertyhdl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-26 10:28:04 $
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
#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef _XMLOFF_FORMENUMS_HXX_
#include "formenums.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::beans;

    //=====================================================================
    //= OControlPropertyHandlerFactory
    //=====================================================================
    //---------------------------------------------------------------------
    OControlPropertyHandlerFactory::OControlPropertyHandlerFactory()
        :m_aTextAlignHandler(OEnumMapper::getEnumMap(OEnumMapper::epTextAlign), -1)
    {
    }

    //---------------------------------------------------------------------
    OControlPropertyHandlerFactory::~OControlPropertyHandlerFactory()
    {
    }

    //---------------------------------------------------------------------
    const XMLPropertyHandler* OControlPropertyHandlerFactory::GetPropertyHandler(sal_Int32 _nType) const
    {
        switch (_nType)
        {
            case XML_TYPE_TEXT_ALIGN:
                return &m_aTextAlignHandler;
            case XML_TYPE_CONTROL_BORDER:
                return &m_aControlBorderHandler;
            case XML_TYPE_ROTATION_ANGLE:
                return &m_aRotationAngleHandler;
            case XML_TYPE_FONT_WIDTH:
                return &m_aFontWidthHandler;
            default:
                return XMLPropertyHandlerFactory::GetPropertyHandler(_nType);
        }
    }

    //=====================================================================
    //= OControlBorderHandler
    //=====================================================================
    //---------------------------------------------------------------------
    OControlBorderHandler::OControlBorderHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderHandler::importXML( const ::rtl::OUString& _rStrImpValue, Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        ::rtl::OUString sToken;
        SvXMLTokenEnumerator aTokens(_rStrImpValue);

        // the tokens in the attribute value describing different aspects of the border
        enum BorderTokens
        {
            btWidth = 0,
            btStyle = 1,
            btColor = 2
        };

        sal_uInt16 nConvertedStyle = (sal_uInt16)-1;

        BorderTokens eToken = btWidth;
        sal_Bool bConversionSuccess = sal_True;
        while   (   bConversionSuccess              // could convert the previous token
                &&  aTokens.getNextToken(sToken)    // have a new token
                &&  (0 != sToken.getLength())       // really have a new token
                &&  (eToken <= btColor)             // did not exceed the maximum border token count
                )
        {
            switch (eToken)
            {
                case btWidth:
                    // does not really matter for us. Though we write different widths for 3D and flat borders,
                    // it's just for convenience.
                    break;
                case btStyle:
                    bConversionSuccess = _rUnitConverter.convertEnum(nConvertedStyle, sToken, OEnumMapper::getEnumMap(OEnumMapper::epBorderWidth));
                    break;
                case btColor:
                    // ignore this. Our borders do not have a color
                    break;
            }

            eToken = static_cast<BorderTokens>(1 + static_cast<sal_Int32>(eToken));
        }

        if ((sal_uInt16)-1 == nConvertedStyle)
            return sal_False;

        // if we're here, the string could have had more or less than the requested 3 tokens, but we ignore this.
        // At least we have a valid style, which is everything we're interested in.
        _rValue <<= (sal_Int16)nConvertedStyle;
        return sal_True;
    }

    //---------------------------------------------------------------------
    sal_Bool OControlBorderHandler::exportXML( ::rtl::OUString& _rStrExpValue, const Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
    {
        sal_Bool bSuccess = sal_False;
        sal_Int16 nBorder = 0;

        ::rtl::OUStringBuffer aOut;
        if (bSuccess = (_rValue >>= nBorder))
        {
            switch (nBorder)
            {
                case 0: // none
                    return sal_False;
                case 1: // 3D
                {
                    aOut.appendAscii(sXML_middle);                  // width
                    aOut.append(sal_Unicode(' '));                  // separator
                    aOut.appendAscii(sXML_groove);                  // style
                    aOut.append(sal_Unicode(' '));                  // separator
                    _rUnitConverter.convertColor( aOut, (Color)0 ); // color
                }
                break;
                case 2: // flat
                {
                    aOut.appendAscii(sXML_thin);                    // width
                    aOut.append(sal_Unicode(' '));                  // separator
                    aOut.appendAscii(sXML_solid);                   // style (carved in)
                    aOut.append(sal_Unicode(' '));                  // separator
                    _rUnitConverter.convertColor( aOut, (Color)0 ); // color
                }
                break;
                default:
                    // unknown
                    return sal_False;
            }
        }

        _rStrExpValue = aOut.makeStringAndClear();
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
    //= OEnumInt16Handler
    //=====================================================================
    //---------------------------------------------------------------------
    ORotationAngleHandler::ORotationAngleHandler()
    {
    }

    //---------------------------------------------------------------------
    sal_Bool ORotationAngleHandler::importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
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
    sal_Bool ORotationAngleHandler::exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const
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

    //=====================================================================
    //= OEnumInt16Handler
    //=====================================================================
    //---------------------------------------------------------------------
    OEnumInt16Handler::OEnumInt16Handler(const SvXMLEnumMapEntry* _pMap, sal_Int16 _nVoidValue)
        :m_pMap(_pMap)
        ,m_nVoidValue(_nVoidValue)
    {
    }

    //---------------------------------------------------------------------
    sal_Bool OEnumInt16Handler::importXML(const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter) const
    {
        // extract the value
        sal_uInt16 nValue = m_nVoidValue;
        sal_Bool bSuccess = _rUnitConverter.convertEnum(nValue, _rStrImpValue, m_pMap);
        if (!bSuccess || (m_nVoidValue == nValue))
            _rValue.clear();
        else
            _rValue <<= (sal_Int16)nValue;

        return bSuccess;
    }

    //---------------------------------------------------------------------
    sal_Bool OEnumInt16Handler::exportXML(::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter) const
    {
        // extract the value
        sal_Int16 nValue = m_nVoidValue;
        if (_rValue.hasValue())
            _rValue >>= nValue;

        // convert it
        ::rtl::OUStringBuffer aBuffer;
        sal_Bool bSuccess = _rUnitConverter.convertEnum(aBuffer, nValue, m_pMap);
        _rStrExpValue = aBuffer.makeStringAndClear();

        // outta here
        return bSuccess;
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
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

