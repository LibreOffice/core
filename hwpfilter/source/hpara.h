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

#pragma once

#include "hwplib.h"
#include "hwpfile.h"
#include "hinfo.h"
#include <tools/long.hxx>
#include <map>
#include <memory>
#include <vector>

struct HBox;

/**
 * etc flag
 * 0 bit : Use page columns
 * 1 bit : Use page lows
 * 2 bit : Use section break
 * 3 bit : Block start
 * 4 bit : In Block
 * 5 bit : Block end
 * 6 bit : Preserve widow orphan
 * 7 bit : Reserved
 */
enum
{
    PA_USER_COLUMN    =   1,
    PA_USER_PAGE      =   2,                      /* p user_page definition */
    PA_SECTION_BREAK  =   4,
    PA_BLOCK_BEGIN    =   8,
    PA_IN_BLOCK       =   16,
    PA_BLOCK_END      =   32,
    PA_WIDOW_ORPHAN   =   64
};

class HWPPara;

struct LineInfo
{
    hunit         pgy;                            /* internal */

    void  Read(HWPFile &hwpf, HWPPara const *para);
};
/**
 * It represents the paragraph.
 * @short Paragraph
 */
class DLLEXPORT HWPPara
{
    private:
        HWPPara       *_next;

    public:
// paragraph information
/**
 * Zero is for the new paragraph style.
 */
        unsigned char     reuse_shape;            /* the new shape if 0 */
        unsigned short    nch;
        unsigned short    nline;

// realking
        hunit         begin_ypos;
        unsigned char     scflag;

/**
 * If the value is 0, all character of paragraph have same style given cshape
 */
        unsigned char     contain_cshape;         /* if it's 0, all the characters are representing with representative shape */
        unsigned char     etcflag;
/**
 * Checks the special characters in the paragraph
 */
        uint              ctrlflag;
        unsigned char     pstyno;
        std::shared_ptr<CharShape> cshape;                     /* When characters are all the same shape */
        std::shared_ptr<ParaShape> pshape;                     /* if reuse flag is 0, */

        std::unique_ptr<LineInfo[]> linfo;
        std::vector<std::shared_ptr<CharShape>>   cshapep;
/**
 * Box object list
 */
        std::vector<std::unique_ptr<HBox>> hhstr;

        HWPPara(void);
        ~HWPPara(void);

        bool  Read(HWPFile &hwpf, unsigned char flag);

        void  SetNext(HWPPara *n) { _next = n; };

/* Functions for layout */
/**
 * Returns the character style of paragraph.
 */
        CharShape *GetCharShape(int pos);
/**
 * Returns the style of paragraph.
 */
        ParaShape& GetParaShape(void) { return *pshape; }

/**
 * Returns next paragraph.
 */
        HWPPara *Next(void) { return _next;}

    private:
        std::unique_ptr<HBox> readHBox(HWPFile &);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
