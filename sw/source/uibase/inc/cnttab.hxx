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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CNTTAB_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CNTTAB_HXX

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
#include <svtools/treelistbox.hxx>
#include <vcl/menubtn.hxx>
#include <svx/langbox.hxx>

#define TOX_PAGE_SELECT 1
#define TOX_PAGE_ENTRY  2
#define TOX_PAGE_STYLES 3

#define TO_CONTENT      1
#define TO_INDEX        2
#define TO_ILLUSTRATION 4
#define TO_TABLE        8
#define TO_USER         16
#define TO_OBJECT       32
#define TO_AUTHORITIES  64
#define TO_BIBLIOGRAPHY 128

struct CurTOXType
{
    TOXTypes    eType;
    sal_uInt16      nIndex; //for TOX_USER only

    bool operator==(const CurTOXType& rCmp)
    {
        return eType == rCmp.eType && nIndex == rCmp.nIndex;
    }
    sal_uInt16 GetFlatIndex() const;

    CurTOXType () : eType (TOX_INDEX), nIndex (0) {};

    CurTOXType (TOXTypes t) : eType (t), nIndex (0) {};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
