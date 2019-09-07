/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <svtools/simptabl.hxx>
#include <vcl/uitest/uiobject.hxx>

class SvSimpleTable;

class SimpleTableUIObject : public TreeListUIObject
{
public:
    SimpleTableUIObject(const VclPtr<SvSimpleTable>& xTable);

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> createFromContainer(vcl::Window* pWindow);

protected:
    virtual OUString get_type() const override;

private:
    VclPtr<SvSimpleTable> mxTable;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
