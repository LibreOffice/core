/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include "uiobject.hxx"
#include <vcl/xtextedt.hxx>

namespace basctl
{
EditorWindowUIObject::EditorWindowUIObject(const VclPtr<basctl::EditorWindow>& xEditorWindow)
    : WindowUIObject(xEditorWindow)
    , mxEditorWindow(xEditorWindow)
{
}

StringMap EditorWindowUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    ExtTextEngine* pEditEngine = mxEditorWindow->GetEditEngine();
    sal_Int32 i, nParas;
    OUStringBuffer aRes;
    for (i = 0, nParas = pEditEngine->GetParagraphCount(); i < nParas; ++i)
    {
        aRes.append(pEditEngine->GetText(i) + "\n");
    }

    aMap[u"Text"_ustr] = aRes.makeStringAndClear();

    return aMap;
}

std::unique_ptr<UIObject> EditorWindowUIObject::create(vcl::Window* pWindow)
{
    basctl::EditorWindow* pEditorWindow = dynamic_cast<basctl::EditorWindow*>(pWindow);
    assert(pEditorWindow);
    return std::unique_ptr<UIObject>(new EditorWindowUIObject(pEditorWindow));
}

OUString EditorWindowUIObject::get_name() const { return u"EditorWindowUIObject"_ustr; }

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
