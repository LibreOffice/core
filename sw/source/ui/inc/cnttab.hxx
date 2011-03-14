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
#include <svtools/svtreebx.hxx>
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
