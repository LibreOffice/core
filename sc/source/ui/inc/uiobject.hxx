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

class ScGridWindow;
class ScDBFunc;
class ScDrawView;
class ScTabViewShell;

class ScGridWinUIObject : public WindowUIObject
{
    VclPtr<ScGridWindow> mxGridWindow;

public:

    ScGridWinUIObject(const VclPtr<ScGridWindow>& xGridWin);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual std::set<OUString> get_children() const override;

protected:

    virtual OUString get_name() const override;

private:

    ScDBFunc* getDBFunc();
    ScDrawView* getDrawView();
    ScTabViewShell* getViewShell();
};

class ScNavigatorDlg;

class ScNavigatorDlgUIObject : public WindowUIObject
{
    VclPtr<ScNavigatorDlg> mxScNavigatorDlg;

public:

    ScNavigatorDlgUIObject(const VclPtr<ScNavigatorDlg>& xScNavigatorDlg);

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
