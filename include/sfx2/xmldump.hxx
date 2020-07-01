/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace sfx2
{
/// Implemented by objects that can be dumped to xml for debugging purposes.
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI XmlDump
{
public:
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const = 0;

protected:
    ~XmlDump() noexcept = default;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
