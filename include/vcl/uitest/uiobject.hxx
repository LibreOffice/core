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
     * returns the state of the wrapped UI object
     */
    virtual StringMap get_state();

    /**
     * executes an action on the wrapped UI object,
     * possibly with some additional parameters
     */
    virtual void execute(const OUString& rAction,
            const StringMap& rParameters);

    virtual UIObjectType getType() const;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
