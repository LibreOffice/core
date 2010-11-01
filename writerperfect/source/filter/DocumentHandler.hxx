/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _OODOCUMENTHANDLER_HXX_
#define _OODOCUMENTHANDLER_HXX_

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#if defined _MSC_VER
#pragma warning( push, 1 )
#endif
#include <libwpd/libwpd.h>
#if defined _MSC_VER
#pragma warning( pop )
#endif

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class DocumentHandler
{
public:
        DocumentHandler(Reference < XDocumentHandler > &xHandler);
        void startDocument();
        void endDocument();
        void startElement(const char *psName, const WPXPropertyList &xPropList);
        void endElement(const char *psName);
        void characters(const WPXString &sCharacters);

private:
        Reference < XDocumentHandler > mxHandler;
};

#endif // _OODOCUMENTHANDLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
