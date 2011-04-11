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
#ifndef CONFIG_HXX
#define CONFIG_HXX

#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include "format.hxx"

#include "cfgitem.hxx"


class SmPreferenceDialog;
class SmPrintDialog;
class SmPrintOptionDialog;
class SfxItemSet;

class SmConfig : public SmMathConfig, public SfxBroadcaster
{
    SmFontPickList  vFontPickList[7];

public:
    SmConfig();
    virtual ~SmConfig();

    SmFontPickList & GetFontPickList(sal_uInt16 nIdent) { return vFontPickList[nIdent]; }

    void ItemSetToConfig(const SfxItemSet &rSet);
    void ConfigToItemSet(SfxItemSet &rSet) const;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
