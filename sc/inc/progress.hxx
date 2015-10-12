/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_INC_PROGRESS_HXX
#define INCLUDED_SC_INC_PROGRESS_HXX

#include <rtl/ustring.hxx>
#include <sfx2/progress.hxx>
#include "scdllapi.h"

class ScDocument;

/*
 * #i102566
 * Drawing a progress bar update is not cheap, so if we draw it on every
 * percentage change of 200 calculations we get one progress draw per 2
 * calculations which is slower than doing the calculations themselves. So as a
 * rough guide only do an update per MIN_NO_CODES_PER_PROGRESS_UPDATE
 * calculations
 */
#define MIN_NO_CODES_PER_PROGRESS_UPDATE 100

class SC_DLLPUBLIC ScProgress
{
private:
    static  SfxProgress*    pGlobalProgress;
    static  sal_uLong        nGlobalRange;
    static  sal_uLong       nGlobalPercent;
    static  bool            bGlobalNoUserBreak;
    static  ScProgress*     pInterpretProgress;
    static  ScProgress*     pOldInterpretProgress;
    static  sal_uLong       nInterpretProgress;
    static  bool            bAllowInterpretProgress;
    static  ScDocument*     pInterpretDoc;
    static  bool            bIdleWasEnabled;
            bool            bEnabled;

            SfxProgress*    pProgress;

                            ScProgress( const ScProgress& ) = delete;
            ScProgress&     operator=( const ScProgress& ) = delete;

    static  void            CalcGlobalPercent( sal_uLong nVal )
                                {
                                    nGlobalPercent = nGlobalRange ?
                                        nVal * 100 / nGlobalRange : 0;
                                }

public:
    static  bool            IsUserBreak() { return !bGlobalNoUserBreak; }
    static  void            CreateInterpretProgress( ScDocument* pDoc,
                                                    bool bWait = true );
    static  ScProgress*     GetInterpretProgress() { return pInterpretProgress; }
    static  void            DeleteInterpretProgress();

                            ScProgress( SfxObjectShell* pObjSh,
                                         const OUString& rText,
                                         sal_uLong nRange, bool bAllDocs = false,
                                         bool bWait = true );
                            ~ScProgress();

#ifdef SC_PROGRESS_CXX
                            /// for DummyInterpret only, never use otherwise!!!
                            ScProgress();
#endif

            bool            SetStateText( sal_uLong nVal, const OUString &rVal, sal_uLong nNewRange = 0 )
                                {
                                    if ( pProgress )
                                    {
                                        if ( nNewRange )
                                            nGlobalRange = nNewRange;
                                        CalcGlobalPercent( nVal );
                                        if ( !pProgress->SetStateText( nVal, rVal, nNewRange ) )
                                            bGlobalNoUserBreak = false;
                                        return bGlobalNoUserBreak;
                                    }
                                    return true;
                                }
            bool            SetState( sal_uLong nVal, sal_uLong nNewRange = 0 )
                                {
                                    if ( pProgress )
                                    {
                                        if ( nNewRange )
                                            nGlobalRange = nNewRange;
                                        CalcGlobalPercent( nVal );
                                        if ( !pProgress->SetState( nVal, nNewRange ) )
                                            bGlobalNoUserBreak = false;
                                        return bGlobalNoUserBreak;
                                    }
                                    return true;
                                }
            bool            SetStateCountDown( sal_uLong nVal )
                                {
                                    if ( pProgress )
                                    {
                                        CalcGlobalPercent( nGlobalRange - nVal );
                                        if ( !pProgress->SetState( nGlobalRange - nVal ) )
                                            bGlobalNoUserBreak = false;
                                        return bGlobalNoUserBreak;
                                    }
                                    return true;
                                }
            bool            SetStateOnPercent( sal_uLong nVal )
                                {   /// only if percentage increased
                                    if ( nGlobalRange && (nVal * 100 /
                                            nGlobalRange) > nGlobalPercent )
                                        return SetState( nVal );
                                    return true;
                                }
            bool            SetStateCountDownOnPercent( sal_uLong nVal )
                                {   /// only if percentage increased
                                    if ( nGlobalRange &&
                                            ((nGlobalRange - nVal) * 100 /
                                            nGlobalRange) > nGlobalPercent )
                                        return SetStateCountDown( nVal );
                                    return true;
                                }
            sal_uLong           GetState()
                                {
                                    if ( pProgress )
                                        return pProgress->GetState();
                                    return 0;
                                }
            bool                Enabled() const { return bEnabled; }
            void                Disable() { bEnabled = false; }
            void                Enable() { bEnabled = true; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
