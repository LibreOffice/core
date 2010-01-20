/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typemap.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <sfx2/objitem.hxx>
#include <sfx2/msg.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/cntritem.hxx>
#include <svx/shdditem.hxx>
#include <svx/postitem.hxx>
#ifndef _SVX_CLIPBOARDCTL_HXX_
#include "clipfmtitem.hxx"
#endif
#include <svx/fhgtitem.hxx>
#include "fontitem.hxx"
#include <svx/charreliefitem.hxx>
#include <svx/escpitem.hxx>
#include <svx/colritem.hxx>
#include <svx/wrlmitem.hxx>
#include <svx/charscaleitem.hxx>
#include <svx/akrnitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/lspcitem.hxx>
#include <svx/langitem.hxx>
#include <svx/kernitem.hxx>
#include <svx/tstpitem.hxx>
#include <svx/adjitem.hxx>
#ifndef _SVX_EMPHITEM_HXX
#include <svx/emphitem.hxx>
#endif

#define SFX_TYPEMAP
#include "svxslots.hxx"

