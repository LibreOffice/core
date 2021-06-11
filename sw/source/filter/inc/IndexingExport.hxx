/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <ModelTraverser.hxx>
#include <tools/XmlWriter.hxx>

namespace sw
{
class SW_DLLPUBLIC IndexingExport
{
private:
    ModelTraverser m_aModelTraverser;
    tools::XmlWriter m_aXmlWriter;

public:
    IndexingExport(SvStream& rStream, SwDoc* pDoc);

    bool runExport();
};

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
