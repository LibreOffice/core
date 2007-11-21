/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: signaturestate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 16:45:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SFX2_SIGNATURESTATE_HXX
#define SFX2_SIGNATURESTATE_HXX

// xmlsec05, check with SFX team
#define SIGNATURESTATE_UNKNOWN                  (sal_Int16)(-1)
#define SIGNATURESTATE_NOSIGNATURES             (sal_Int16)0
#define SIGNATURESTATE_SIGNATURES_OK            (sal_Int16)1
#define SIGNATURESTATE_SIGNATURES_BROKEN        (sal_Int16)2
#define SIGNATURESTATE_SIGNATURES_INVALID       (sal_Int16)3    // State was SIGNATURES_OK, but doc is modified now
#define SIGNATURESTATE_SIGNATURES_NOTVALIDATED  (sal_Int16)4    // signature is OK, but certificate could not be validated

#endif // SFX2_SIGNATURESTATE_HXX
