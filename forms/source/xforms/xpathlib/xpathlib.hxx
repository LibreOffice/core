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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_XPATHLIB_XPATHLIB_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_XPATHLIB_XPATHLIB_HXX

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/*
   entry functions for libxml xpath engine

*/

/**
 * xmlXPathParserContext:
 *
 * An XPath parser context. It contains pure parsing information,
 * an xmlXPathContext, and the stack of objects.
 */

extern "C"
{

// XForms

// XForms Core Functions
// boolean functions
void xforms_booleanFromStringFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_ifFunction(xmlXPathParserContextPtr ctxt, int nargs);

// Number Functions
void xforms_avgFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_minFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_maxFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_countNonEmptyFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_indexFunction(xmlXPathParserContextPtr ctxt, int nargs);

// String Functions
void xforms_propertyFunction(xmlXPathParserContextPtr ctxt, int nargs);

// Date and Time Functions
void xforms_nowFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_daysFromDateFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_secondsFromDateTimeFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_secondsFuction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_monthsFuction(xmlXPathParserContextPtr ctxt, int nargs);

// Node-set Functions
void xforms_instanceFuction(xmlXPathParserContextPtr ctxt, int nargs);

// Node-set Functions; XForms 1.1
void xforms_currentFunction(xmlXPathParserContextPtr ctxt, int nargs);

// --- lookup ---
xmlXPathFunction xforms_lookupFunc(void *ctxt, const xmlChar *name, const xmlChar *ns_uri);

}

#endif // INCLUDED_FORMS_SOURCE_XFORMS_XPATHLIB_XPATHLIB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
