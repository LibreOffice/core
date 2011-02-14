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

#ifndef SC_PROGRESS_HXX
#define SC_PROGRESS_HXX

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
    static  sal_uLong           nGlobalRange;
    static  sal_uLong           nGlobalPercent;
    static  sal_Bool            bGlobalNoUserBreak;
    static  ScProgress*     pInterpretProgress;
    static  ScProgress*     pOldInterpretProgress;
    static  sal_uLong           nInterpretProgress;
    static  sal_Bool            bAllowInterpretProgress;
    static  ScDocument*     pInterpretDoc;
    static  sal_Bool            bIdleWasDisabled;

            SfxProgress*    pProgress;

                            // not implemented
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
                                         sal_uLong nRange, sal_Bool bAllDocs = sal_False,
                                         sal_Bool bWait = sal_True );
                            ~ScProgress();

#ifdef SC_PROGRESS_CXX
                            // nur fuer DummyInterpret, sonst nie benutzen!!!
                            ScProgress();
#endif
                            // kann NULL sein!
            SfxProgress*    GetSfxProgress() const { return pProgress; }

            sal_Bool            SetStateText( sal_uLong nVal, const String &rVal, sal_uLong nNewRange = 0 )
                                {
                                    if ( pProgress )
                                    {
                                        if ( nNewRange )
                                            nGlobalRange = nNewRange;
                                        CalcGlobalPercent( nVal );
                                        if ( !pProgress->SetStateText( nVal, rVal, nNewRange ) )
                                            bGlobalNoUserBreak = sal_False;
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
                                            bGlobalNoUserBreak = sal_False;
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
                                            bGlobalNoUserBreak = sal_False;
                                        return bGlobalNoUserBreak;
                                    }
                                    return sal_True;
                                }
            sal_Bool            SetStateOnPercent( sal_uLong nVal )
                                {   // nur wenn Prozent mehr als vorher
                                    if ( nGlobalRange && (nVal * 100 /
                                            nGlobalRange) > nGlobalPercent )
                                        return SetState( nVal );
                                    return sal_True;
                                }
            sal_Bool            SetStateCountDownOnPercent( sal_uLong nVal )
                                {   // nur wenn Prozent mehr als vorher
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

