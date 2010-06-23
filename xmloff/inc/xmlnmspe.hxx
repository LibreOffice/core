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

#ifndef _XMLOFF_XMLNMSPE_HXX
#define _XMLOFF_XMLNMSPE_HXX

#include <sal/types.h>

#define XML_NAMESPACE( prefix, key ) \
const sal_uInt16 XML_NAMESPACE_##prefix         = key; \
const sal_uInt16 XML_NAMESPACE_##prefix##_IDX   = key;

#define XML_OLD_NAMESPACE( prefix, index ) \
const sal_uInt16 XML_OLD_NAMESPACE_##prefix##_IDX = \
    (_XML_OLD_NAMESPACE_BASE+index);

// current namespaces
// These namespaces have the same index in the namespace table as prefix used.
// If a namespace is added, _XML_OLD_NAMESPACE_BASE has to be adjusted!
XML_NAMESPACE( OFFICE,          0U )
XML_NAMESPACE( STYLE,           1U )
XML_NAMESPACE( TEXT ,           2U )
XML_NAMESPACE( TABLE,           3U )
XML_NAMESPACE( DRAW ,           4U )
XML_NAMESPACE( FO   ,           5U )
XML_NAMESPACE( XLINK,           6U )
XML_NAMESPACE( DC   ,           7U )
XML_NAMESPACE( META ,           8U )
XML_NAMESPACE( NUMBER,          9U )
XML_NAMESPACE( PRESENTATION,    10U )
XML_NAMESPACE( SVG,             11U )
XML_NAMESPACE( CHART,           12U )
XML_NAMESPACE( DR3D,            13U )
XML_NAMESPACE( MATH,            14U )
XML_NAMESPACE( FORM,            15U )
XML_NAMESPACE( SCRIPT,          16U )
XML_NAMESPACE( BLOCKLIST,       17U )
XML_NAMESPACE( FRAMEWORK,       18U )
XML_NAMESPACE( CONFIG,          19U )
XML_NAMESPACE( OOO,             20U )
XML_NAMESPACE( OOOW,            21U )
XML_NAMESPACE( OOOC,            22U )
XML_NAMESPACE( DOM,             23U )
XML_NAMESPACE( TCD,             24U )       // text conversion dictionary
XML_NAMESPACE( DB,              25U )
XML_NAMESPACE( DLG,             26U )
XML_NAMESPACE( XFORMS,          27U )
XML_NAMESPACE( XSD,             28U )
XML_NAMESPACE( XSI,             29U )
XML_NAMESPACE( SMIL,            30U )
XML_NAMESPACE( ANIMATION,       31U )
XML_NAMESPACE( XML,             32U )
XML_NAMESPACE( REPORT,          33U )
XML_NAMESPACE( OF,              34U )       // OpenFormula aka ODFF
XML_NAMESPACE( XHTML,           35U )
XML_NAMESPACE( GRDDL,           36U )

// namespaces for odf extended formats

#define XML_NAMESPACE_EXT( prefix, key ) \
const sal_uInt16 XML_NAMESPACE_##prefix##_EXT       = key; \
const sal_uInt16 XML_NAMESPACE_##prefix##_EXT_IDX   = key;

XML_NAMESPACE_EXT( OFFICE,      37U )
XML_NAMESPACE_EXT( TABLE,       38U )
XML_NAMESPACE_EXT( CHART,       39U )
XML_NAMESPACE_EXT( DRAW,        40U )

#define _XML_OLD_NAMESPACE_BASE 41U

// namespaces used in the technical preview (SO 5.2)
XML_OLD_NAMESPACE( FO,      0U )
XML_OLD_NAMESPACE( XLINK,   1U )

XML_OLD_NAMESPACE( OFFICE,  2U )
XML_OLD_NAMESPACE( STYLE,   3U )
XML_OLD_NAMESPACE( TEXT,    4U )
XML_OLD_NAMESPACE( TABLE,   5U )
XML_OLD_NAMESPACE( META,    6U )

// experimental namespaces
XML_NAMESPACE( FIELD,           100U )


#endif  //  _XMLOFF_XMLNMSPE_HXX
