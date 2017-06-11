/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <i18nlangtag/languagetag.hxx>
#include <stdio.h>

int main(int /*argc*/, char **argv)
{
    LanguageTag aTag(OUString::createFromAscii(argv[1]));
    OUString aLocaleStr = aTag.getGlibcLocaleString("");
    printf("%s", aLocaleStr.toUtf8().getStr());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
