/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_TEXT_XFOOTNOTE_HXX
#define INCLUDED_TEST_TEXT_XFOOTNOTE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <test/testdllapi.hxx>

#include <com/sun/star/text/XFootnote.hpp>

namespace apitest {

class OOO_DLLPUBLIC_TEST XFootnote
{
public:
    virtual ~XFootnote() {}

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;

    void testgetLabel();
    void testsetLabel();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
