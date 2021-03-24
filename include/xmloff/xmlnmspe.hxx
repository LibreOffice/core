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

#ifndef INCLUDED_XMLOFF_XMLNMSPE_HXX
#define INCLUDED_XMLOFF_XMLNMSPE_HXX

#include <sal/types.h>

#define XML_NAMESPACE( prefix, key ) \
const sal_uInt16 prefix = key;

// current namespaces
// These namespaces have the same index in the namespace table as prefix used.
// If a namespace is added, XML_OLD_NAMESPACE_BASE has to be adjusted!
XML_NAMESPACE( XML_NAMESPACE_OFFICE,          0U )
XML_NAMESPACE( XML_NAMESPACE_STYLE,           1U )
XML_NAMESPACE( XML_NAMESPACE_TEXT ,           2U )
XML_NAMESPACE( XML_NAMESPACE_TABLE,           3U )
XML_NAMESPACE( XML_NAMESPACE_DRAW ,           4U )
XML_NAMESPACE( XML_NAMESPACE_FO   ,           5U )
XML_NAMESPACE( XML_NAMESPACE_XLINK,           6U )
XML_NAMESPACE( XML_NAMESPACE_DC   ,           7U )
XML_NAMESPACE( XML_NAMESPACE_META ,           8U )
XML_NAMESPACE( XML_NAMESPACE_NUMBER,          9U )
XML_NAMESPACE( XML_NAMESPACE_PRESENTATION,    10U )
XML_NAMESPACE( XML_NAMESPACE_SVG,             11U )
XML_NAMESPACE( XML_NAMESPACE_CHART,           12U )
XML_NAMESPACE( XML_NAMESPACE_DR3D,            13U )
XML_NAMESPACE( XML_NAMESPACE_MATH,            14U )
XML_NAMESPACE( XML_NAMESPACE_FORM,            15U )
XML_NAMESPACE( XML_NAMESPACE_SCRIPT,          16U )
XML_NAMESPACE( XML_NAMESPACE_BLOCKLIST,       17U )
XML_NAMESPACE( XML_NAMESPACE_FRAMEWORK,       18U )
XML_NAMESPACE( XML_NAMESPACE_CONFIG,          19U )
XML_NAMESPACE( XML_NAMESPACE_OOO,             20U )
XML_NAMESPACE( XML_NAMESPACE_OOOW,            21U )
XML_NAMESPACE( XML_NAMESPACE_OOOC,            22U )
XML_NAMESPACE( XML_NAMESPACE_DOM,             23U )
XML_NAMESPACE( XML_NAMESPACE_TCD,             24U )   // text conversion dictionary
XML_NAMESPACE( XML_NAMESPACE_DB,              25U )
XML_NAMESPACE( XML_NAMESPACE_DLG,             26U )
XML_NAMESPACE( XML_NAMESPACE_XFORMS,          27U )
XML_NAMESPACE( XML_NAMESPACE_XSD,             28U )
XML_NAMESPACE( XML_NAMESPACE_XSI,             29U )
XML_NAMESPACE( XML_NAMESPACE_SMIL,            30U )
XML_NAMESPACE( XML_NAMESPACE_ANIMATION,       31U )
XML_NAMESPACE( XML_NAMESPACE_XML,             32U )
XML_NAMESPACE( XML_NAMESPACE_REPORT,          33U )
XML_NAMESPACE( XML_NAMESPACE_OF,              34U )   // OpenFormula aka ODFF
XML_NAMESPACE( XML_NAMESPACE_XHTML,           35U )
XML_NAMESPACE( XML_NAMESPACE_GRDDL,           36U )
XML_NAMESPACE( XML_NAMESPACE_VERSIONS_LIST,   37U )

// namespaces for odf extended formats

#define XML_NAMESPACE_EXT_BASE 38U
#define XML_NAMESPACE_EXT( prefix, index ) \
const sal_uInt16 prefix = (XML_NAMESPACE_EXT_BASE+index);

XML_NAMESPACE_EXT( XML_NAMESPACE_OFFICE_EXT,       0U )
XML_NAMESPACE_EXT( XML_NAMESPACE_TABLE_EXT,        1U )
XML_NAMESPACE_EXT( XML_NAMESPACE_CHART_EXT,        2U )
XML_NAMESPACE_EXT( XML_NAMESPACE_DRAW_EXT,         3U )
XML_NAMESPACE_EXT( XML_NAMESPACE_CALC_EXT,         4U )
XML_NAMESPACE_EXT( XML_NAMESPACE_LO_EXT,           5U )

// namespaces for OOo formats

#define XML_NAMESPACE_OOO_BASE 44U
#define XML_NAMESPACE_OOO( prefix, index ) \
const sal_uInt16 prefix = (XML_NAMESPACE_OOO_BASE+index);

XML_NAMESPACE_OOO( XML_NAMESPACE_OFFICE_OOO,         0U )
XML_NAMESPACE_OOO( XML_NAMESPACE_META_OOO,           1U )
XML_NAMESPACE_OOO( XML_NAMESPACE_STYLE_OOO,          2U )
XML_NAMESPACE_OOO( XML_NAMESPACE_NUMBER_OOO,         3U )
XML_NAMESPACE_OOO( XML_NAMESPACE_TEXT_OOO,           4U )
XML_NAMESPACE_OOO( XML_NAMESPACE_TABLE_OOO,          5U )
XML_NAMESPACE_OOO( XML_NAMESPACE_DRAW_OOO,           6U )
XML_NAMESPACE_OOO( XML_NAMESPACE_DR3D_OOO,           7U )
XML_NAMESPACE_OOO( XML_NAMESPACE_PRESENTATION_OOO,   8U )
XML_NAMESPACE_OOO( XML_NAMESPACE_CHART_OOO,          9U )
XML_NAMESPACE_OOO( XML_NAMESPACE_CONFIG_OOO,        10U )
XML_NAMESPACE_OOO( XML_NAMESPACE_FORM_OOO,          11U )
XML_NAMESPACE_OOO( XML_NAMESPACE_SCRIPT_OOO,        12U )

#define XML_NAMESPACE_COMPAT_BASE 57U
#define XML_NAMESPACE_COMPAT( prefix, index ) \
const sal_uInt16 prefix = (XML_NAMESPACE_COMPAT_BASE+index);

XML_NAMESPACE_COMPAT( XML_NAMESPACE_SVG_COMPAT,      0U )
XML_NAMESPACE_COMPAT( XML_NAMESPACE_FO_COMPAT,       1U )
XML_NAMESPACE_COMPAT( XML_NAMESPACE_SMIL_COMPAT,     2U )

#define XML_NAMESPACE_OASIS_BASE 60U
#define XML_NAMESPACE_OASIS( prefix, index ) \
const sal_uInt16 prefix = (XML_NAMESPACE_OASIS_BASE+index);

XML_NAMESPACE_OASIS( XML_NAMESPACE_DB_OASIS,         0U )
XML_NAMESPACE_OASIS( XML_NAMESPACE_REPORT_OASIS,     1U )

#define XML_OLD_NAMESPACE_BASE 62U
#define XML_OLD_NAMESPACE( prefix, index ) \
const sal_uInt16 prefix = (XML_OLD_NAMESPACE_BASE+index);

// namespaces used in the technical preview (SO 5.2)
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_FO,             0U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_XLINK,          1U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_OFFICE,         2U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_STYLE,          3U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_TEXT,           4U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_TABLE,          5U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_META,           6U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_DRAW,           7U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_NUMBER,         8U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_PRESENTATION,   9U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_CHART,         10U )
XML_OLD_NAMESPACE( XML_OLD_NAMESPACE_SMIL,          11U )

// experimental namespaces
XML_NAMESPACE( XML_NAMESPACE_FIELD,           100U )
XML_NAMESPACE( XML_NAMESPACE_CSS3TEXT,        103U )  // CSS Text Level 3
XML_NAMESPACE( XML_NAMESPACE_FORMX,           101U )  // form interop extensions

// OOo extension digital signatures, used in ODF 1.1
const sal_uInt16 XML_NAMESPACE_DSIG_OOO =        200;
// ODF 1.2 digital signature namespaces
const sal_uInt16 XML_NAMESPACE_DSIG =            201;
const sal_uInt16 XML_NAMESPACE_DS =              202;
const sal_uInt16 XML_NAMESPACE_XADES132 =        203;
const sal_uInt16 XML_NAMESPACE_XADES141 =        204;

#endif // INCLUDED_XMLOFF_XMLNMSPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
