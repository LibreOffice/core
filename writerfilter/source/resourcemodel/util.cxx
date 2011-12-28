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

#if OSL_DEBUG_LEVEL > 1

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
