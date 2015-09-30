/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_SOURCE_XSLTFILTER_OLEHANDLER_HXX
#define INCLUDED_FILTER_SOURCE_XSLTFILTER_OLEHANDLER_HXX
#include <cstdio>
#include <cstring>
#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/variables.h>

#include <cppuhelper/factory.hxx>
#include <osl/module.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;

namespace XSLT
{
    /*
     * OleHandler provides implementations for the XSLT extension functions used by the WordML 2003 XSLT filters.
     *
     * The extension functions takes base64 encoded string representations of embedded OLE objects provided by the XML filter framework,
     * stores them into a com.sun.star.embed.OLESimpleStorage and retrieves them later as individual base64 OLE objects.
     *
     * The implementation is ported from the former Java based implementation XSLTOleExtrater (sic)
     *
     * I believe the whole thing should provide round-trip editing of embedded OLE objects.
     * I'm not sure if it currently does anything meaningful, because the Java implementation seems to be broken both in OOo and LibO.
     *
     */
    class OleHandler
    {
    public:
        OleHandler(const css::uno::Reference<XComponentContext>& rxContext){
            m_xContext = rxContext;
        }
        void SAL_CALL          insertByName(const OUString& streamName, const OString& content);
        const OString SAL_CALL getByName(const OUString& streamName);

    private:
        css::uno::Reference<XComponentContext> m_xContext;
        css::uno::Reference<XNameContainer> m_storage;
        css::uno::Reference<XStream> m_rootStream;

        void SAL_CALL    ensureCreateRootStorage();
        OString SAL_CALL encodeSubStorage(const OUString& streamName);
        void SAL_CALL    insertSubStorage(const OUString& streamName, const OString& content);
        void SAL_CALL    initRootStorageFromBase64(const OString& content);
        css::uno::Reference<XStream> SAL_CALL createTempFile();
    };
}


#endif // INCLUDED_FILTER_SOURCE_XSLTFILTER_OLEHANDLER_HXX
