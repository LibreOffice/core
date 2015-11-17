/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_EDITENG_FIELDUPDATER_HXX
#define INCLUDED_EDITENG_FIELDUPDATER_HXX

#include <editeng/editengdllapi.h>
#include <memory>

class EditTextObject;

namespace editeng {

class FieldUpdaterImpl;

/**
 * Wrapper for EditTextObject to handle updating of fields without exposing
 * the internals of EditTextObject structure.
 */
class EDITENG_DLLPUBLIC FieldUpdater
{
    std::unique_ptr<FieldUpdaterImpl> mpImpl;

    FieldUpdater(); // disabled
public:
    FieldUpdater(EditTextObject& rObj);
    FieldUpdater(const FieldUpdater& r);
    ~FieldUpdater();

    /**
     * Set a new table ID to all table fields.
     *
     * @param nTab new table ID
     */
    void updateTableFields(int nTab);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
