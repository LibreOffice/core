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

#include "bootstrp/prj.hxx"
#include <comphelper/string.hxx>

SimpleConfig::SimpleConfig(const String &rSimpleConfigFileName)
{
    aFileStream.Open(rSimpleConfigFileName, STREAM_READ);
}

SimpleConfig::~SimpleConfig()
{
    aFileStream.Close ();
}

rtl::OString SimpleConfig::getNext()
{
    if (aStringBuffer.isEmpty())
      while ((aStringBuffer = GetNextLine()) == "\t") ; //solange bis != "\t"
    if (aStringBuffer.isEmpty())
        return rtl::OString();

    rtl::OString aString = comphelper::string::getToken(aStringBuffer, 0, '\t');

    aStringBuffer = aStringBuffer.copy(aString.getLength()+1);

    aStringBuffer = comphelper::string::stripStart(aStringBuffer, '\t');

    return aString;
}

rtl::OString SimpleConfig::GetNextLine()
{
    rtl::OString aTmpStr;
    aFileStream.ReadLine(aTmpStr);
    if (aTmpStr[0] == '#')
        return rtl::OString('\t');
    return aTmpStr.trim().replace(' ', '\t');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
