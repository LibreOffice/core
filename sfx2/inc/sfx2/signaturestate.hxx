/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: signaturestate.hxx,v $
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

#ifndef SFX2_SIGNATURESTATE_HXX
#define SFX2_SIGNATURESTATE_HXX

// xmlsec05, check with SFX team
#define SIGNATURESTATE_UNKNOWN                  (sal_Int16)(-1)
#define SIGNATURESTATE_NOSIGNATURES             (sal_Int16)0
#define SIGNATURESTATE_SIGNATURES_OK            (sal_Int16)1
#define SIGNATURESTATE_SIGNATURES_BROKEN        (sal_Int16)2
// State was SIGNATURES_OK, but doc is modified now
#define SIGNATURESTATE_SIGNATURES_INVALID       (sal_Int16)3
// signature is OK, but certificate could not be validated
#define SIGNATURESTATE_SIGNATURES_NOTVALIDATED  (sal_Int16)4
//signatur and certificate are ok,but not al files are signed, as was the case in
//OOo 2.x - OOo 3.1.1. This state is only used  together with document signatures.
#define SIGNATURESTATE_SIGNATURES_PARTIAL_OK    (sal_Int16)5

#endif // SFX2_SIGNATURESTATE_HXX
