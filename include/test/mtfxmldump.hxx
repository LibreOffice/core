/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MFTXMLDUMP_HXX
#define MFTXMLDUMP_HXX

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <vcl/gdimtf.hxx>
#include <vector>

class OOO_DLLPUBLIC_TEST MetafileXmlDump
{
    std::vector<bool> maFilter;
    SvStream& mrStream;

public:
    MetafileXmlDump(SvStream& rStream);
    virtual ~MetafileXmlDump();

    void filterActionType(const sal_uInt16 nActionType, bool bShouldFilter);
    void filterAllActionTypes();
    void filterNoneActionTypes();

    void dump(GDIMetaFile& rMetaFile);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
