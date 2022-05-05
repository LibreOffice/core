/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <unordered_map>
#include <set>
#include <vector>
#include <memory>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.h>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <svx/swframetypes.hxx>
#include <calbck.hxx>

#include "swdllapi.h"

class SdrPage;
class SdrObject;
class SfxItemSet;
class SwFrameFormat;
class SwFrameFormats;
class SwFormatContent;
class SwDoc;

namespace tools
{
class Rectangle;
}
class ZSortFly;
namespace com::sun::star::drawing
{
class XShape;
}
namespace com::sun::star::text
{
class XTextFrame;
}

class SwTextBoxHandler : SwClient
{
    bool m_bLocked;
    std::unordered_map<const SdrObject*, SwFrameFormat*> m_TextBoxTable;
    SwFrameFormat& m_rShape;

public:
    SwTextBoxHandler() = delete;

    SwTextBoxHandler(SwFrameFormat&);

    SwTextBoxHandler(const SwTextBoxHandler&) = default;

    ~SwTextBoxHandler();

    void Create(SdrObject*);
    void Destroy(const SdrObject*);
    void DestroyAll();
    void Replace(SdrObject*, css::uno::Reference<css::text::XTextFrame>&);

    void Sync(const SfxItemSet&, const SdrObject*);
    void SyncAll(const SfxItemSet&);

    const SwFrameFormat& GetShape() const { return m_rShape; }
    SwFrameFormat* Get(const SdrObject* pObj) const { return m_TextBoxTable.at(pObj); }
    void Set(SdrObject*, SwFrameFormat*);

    size_t Count() const { return m_TextBoxTable.size(); }

    bool Has(const SdrObject* pObj) const { return m_TextBoxTable.count(pObj); }

    virtual void SwClientNotify(const SwModify&, const SfxHint& rHint) override;

private:
    void Lock() { m_bLocked = true; }
    void Unlock() { m_bLocked = false; }
    bool IsLocked() const { return m_bLocked; }
};

typedef std::shared_ptr<SwTextBoxHandler> SwTextBoxHandlerPtr;
typedef OUString SwTextBoxHandlingException;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
