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

#include <tools/list.hxx>

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif
#include "bf_sfx2/minarray.hxx"
namespace binfilter {

class SfxObjectShell;
SV_DECL_PTRARR( SfxObjectShellArr_Impl, SfxObjectShell*, 4, 4 )//STRIP008 ;

class SfxObjectFactory;
typedef SfxObjectFactory* SfxObjectFactoryPtr;
SV_DECL_PTRARR( SfxObjectFactoryArr_Impl, SfxObjectFactoryPtr, 3, 3 )//STRIP008 ;

class SfxModule;
SV_DECL_PTRARR( SfxModuleArr_Impl, SfxModule*, 2, 2 )//STRIP008 ;

class SfxFilter;
DECL_PTRARRAY( SfxFilterArr_Impl, SfxFilter*, 4, 4 )//STRIP008 ;

DECLARE_LIST( SfxFilterList_Impl, SfxFilter* )//STRIP008 DECLARE_LIST( SfxFilterList_Impl, SfxFilter* );

class SfxSlot;
typedef SfxSlot* SfxSlotPtr;
SV_DECL_PTRARR( SfxSlotArr_Impl, SfxSlotPtr, 20, 20 )//STRIP008 ;

}//end of namespace binfilter
#endif
