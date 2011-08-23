/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <stdio.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "testlistener.hxx"

#define U2S(s) OUStringToOString(s, RTL_TEXTENCODING_UTF8).getStr()


namespace DOM { namespace events
{

    Reference< XInterface > CTestListener::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        // XXX
        // return static_cast< XXPathAPI* >(new CTestListener());
        return Reference< XInterface >(static_cast<XEventListener*>(new CTestListener(rSMgr)));
    }

    const char* CTestListener::aImplementationName = "com.sun.star.comp.xml.dom.events.TestListener";
    const char* CTestListener::aSupportedServiceNames[] = {
        "com.sun.star.comp.xml.dom.events.TestListener",
        NULL
    };

    OUString CTestListener::_getImplementationName()
    {
        return OUString::createFromAscii(aImplementationName);
    }
    Sequence<OUString> CTestListener::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=NULL; i++) {
            aSequence.realloc(i+1);
            aSequence[i]=(OUString::createFromAscii(aSupportedServiceNames[i]));
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CTestListener::getSupportedServiceNames()
        throw (RuntimeException)
    {
        return CTestListener::_getSupportedServiceNames();
    }

    OUString SAL_CALL CTestListener::getImplementationName()
        throw (RuntimeException)
    {
        return CTestListener::_getImplementationName();
    }

    sal_Bool SAL_CALL CTestListener::supportsService(const OUString& aServiceName)
        throw (RuntimeException)
    {
        Sequence< OUString > supported = CTestListener::_getSupportedServiceNames();
        for (sal_Int32 i=0; i<supported.getLength(); i++)
        {
            if (supported[i] == aServiceName) return sal_True;
        }
        return sal_False;
    }

    // --- XInitialize

    void SAL_CALL CTestListener::initialize(const Sequence< Any >& args) throw(RuntimeException)
    {
        if (args.getLength() < 3) throw IllegalArgumentException(
            OUString::createFromAscii("Wrong number of arguments"), Reference< XInterface >(), 0);

        Reference <XEventTarget> aTarget;
        if(! (args[0] >>= aTarget)) throw IllegalArgumentException(
                OUString::createFromAscii("Illegal argument 1"), Reference< XInterface >(), 1);

        OUString aType;
        if (! (args[1] >>= aType))
            throw IllegalArgumentException(OUString::createFromAscii("Illegal argument 2"), Reference< XInterface >(), 2);

        sal_Bool bCapture = sal_False;
        if(! (args[2]  >>=  bCapture)) throw IllegalArgumentException(
            OUString::createFromAscii("Illegal argument 3"), Reference< XInterface >(), 3);

        if(! (args[3] >>= m_name)) m_name = OUString::createFromAscii("<unnamed listener>");

        m_target = aTarget;
        m_type = aType;
        m_capture = bCapture;

        m_target->addEventListener(m_type, Reference< XEventListener >(this), m_capture);
        

    }

    CTestListener::~CTestListener()
    {
        fprintf(stderr, "CTestListener::~CTestListener()\n");
        if( m_target.is())
            m_target->removeEventListener(m_type, Reference< XEventListener >(this), m_capture);
    }

    // --- XEventListener

    void SAL_CALL CTestListener::handleEvent(const Reference< XEvent >& evt) throw (RuntimeException)
    {
        FILE* f = fopen("C:\\listener.out", "a");
        fprintf(f, "CTestListener::handleEvent in %s\n", U2S(m_name));
        fprintf(f, "    type: %s\n\n", OUStringToOString(evt->getType(), RTL_TEXTENCODING_ASCII_US).getStr());        
        fclose(f);

    }

}}
