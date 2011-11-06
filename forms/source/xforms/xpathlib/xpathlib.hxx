/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/*
   entry functions for libxml xpath engine

*/

/**
 * xmlXPathParserContext:
 *
 * An XPath parser context. It contains pure parsing informations,
 * an xmlXPathContext, and the stack of objects.
 */
#if 0
// for reference from xpath.h
struct _xmlXPathParserContext {
    const xmlChar *cur;         /* the current char being parsed */
    const xmlChar *base;            /* the full expression */

    int error;              /* error code */

    xmlXPathContextPtr  context;    /* the evaluation context */
    xmlXPathObjectPtr     value;    /* the current value */
    int                 valueNr;    /* number of values stacked */
    int                valueMax;    /* max number of values stacked */
    xmlXPathObjectPtr *valueTab;    /* stack of values */

    xmlXPathCompExprPtr comp;       /* the precompiled expression */
    int xptr;               /* it this an XPointer expression */
    xmlNodePtr         ancestor;    /* used for walking preceding axis */
};

#endif

extern "C"
{

// XForms
/*
void xforms_getInstanceDocumentFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_rebuildFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_recalculateFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_revalidateFunction(xmlXPathParserContextPtr ctxt, int nargs);
void xforms_refreshFunction(xmlXPathParserContextPtr ctxt, int nargs);
*/

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
void xforms_versionFunction(xmlXPathParserContextPtr ctxt, int nargs);

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
