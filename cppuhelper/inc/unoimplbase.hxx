/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cppuhelper/cppuhelperdllapi.h>
#include <mutex>

namespace cppuhelper
{
/**
This is a straight copy of the include/comphelper/unoimplbase.hxx file, copied here
because it is nigh impossible to move shared code down into the URE layer.
<br/>
This class is meant to be used as a base class for UNO object implementations that
want to use std::mutex for locking.
It meant to be virtually inherited, so the base class is shared between
the UNO object and helper classes like comphelper::OPropertySetHelper
*/
class CPPUHELPER_DLLPUBLIC UnoImplBase
{
public:
    virtual ~UnoImplBase();

protected:
    mutable std::mutex m_aMutex;
    bool m_bDisposed = false;
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
