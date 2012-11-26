/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVTOOLS_TTPROPS_HXX
#define _SVTOOLS_TTPROPS_HXX

#include "svtools/svtdllapi.h"
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
    TTProperties()
            : nDonePRs( 0 )
            , mppArgs( NULL )
            , mpBmp( NULL )
            , nPropertyVersion( 0 )
                    {}
    virtual ~TTProperties();

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

