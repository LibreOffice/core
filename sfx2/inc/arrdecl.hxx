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
#ifndef _SFX_ARRDECL_HXX
#define _SFX_ARRDECL_HXX

#include <sfx2/minarray.hxx>
#include <vector>

class SfxObjectShell;
class SfxObjectShellArr_Impl : public std::vector<SfxObjectShell*> {};

class SfxViewFrame;
class SfxViewFrameArr_Impl : public std::vector<SfxViewFrame*> {};

class SfxViewShell;
class SfxViewShellArr_Impl : public std::vector<SfxViewShell*> {};

struct SfxTbxCtrlFactory;
class SfxTbxCtrlFactArr_Impl : public std::vector<SfxTbxCtrlFactory*>
{
public:
    // de-allocates child objects
    ~SfxTbxCtrlFactArr_Impl();
};

struct SfxStbCtrlFactory;
class SfxStbCtrlFactArr_Impl : public std::vector<SfxStbCtrlFactory*>
{
public:
    // de-allocates child objects
    ~SfxStbCtrlFactArr_Impl();
};

struct SfxMenuCtrlFactory;
class SfxMenuCtrlFactArr_Impl : public std::vector<SfxMenuCtrlFactory*>
{
public:
    // de-allocates child objects
    ~SfxMenuCtrlFactArr_Impl();
};

struct SfxChildWinFactory;
class SfxChildWinFactArr_Impl : public std::vector<SfxChildWinFactory*>
{
public:
    // de-allocates child objects
    ~SfxChildWinFactArr_Impl();
};

class SfxModule;
typedef ::std::vector<SfxModule*> SfxModuleArr_Impl;

class SfxFrame;
typedef ::std::vector<SfxFrame*> SfxFrameArr_Impl;

class SfxFilter;
typedef ::std::vector< SfxFilter* > SfxFilterList_Impl;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
