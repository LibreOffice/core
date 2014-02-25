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

#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <fltini.hxx>
#include <w1par.hxx>
#include <swfltopt.hxx>
#include <mdiexp.hxx>
#include <swerror.h>
#include <statstr.hrc>

// Initializing the field FilterFlags
static sal_uLong WW1_Read_FieldIniFlags()
{
    // sal_uInt16 i;
    static const sal_Char* aNames[ 1 ] = { "WinWord/WW1F" };
    sal_uInt32 aVal[ 1 ];
    SwFilterOptions aOpt( 1, aNames, aVal );
    sal_uLong nFieldFlags = aVal[ 0 ];

    if ( SwFltGetFlag( nFieldFlags, SwFltControlStack::HYPO ) )
    {
        SwFltSetFlag( nFieldFlags, SwFltControlStack::BOOK_TO_VAR_REF );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_DO_ID );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::TAGS_IN_TEXT );
        SwFltSetFlag( nFieldFlags, SwFltControlStack::ALLOW_FLD_CR );
    }
    return nFieldFlags;
}

// StarWriter-Interface
// One method returns the call interface for the Writer.
// Read() reads a file. For this, two objects get created. The Shell,
// which collects all information, and the manager, which reads it
// from the file.
sal_uLong WW1Reader::Read(SwDoc& rDoc, const OUString& rBaseURL, SwPaM& rPam, const OUString& /*cName*/)
{
    sal_uLong nRet = ERR_SWG_READ_ERROR;
    OSL_ENSURE(pStrm!=NULL, "W1-Read ohne Stream");
    if (pStrm != NULL)
    {
        sal_Bool bNew = !bInsertMode;           // New Doc ( no insert )

        sal_uLong nFieldFlags = WW1_Read_FieldIniFlags();
        Ww1Shell* pRdr = new Ww1Shell( rDoc, rPam, rBaseURL, bNew, nFieldFlags );
        if( pRdr )
        {
            Ww1Manager* pMan = new Ww1Manager( *pStrm, nFieldFlags );
            if( pMan )
            {
                if( !pMan->GetError() )
                {
                    ::StartProgress( STR_STATSTR_W4WREAD, 0, 100,
                                        rDoc.GetDocShell() );
                    ::SetProgressState( 0, rDoc.GetDocShell() );
                    // just push everything over now
                    *pRdr << *pMan;
                    if( !pMan->GetError() )
                        // signal absence of errors
                        nRet = 0; // would be better: WARN_SWG_FEATURES_LOST;
                    ::EndProgress( rDoc.GetDocShell() );
                }
                else
                {
                    if( pMan->GetFib().GetFIB().fComplexGet() )
                        nRet = ERR_WW6_FASTSAVE_ERR;
                }
            }
            delete pMan;
        }
        delete pRdr;
    }
    Ww1Sprm::DeinitTab();
    return nRet;
}

// The shell is the interface from filter to Writer. It is derived
// from SwFltShell and contains all relevant extensions for ww1.
// SwFltShell is used in common with ww-filter. Information read from
// the file gets 'piped' into the shell, like would be done for a
// stream.
Ww1Shell::Ww1Shell( SwDoc& rD, SwPaM& rPam, const OUString& rBaseURL, sal_Bool bNew, sal_uLong nFieldFlags)
    : SwFltShell(&rD, rPam, rBaseURL, bNew, nFieldFlags)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
