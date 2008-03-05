/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:11:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <stdlib.h>
#include <fstream>
#include <string>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter
{
using namespace com::sun::star;
using namespace std;
using text::TextContentAnchorType;
static string & logger_file()
{
    static string _logger_file = string(getenv("TEMP")?getenv("TEMP"):"/tmp") + "/writerfilter.ooxml.tmp";
    return _logger_file;
}

static ofstream & logger_stream()
{
    static ofstream _logger_stream(logger_file().c_str());
    return _logger_stream;
}


void logger(string prefix, string message)
{
    logger_stream() << prefix <<  ":" << message << endl;
    logger_stream().flush();
}

string propertysetToString(uno::Reference<beans::XPropertySet> const & xPropSet)
{
    string sResult;

    static int nAttribNames = 9;
    static string sPropertyAttribNames[9] =
        {
            "MAYBEVOID",
            "BOUND",
            "CONSTRAINED",
            "TRANSIENT",
            "READONLY",
            "MAYBEAMBIGUOUS",
            "MAYBEDEFAULT",
            "REMOVEABLE",
            "OPTIONAL"
        };

    uno::Reference<beans::XPropertySetInfo> xPropSetInfo
        (xPropSet->getPropertySetInfo());

    if (xPropSetInfo.is())
    {
        uno::Sequence<beans::Property> aProps(xPropSetInfo->getProperties());

        sResult +="<propertyset>";

        for (sal_Int32 n = 0; n < aProps.getLength(); n++)
        {
            ::rtl::OUString sPropName(aProps[n].Name);

            if (xPropSetInfo->hasPropertyByName(sPropName))
            {
                uno::Any aAny;
                try
                {
                    xPropSet->getPropertyValue(sPropName) >>= aAny;
                }
                catch (beans::UnknownPropertyException)
                {
                    sResult += "<unknown-property>";
                    sResult += OUStringToOString
                        (sPropName, RTL_TEXTENCODING_ASCII_US).getStr();
                    sResult += "</unknown-property>";
                }

                if (aAny.hasValue())
                {
                    sResult += "<property name=\"";
                    sResult += OUStringToOString
                        (sPropName, RTL_TEXTENCODING_ASCII_US).getStr();
                    sResult +="\" type=\"";

                    ::rtl::OUString sPropType(aProps[n].Type.getTypeName());
                    sResult += OUStringToOString
                        (sPropType, RTL_TEXTENCODING_ASCII_US).getStr();

                    sResult += "\" attribs=\"";

                    sal_uInt16 nMask = 1;
                    bool bFirstAttrib = true;
                    sal_uInt16 nAttribs = aProps[n].Attributes;
                    for (int i = 0; i < nAttribNames; i++)
                    {
                        if ((nAttribs & nMask) != 0)
                        {
                            if (bFirstAttrib)
                                bFirstAttrib = false;
                            else
                                sResult += "|";

                            sResult += sPropertyAttribNames[i];
                        }

                        nMask <<= 1;
                    }

                    sResult += "\">";

                    char buffer[256];
                    if (sPropType ==
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                        ("byte")))
                    {
                        sal_Int8 nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%d", nValue);
                        sResult += buffer;
                    }
                    if (sPropType ==
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                        ("short")))
                    {
                        sal_Int16 nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%d", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("long")))
                    {
                        sal_Int32 nValue = 0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%" SAL_PRIdINT32, nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("float")))
                    {
                        float nValue = 0.0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%f", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("double")))
                    {
                        double nValue = 0.0;
                        aAny >>= nValue;

                        snprintf(buffer, sizeof(buffer), "%lf", nValue);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("boolean")))
                    {
                        sal_Bool nValue = sal_False;
                        aAny >>= nValue;

                        if (nValue)
                            sResult += "true";
                        else
                            sResult += "false";
                    }
                    else if (sPropType ==
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                             ("string")))
                    {
                        ::rtl::OUString sValue;
                        aAny >>= sValue;

                        sResult += OUStringToOString
                            (sValue, RTL_TEXTENCODING_ASCII_US).getStr();
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.text.TextContentAnchorType")))
                    {
                        text::TextContentAnchorType nValue;
                        aAny >>= nValue;

                        switch (nValue)
                        {
                        case text::TextContentAnchorType_AT_PARAGRAPH:
                            sResult += "AT_PARAGRAPH";
                            break;
                        case text::TextContentAnchorType_AS_CHARACTER:
                            sResult += "AS_CHARACTER";
                            break;
                        case text::TextContentAnchorType_AT_PAGE:
                            sResult += "AT_PAGE";
                            break;
                        case text::TextContentAnchorType_AT_FRAME:
                            sResult += "AT_FRAME";
                            break;
                        case text::TextContentAnchorType_AT_CHARACTER:
                            sResult += "AT_CHARACTER";
                            break;
                        case text::TextContentAnchorType_MAKE_FIXED_SIZE:
                            sResult += "MAKE_FIXED_SIZE";
                            break;
                        default:
                            break;
                        }
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.awt.Point")))
                    {
                        awt::Point aPoint;
                        aAny >>= aPoint;

                        snprintf(buffer, sizeof(buffer), "(%" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ")", aPoint.X,
                                 aPoint.Y);

                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.awt.Rectangle")))
                    {
                        awt::Rectangle aRect;
                        aAny >>= aRect;

                        snprintf(buffer, sizeof(buffer), "(%" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 ")",
                                 aRect.X, aRect.Y, aRect.Width, aRect.Height);
                        sResult += buffer;
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.drawing.FillStyle")))
                    {
                        drawing::FillStyle nValue;
                        aAny >>= nValue;

                        switch (nValue)
                        {
                        case drawing::FillStyle_NONE:
                            sResult += "NONE";
                            break;
                        case drawing::FillStyle_SOLID:
                            sResult += "SOLID";
                            break;
                        case drawing::FillStyle_GRADIENT:
                            sResult += "GRADIENT";
                            break;
                        case drawing::FillStyle_HATCH:
                            sResult += "HATCH";
                            break;
                        case drawing::FillStyle_BITMAP:
                            sResult += "BITMAP";
                            break;
                        case drawing::FillStyle_MAKE_FIXED_SIZE:
                            sResult += "MAKE_FIXED_SIZE";
                            break;
                        }
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.drawing.BitmapMode")))
                    {
                        drawing::BitmapMode nValue;
                        aAny >>= nValue;

                        switch (nValue)
                        {
                        case drawing::BitmapMode_REPEAT:
                            sResult += "REPEAT";
                            break;
                        case drawing::BitmapMode_STRETCH:
                            sResult += "STRETCH";
                            break;
                        case drawing::BitmapMode_NO_REPEAT:
                            sResult += "NO_REPEAT";
                            break;
                        case drawing::BitmapMode_MAKE_FIXED_SIZE:
                            sResult += "MAKE_FIXED_SIZE";
                            break;
                        }
                    }
                    else if (sPropType ==
                             ::rtl::OUString
                             (RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.drawing.HomogenMatrix3")))
                    {
                        drawing::HomogenMatrix3 aMatrix;
                        aAny >>= aMatrix;

                        snprintf(buffer, sizeof(buffer),
                                 "((%f %f %f)(%f %f %f)(%f %f %f))",
                                 aMatrix.Line1.Column1,
                                 aMatrix.Line1.Column2,
                                 aMatrix.Line1.Column3,
                                 aMatrix.Line2.Column1,
                                 aMatrix.Line2.Column2,
                                 aMatrix.Line2.Column3,
                                 aMatrix.Line3.Column1,
                                 aMatrix.Line3.Column2,
                                 aMatrix.Line3.Column3);
                        sResult += buffer;
                    }

                    sResult += "</property>";
                }
            }
        }
        sResult += "</propertyset>";
    }

    return sResult;
}
}
