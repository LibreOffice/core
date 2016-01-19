/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uiobject.hxx>

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>

class WindowUIObject : public UIObject
{
    VclPtr<vcl::Window> mxWindow;

public:

    WindowUIObject(VclPtr<vcl::Window> xWindow);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual UIObjectType get_type() const override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual void dumpState() const override;

    virtual void dumpHierarchy() const override;

protected:

    virtual OUString get_name() const;

};

class ButtonUIObject : public WindowUIObject
{
    VclPtr<Button> mxButton;
public:

    ButtonUIObject(VclPtr<Button> xButton);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual UIObjectType get_type() const override;

protected:

    virtual OUString get_name() const override;
};

class DialogUIObject : public WindowUIObject
{
    VclPtr<Dialog> mxDialog;

public:

    DialogUIObject(VclPtr<Dialog> xDialog);

    virtual UIObjectType get_type() const override;

    virtual StringMap get_state() override;

protected:

    virtual OUString get_name() const override;
};

class EditUIObject : public WindowUIObject
{
    VclPtr<Edit> mxEdit;

public:

    EditUIObject(VclPtr<Edit> xEdit);

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    virtual UIObjectType get_type() const override;

protected:

    virtual OUString get_name() const override;
};

// TODO: moggi: maybe let it inherit from the button case
class CheckBoxUIObject : public WindowUIObject
{
private:
    VclPtr<CheckBox> mxCheckBox;

public:
    CheckBoxUIObject(VclPtr<CheckBox> xCheckbox);

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    virtual UIObjectType get_type() const override;

protected:

    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
