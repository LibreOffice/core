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
#ifndef _CNTTAB_HXX
#define _CNTTAB_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/edit.hxx>

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

#include "tox.hxx"
#include <toxmgr.hxx>
#include <svx/checklbx.hxx>
#include <tools/resary.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/menubtn.hxx>
#include <svx/langbox.hxx>

#define TOX_PAGE_SELECT 1
#define TOX_PAGE_ENTRY  2
#define TOX_PAGE_STYLES 3

struct CurTOXType
{
    TOXTypes    eType;
    sal_uInt16      nIndex; //for TOX_USER only

    sal_Bool operator==(const CurTOXType aCmp)
       {
        return eType == aCmp.eType && nIndex == aCmp.nIndex;
       }
    sal_uInt16 GetFlatIndex() const;

    CurTOXType () : eType (TOX_INDEX), nIndex (0) {};

    CurTOXType (TOXTypes t, sal_uInt16 i) : eType (t), nIndex (i) {};
};

class SwOLENames : public Resource
{
    ResStringArray      aNamesAry;
public:
    SwOLENames(const ResId& rResId) :
        Resource(rResId),
        aNamesAry(ResId(1,*rResId.GetResMgr())){FreeResource();}

    ResStringArray&     GetNames() { return aNamesAry;}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
