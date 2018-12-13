/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
//#include <sal/main.h>
#include <rtl/string.hxx>

using namespace std;
using namespace rtl;

// rtl::OString is the LibreOffice string to do stuff like in the example video
// you can find docs about it here:
// https://api.libreoffice.org/docs/cpp/ref/a00148.html
//
// note: In "real world" LibreOffice code, you will much more likely use rtl::OUString
// which handles Unicode properly too (e.g. for Umlauts or ~any non-ASCII character):
// https://api.libreoffice.org/docs/cpp/ref/a00152.html
//
void namedDiamond(OString& name)
{
    for (int i = 0; i < name.getLength(); i++)
    {
        cout << name.copy(0, i + 1) << endl;
    }
    for (int i = 0; i < name.getLength(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            cout << " ";
        }
        cout << name.copy(i) << endl;
    }
}

// "int main()" works for simple stuff like this -- but in general, in
// LibreOffice programs you should use "SAL_IMPLEMENT_MAIN()" or a similar
// macro from sal/main.h.
// Those do a bit of setup (which might differ on platforms), including:
// - starting a global Timer
// - storing the arguments that were passed, so thet they can later be
//   retrieved e.g. by osl_getCommandArg(...)
//   https://api.libreoffice.org/docs/cpp/ref/a00380.html#a7d4dde17dd96c3b853d4f120862e9c4b
//   (this needs SAL_IMPLEMENT_MAIN_WITH_ARGS of course)
int main()
{
    cout << "Hello World!" << endl;
    OString name = "Bjoern";
    namedDiamond(name);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
