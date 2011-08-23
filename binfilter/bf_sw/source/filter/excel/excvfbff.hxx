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
#ifndef _EXCVFBFF_HXX
#define _EXCVFBFF_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
class String;
namespace binfilter {



class ValueFormBuffer;

#if defined(BLC) || defined(MAC)
// BLC will es auf jeden Fall hier
typedef void				( ValueFormBuffer::*Fkt_rString )( String & );
typedef sal_uInt32          ( ValueFormBuffer::*Fkt_USHORT )( USHORT );
#endif

class ValueFormBuffer
    {
    private:
#if !defined(BLC) && !defined(MAC)
// MSC will es auf jeden Fall hier
        typedef void		( ValueFormBuffer::*Fkt_rString )( String & );
        typedef sal_uInt32  ( ValueFormBuffer::*Fkt_USHORT )( USHORT );
#endif

        sal_uInt32          *pHandles;      // Array mit Handles...
        USHORT				nMax;  			// Groesse des Arrays
        USHORT				nCount;			// Index des naechsten freien Eintrags
        sal_uInt32          nDefaultHandle;

        // nur fuer Excel5
        static const USHORT nAnzBuiltin;	// bekannte Formate
        static const USHORT nNewFormats;	// selbstdefinierte
        static const sal_Char	*pBuiltinFormats[];	// vordefinierte Formate

        Fkt_rString			_NewValueFormat;
        Fkt_USHORT			_GetValueFormat;
        // ----------------------------------------------------------
        void				Init( void );
                            // fuer 1. Nutzung
        void				__NewValueFormat( String &rFormString );
        sal_uInt32          __GetValueFormat( USHORT nExcIndex );
                            // fuer n-te Nutzung
        void				_NewValueFormatX( String &rFormString );
        void				_NewValueFormat5( String &rFormString );
        sal_uInt32          _GetValueFormatX5( USHORT nExcIndex );
    public:
        ValueFormBuffer( const USHORT nSize = 2048 );
        ~ValueFormBuffer();

        inline void			NewValueFormat( String &rFormString );
        inline ULONG		GetValueFormat( USHORT nExcIndex );
#ifdef USED
        void				Reset( void );
#endif
    };

inline void ValueFormBuffer::NewValueFormat( String &rFormString )
    {
    ( this->*_NewValueFormat )( rFormString );
    }

inline ULONG ValueFormBuffer::GetValueFormat( USHORT nExcIndex )
    {
    return ( this->*_GetValueFormat )( nExcIndex );
    }


} //namespace binfilter
#endif

