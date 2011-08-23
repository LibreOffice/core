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
#ifndef _SFXMODULE_HXX
#define _SFXMODULE_HXX	// intern
#define _SFXMOD_HXX		// extern

#include <bf_sfx2/shell.hxx>

#include <sal/types.h>

class ResMgr;

namespace binfilter {

class SfxObjectFactory;
class SfxObjectFactory;
class SfxModuleArr_Impl;
class SfxModule_Impl;
class SfxSlotPool;

class ISfxModule
{
public:
    virtual					~ISfxModule(){};
    virtual ResMgr*			GetResMgr() = 0;
};

//====================================================================

class SfxModule : public SfxShell, public ISfxModule
{
private:
    ResMgr*                     pResMgr;
    sal_Bool                    bDummy : 1;

protected:
    virtual sal_Bool                QueryUnload();

public:
                                TYPEINFO();

                                SfxModule( ResMgr* pMgrP, sal_Bool bDummy,
                                    SfxObjectFactory* pFactoryP, ... );
                                ~SfxModule();

    virtual	SfxModule*			Load();
    virtual void            	Free();
    sal_Bool						IsLoaded() const { return !bDummy; }

    virtual ResMgr*             GetResMgr();

#if _SOLAR__PRIVATE
    static SfxModuleArr_Impl&   GetModules_Impl();
#endif
};

#define SFX_IMPL_MODULE_LIBRARY( LibName )									\
                                                                            \
        extern "C" void _CDECLARE_ Init##LibName##Dll()                     \
        {																	\
            LibName##DLL::Init();											\
        }																	\
        extern "C" void _CDECLARE_ DeInit##LibName##Dll()                   \
        {																	\
            LibName##DLL::Exit();											\
        }

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
