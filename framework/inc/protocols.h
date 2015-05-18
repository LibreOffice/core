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

/*TODO outline this implementation :-) */

#ifndef INCLUDED_FRAMEWORK_INC_PROTOCOLS_H
#define INCLUDED_FRAMEWORK_INC_PROTOCOLS_H

#include <rtl/ustring.hxx>

namespace framework{

/**
    some protocols must be checked during loading or dispatching URLs manually
    It can be necessary to decide, if a URL represent a non visible content or
    a real visible component.
 */

// indicates a loadable content in general!
#define SPECIALPROTOCOL_PRIVATE           "private:"
// indicates loading of components using a model directly
#define SPECIALPROTOCOL_PRIVATE_OBJECT    "private:object"
// indicates loading of components using a stream only
#define SPECIALPROTOCOL_PRIVATE_STREAM    "private:stream"
// indicates creation of empty documents
#define SPECIALPROTOCOL_PRIVATE_FACTORY   "private:factory"
// internal protocol of the sfx project for generic dispatch funtionality
#define SPECIALPROTOCOL_SLOT              "slot:"
// external representation of the slot protocol using names instead of id's
#define SPECIALPROTOCOL_UNO               ".uno:"
// special sfx protocol to execute macros
#define SPECIALPROTOCOL_MACRO             "macro:"
// generic way to start uno services during dispatch
#define SPECIALPROTOCOL_SERVICE           "service:"
// for sending mails
#define SPECIALPROTOCOL_MAILTO            "mailto:"
// for sending news
#define SPECIALPROTOCOL_NEWS              "news:"

class ProtocolCheck
{
    public:

    /**
        enums for well known protocols
     */
    enum EProtocol
    {
        E_UNKNOWN_PROTOCOL  ,
        E_PRIVATE           ,
        E_PRIVATE_OBJECT    ,
        E_PRIVATE_STREAM    ,
        E_PRIVATE_FACTORY   ,
        E_SLOT              ,
        E_UNO               ,
        E_MACRO             ,
        E_SERVICE           ,
        E_MAILTO            ,
        E_NEWS
    };

    /**
        it checks, if the given URL string match one of the well known protocols.
        It returns the right enum value.
        Protocols are defined above ...
     */
    static EProtocol specifyProtocol( const OUString& sURL )
    {
        // because "private:" is part of e.g. "private:object" too ...
        // we must check it before all other ones!!!
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE)))
            return E_PRIVATE;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE_OBJECT)))
            return E_PRIVATE_OBJECT;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE_STREAM)))
            return E_PRIVATE_STREAM;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE_FACTORY)))
            return E_PRIVATE_FACTORY;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_SLOT)))
            return E_SLOT;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_UNO)))
            return E_UNO;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_MACRO)))
            return E_MACRO;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_SERVICE)))
            return E_SERVICE;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_MAILTO)))
            return E_MAILTO;
        else
        if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_NEWS)))
            return E_NEWS;
        else
            return E_UNKNOWN_PROTOCOL;
    }

    /**
        it checks if given URL match the required protocol only
        It should be used instead of specifyProtocol() if only this question
        is interesting to perform the code. We must not check for all possible protocols here...
     */
    static bool isProtocol( const OUString& sURL, EProtocol eRequired )
    {
        bool bRet = sal_False;
        switch(eRequired)
        {
            case E_PRIVATE:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE));
                break;
            case E_PRIVATE_OBJECT:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE_OBJECT));
                break;
            case E_PRIVATE_STREAM:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE_STREAM));
                break;
            case E_PRIVATE_FACTORY:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_PRIVATE_FACTORY));
                break;
            case E_SLOT:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_SLOT));
                break;
            case E_UNO:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_UNO));
                break;
            case E_MACRO:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_MACRO));
                break;
            case E_SERVICE:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_SERVICE));
                break;
            case E_MAILTO:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_MAILTO));
                break;
            case E_NEWS:
                bRet = sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(SPECIALPROTOCOL_NEWS));
                break;
            default:
                bRet = sal_False;
                break;
        }
        return bRet;
    }
};

}

#endif // INCLUDED_FRAMEWORK_INC_PROTOCOLS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
