/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_XMLTOOLS_HXX
#define INCLUDED_COMPHELPER_XMLTOOLS_HXX

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{
    namespace xml
    {
        COMPHELPER_DLLPUBLIC OString makeXMLChaff();
        COMPHELPER_DLLPUBLIC OString generateGUIDString();

    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
