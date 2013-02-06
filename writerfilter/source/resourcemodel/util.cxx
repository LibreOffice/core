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
        OUString aOUStr = textRange->getString();
        OString aOStr(aOUStr.getStr(), aOUStr.getLength(),  RTL_TEXTENCODING_ASCII_US );

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
