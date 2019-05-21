/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#ifndef INCLUDED_CUI_SOURCE_INC_CUSTOMNOTEBOOKBARGENERATOR_HXX
#define INCLUDED_CUI_SOURCE_INC_CUSTOMNOTEBOOKBARGENERATOR_HXX

#include "cfg.hxx"

using namespace css::uno;

class CustomNotebookbarGenerator
{
public:
    CustomNotebookbarGenerator();
    static OUString getCustomizedUIPath();
    static char* convertToCharPointer(const OUString& sString);
    static Sequence<OUString> getCustomizedUIItem(OUString sNotebookbarConfigType);
    static void getFileNameAndAppName(OUString& sAppName, OUString& sNotebookbarUIFileName);
    static void modifyCustomizedUIFile(Sequence<OUString> sUIItemProperties);
    static void createCustomizedUIFile();
    static void setCustomizedUIItem(Sequence<OUString> sUIItemProperties,
                                    OUString sNotebookbarConfigType);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */