/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/uitest/uiobject.hxx>
#include "baside2.hxx"

namespace basctl
{
class EditorWindow;

class EditorWindowUIObject : public WindowUIObject
{
    VclPtr<basctl::EditorWindow> mxEditorWindow;

public:
    EditorWindowUIObject(const VclPtr<basctl::EditorWindow>& xEditorWindow);

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:
    virtual OUString get_name() const override;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
