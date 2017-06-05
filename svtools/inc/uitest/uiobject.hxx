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

class SvTreeListBox;
class SvTreeListEntry;
class SvSimpleTable;

class TreeListUIObject : public WindowUIObject
{
public:
    TreeListUIObject(const VclPtr<SvTreeListBox>& xTreeList);

    virtual StringMap get_state() override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

protected:

    virtual OUString get_name() const override;

private:

    VclPtr<SvTreeListBox> mxTreeList;
};

class TreeListEntryUIObject : public UIObject
{
public:

    TreeListEntryUIObject(const VclPtr<SvTreeListBox>& xTreeList, SvTreeListEntry* pEntry);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    virtual std::unique_ptr<UIObject> get_child(const OUString& rID) override;

    virtual std::set<OUString> get_children() const override;

    virtual OUString get_type() const override;

private:

    VclPtr<SvTreeListBox> mxTreeList;

    SvTreeListEntry* mpEntry;
};

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
