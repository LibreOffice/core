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

#ifndef _SVTOOLS_TTPROPS_HXX
#define _SVTOOLS_TTPROPS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _RTTI_HXX // manual
#include <tools/rtti.hxx>
#endif
#ifndef _VCL_APPTYPES_HXX //autogen
#include <vcl/apptypes.hxx>
#endif

#ifndef _SVTOOLS_SVTDATA_HXX
#include <bf_svtools/svtdata.hxx>
#endif

class Bitmap;

namespace binfilter
{

class SfxPoolItem;

// Defines zur Steuerung der Ausführung von Slots
#define EXECUTE_NO					0
#define EXECUTE_POSSIBLE			1
#define EXECUTE_YES					2
#define EXECUTEMODE_ASYNCHRON		1
#define EXECUTEMODE_DIALOGASYNCHRON	2

#define SFX_USE_BINDINGS		0x8000

// Property Requests(PR)
#define TT_PR_ONCE		0x100

#define TT_PR_SLOTS		( 0x001 | TT_PR_ONCE )
#define TT_PR_DISPATCHER (0x002 )
#define TT_PR_IMG		( 0x004 )

#define TT_PR_ERR_NODISPATCHER	01
#define TT_PR_ERR_NOEXECUTE		02

///	To detect inconsistencies
#define TT_PROPERTIES_VERSION	1

class  TTProperties : public ApplicationProperty
{
    USHORT nDonePRs;					// Verwaltung für die Properties, die nur einmal gerufen werden mussen.
    BOOL RequestProperty( USHORT nRequest );

    BOOL HasSlots(){ return nPropertyVersion == TT_PROPERTIES_VERSION; }

public:
    TYPEINFO();

    TTProperties()
            : nDonePRs( 0 )
            , mppArgs( NULL )
            , mpBmp( NULL )
            , nPropertyVersion( 0 )
                    {}

    BOOL GetSlots();

    /// ExecuteFunction
    USHORT mnSID;
    SfxPoolItem** mppArgs;
    USHORT mnMode;
    USHORT ExecuteFunction( USHORT nSID, SfxPoolItem** ppArgs = NULL, USHORT nMode = 0 );

    /// Img
    Bitmap *mpBmp;
    BOOL Img( Bitmap *pBmp );

    static SvtResId GetSvtResId( USHORT nId );

    ULONG nSidOpenUrl;
    ULONG nSidFileName;
    ULONG nSidNewDocDirect;
    ULONG nSidCopy;
    ULONG nSidPaste;
    ULONG nSidSourceView;
    ULONG nSidSelectAll;
    ULONG nSidReferer;

    USHORT nActualPR;
    USHORT nPropertyVersion;		// Wird bei jedem call gesetzt.
};

}

#endif // _SVTOOLS_TTPROPS_HXX

