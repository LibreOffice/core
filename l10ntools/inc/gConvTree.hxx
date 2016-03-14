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

#ifndef GCONTREE_HXX
#define GCONTREE_HXX
#include "gConv.hxx"



/*****************************************************************************
 ************************   G C O N T R E E . H X X   ************************
 *****************************************************************************
 * This is the class header for .tree conversion
 *****************************************************************************/



/********************   C L A S S   D E F I N I T I O N   ********************/
class convert_tree : public convert_gen
{
  public:
    typedef enum
    {
      STATE_TAG_NONE,
      STATE_TAG_HELPSEC,
      STATE_TAG_NODE,
      STATE_TAG_TOPIC,
      STATE_TAG_VALUE
    } STATE_TAG;
    typedef enum
    {
      STATE_VAL_NONE,
      STATE_VAL_APPL,
      STATE_VAL_ID,
      STATE_VAL_TITLE
    } STATE_VAL;

    convert_tree(l10nMem& crMemory);
    virtual ~convert_tree();

    void         setString         (char *yytext);
    void         setState          (char *yytext, STATE_TAG eNewStateTag, STATE_VAL eNewStateVAL);
    void         setValue          (char *yytext);
    std::string& copySourceSpecial (char *yytext, int iType);
    void         writeSourceFile   (std::string& sText, int inx);


  private:
    std::string     msLine;
    std::string     msId;
    std::string     msAppl;
    std::ofstream  *mcOutputFiles;
    STATE_TAG       meStateTag;
    STATE_VAL       meStateVal;
    int             miCntLanguages;

    void execute() override;
};
#endif
