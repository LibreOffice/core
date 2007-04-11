/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ttprops.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:41:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVTOOLS_TTPROPS_HXX
#define _SVTOOLS_TTPROPS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _RTTI_HXX // manual
#include <tools/rtti.hxx>
#endif
#ifndef _VCL_APPTYPES_HXX //autogen
#include <vcl/apptypes.hxx>
#endif

#ifndef _SVTOOLS_SVTDATA_HXX
#include <svtools/svtdata.hxx>
#endif

class SfxPoolItem;
class Bitmap;

// Defines zur Steuerung der Ausführung von Slots
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
    USHORT nDonePRs;                    // Verwaltung für die Properties, die nur einmal gerufen werden mussen.
    SVT_DLLPRIVATE BOOL RequestProperty( USHORT nRequest );

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
    USHORT nPropertyVersion;        // Wird bei jedem call gesetzt.
};

#endif // _SVTOOLS_TTPROPS_HXX

