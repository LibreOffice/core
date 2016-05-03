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


extern int srclex(void);



class convert_src : public convert_gen
{
    public:
//        bool mbExpectValue;

        convert_src(l10nMem& crMemory);
        ~convert_src() override {};

        void setValue(string& syyText);
        bool setLang(string& syyText);
        void setName(string& syyText, bool revertLevel);
        void setCmd(string& syyText);
        void startBlock();
        void stopBlock();
        void defMacro();
        void endMacro();

//        void setId         (char *syyText, bool bIde);
//        void setText       (char *syyText);
//        void setList       (char *syyText);
//        void setListItem   (char const *syyText, bool bIsStart);
//        void setNL         (char *syyText, bool bMacro);

    private:
        vector<string> mcStack;
        int  miLevel;
        bool mbMacroActive;
        void doExecute() override;
#if 0
        string              msValue;
        string              msName;
        string              msTextName;
        string              msCmd;
        string              msGroup;
        bool                     mbEnUs;
        bool                     mbExpectName;
        bool                     mbAutoPush;
        bool                     mbValuePresent;
        bool                     mbInList;
        bool                     mbInListItem;
        int                      miListCount;

        static void trim(string& sText);
        void buildKey(string& sKey);
        void insertLanguagePart(string& sKey, string& sTextType);
#endif
};
#endif
