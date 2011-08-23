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

#ifndef _SO2DLL_HXX
#define _SO2DLL_HXX

#include <tools/pstm.hxx>
#include <tools/link.hxx>
#include <tools/globname.hxx>

#include "bf_so3/so3dllapi.h"

/*************************************************************************
*************************************************************************/
class SotFactory;
class SotFactoryList;
class SvAdviseSinkIndex;
class WorkWindow;
class MDIWindow;
class ImpSO2Clipboard;
class ResMgr;
class SvSoIPCServer;
class SvIPCServerInfo;
class Impl_DeathTimer;
class Timer;

namespace binfilter
{
class SvInPlaceClient;
class SvInPlaceClientList;
class SvInPlaceObjectList;
class ImplSvEditObjectProtocol;
class SvContainerEnvironmentList;
class SvInPlaceEnvironment;
class SvVerbList;
struct SvBindingData;
class SvObjectList;
class ImpFactory;

//=========================================================================
struct SO3_DLLPRIVATE ConvertTo_Impl
/*  [Beschreibung]

    In dieser Struktur werden Informationen "uber den Vorg"anger und den
    Nachfolger von Datenformaten und dazugeh"origen CLSId's gepflegt.
    Die Struktur kann nur in einem Array sinnvoll eingesetzt werden.
*/
{
    SvGlobalName    aName;          // CLSID    des Servers
    SvGlobalName    aSvName;        // Starview-Klasse die diese CLSID supported
    long            aFormat;        // Format

    ConvertTo_Impl() {};
    ConvertTo_Impl( const SvGlobalName & a1, long a2 )
        : aName( a1 )
        , aSvName( a1 )
        , aFormat( a2 )
    {}
    ConvertTo_Impl( const SvGlobalName & a1,
                    const SvGlobalName & rSvClass,
                    long a2 )
        : aName( a1 )
        , aSvName( rSvClass )
        , aFormat( a2 )
    {}
};
#define SO3_OFFICE_VERSIONS	5

class SO3_DLLPUBLIC SoDll
{
friend class ImpFactory;
friend class SvFactory;
friend class Impl_DeathTimer;
    BOOL            bInit;
    BOOL            bSelfInit;
    ResMgr*         pResMgr;

public:
    // Factories
    SotFactory *     pSvObjectFactory;
    SotFactory *     pSvStorageStreamFactory;
    SotFactory *     pSvStorageFactory;
    SotFactory *     pSvEmbeddedObjectFactory;
    SotFactory *     pSvEmbeddedClientFactory;
    SotFactory *     pSvInPlaceObjectFactory;
    SotFactory *     pSvPlugInObjectFactory;
    SotFactory *     pSvAppletObjectFactory;
    SotFactory *     pSvInPlaceClientFactory;
    SotFactory *     pSvPersistFactory;
    SotFactory *     pSvPseudoObjectFactory;
    SotFactory *     pSvSimplePersistFactory;
    SotFactory *     pSvObjectContainerFactory;
    SotFactory *     pSvFactory_ImplFactory;
    SotFactory *	 pDfltPlugInFactory;
    SotFactory *	 pDfltAppletFactory;
    SotFactory *	 pSoBindingFactory;

    // Listen
    SvContainerEnvironmentList *pContEnvList;

    // InPlace-Bearbeitung
    SvInPlaceClientList *		pIPActiveClientList;
    SvInPlaceObjectList *		pIPActiveObjectList;
    SvInPlaceEnvironment * 		pUIShowIPEnv;
    ImplSvEditObjectProtocol *	pIEOPDflt;

    // Sonstiges
    SvClassManager  			aInfoClassMgr;
    SvGlobalName    			aSvInterface;

    // PlugIn
    SvVerbList *				pPlugInVerbList;
    ULONG						nPlugInDocFormat;

    // PlugIn
    SvVerbList *				pAppletVerbList;
    ULONG						nAppletDocFormat;

    Timer *						pDeathTimer;
    void * 						pDead;
    SvBindingData*              pBindingData;
    ConvertTo_Impl 				(*pConvTable)[SO3_OFFICE_VERSIONS];
    USHORT						nConvTableEntries;

                            SoDll();
                            ~SoDll();
    ResMgr *    			GetResMgr();
    SvIPCServerInfo * 		GetIPCServerInfo();

    static SoDll*			GetOrCreate();
};

}
#endif //_SO2DLL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
