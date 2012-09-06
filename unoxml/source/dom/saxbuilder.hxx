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

#ifndef DOM_SAXBUILDER_HXX
#define DOM_SAXBUILDER_HXX

#include <stack>
#include <map>

#include <sal/types.h>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder2.hpp>
#include <com/sun/star/xml/dom/SAXDocumentBuilderState.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::sax;

using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::lang::XMultiServiceFactory;

namespace DOM
{

    typedef std::stack< Reference< XNode > > NodeStack;
    typedef std::map< OUString, OUString > NSMap;
    typedef std::map< OUString, OUString > AttrMap;
    typedef std::stack< NSMap > NSStack;

    class  CSAXDocumentBuilder
        : public ::cppu::WeakImplHelper2< XSAXDocumentBuilder2, XServiceInfo >
    {

    private:
        ::osl::Mutex m_Mutex;
        const Reference< XMultiServiceFactory > m_aServiceManager;

        SAXDocumentBuilderState m_aState;
        NodeStack m_aNodeStack;
        NSStack m_aNSStack;

        OUString resolvePrefix(const OUString& aPrefix);

        Reference< XDocument > m_aDocument;
        Reference< XDocumentFragment > m_aFragment;
        Reference< XLocator > m_aLocator;


    public:

        // call for factory
        static Reference< XInterface > getInstance(const Reference < XMultiServiceFactory >& xFactory);

        // static helpers for service info and component management
        static const char* aImplementationName;
    static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(const Reference< XMultiServiceFactory >& rSMgr);

        CSAXDocumentBuilder(const Reference< XMultiServiceFactory >& mgr);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL endDocument()
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL startElement( const OUString& aName,
             const Reference< XAttributeList >& xAttribs )
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL endElement( const OUString& aName )
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL characters( const OUString& aChars )
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL processingInstruction( const OUString& aTarget,
             const OUString& aData )
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );
        virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator )
            throw( RuntimeException, com::sun::star::xml::sax::SAXException );


        // XSAXDocumentBuilder
        virtual SAXDocumentBuilderState SAL_CALL getState()
            throw (RuntimeException);
        virtual void SAL_CALL reset()
            throw (RuntimeException);
        virtual Reference< XDocument > SAL_CALL getDocument()
            throw (RuntimeException);
        virtual Reference< XDocumentFragment > SAL_CALL getDocumentFragment()
            throw (RuntimeException);
        virtual void SAL_CALL startDocumentFragment(const Reference< XDocument >& ownerDoc)
            throw (RuntimeException);
        virtual void SAL_CALL endDocumentFragment()
            throw (RuntimeException);


    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
