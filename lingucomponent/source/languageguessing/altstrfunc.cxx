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

#include "altstrfunc.hxx"

#include <sal/types.h>

int start(const std::string &s1, const std::string &s2){
    size_t i;
    int ret = 0;

    size_t min = s1.length();
    if (min > s2.length())
        min = s2.length();

    for(i = 0; i < min && s2[i] && s1[i] && !ret; i++){
        ret = toupper(s1[i]) - toupper(s2[i]);
        if(s1[i] == '.' || s2[i] == '.'){ret = 0;}//. is a neutral character
    }
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
