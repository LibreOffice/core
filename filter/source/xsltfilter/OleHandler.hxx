/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Peter Jentsch <pjotr@guineapics.de> ]
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Peter Jentsch <pjotr@guineapics.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef OLEHANDLER_HXX_
#define OLEHANDLER_HXX_
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
#include <xmloff/xmluconv.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase.hxx>
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
using namespace ::rtl;

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
        OleHandler(const Reference<XMultiServiceFactory>& msf){
            m_msf =msf;
        }
        void SAL_CALL
        insertByName(const OUString& streamName, const OString& content);
        const OString SAL_CALL
        getByName(const OUString& streamName);

    private:
        Reference<XMultiServiceFactory> m_msf;
        Reference<XNameContainer> m_storage;
        Reference<XStream> m_rootStream;
        void SAL_CALL
        ensureCreateRootStorage();
        OString SAL_CALL
        encodeSubStorage(const OUString& streamName);
        void SAL_CALL
        insertSubStorage(const OUString& streamName, const OString& content);
        void SAL_CALL
        initRootStorageFromBase64(const OString& content);
        Reference<XStream> SAL_CALL
        createTempFile();
    };
}


#endif /* OLEHANDLER_HXX_ */
