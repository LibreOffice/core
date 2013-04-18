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
 */

#ifndef __TEST_TEXT_XTEXT_HXX__
#define __TEST_TEXT_XTEXT_HXX__

#include "test/testdllapi.hxx"

#include <com/sun/star/text/XText.hpp>

using namespace com::sun::star;

namespace apitest {

class OOO_DLLPUBLIC_TEST XText
{
public:
    virtual ~XText();

    virtual uno::Reference<uno::XInterface> init() = 0;
    virtual uno::Reference<text::XTextContent> getTextContent() = 0;

    void testInsertRemoveTextContent();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
