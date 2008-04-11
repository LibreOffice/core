/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stdtext.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _VCL_STDTEXT_HXX
#define _VCL_STDTEXT_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

class Window;

// ------------------
// - Standard-Texte -
// ------------------

#define STANDARD_TEXT_FIRST                     STANDARD_TEXT_SERVICE_NOT_AVAILABLE
#define STANDARD_TEXT_SERVICE_NOT_AVAILABLE     ((USHORT)0)
#define STANDARD_TEXT_LAST                      STANDARD_TEXT_SERVICE_NOT_AVAILABLE

XubString VCL_DLLPUBLIC GetStandardText( USHORT nStdText );

// -------------------------------------
// - Hilfsmethoden fuer Standard-Texte -
// -------------------------------------

void VCL_DLLPUBLIC ShowServiceNotAvailableError( Window* pParent, const XubString& rServiceName, BOOL bError );

#endif  // _VCL_STDTEXT_HXX
