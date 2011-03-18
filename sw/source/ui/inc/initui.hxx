/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _INITUI_HXX
#define _INITUI_HXX

#include "tools/solar.h"
#include "swdllapi.h"

/*
 * Forward Declarations
 */
class String;
class SwThesaurus;
class SvStringsDtor;

/*
 * Extern Definitions
 */
extern  SwThesaurus*    pThes;

extern  String*         pOldGrfCat;
extern  String*         pOldTabCat;
extern  String*         pOldFrmCat;

extern  String*         pCurrGlosGroup;

SW_DLLPUBLIC String* GetOldGrfCat();
SW_DLLPUBLIC String* GetOldTabCat();
SW_DLLPUBLIC String* GetOldFrmCat();
SW_DLLPUBLIC String* GetOldDrwCat();

SW_DLLPUBLIC String* GetCurrGlosGroup();
SW_DLLPUBLIC void SetCurrGlosGroup(String* pStr);

extern SvStringsDtor*   pDBNameList;

extern SvStringsDtor*   pAuthFieldNameList;
extern SvStringsDtor*   pAuthFieldTypeList;

// stellt die Textbausteinverwaltung zur Verfuegung
class SwGlossaries;
SW_DLLPUBLIC SwGlossaries* GetGlossaries();

class SwGlossaryList;

sal_Bool HasGlossaryList();
SwGlossaryList* GetGlossaryList();

extern  void _InitUI();
extern  void _FinitUI();
extern  void _InitSpell();
extern  void _FinitSpell();


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
