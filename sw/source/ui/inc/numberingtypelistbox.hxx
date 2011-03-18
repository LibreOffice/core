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
#ifndef _NUMBERINGTYPELISTBOX_HXX
#define _NUMBERINGTYPELISTBOX_HXX

#include <vcl/lstbox.hxx>
#include "swdllapi.h"

#define INSERT_NUM_TYPE_NO_NUMBERING                0x01
#define INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING        0x02
#define INSERT_NUM_TYPE_BITMAP                      0x04
#define INSERT_NUM_TYPE_BULLET                      0x08
#define INSERT_NUM_EXTENDED_TYPES                   0x10

struct SwNumberingTypeListBox_Impl;

class SW_DLLPUBLIC SwNumberingTypeListBox : public ListBox
{
    SwNumberingTypeListBox_Impl* pImpl;

public:
    SwNumberingTypeListBox( Window* pWin, const ResId& rResId,
        sal_uInt16 nTypeFlags = INSERT_NUM_TYPE_PAGE_STYLE_NUMBERING|INSERT_NUM_TYPE_NO_NUMBERING|INSERT_NUM_EXTENDED_TYPES );
    ~SwNumberingTypeListBox();

    void        Reload(sal_uInt16 nTypeFlags);

    sal_Int16   GetSelectedNumberingType();
    sal_Bool    SelectNumberingType(sal_Int16 nType);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
