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

#include "svtools/svtdllapi.h"
#include <tools/rtti.hxx>
#include <vcl/apptypes.hxx>
#include <svtools/svtdata.hxx>

class SfxPoolItem;
class Bitmap;

// Defines zur Steuerung der Ausf�hrung von Slots
#define EXECUTE_NO                  0
#define EXECUTE_POSSIBLE            1
#define EXECUTE_YES                 2
#define EXECUTEMODE_ASYNCHRON       1
#define EXECUTEMODE_DIALOGASYNCHRON 2

#define SFX_USE_BINDINGS        0x8000

// Property Requests(PR)
#define TT_PR_ONCE      0x100

#define TT_PR_SLOTS     ( 0x001 | TT_PR_ONCE )
#define TT_PR_DISPATCHER (0x002 )
#define TT_PR_IMG       ( 0x004 )

#define TT_PR_ERR_NODISPATCHER  01
#define TT_PR_ERR_NOEXECUTE     02

/// To detect inconsistencies
#define TT_PROPERTIES_VERSION   1

class SVT_DLLPUBLIC TTProperties : public ApplicationProperty
{
    sal_uInt16 nDonePRs;                    // Verwaltung f�r die Properties, die nur einmal gerufen werden mussen.
    SVT_DLLPRIVATE sal_Bool RequestProperty( sal_uInt16 nRequest );

    sal_Bool HasSlots(){ return nPropertyVersion == TT_PROPERTIES_VERSION; }

public:
    TYPEINFO();

    TTProperties()
            : nDonePRs( 0 )
            , mppArgs( NULL )
            , mpBmp( NULL )
            , nPropertyVersion( 0 )
                    {}

    sal_Bool GetSlots();

    /// ExecuteFunction
    sal_uInt16 mnSID;
    SfxPoolItem** mppArgs;
    sal_uInt16 mnMode;
    sal_uInt16 ExecuteFunction( sal_uInt16 nSID, SfxPoolItem** ppArgs = NULL, sal_uInt16 nMode = 0 );

    /// Img
    Bitmap *mpBmp;
    sal_Bool Img( Bitmap *pBmp );

    static SvtResId GetSvtResId( sal_uInt16 nId );

    sal_uLong nSidOpenUrl;
    sal_uLong nSidFileName;
    sal_uLong nSidNewDocDirect;
    sal_uLong nSidCopy;
    sal_uLong nSidPaste;
    sal_uLong nSidSourceView;
    sal_uLong nSidSelectAll;
    sal_uLong nSidReferer;

    sal_uInt16 nActualPR;
    sal_uInt16 nPropertyVersion;        // Wird bei jedem call gesetzt.
};

#endif // _SVTOOLS_TTPROPS_HXX

