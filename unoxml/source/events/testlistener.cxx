/*************************************************************************
 *
 *  $RCSfile: testlistener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: lo $ $Date: 2004-02-27 17:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
