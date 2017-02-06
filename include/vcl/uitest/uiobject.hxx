/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UITEST_UIOBJECT_HXX
#define INCLUDED_VCL_UITEST_UIOBJECT_HXX

#include <rtl/ustring.hxx>
#include <map>
#include <memory>

#include <vcl/window.hxx>
#include <vcl/spin.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/tabctrl.hxx>

#include <vcl/dllapi.h>

#include <set>

typedef std::map<const OUString, OUString> StringMap;

/**
 * This class wraps a UI object like vcl::Window and provides
 * an interface for the UI testing.
 *
 * This class should only have virtual methods.
 */
class UITEST_DLLPUBLIC UIObject
{
    UIObject(UIObject &) = delete;
    void operator =(UIObject) = delete;

public:
    UIObject() = default;

    virtual ~UIObject();

    /**
     * Returns the state of the wrapped UI object as a
     * string key value map.
     */
    virtual StringMap get_state();

    /**
     * Executes an action on the wrapped UI object,
     * possibly with some additional parameters
     */
    virtual void execute(const OUString& rAction,
            const StringMap& rParameters);

    /**
     * Returns the type of the UIObject. Additional information might
     * be available through UIObject::get_state().
     */
    virtual OUString get_type() const;

    /**
     * Returns the child of the current UIObject with the corresponding id.
     * If no object with that id is being found returns a nullptr.
     *
     */
    virtual std::unique_ptr<UIObject> get_child(const OUString& rID);

    /**
     * Returns a set containing all descendants of the object.
     */
    virtual std::set<OUString> get_children() const;

    /**
     * Currently an internal method to dump the state of the current UIObject as represented by get_state().
     *
     * This method should not be exposed to the outside world.
     *
     */
    virtual OUString dumpState() const;

    /**
     * Currently an internal method to dump the parent-child relationship starting from the current top focus window.
     *
     * This method should not be exposed to the outside world.
     *
     */
    virtual OUString dumpHierarchy() const;
};

class UITEST_DLLPUBLIC WindowUIObject : public UIObject
{
    VclPtr<vcl::Window> mxWindow;

public:

    WindowUIObject(VclPtr<vcl::Window> xWindow);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual OUString get_type() const override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

    virtual OUString dumpState() const override;

    virtual OUString dumpHierarchy() const override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const;

};

// TODO: moggi: what about push buttons?
class UITEST_DLLPUBLIC ButtonUIObject : public WindowUIObject
{
    VclPtr<Button> mxButton;
public:

    ButtonUIObject(VclPtr<Button> xButton);
    virtual ~ButtonUIObject() override;

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC DialogUIObject : public WindowUIObject
{
    VclPtr<Dialog> mxDialog;

public:

    DialogUIObject(VclPtr<Dialog> xDialog);
    virtual ~DialogUIObject() override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC EditUIObject : public WindowUIObject
{
    VclPtr<Edit> mxEdit;

public:

    EditUIObject(VclPtr<Edit> xEdit);
    virtual ~EditUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

// TODO: moggi: maybe let it inherit from the button case
class UITEST_DLLPUBLIC CheckBoxUIObject : public WindowUIObject
{
private:
    VclPtr<CheckBox> mxCheckBox;

public:
    CheckBoxUIObject(VclPtr<CheckBox> xCheckbox);
    virtual ~CheckBoxUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC RadioButtonUIObject : public WindowUIObject
{
private:
    VclPtr<RadioButton> mxRadioButton;

public:
    RadioButtonUIObject(VclPtr<RadioButton> xCheckbox);
    virtual ~RadioButtonUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC TabPageUIObject : public WindowUIObject
{
private:
    VclPtr<TabPage> mxTabPage;
public:
    TabPageUIObject(VclPtr<TabPage> xTabPage);
    virtual ~TabPageUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC ListBoxUIObject : public WindowUIObject
{
private:
    VclPtr<ListBox> mxListBox;

public:

    ListBoxUIObject(VclPtr<ListBox> xListBox);
    virtual ~ListBoxUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

// TODO: moggi: should it inherit from EditUIObject?
class UITEST_DLLPUBLIC ComboBoxUIObject : public WindowUIObject
{
private:
    VclPtr<ComboBox> mxComboBox;

public:

    ComboBoxUIObject(VclPtr<ComboBox> xListBox);
    virtual ~ComboBoxUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC SpinUIObject : public WindowUIObject
{
private:
    VclPtr<SpinButton> mxSpinButton;

public:

    SpinUIObject(VclPtr<SpinButton> xSpinButton);
    virtual ~SpinUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC SpinFieldUIObject : public EditUIObject
{
    VclPtr<SpinField> mxSpinField;

public:

    SpinFieldUIObject(VclPtr<SpinField> xEdit);
    virtual ~SpinFieldUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class UITEST_DLLPUBLIC TabControlUIObject : public WindowUIObject
{
private:
    VclPtr<TabControl> mxTabControl;

public:

    TabControlUIObject(VclPtr<TabControl> mxTabControl);
    virtual ~TabControlUIObject() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
