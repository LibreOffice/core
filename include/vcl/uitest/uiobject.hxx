/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <map>
#include <memory>

#include <vcl/window.hxx>
#include <vcl/dllapi.h>

enum class UIObjectType
{
    WINDOW,
    DIALOG,
    BUTTON,
    EDIT,
    CHECKBOX,
    LISTBOX,
    COMBOBOX,
    SPINBUTTON,
    TABPAGE,
    SPINFIELD,
    UNKNOWN
};

typedef std::map<const OUString, OUString> StringMap;

/**
 * This class wraps a UI object like vcl::Window and provides
 * an interface for the UI testing.
 *
 * This class should only have virtual methods.
 */
class UITEST_DLLPUBLIC UIObject
{
public:

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
     *
     */
    virtual UIObjectType get_type() const;

    /**
     * Returns the child of the current UIObject with the corresponding id.
     * If no object with that id is being found returns a nullptr.
     *
     */
    virtual std::unique_ptr<UIObject> get_child(const OUString& rID);

    /**
     * Currently an internal method to dump the state of the current UIObject as represented by get_state().
     *
     * This method should not be exposed to the outside world.
     *
     */
    virtual void dumpState() const;

    /**
     * Currently an internal method to dump the parent-child relationship starting from the current top focus window.
     *
     * This method should not be exposed to the outside world.
     *
     */
    virtual void dumpHierarchy() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
