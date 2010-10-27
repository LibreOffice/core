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
#ifndef _SXOPITM_HXX
#define _SXOPITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>

//------------------------------
// class SdrObjPrintableItem
//------------------------------
class SdrObjPrintableItem: public SdrYesNoItem {
public:
    SdrObjPrintableItem(bool bOn=false): SdrYesNoItem(SDRATTR_OBJPRINTABLE,bOn) {}
    SdrObjPrintableItem(SvStream& rIn): SdrYesNoItem(SDRATTR_OBJPRINTABLE,rIn) {}
};

class SdrObjVisibleItem: public SdrYesNoItem {
public:
    SdrObjVisibleItem(bool bOn=true): SdrYesNoItem(SDRATTR_OBJVISIBLE,bOn) {}
    SdrObjVisibleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_OBJVISIBLE,rIn) {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
