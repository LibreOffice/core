/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: namespaces.hxx,v $
 * $Revision: 1.5.4.1 $
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
const sal_Int32 NMSP_ACTIVATION             = 12 << 16;
// DrawingML
const sal_Int32 NMSP_DRAWINGML              = 21 << 16;
const sal_Int32 NMSP_DIAGRAM                = 22 << 16;
const sal_Int32 NMSP_CHART                  = 23 << 16;
const sal_Int32 NMSP_CDR                    = 24 << 16; // ChartDrawingML
// VML
const sal_Int32 NMSP_VML                    = 31 << 16;
const sal_Int32 NMSP_OFFICE                 = 32 << 16;
const sal_Int32 NMSP_VML_DOC                = 33 << 16; // Word VML
const sal_Int32 NMSP_VML_XLS                = 34 << 16; // Excel VML
const sal_Int32 NMSP_VML_PPT                = 35 << 16; // PowerPoint VML
const sal_Int32 NMSP_AX                     = 36 << 16; // ActiveX (for OCX form controls)
// SpreadsheetML
const sal_Int32 NMSP_XLS                    = 41 << 16;
const sal_Int32 NMSP_XDR                    = 42 << 16; // SpreadsheetDrawingML
const sal_Int32 NMSP_XM                     = 43 << 16; // Macro sheet
// PresentationML
const sal_Int32 NMSP_PPT                    = 51 << 16;
// Document properties
const sal_Int32 NMSP_COREPR                 = 61 << 16;
const sal_Int32 NMSP_DC                     = 62 << 16;
const sal_Int32 NMSP_DCTERMS                = 63 << 16;
const sal_Int32 NMSP_EXTPR                  = 64 << 16;
const sal_Int32 NMSP_CUSTPR                 = 65 << 16;
const sal_Int32 NMSP_VT                     = 66 << 16;

// ----------------------------------------------------------------------------

const sal_Int32 TOKEN_MASK                  = SAL_MAX_UINT16;
const sal_Int32 NMSP_MASK                   = SAL_MAX_INT16 << 16;

/** Returns the token identifier of the passed element without namespace. */
inline sal_Int32 getToken( sal_Int32 nElement ) { return nElement & TOKEN_MASK; }

/** Returns the namespace of the passed element without token identifier. */
inline sal_Int32 getNamespace( sal_Int32 nElement ) { return nElement & NMSP_MASK; }

// ----------------------------------------------------------------------------

// defines for tokens with specific namespaces, can be used in switch/cases
#define A_TOKEN( token )            (::oox::NMSP_DRAWINGML | XML_##token)
#define AX_TOKEN( token )           (::oox::NMSP_AX | XML_##token)
#define C_TOKEN( token )            (::oox::NMSP_CHART | XML_##token)
#define CDR_TOKEN( token )          (::oox::NMSP_CDR | XML_##token)
#define O_TOKEN( token )            (::oox::NMSP_OFFICE | XML_##token)
#define PPT_TOKEN( token )          (::oox::NMSP_PPT | XML_##token)
#define R_TOKEN( token )            (::oox::NMSP_RELATIONSHIPS | XML_##token)
#define VML_TOKEN( token )          (::oox::NMSP_VML | XML_##token)
#define VMLX_TOKEN( token )         (::oox::NMSP_VML_XLS | XML_##token)
#define XDR_TOKEN( token )          (::oox::NMSP_XDR | XML_##token)
#define XLS_TOKEN( token )          (::oox::NMSP_XLS | XML_##token)
#define XM_TOKEN( token )           (::oox::NMSP_XM | XML_##token)
#define XML_TOKEN( token )          (::oox::NMSP_XML | XML_##token)

// ============================================================================

} // namespace oox

#endif // OOX_CORE_NAMESPACES_HXX

