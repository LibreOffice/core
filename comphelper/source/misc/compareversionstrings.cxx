/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ctype.h>
#include <string.h>

#include <comphelper/string.hxx>
#include <rtl/ustring.hxx>

namespace comphelper { namespace string {

namespace {

// BSD licensed, from http://git.musl-libc.org/cgit/musl/plain/src/string/strverscmp.c

int strverscmp(const char *l, const char *r)
{
    int haszero=1;
    while (*l==*r) {
        if (!*l) return 0;

        if (*l=='0') {
            if (haszero==1) {
                haszero=0;
            }
        } else if (isdigit(*l)) {
            if (haszero==1) {
                haszero=2;
            }
        } else {
            haszero=1;
        }
        l++; r++;
    }
    if (haszero==1 && (*l=='0' || *r=='0')) {
        haszero=0;
    }
    if ((isdigit(*l) && isdigit(*r) ) && haszero) {
        size_t lenl=0, lenr=0;
        while (isdigit(l[lenl]) ) lenl++;
        while (isdigit(r[lenr]) ) lenr++;
        if (lenl==lenr) {
            return (*l -  *r);
        } else if (lenl>lenr) {
            return 1;
        } else {
            return -1;
        }
    } else {
        return (*l -  *r);
    }
}

} // anonymous namespace

int compareVersionStrings(const OUString& a, const OUString& b)
{
    return strverscmp(OUStringToOString(a, RTL_TEXTENCODING_UTF8).getStr(), OUStringToOString(b, RTL_TEXTENCODING_UTF8).getStr());
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
