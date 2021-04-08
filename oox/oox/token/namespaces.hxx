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

#ifndef OOX_TOKEN_NAMESPACES_HXX
#define OOX_TOKEN_NAMESPACES_HXX

#include <sal/types.h>
#include <oox/token/tokens.hxx>

namespace oox {
const size_t NMSP_SHIFT = 16;
const sal_Int32 NMSP_a14 = 1 << NMSP_SHIFT;
const sal_Int32 NMSP_ax = 2 << NMSP_SHIFT;
const sal_Int32 NMSP_c15 = 3 << NMSP_SHIFT;
const sal_Int32 NMSP_dc = 4 << NMSP_SHIFT;
const sal_Int32 NMSP_dcTerms = 5 << NMSP_SHIFT;
const sal_Int32 NMSP_dcmiType = 6 << NMSP_SHIFT;
const sal_Int32 NMSP_dml = 7 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlChart = 8 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlChartDr = 9 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlDiagram = 10 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlLockedCanvas = 11 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlPicture = 12 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlSpreadDr = 13 << NMSP_SHIFT;
const sal_Int32 NMSP_dmlWordDr = 14 << NMSP_SHIFT;
const sal_Int32 NMSP_doc = 15 << NMSP_SHIFT;
const sal_Int32 NMSP_dsp = 16 << NMSP_SHIFT;
const sal_Int32 NMSP_loext = 17 << NMSP_SHIFT;
const sal_Int32 NMSP_mce = 18 << NMSP_SHIFT;
const sal_Int32 NMSP_mceTest = 19 << NMSP_SHIFT;
const sal_Int32 NMSP_officeCustomPr = 20 << NMSP_SHIFT;
const sal_Int32 NMSP_officeDocPropsVT = 21 << NMSP_SHIFT;
const sal_Int32 NMSP_officeExtPr = 22 << NMSP_SHIFT;
const sal_Int32 NMSP_officeMath = 23 << NMSP_SHIFT;
const sal_Int32 NMSP_officeRel = 24 << NMSP_SHIFT;
const sal_Int32 NMSP_officeRelTheme = 25 << NMSP_SHIFT;
const sal_Int32 NMSP_p14 = 26 << NMSP_SHIFT;
const sal_Int32 NMSP_p15 = 27 << NMSP_SHIFT;
const sal_Int32 NMSP_packageContentTypes = 28 << NMSP_SHIFT;
const sal_Int32 NMSP_packageMetaCorePr = 29 << NMSP_SHIFT;
const sal_Int32 NMSP_packageRel = 30 << NMSP_SHIFT;
const sal_Int32 NMSP_ppt = 31 << NMSP_SHIFT;
const sal_Int32 NMSP_schema = 32 << NMSP_SHIFT;
const sal_Int32 NMSP_vml = 33 << NMSP_SHIFT;
const sal_Int32 NMSP_vmlExcel = 34 << NMSP_SHIFT;
const sal_Int32 NMSP_vmlOffice = 35 << NMSP_SHIFT;
const sal_Int32 NMSP_vmlPowerpoint = 36 << NMSP_SHIFT;
const sal_Int32 NMSP_vmlWord = 37 << NMSP_SHIFT;
const sal_Int32 NMSP_w14 = 38 << NMSP_SHIFT;
const sal_Int32 NMSP_wp14 = 39 << NMSP_SHIFT;
const sal_Int32 NMSP_wpg = 40 << NMSP_SHIFT;
const sal_Int32 NMSP_wps = 41 << NMSP_SHIFT;
const sal_Int32 NMSP_x12ac = 42 << NMSP_SHIFT;
const sal_Int32 NMSP_xls = 43 << NMSP_SHIFT;
const sal_Int32 NMSP_xls14Lst = 44 << NMSP_SHIFT;
const sal_Int32 NMSP_xm = 45 << NMSP_SHIFT;
const sal_Int32 NMSP_xml = 46 << NMSP_SHIFT;
const sal_Int32 NMSP_xsi = 47 << NMSP_SHIFT;
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



const sal_Int32 TOKEN_MASK          = static_cast< sal_Int32 >( (1 << NMSP_SHIFT) - 1 );
const sal_Int32 NMSP_MASK           = static_cast< sal_Int32 >( SAL_MAX_INT32 & ~TOKEN_MASK );

/** Returns the raw token identifier without namespace of the passed token. */
inline sal_Int32 getBaseToken( sal_Int32 nToken ) { return nToken & TOKEN_MASK; }

/** Returns the namespace without token identifier of the passed token. */
inline sal_Int32 getNamespace( sal_Int32 nToken ) { return nToken & NMSP_MASK; }


// defines for tokens with specific namespaces
#define OOX_TOKEN( namespace, token ) (::oox::NMSP_##namespace | ::oox::XML_##token)
#define OOX_NS( namespace ) (::oox::NMSP_##namespace)

#define A_TOKEN( token )        OOX_TOKEN( dml, token )
#define AX_TOKEN( token )       OOX_TOKEN( ax, token )
#define C_TOKEN( token )        OOX_TOKEN( dmlChart, token )
#define CDR_TOKEN( token )      OOX_TOKEN( dmlChartDr, token )
#define DGM_TOKEN( token )      OOX_TOKEN( dmlDiagram, token )
#define MCE_TOKEN( token )      OOX_TOKEN( mce, token)
#define O_TOKEN( token )        OOX_TOKEN( vmlOffice, token )
#define PC_TOKEN( token )       OOX_TOKEN( packageContentTypes, token )
#define PPT_TOKEN( token )      OOX_TOKEN( ppt, token )
#define P14_TOKEN( token )      OOX_TOKEN( p14, token )
#define P15_TOKEN( token )      OOX_TOKEN( p15, token )
#define C15_TOKEN( token )      OOX_TOKEN( c15, token )
#define PR_TOKEN( token )       OOX_TOKEN( packageRel, token )
#define R_TOKEN( token )        OOX_TOKEN( officeRel, token )
#define VML_TOKEN( token )      OOX_TOKEN( vml, token )
#define VMLX_TOKEN( token )     OOX_TOKEN( vmlExcel, token )
#define X12AC_TOKEN( token )    OOX_TOKEN( x12ac, token )
#define XDR_TOKEN( token )      OOX_TOKEN( dmlSpreadDr, token )
#define XLS_TOKEN( token )      OOX_TOKEN( xls, token )
#define XLS14_TOKEN( token )    OOX_TOKEN( xls14Lst, token )
#define XM_TOKEN( token )       OOX_TOKEN( xm, token )
#define XML_TOKEN( token )      OOX_TOKEN( xml, token )
#define VMLPPT_TOKEN( token )   OOX_TOKEN( vmlPowerpoint, token )
#define DSP_TOKEN( token )      OOX_TOKEN( dsp, token )
#define LC_TOKEN( token )       OOX_TOKEN( dmlLockedCanvas, token )
#define WPS_TOKEN( token )      OOX_TOKEN( wps, token )
#define WPG_TOKEN( token )      OOX_TOKEN( wpg, token )
#define W_TOKEN( token )        OOX_TOKEN( doc, token )
#define LOEXT_TOKEN( token )    OOX_TOKEN( loext, token )
#define M_TOKEN(token)          OOX_TOKEN(officeMath, token)



} // namespace oox

#endif
