/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _DOCXHELPER_HXX_
#define _DOCXHELPER_HXX_

#include <rtl/ustring.hxx>

struct DocxStringTokenMap
{
    const char* pToken;
    sal_Int32 nToken;
};

sal_Int32 DocxStringGetToken(DocxStringTokenMap const * pMap, OUString aName);

#endif // _DOCXHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
