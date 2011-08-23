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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#define _SVSTDARR_LONGS
#define _SVSTDARR_USHORTS

#include <ctype.h>
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif



#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

namespace binfilter {

//-------------------------------------------------------------------

//JP 16.12.99: definition:
//		from pos cPosEnDash to cPosEmDash all chars changed to endashes,
//		from pos cPosEmDash to cPosEnd    all chars changed to emdashes
// 		all other chars are changed to the user configuration




/*N*/ SvxSwAutoFmtFlags* SwEditShell::pAutoFmtFlags = 0;

// Anzahl von Num-/Bullet-Absatzvorlagen. MAXLEVEL wird demnaechst auf
// x erhoeht, die Anzahl Vorlagen aber nicht (Ueberbleibsel aus <= 4.0)

    static BOOL  m_bAskForCancelUndoWhileBufferOverflow;
    static short m_nActionWhileAutoformatUndoBufferOverflow;
}
