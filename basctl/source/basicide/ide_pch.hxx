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
#include <svheader.hxx>

#define ITEMID_SEARCH           0
#define ITEMID_SIZE             0

#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/menu.hxx>

#include <svl/svarray.hxx>
#include <svl/itemset.hxx>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <svl/srchitem.hxx>

#define _BASIC_TEXTPORTIONS
#include <basic/sbdef.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/basmgr.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/event.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/progress.hxx>
#include <editeng/sizeitem.hxx>


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
