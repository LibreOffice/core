/*************************************************************************
 *
 *  $RCSfile: errhdl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-25 09:27:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _ERRHDL_HXX
#define _ERRHDL_HXX

#ifndef PRODUCT

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

extern BOOL bAssert;                // TRUE, wenn eine ASSERT-Box hochkam


// -----------------------------------------------------------------------
// Ausgabe einer Fehlermeldung inkl. Dateiname und Zeilennummer
// wo der Fehler auftrat.
// Die Funktion darf nicht direkt benutzt werden!
// -----------------------------------------------------------------------
SW_DLLPUBLIC void AssertFail( const sal_Char*, const sal_Char*, USHORT );
SW_DLLPUBLIC void AssertFail( USHORT, const sal_Char*, USHORT );

#define ASSERT( cond, message ) \
    if( !(cond) ) { \
        const char   *_pErrorText = #message; \
        const char   *_pFileName  = __FILE__; \
       ::AssertFail( _pErrorText, _pFileName, __LINE__ ); \
    }

// -----------------------------------------------------------------------
// Prueft ob die angegebene Bedingung wahr ist, wenn nicht wird eine
// Fehlermeldung die ueber die ID Identifiziert wird, ausgegeben.
// -----------------------------------------------------------------------
#define ASSERT_ID( cond, id ) \
    if( !(cond) ) { \
        const char   *_pFileName  = __FILE__; \
       ::AssertFail( (USHORT)id, _pFileName, __LINE__ ); \
    }

// -----------------------------------------------------------------------
// Beim Bilden der Produktversion werden alle Debug-Utilities automatisch
// ignoriert
// -----------------------------------------------------------------------
#else
#define ASSERT( cond, message )     ;
#define ASSERT_ID( cond, id )       ;
#endif // PRODUCT



#endif
