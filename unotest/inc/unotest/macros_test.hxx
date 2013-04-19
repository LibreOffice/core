/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTEST_MACROS_TEST_HXX
#define INCLUDED_UNOTEST_MACROS_TEST_HXX

#include <rtl/ustring.hxx>
#include "unotest/detail/unotestdllapi.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

struct TestMacroInfo
{
    OUString sFileBaseName;
    OUString sMacroUrl;
};

namespace unotest {

class OOO_DLLPUBLIC_UNOTEST MacrosTest
{
public:
    com::sun::star::uno::Reference< com::sun::star::lang::XComponent > loadFromDesktop(const OUString& rURL, const char* pDocService = NULL);

protected:
    com::sun::star::uno::Reference< com::sun::star::frame::XDesktop2> mxDesktop;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
