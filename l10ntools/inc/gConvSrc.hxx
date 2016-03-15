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
#ifndef GCONSRCHXX
#define GCONSRCHXX
#include "gConv.hxx"



class convert_src : public convert_gen
{
    public:
        bool mbExpectValue;

        convert_src(l10nMem& crMemory);
        ~convert_src();

        void setValue      (char *syyText, char *sbuildValue);
        void setLang       (char *syyText, bool bEnUs);
        void setId         (char *syyText, bool bIde);
        void setText       (char *syyText);
        void setName       (char *syyText);
        void setCmd        (char *syyText);
        void setMacro      (char *syyText);
        void setList       (char *syyText);
        void setListItem   (char const *syyText, bool bIsStart);
        void setNL         (char *syyText, bool bMacro);
        void startBlock    (char *syyText);
        void stopBlock     (char *syyText);

    private:
        std::vector<std::string> mcStack;
        std::string              msValue;
        std::string              msName;
        std::string              msTextName;
        std::string              msCmd;
        bool                     mbEnUs;
        bool                     mbExpectName;
        bool                     mbExpectMacro;
        bool                     mbAutoPush;
        bool                     mbValuePresent;
        bool                     mbInList;
        bool                     mbInListItem;
        int                      miListCount;
        int                      miMacroLevel;
        void execute() override;
        void trim(std::string& sText);
        void buildKey(std::string& sKey);
        void insertLanguagePart(std::string& sKey, std::string& sTextType);
};
#endif
