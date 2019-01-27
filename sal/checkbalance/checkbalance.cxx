/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stack>
#include <rtl/string.hxx>
#include <sal/log.hxx>

using namespace std;
using namespace rtl;

namespace
{
    static const sal_Char cCurlyOpen = '{';
    static const sal_Char cCurlyClose = '}';
    static const sal_Char cParaOpen = '(';
    static const sal_Char cParaClose = ')';
}

int checkBalance(const OString& rSource)
{
    stack<sal_Char> vExpectedNextClosing;
    for(int nIdx = 0; nIdx < rSource.getLength(); ++nIdx)
    {
        const sal_Char aChar = rSource[nIdx];
        switch(aChar)
        {
            case cCurlyOpen:
            case cParaOpen:
                vExpectedNextClosing.push( aChar == cCurlyOpen ? cCurlyClose : cParaClose);
            break;
            case cCurlyClose:
            case cParaClose:
                if(vExpectedNextClosing.empty() || aChar != vExpectedNextClosing.top())
                    return nIdx;
                vExpectedNextClosing.pop();
        }
    }
    return vExpectedNextClosing.empty() ? -1 : rSource.getLength();
}

int main()
{
    SAL_WARN_IF(checkBalance(OString("if (a(4) > 9) { foo(a(2)); }")) != -1,    "sal.checkbalance", "expected this to be valid.");
    SAL_WARN_IF(checkBalance(OString("for (i=0;i<a(3};i++) { foo(); )")) != 14, "sal.checkbalance", "expected this to find an error on position 14.");
    SAL_WARN_IF(checkBalance(OString("while (true) foo(); } { ()")) != 20,      "sal.checkbalance", "expected this to find an error on position 20.");
    SAL_WARN_IF(checkBalance(OString("if (x) {")) != 8,      "sal.checkbalance", "expected this to find an error on position 8.");

    SAL_INFO("sal.checkbalance", "all testcases checked.");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
