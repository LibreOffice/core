/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namespaces.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:35:29 $
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

#ifndef OOX_CORE_NAMESPACES_HXX
#define OOX_CORE_NAMESPACES_HXX

#include <sal/types.h>

namespace oox {

// ============================================================================

// relations, XML
const sal_Int32 NMSP_XML                    = 1 << 16;
const sal_Int32 NMSP_PACKAGE_RELATIONSHIPS  = 2 << 16;
const sal_Int32 NMSP_RELATIONSHIPS          = 3 << 16;
const sal_Int32 NMSP_CONTENT_TYPES          = 4 << 16;
// office global
const sal_Int32 NMSP_OFFICE                 = 11 << 16;
const sal_Int32 NMSP_ACTIVATION             = 12 << 16;
// DrawingML
const sal_Int32 NMSP_DRAWINGML              = 21 << 16;
const sal_Int32 NMSP_DIAGRAM                = 22 << 16;
const sal_Int32 NMSP_CHART                  = 23 << 16;
const sal_Int32 NMSP_VML                    = 24 << 16;
// WordML
const sal_Int32 NMSP_WORD                   = 31 << 16; // Word VML
// SpreadsheetML
const sal_Int32 NMSP_XLS                    = 41 << 16;
const sal_Int32 NMSP_XDR                    = 42 << 16; // SpreadsheetDrawingML
const sal_Int32 NMSP_XM                     = 43 << 16; // Macro sheet
const sal_Int32 NMSP_EXCEL                  = 44 << 16; // Excel VML
// PresentationML
const sal_Int32 NMSP_PPT                    = 51 << 16;
const sal_Int32 NMSP_POWERPOINT             = 52 << 16; // PowerPoint VML

// ----------------------------------------------------------------------------

const sal_Int32 TOKEN_MASK                  = SAL_MAX_UINT16;
const sal_Int32 NMSP_MASK                   = SAL_MAX_INT16 << 16;

/** Returns the token identifier of the passed element without namespace. */
inline sal_Int32 getToken( sal_Int32 nElement ) { return nElement & TOKEN_MASK; }

/** Returns the namespace of the passed element without token identifier. */
inline sal_Int32 getNamespace( sal_Int32 nElement ) { return nElement & NMSP_MASK; }

// ============================================================================

} // namespace oox

#endif // OOX_CORE_NAMESPACES_HXX

