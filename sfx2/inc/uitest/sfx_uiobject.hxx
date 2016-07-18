/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uiobject.hxx>
#include <memory>

class SfxTabDialog;

namespace sfx2 { namespace sidebar {

class Panel;
class Deck;

} }

class SfxTabDialogUIObject : public WindowUIObject
{
private:

    VclPtr<SfxTabDialog> mxTabDialog;

public:

    SfxTabDialogUIObject(VclPtr<SfxTabDialog> xTabDialog);

    virtual ~SfxTabDialogUIObject();

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class DeckUIObject : public WindowUIObject
{
private:

    VclPtr<sfx2::sidebar::Deck> mxDeck;
public:

    DeckUIObject(VclPtr<sfx2::sidebar::Deck> xDeck);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

class PanelUIObject : public WindowUIObject
{
private:

    VclPtr<sfx2::sidebar::Panel> mxPanel;

public:

    PanelUIObject(VclPtr<sfx2::sidebar::Panel> xPanel);

    virtual StringMap get_state() override;

    virtual void execute(const OUString& rAction,
            const StringMap& rParameters) override;

    static std::unique_ptr<UIObject> create(vcl::Window* pWindow);

protected:

    virtual OUString get_name() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
