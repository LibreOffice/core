/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMenu.hxx>
#include <QtInstanceMenu.moc>

#include <QtInstance.hxx>
#include <QtTools.hxx>

#include <vcl/svapp.hxx>
#include <vcl/qt/QtUtils.hxx>

// Property for storing an action name in a menu item
const char* const PROPERTY_ACTION_NAME = "action-name";

QtInstanceMenu::QtInstanceMenu(QMenu* pMenu)
    : m_pMenu(pMenu)
{
    assert(m_pMenu);
}

OUString QtInstanceMenu::popup_at_rect(weld::Widget*, const tools::Rectangle&, weld::Placement)
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceMenu::set_sensitive(const OUString& rIdent, bool bSensitive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            pAction->setEnabled(bSensitive);
    });
}

bool QtInstanceMenu::get_sensitive(const OUString& rIdent) const
{
    SolarMutexGuard g;

    bool bSensitive = false;
    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            bSensitive = pAction->isEnabled();
    });

    return bSensitive;
}

void QtInstanceMenu::set_label(const OUString& rIdent, const OUString& rLabel)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            pAction->setText(toQString(rLabel));
    });
}

OUString QtInstanceMenu::get_label(const OUString& rIdent) const
{
    SolarMutexGuard g;

    OUString sLabel;
    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            sLabel = toOUString(pAction->text());
    });

    return sLabel;
}

void QtInstanceMenu::set_active(const OUString& rIdent, bool bActive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            pAction->setChecked(bActive);
    });
}

bool QtInstanceMenu::get_active(const OUString& rIdent) const
{
    SolarMutexGuard g;

    bool bActive = false;
    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            bActive = pAction->isChecked();
    });

    return bActive;
}

void QtInstanceMenu::set_visible(const OUString& rIdent, bool bVisible)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            pAction->setVisible(bVisible);
    });
}

void QtInstanceMenu::insert(int, const OUString&, const OUString&, const OUString*, VirtualDevice*,
                            const css::uno::Reference<css::graphic::XGraphic>&, TriState)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenu::set_item_help_id(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceMenu::remove(const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rId))
            m_pMenu->removeAction(pAction);
    });
}

void QtInstanceMenu::clear()
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pMenu->clear(); });
}

void QtInstanceMenu::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceMenu::n_children() const
{
    SolarMutexGuard g;

    int nChildCount = 0;
    GetQtInstance().RunInMainThread([&] { nChildCount = m_pMenu->actions().size(); });

    return nChildCount;
}

OUString QtInstanceMenu::get_id(int nPos) const
{
    SolarMutexGuard g;

    OUString sId;
    GetQtInstance().RunInMainThread([&] {
        QList<QAction*> aActions = m_pMenu->actions();
        if (nPos < aActions.size())
            sId = toOUString(aActions.at(nPos)->objectName());
    });

    return sId;
}

void QtInstanceMenu::setActionName(QAction& rAction, const OUString& rActionName)
{
    rAction.setProperty(PROPERTY_ACTION_NAME, toQString(rActionName));
}

QAction* QtInstanceMenu::getAction(const OUString& rIdent) const
{
    QList<QAction*> aActions = m_pMenu->actions();
    for (QAction* pAction : aActions)
    {
        if (pAction && pAction->objectName() == toQString(rIdent))
            return pAction;
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
