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
#include <stdio.h>
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
#include <resourcemodel/TagLogger.hxx>
#include <resourcemodel/util.hxx>

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

    string xmlify(const string & str)
    {
        string result = "";
        char sBuffer[16];
        
        for (string::const_iterator aIt = str.begin(); aIt != str.end(); ++aIt)
        {
            char c = *aIt;
            
            if (isprint(c) && c != '\"')
            {
                if (c == '<')
                    result += "&lt;";
                else if (c == '>')
                    result += "&gt;";
                else if (c == '&')
                    result += "&amp;";
                else
                    result += c;
            }
            else
            {
                snprintf(sBuffer, sizeof(sBuffer), "\\%03d", c);
                result += sBuffer;
            }
        }
        
        return result;
    }
    
#ifdef DEBUG
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
    
    static const ::rtl::OUString sMetaFile(RTL_CONSTASCII_USTRINGPARAM("MetaFile"));
    
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
                bool bPropertyFound = true;
                uno::Any aAny;
                try 
                {
                    if (sPropName == sMetaFile)
                        bPropertyFound = false;
                    else
                        xPropSet->getPropertyValue(sPropName) >>= aAny;					
                }
                catch (beans::UnknownPropertyException)
                {
                    bPropertyFound = false;
                }
                
                if (bPropertyFound)
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
            else
            {
                sResult += "<unknown-property>";
                sResult += OUStringToOString
                (sPropName, RTL_TEXTENCODING_ASCII_US).getStr();
                sResult += "</unknown-property>";
            }
        }
        sResult += "</propertyset>";
    }
    
    return sResult;
}

string toString(uno::Reference< text::XTextRange > textRange)
{
    string result;
    
    if (textRange.get()) 
    {
        rtl::OUString aOUStr = textRange->getString();
        rtl::OString aOStr(aOUStr.getStr(), aOUStr.getLength(),  RTL_TEXTENCODING_ASCII_US );
        
        result = aOStr.getStr();
    }
    else
    {
        result="(nil)";
    }
    
    return result;
}
    
string toString(const string & rString)
{
    return rString;
}
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
