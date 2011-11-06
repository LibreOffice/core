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


#include <stdio.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "testlistener.hxx"

#define U2S(s) OUStringToOString(s, RTL_TEXTENCODING_UTF8).getStr()


using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::IllegalArgumentException;


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
