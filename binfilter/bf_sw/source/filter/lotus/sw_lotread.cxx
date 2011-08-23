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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <tools/debug.hxx>

#include "fltglbls.hxx"
#include "lotpar.hxx"
namespace binfilter {



// ---------------------------------------------- SwLotusParser::Parse() -
void SwLotusParser::Parse( void )
{
    USHORT nOpcode, nLen;
    enum { SearchBof, SearchDim, InWKS, InWK1, Ende } eAkt;
    BOOL bInTab = TRUE;

    eAkt = SearchBof;
    while( eAkt != Ende )
    {
        *pIn >> nOpcode >> nLen;
        if( pIn->IsEof() )
            eAkt = Ende;

        nBytesLeft = nLen;
        switch( eAkt )
        {
        case SearchBof: // ----------------------------
            if( 0x00 ==  nOpcode )
            {
                Bof();
                eAkt = SearchDim;
            }
            break;
        case SearchDim: // ----------------------------
            if( 0x06 == nOpcode )				// DIMENSIONS	[S1   ]
            {
                Dimensions();
                switch( eDateiTyp )
                {
                case WKS:
                    eAkt = InWKS;
                    pLotGlob->CreateTable();
                    break;
                case WK1:
                    eAkt = InWK1;
                    pLotGlob->CreateTable();
                    break;
                case WKUnknown:
                case WK3:
                case WK4:
                    eAkt = Ende; break;
                }
            }
            break;
        case InWKS:		// ----------------------------
        case InWK1:
            {
            switch( nOpcode )
                {
                case 0x01:							// EOF			[S1   ]
//Leere Methode!!					Eof();
                    eAkt = Ende;
                    break;
                case 0x0C:	Blank1(); break;		// BLANK		[S1   ]
                case 0x0D:	Integer1(); break;		// INTEGER		[S1   ]
                case 0x0E:	Number1(); break;		// NUMBER		[S1   ]
                case 0x0F:	Label1(); break;		// LABEL		[S1   ]
                case 0x10:	Formula1(); break;		// FORMULA		[S1   ]
                }
            }
            break;


        case Ende: // ---------------------------------
            break;

        default:
            DBG_ERROR( "-SwLotusParser::Read(): Unbekannter Zustand!" );
        }
        ClearBytesLeft();
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
