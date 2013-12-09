/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
XML_NAMESPACE_EXT( CALC,        41U )
XML_NAMESPACE_EXT( LO,          42U )

#define _XML_OLD_NAMESPACE_BASE 43U

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
XML_NAMESPACE( CSS3TEXT,        103U )      // CSS Text Level 3
XML_NAMESPACE( FORMX,           101U )      // form interop extensions


#endif  //  _XMLOFF_XMLNMSPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
