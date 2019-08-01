/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <map>
#include <memory>
#include <string>

struct CallbackFlushHandler
{
};

struct LibLODocument_Impl
{
    std::map<size_t, std::shared_ptr<CallbackFlushHandler>> mpCallbackFlushHandlers;
};

void foo(LibLODocument_Impl* pDoc)
{
    std::map<int, int> aMap;
    if (aMap[0]) // expected-error {{will create an empty entry in the map, you sure about that, rather use count()2 [loplugin:mapindex]}}
        ;

    // expected-error@+1 {{will create an empty entry in the map, you sure about that, rather use count()1 [loplugin:mapindex]}}
    if (pDoc->mpCallbackFlushHandlers[0])
        ;
}

void no_warning_expected(const std::string& payload)
{
    for (size_t numberPos = 0; numberPos < payload.length(); ++numberPos)
    {
        if (payload[numberPos] == ',')
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
