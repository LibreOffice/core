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

#pragma once

#include <sal/config.h>

#include <string_view>

#include <o3tl/string_view.hxx>

namespace framework{

/**
    some protocols must be checked during loading or dispatching URLs manually
    It can be necessary to decide, if a URL represent a non visible content or
    a real visible component.
 */

// indicates a loadable content in general!
#define SPECIALPROTOCOL_PRIVATE           "private:"
// indicates loading of components using a model directly
#define SPECIALPROTOCOL_PRIVATE_OBJECT    u"private:object"
// indicates loading of components using a stream only
#define SPECIALPROTOCOL_PRIVATE_STREAM    u"private:stream"
// indicates creation of empty documents
#define SPECIALPROTOCOL_PRIVATE_FACTORY   u"private:factory"
// internal protocol of the sfx project for generic dispatch functionality
#define SPECIALPROTOCOL_SLOT              u"slot:"
// external representation of the slot protocol using names instead of id's
#define SPECIALPROTOCOL_UNO               u".uno:"
// special sfx protocol to execute macros
#define SPECIALPROTOCOL_MACRO             u"macro:"
// generic way to start uno services during dispatch
#define SPECIALPROTOCOL_SERVICE           u"service:"
// for sending mails
#define SPECIALPROTOCOL_MAILTO            u"mailto:"
// for sending news
#define SPECIALPROTOCOL_NEWS              u"news:"

/** well known protocols */
enum class EProtocol
{
    PrivateObject,
    PrivateStream,
    PrivateFactory,
    Slot,
    Uno,
    Macro,
    Service,
    MailTo,
    News
};

class ProtocolCheck
{
    public:

    /**
        it checks if given URL match the required protocol only
        It should be used instead of specifyProtocol() if only this question
        is interesting to perform the code. We must not check for all possible protocols here...
     */
    static bool isProtocol( std::u16string_view sURL, EProtocol eRequired )
    {
        bool bRet = false;
        switch(eRequired)
        {
            case EProtocol::PrivateObject:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_PRIVATE_OBJECT);
                break;
            case EProtocol::PrivateStream:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_PRIVATE_STREAM);
                break;
            case EProtocol::PrivateFactory:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_PRIVATE_FACTORY);
                break;
            case EProtocol::Slot:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_SLOT);
                break;
            case EProtocol::Uno:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_UNO);
                break;
            case EProtocol::Macro:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_MACRO);
                break;
            case EProtocol::Service:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_SERVICE);
                break;
            case EProtocol::MailTo:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_MAILTO);
                break;
            case EProtocol::News:
                bRet = o3tl::starts_with(sURL, SPECIALPROTOCOL_NEWS);
                break;
            default:
                bRet = false;
                break;
        }
        return bRet;
    }
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
