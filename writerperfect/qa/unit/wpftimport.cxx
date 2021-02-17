/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "wpftimport.hxx"

#include <cppunit/plugin/TestPlugIn.h>

#include <comphelper/seqstream.hxx>

#include <rtl/ref.hxx>

#include <sal/types.h>

css::uno::Reference<css::io::XInputStream> createDummyInput()
{
    rtl::Reference<comphelper::SequenceInputStream> xDummyInput(
        new comphelper::SequenceInputStream(css::uno::Sequence<sal_Int8>()));
    return xDummyInput;
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
