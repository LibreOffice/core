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

/*
  Issue http://udk.openoffice.org/issues/show_bug.cgi?id=92388

  Mac OS X does not seem to support "__cxa__atexit", thus leading
  to the situation that "__attribute__((destructor))__" functions
  (in particular "rtl_{memory|cache|arena}_fini") become called
  _before_ global C++ object d'tors.

  Using a C++ dummy object instead.
*/

extern "C" void rtl_memory_fini (void);
extern "C" void rtl_cache_fini (void);
extern "C" void rtl_arena_fini (void);

struct RTL_Alloc_Fini
{
  ~RTL_Alloc_Fini() ;
};

RTL_Alloc_Fini::~RTL_Alloc_Fini()
{
  rtl_memory_fini();
  rtl_cache_fini();
  rtl_arena_fini();
}

static RTL_Alloc_Fini g_RTL_Alloc_Fini;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
