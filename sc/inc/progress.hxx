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

#ifndef SC_PROGRESS_HXX
#define SC_PROGRESS_HXX

#include <sfx2/progress.hxx>
#include <tools/string.hxx>
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
    static  sal_uLong           nGlobalRange;
    static  sal_uLong           nGlobalPercent;
    static  sal_Bool            bGlobalNoUserBreak;
    static  ScProgress*     pInterpretProgress;
    static  ScProgress*     pOldInterpretProgress;
    static  sal_uLong           nInterpretProgress;
    static  sal_Bool            bAllowInterpretProgress;
    static  ScDocument*     pInterpretDoc;
    static  bool            bIdleWasEnabled;

            SfxProgress*    pProgress;

                            /// not implemented
                            ScProgress( const ScProgress& );
            ScProgress&     operator=( const ScProgress& );

    static  void            CalcGlobalPercent( sal_uLong nVal )
                                {
                                    nGlobalPercent = nGlobalRange ?
                                        nVal * 100 / nGlobalRange : 0;
                                }

public:
    static  SfxProgress*    GetGlobalSfxProgress() { return pGlobalProgress; }
    static  sal_Bool            IsUserBreak() { return !bGlobalNoUserBreak; }
    static  void            CreateInterpretProgress( ScDocument* pDoc,
                                                    sal_Bool bWait = sal_True );
    static  ScProgress*     GetInterpretProgress() { return pInterpretProgress; }
    static  void            DeleteInterpretProgress();
    static  sal_uLong           GetInterpretCount() { return nInterpretProgress; }
    static  sal_uLong           GetGlobalRange()    { return nGlobalRange; }
    static  sal_uLong           GetGlobalPercent()  { return nGlobalPercent; }

                            ScProgress( SfxObjectShell* pObjSh,
                                         const String& rText,
                                         sal_uLong nRange, sal_Bool bAllDocs = false,
                                         sal_Bool bWait = sal_True );
                            ~ScProgress();

#ifdef SC_PROGRESS_CXX
                            /// for DummyInterpret only, never use otherwise!!!
                            ScProgress();
#endif
                            /// might be NULL!
            SfxProgress*    GetSfxProgress() const { return pProgress; }

            sal_Bool            SetStateText( sal_uLong nVal, const String &rVal, sal_uLong nNewRange = 0 )
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
                                    return sal_True;
                                }
            sal_Bool            SetState( sal_uLong nVal, sal_uLong nNewRange = 0 )
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
                                    return sal_True;
                                }
            sal_Bool            SetStateCountDown( sal_uLong nVal )
                                {
                                    if ( pProgress )
                                    {
                                        CalcGlobalPercent( nGlobalRange - nVal );
                                        if ( !pProgress->SetState( nGlobalRange - nVal ) )
                                            bGlobalNoUserBreak = false;
                                        return bGlobalNoUserBreak;
                                    }
                                    return sal_True;
                                }
            sal_Bool            SetStateOnPercent( sal_uLong nVal )
                                {   /// only if percentage increased
                                    if ( nGlobalRange && (nVal * 100 /
                                            nGlobalRange) > nGlobalPercent )
                                        return SetState( nVal );
                                    return sal_True;
                                }
            sal_Bool            SetStateCountDownOnPercent( sal_uLong nVal )
                                {   /// only if percentage increased
                                    if ( nGlobalRange &&
                                            ((nGlobalRange - nVal) * 100 /
                                            nGlobalRange) > nGlobalPercent )
                                        return SetStateCountDown( nVal );
                                    return sal_True;
                                }
            sal_uLong           GetState()
                                {
                                    if ( pProgress )
                                        return pProgress->GetState();
                                    return 0;
                                }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
