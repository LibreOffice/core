/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceMenu.hxx>
#include <QtInstanceToolbar.hxx>
#include <QtInstanceToolbar.moc>

QtInstanceToolbar::QtInstanceToolbar(QToolBar* pToolBar)
    : QtInstanceWidget(pToolBar)
    , m_pToolBar(pToolBar)
{
    assert(m_pToolBar);

    const QList<QAction*> aActions = m_pToolBar->actions();
    for (QAction* pAction : aActions)
    {
        if (QToolButton* pButton = qobject_cast<QToolButton*>(m_pToolBar->widgetForAction(pAction)))
        {
            connect(pButton, &QToolButton::clicked, pButton,
                    [this, pButton] { toolButtonClicked(pButton->objectName()); });
        }
    }
}

void QtInstanceToolbar::set_item_sensitive(const OUString& rIdent, bool bSensitive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(rIdent).setEnabled(bSensitive); });
}

bool QtInstanceToolbar::get_item_sensitive(const OUString& rIdent) const
{
    SolarMutexGuard g;

    bool bSensitive = false;
    GetQtInstance().RunInMainThread([&] { bSensitive = getWidget(rIdent).isEnabled(); });

    return bSensitive;
}
void QtInstanceToolbar::set_item_active(const OUString& rIdent, bool bActive)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QToolButton& rToolButton = getToolButton(rIdent);
        assert(rToolButton.isCheckable() && "Tool button is not checkable");
        rToolButton.setChecked(bActive);
    });
}

bool QtInstanceToolbar::get_item_active(const OUString& rIdent) const
{
    SolarMutexGuard g;

    bool bActive = false;
    GetQtInstance().RunInMainThread([&] {
        QToolButton& rToolButton = getToolButton(rIdent);
        assert(rToolButton.isCheckable() && "Tool button is not checkable");
        bActive = rToolButton.isChecked();
    });

    return bActive;
}

void QtInstanceToolbar::set_menu_item_active(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceToolbar::get_menu_item_active(const OUString&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceToolbar::set_item_menu(const OUString& rIdent, weld::Menu* pMenu)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QtInstanceMenu* pQtInstanceMenu = dynamic_cast<QtInstanceMenu*>(pMenu);
        assert((!pMenu || pQtInstanceMenu) && "Non-Qt menu");
        QMenu* pQMenu = pQtInstanceMenu ? pQtInstanceMenu->getMenu() : nullptr;
        getToolButton(rIdent).setMenu(pQMenu);
    });
}

void QtInstanceToolbar::set_item_popover(const OUString&, weld::Widget*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_visible(const OUString& rIdent, bool bVisible)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(rIdent).setVisible(bVisible); });
}

void QtInstanceToolbar::set_item_help_id(const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceToolbar::get_item_visible(const OUString& rIdent) const
{
    SolarMutexGuard g;

    bool bVisible = false;
    GetQtInstance().RunInMainThread([&] { bVisible = getWidget(rIdent).isVisible(); });

    return bVisible;
}

void QtInstanceToolbar::set_item_label(const OUString& rIdent, const OUString& rLabel)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getToolButton(rIdent).setText(toQString(rLabel)); });
}

OUString QtInstanceToolbar::get_item_label(const OUString& rIdent) const
{
    SolarMutexGuard g;

    OUString sLabel;
    GetQtInstance().RunInMainThread([&] { sLabel = toOUString(getToolButton(rIdent).text()); });

    return sLabel;
}

void QtInstanceToolbar::set_item_tooltip_text(const OUString& rIdent, const OUString& rTip)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(rIdent).setToolTip(toQString(rTip)); });
}

OUString QtInstanceToolbar::get_item_tooltip_text(const OUString& rIdent) const
{
    SolarMutexGuard g;

    OUString sToolTip;
    GetQtInstance().RunInMainThread(
        [&] { sToolTip = toOUString(getToolButton(rIdent).toolTip()); });

    return sToolTip;
}

void QtInstanceToolbar::set_item_icon_name(const OUString& rIdent, const OUString& rIconName)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread(
        [&] { getToolButton(rIdent).setIcon(loadQPixmapIcon(rIconName)); });
}

void QtInstanceToolbar::set_item_image_mirrored(const OUString&, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::set_item_image(const OUString& rIdent,
                                       const css::uno::Reference<css::graphic::XGraphic>& rIcon)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getToolButton(rIdent).setIcon(toQPixmap(rIcon)); });
}

void QtInstanceToolbar::set_item_image(const OUString& rIdent, VirtualDevice* pDevice)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        const QPixmap aIconPixmap = pDevice ? toQPixmap(*pDevice) : QPixmap();
        getToolButton(rIdent).setIcon(aIconPixmap);
    });
}

void QtInstanceToolbar::insert_item(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceToolbar::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceToolbar::get_n_items() const
{
    SolarMutexGuard g;

    int nItemCount = 0;
    GetQtInstance().RunInMainThread([&] { nItemCount = m_pToolBar->actions().count(); });

    return nItemCount;
}

OUString QtInstanceToolbar::get_item_ident(int nIndex) const
{
    SolarMutexGuard g;

    OUString sIdent;
    GetQtInstance().RunInMainThread([&] { sIdent = toOUString(getWidget(nIndex).objectName()); });

    return sIdent;
}

void QtInstanceToolbar::set_item_ident(int nIndex, const OUString& rIdent)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(nIndex).setObjectName(toQString(rIdent)); });
}

void QtInstanceToolbar::set_item_label(int nIndex, const OUString& rLabel)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getToolButton(nIndex).setText(toQString(rLabel)); });
}

void QtInstanceToolbar::set_item_image(int nIndex,
                                       const css::uno::Reference<css::graphic::XGraphic>& rIcon)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getToolButton(nIndex).setIcon(toQPixmap(rIcon)); });
}

void QtInstanceToolbar::set_item_tooltip_text(int nIndex, const OUString& rTip)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(nIndex).setToolTip(toQString(rTip)); });
}

void QtInstanceToolbar::set_item_accessible_name(int nIndex, const OUString& rName)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(nIndex).setAccessibleName(toQString(rName)); });
}

void QtInstanceToolbar::set_item_accessible_name(const OUString& rIdent, const OUString& rName)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] { getWidget(rIdent).setAccessibleName(toQString(rName)); });
}

vcl::ImageType QtInstanceToolbar::get_icon_size() const
{
    assert(false && "Not implemented yet");
    return vcl::ImageType::Size32;
}

void QtInstanceToolbar::set_icon_size(vcl::ImageType) { assert(false && "Not implemented yet"); }

sal_uInt16 QtInstanceToolbar::get_modifier_state() const
{
    assert(false && "Not implemented yet");
    return 0;
}

int QtInstanceToolbar::get_drop_index(const Point&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

QWidget& QtInstanceToolbar::getWidget(const OUString& rIdent) const
{
    assert(GetQtInstance().IsMainThread());

    QWidget* pWidget = m_pToolBar->findChild<QWidget*>(toQString(rIdent));
    assert(pWidget && "No widget with the given ID found");
    return *pWidget;
}

QWidget& QtInstanceToolbar::getWidget(int nIndex) const
{
    assert(GetQtInstance().IsMainThread());

    QAction* pAction = m_pToolBar->actions().at(nIndex);
    assert(pAction);
    QWidget* pWidget = m_pToolBar->widgetForAction(pAction);
    assert(pWidget);
    return *pWidget;
}

QToolButton& QtInstanceToolbar::getToolButton(const OUString& rIdent) const
{
    QToolButton* pToolButton = qobject_cast<QToolButton*>(&getWidget(rIdent));
    assert(pToolButton && "Not a tool button");
    return *pToolButton;
}

QToolButton& QtInstanceToolbar::getToolButton(int nIndex) const
{
    QToolButton* pToolButton = qobject_cast<QToolButton*>(&getWidget(nIndex));
    assert(pToolButton && "Not a tool button");
    return *pToolButton;
}

void QtInstanceToolbar::toolButtonClicked(const QString& rId)
{
    SolarMutexGuard g;

    signal_clicked(toOUString(rId));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
