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
#ifndef _INITUI_HXX
#define _INITUI_HXX

#include "tools/solar.h"
#include "swdllapi.h"
#include "rtl/ustring.hxx"

#include <vector>

/*
 * Forward Declarations
 */
class String;
class SwThesaurus;

/*
 * Extern Definitions
 */
extern  SwThesaurus*    pThes;

SW_DLLPUBLIC OUString GetOldGrfCat();
SW_DLLPUBLIC void SetOldGrfCat(OUString sStr);
SW_DLLPUBLIC OUString GetOldTabCat();
SW_DLLPUBLIC void SetOldTabCat(OUString sStr);
SW_DLLPUBLIC OUString GetOldFrmCat();
SW_DLLPUBLIC void SetOldFrmCat(OUString sStr);
SW_DLLPUBLIC OUString GetOldDrwCat();
SW_DLLPUBLIC void SetOldDrwCat(OUString sStr);

SW_DLLPUBLIC OUString GetCurrGlosGroup();
SW_DLLPUBLIC void SetCurrGlosGroup(OUString sStr);

extern std::vector<String>*   pDBNameList;

// provides textblock management
class SwGlossaries;
SW_DLLPUBLIC SwGlossaries* GetGlossaries();

class SwGlossaryList;

bool HasGlossaryList();
SwGlossaryList* GetGlossaryList();

extern  void _InitUI();
extern  void _FinitUI();
extern  void _InitSpell();
extern  void _FinitSpell();


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
