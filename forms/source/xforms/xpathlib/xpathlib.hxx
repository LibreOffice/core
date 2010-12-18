/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
