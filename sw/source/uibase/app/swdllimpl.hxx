/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_APP_SWDLLIMPL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_APP_SWDLLIMPL_HXX

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <memory>

namespace sw { class Filters; }

class SwDLL: private boost::noncopyable
{
public:
    static void RegisterFactories();
    static void RegisterInterfaces();
    static void RegisterControls();

    SwDLL();
    ~SwDLL();

    sw::Filters & getFilters();

private:
    std::unique_ptr< sw::Filters > filters_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
