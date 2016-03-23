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
#ifndef GCONXCU_HXX
#define GCONXCU_HXX
#include "gConv.hxx"



extern int xculex(void);



class xcu_stack_entry;



class convert_xcu : public convert_gen
{
    public:
        bool mbNoCollectingData;

        convert_xcu(l10nMem& crMemory);
        ~convert_xcu() override {};

        void pushKey(char *syyText);
        void popKey(char *syyText);

        void startCollectData(char *syyText);
        void stopCollectData(char *syyText);
        void copySpecial(char *syyText);
        void copyNL(char *syyText);
        void addLevel();

    private:
        vector<string> mcStack;
        int                      miLevel;
        bool                     mbNoTranslate;

        void doExecute() override;
};
#endif
