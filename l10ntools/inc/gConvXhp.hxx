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
#ifndef GCONXHP_HXX
#define GCONXHP_HXX
#include "gConv.hxx"



extern int xhplex(void);



class convert_xhp : public convert_gen
{
    public:
        convert_xhp(l10nMem& crMemory);
        ~convert_xhp() override;

        void setString(char *yytext);
        void openTag(char *yytext);
        void closeTag(char *yytext);
        void closeTagNOvalue(char *yytext);
        void setId(char *yytext);
        void setLang(char *yytext);
        void setRef(char *yytext);
        void openTransTag(char *yytext);
        void closeTransTag(char *yytext);
        void stopTransTag(char *yytext);
        void startComment(char *yytext);
        void stopComment(char *yytext);
        void handleSpecial(char *yytext);
        void handleDataEnd(char *yytext);
        void duplicate(char *yytext);
        string& copySourceSpecial(char *yytext, int iType);
        void writeSourceFile(string& sText, int inx);

    private:
        typedef enum {
            VALUE_NOT_USED,
            VALUE_IS_TAG,
            VALUE_IS_TAG_TRANS,
            VALUE_IS_VALUE,
            VALUE_IS_VALUE_TAG
        } STATE;
        STATE       meExpectValue, mePushValue;
        string msKey, msPushCollect;
        string msLine;
        string    *msLangText;
        ofstream  *mcOutputFiles;
        int             miCntLanguages;
        void            doExecute() override;
};
#endif
