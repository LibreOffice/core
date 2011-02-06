/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef SC_TABBGCOLOR_HXX
#define SC_TABBGCOLOR_HXX

#include "tools/color.hxx"
#include "address.hxx"

#include <vector>

struct ScUndoTabColorInfo
{
    SCTAB mnTabId;
    Color maOldTabBgColor;
    Color maNewTabBgColor;

    explicit ScUndoTabColorInfo(SCTAB nTab);
    ScUndoTabColorInfo(const ScUndoTabColorInfo& r);

    typedef ::std::vector<ScUndoTabColorInfo> List;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
