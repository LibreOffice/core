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

// current namespaces
// These namespaces have the same index in the namespace table as prefix used.
// If a namespace is added, XML_OLD_NAMESPACE_BASE has to be adjusted!
constexpr sal_uInt16 XML_NAMESPACE_OFFICE =           0;
constexpr sal_uInt16 XML_NAMESPACE_STYLE =            1;
constexpr sal_uInt16 XML_NAMESPACE_TEXT =             2;
constexpr sal_uInt16 XML_NAMESPACE_TABLE =            3;
constexpr sal_uInt16 XML_NAMESPACE_DRAW =             4;
constexpr sal_uInt16 XML_NAMESPACE_FO   =             5;
constexpr sal_uInt16 XML_NAMESPACE_XLINK =            6;
constexpr sal_uInt16 XML_NAMESPACE_DC   =             7;
constexpr sal_uInt16 XML_NAMESPACE_META =             8;
constexpr sal_uInt16 XML_NAMESPACE_NUMBER =           9;
constexpr sal_uInt16 XML_NAMESPACE_PRESENTATION =    10;
constexpr sal_uInt16 XML_NAMESPACE_SVG =             11;
constexpr sal_uInt16 XML_NAMESPACE_CHART =           12;
constexpr sal_uInt16 XML_NAMESPACE_DR3D =            13;
constexpr sal_uInt16 XML_NAMESPACE_MATH =            14;
constexpr sal_uInt16 XML_NAMESPACE_FORM =            15;
constexpr sal_uInt16 XML_NAMESPACE_SCRIPT =          16;
constexpr sal_uInt16 XML_NAMESPACE_BLOCKLIST =       17;
constexpr sal_uInt16 XML_NAMESPACE_FRAMEWORK =       18;
constexpr sal_uInt16 XML_NAMESPACE_CONFIG =          19;
constexpr sal_uInt16 XML_NAMESPACE_OOO =             20;
constexpr sal_uInt16 XML_NAMESPACE_OOOW =            21;
constexpr sal_uInt16 XML_NAMESPACE_OOOC =            22;
constexpr sal_uInt16 XML_NAMESPACE_DOM =             23;
constexpr sal_uInt16 XML_NAMESPACE_TCD =             24;   // text conversion dictionary
constexpr sal_uInt16 XML_NAMESPACE_DB =              25;
constexpr sal_uInt16 XML_NAMESPACE_DLG =             26;
constexpr sal_uInt16 XML_NAMESPACE_XFORMS =          27;
constexpr sal_uInt16 XML_NAMESPACE_XSD =             28;
constexpr sal_uInt16 XML_NAMESPACE_XSI =             29;
constexpr sal_uInt16 XML_NAMESPACE_SMIL =            30;
constexpr sal_uInt16 XML_NAMESPACE_ANIMATION =       31;
constexpr sal_uInt16 XML_NAMESPACE_XML =             32;
constexpr sal_uInt16 XML_NAMESPACE_REPORT =          33;
constexpr sal_uInt16 XML_NAMESPACE_OF =              34;   // OpenFormula aka ODFF
constexpr sal_uInt16 XML_NAMESPACE_XHTML =           35;
constexpr sal_uInt16 XML_NAMESPACE_GRDDL =           36;
constexpr sal_uInt16 XML_NAMESPACE_VERSIONS_LIST =   37;

// namespaces for odf extended formats
constexpr sal_uInt16 XML_NAMESPACE_EXT_BASE   = 50;
constexpr sal_uInt16 XML_NAMESPACE_OFFICE_EXT = XML_NAMESPACE_EXT_BASE + 0;
constexpr sal_uInt16 XML_NAMESPACE_TABLE_EXT  = XML_NAMESPACE_EXT_BASE + 1;
constexpr sal_uInt16 XML_NAMESPACE_CHART_EXT  = XML_NAMESPACE_EXT_BASE + 2;
constexpr sal_uInt16 XML_NAMESPACE_DRAW_EXT   = XML_NAMESPACE_EXT_BASE + 3;
constexpr sal_uInt16 XML_NAMESPACE_CALC_EXT   = XML_NAMESPACE_EXT_BASE + 4;
constexpr sal_uInt16 XML_NAMESPACE_LO_EXT     = XML_NAMESPACE_EXT_BASE + 5;

// namespaces for OOo formats
constexpr sal_uInt16 XML_NAMESPACE_OOO_BASE = 60;
constexpr sal_uInt16 XML_NAMESPACE_OFFICE_OOO = XML_NAMESPACE_OOO_BASE +         0;
constexpr sal_uInt16 XML_NAMESPACE_META_OOO = XML_NAMESPACE_OOO_BASE +           1;
constexpr sal_uInt16 XML_NAMESPACE_STYLE_OOO = XML_NAMESPACE_OOO_BASE +          2;
constexpr sal_uInt16 XML_NAMESPACE_NUMBER_OOO = XML_NAMESPACE_OOO_BASE +         3;
constexpr sal_uInt16 XML_NAMESPACE_TEXT_OOO = XML_NAMESPACE_OOO_BASE +           4;
constexpr sal_uInt16 XML_NAMESPACE_TABLE_OOO = XML_NAMESPACE_OOO_BASE +          5;
constexpr sal_uInt16 XML_NAMESPACE_DRAW_OOO = XML_NAMESPACE_OOO_BASE +           6;
constexpr sal_uInt16 XML_NAMESPACE_DR3D_OOO = XML_NAMESPACE_OOO_BASE +           7;
constexpr sal_uInt16 XML_NAMESPACE_PRESENTATION_OOO = XML_NAMESPACE_OOO_BASE +   8;
constexpr sal_uInt16 XML_NAMESPACE_PRESENTATION_OASIS = XML_NAMESPACE_OOO_BASE + 9; // only used for some config files in sd/
constexpr sal_uInt16 XML_NAMESPACE_CHART_OOO = XML_NAMESPACE_OOO_BASE +         10;
constexpr sal_uInt16 XML_NAMESPACE_CONFIG_OOO = XML_NAMESPACE_OOO_BASE +        12;
constexpr sal_uInt16 XML_NAMESPACE_FORM_OOO = XML_NAMESPACE_OOO_BASE +          13;
constexpr sal_uInt16 XML_NAMESPACE_SCRIPT_OOO = XML_NAMESPACE_OOO_BASE +        14;
constexpr sal_uInt16 XML_NAMESPACE_ANIMATION_OOO = XML_NAMESPACE_OOO_BASE +     15;

constexpr sal_uInt16 XML_NAMESPACE_COMPAT_BASE = 80;
constexpr sal_uInt16 XML_NAMESPACE_SVG_COMPAT = XML_NAMESPACE_COMPAT_BASE +      0;
constexpr sal_uInt16 XML_NAMESPACE_FO_COMPAT = XML_NAMESPACE_COMPAT_BASE +       1;
constexpr sal_uInt16 XML_NAMESPACE_SMIL_COMPAT = XML_NAMESPACE_COMPAT_BASE +     2;

constexpr sal_uInt16 XML_NAMESPACE_OASIS_BASE = 90;
constexpr sal_uInt16 XML_NAMESPACE_DB_OASIS = XML_NAMESPACE_OASIS_BASE +         0;
constexpr sal_uInt16 XML_NAMESPACE_REPORT_OASIS = XML_NAMESPACE_OASIS_BASE +     1;

// namespaces used in the technical preview (SO 5.2)
constexpr sal_uInt16 XML_OLD_NAMESPACE_BASE = 100;
constexpr sal_uInt16 XML_NAMESPACE_FO_SO52 = XML_OLD_NAMESPACE_BASE +             0;
constexpr sal_uInt16 XML_NAMESPACE_XLINK_SO52 = XML_OLD_NAMESPACE_BASE +          1;
constexpr sal_uInt16 XML_NAMESPACE_OFFICE_SO52 = XML_OLD_NAMESPACE_BASE +         2;
constexpr sal_uInt16 XML_NAMESPACE_STYLE_SO52 = XML_OLD_NAMESPACE_BASE +          3;
constexpr sal_uInt16 XML_NAMESPACE_TEXT_SO52 = XML_OLD_NAMESPACE_BASE +           4;
constexpr sal_uInt16 XML_NAMESPACE_TABLE_SO52 = XML_OLD_NAMESPACE_BASE +          5;
constexpr sal_uInt16 XML_NAMESPACE_META_SO52 = XML_OLD_NAMESPACE_BASE +           6;
constexpr sal_uInt16 XML_NAMESPACE_DRAW_SO52 = XML_OLD_NAMESPACE_BASE +           7;
constexpr sal_uInt16 XML_NAMESPACE_NUMBER_SO52 = XML_OLD_NAMESPACE_BASE +         8;
constexpr sal_uInt16 XML_NAMESPACE_PRESENTATION_SO52 = XML_OLD_NAMESPACE_BASE +   9;
constexpr sal_uInt16 XML_NAMESPACE_CHART_SO52 = XML_OLD_NAMESPACE_BASE +         10;
constexpr sal_uInt16 XML_NAMESPACE_SMIL_SO52 = XML_OLD_NAMESPACE_BASE +          11;

// experimental namespaces
constexpr sal_uInt16 XML_NAMESPACE_FIELD =           120;
constexpr sal_uInt16 XML_NAMESPACE_CSS3TEXT =        123;  // CSS Text Level 3
constexpr sal_uInt16 XML_NAMESPACE_FORMX =           121;  // form interop extensions


#endif // INCLUDED_XMLOFF_XMLNMSPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
