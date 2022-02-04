/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcusxml.hxx>

#include <vcl/weld.hxx>

ScOrcusXMLTreeParam::EntryData::EntryData(EntryType eType)
    : mnNamespaceID(0)
    , meType(eType)
    , maLinkedPos(ScAddress::INITIALIZE_INVALID)
    , mbRangeParent(false)
    , mbLeafNode(true)
{}

ScOrcusXMLTreeParam::EntryData* ScOrcusXMLTreeParam::getUserData(const weld::TreeView& rControl, const weld::TreeIter& rEntry)
{
    return weld::fromId<ScOrcusXMLTreeParam::EntryData*>(rControl.get_id(rEntry));
}

ScOrcusImportXMLParam::CellLink::CellLink(const ScAddress& rPos, const OString& rPath) :
    maPos(rPos), maPath(rPath) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
