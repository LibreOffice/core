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
#include <QtInstanceWidget.hxx>
#include <QtTools.hxx>

#include <tools/debug.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/qt/QtUtils.hxx>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QActionGroup>
#include <QtGui/QShortcut>
#else
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QShortcut>
#endif

// Property for storing an action name in a menu item
const char* const PROPERTY_ACTION_NAME = "action-name";

QtInstanceMenu::QtInstanceMenu(QMenu* pMenu)
    : m_pMenu(pMenu)
{
    assert(m_pMenu);

    connect(m_pMenu, &QMenu::triggered, this, &QtInstanceMenu::menuActionTriggered);

    // connect slots in order to show help for current entry on F1
    connect(m_pMenu, &QMenu::hovered, this, &QtInstanceMenu::menuActionHovered);
    QKeySequence sequence(QKeySequence::HelpContents);
    QShortcut* pQShortcut = new QShortcut(sequence, m_pMenu);
    connect(pQShortcut, &QShortcut::activated, this, &QtInstanceMenu::showHelp);
    connect(pQShortcut, &QShortcut::activatedAmbiguously, this, &QtInstanceMenu::showHelp);
}

OUString QtInstanceMenu::popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                       weld::Placement ePlace)
{
    SolarMutexGuard g;

    assert(ePlace == weld::Placement::Under && "placement type not supported yet");
    (void)ePlace;

    OUString sActionId;
    GetQtInstance().RunInMainThread([&] {
        m_pMenu->adjustSize();

        QtInstanceWidget* pQtParent = dynamic_cast<QtInstanceWidget*>(pParent);
        assert(pQtParent && "No parent");
        QWidget* pParentWidget = pQtParent->getQWidget();
        assert(pParentWidget);
        const QPoint aPos = pParentWidget->mapToGlobal(toQPoint(rRect.TopLeft()));

        if (const QAction* pTriggeredAction = m_pMenu->exec(aPos))
            sActionId = toOUString(pTriggeredAction->objectName());
    });

    return sActionId;
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

void QtInstanceMenu::set_tooltip_text(const OUString& rIdent, const OUString& rTip)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            pAction->setToolTip(toQString(rTip));
    });
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

void QtInstanceMenu::insert(int nPos, const OUString& rId, const OUString& rStr,
                            const OUString* pIconName, VirtualDevice* pImageSurface,
                            const css::uno::Reference<css::graphic::XGraphic>& rImage,
                            TriState eCheckRadioFalse)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QAction* pAction = new QAction(vclToQtStringWithAccelerator(rStr));

        insertAction(*pAction, rId, nPos);

        pAction->setCheckable(eCheckRadioFalse != TRISTATE_INDET);
        if (eCheckRadioFalse == TRISTATE_FALSE)
        {
            // For Qt, all mutually exclusive actions/menu entries would usually be put
            // into the same QActionGroup. However, LO currently manually implements logic
            // to toggle all other entries off when one gets enabled and doesn't use the concept of groups.
            // Creating a separate QActionGroup for each item causes each item to be displayed
            // with a radio button and leaves control for manually toggling on/off.
            QActionGroup* pActionGroup = new QActionGroup(pAction);
            pActionGroup->addAction(pAction);
        }

        if (pIconName && !pIconName->isEmpty())
            pAction->setIcon(loadQPixmapIcon(*pIconName));
        else if (pImageSurface)
            pAction->setIcon(toQPixmap(*pImageSurface));
        else if (rImage.is())
            pAction->setIcon(toQPixmap(rImage));
    });
}

void QtInstanceMenu::set_item_help_id(const OUString& rIdent, const OUString& rHelpId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (QAction* pAction = getAction(rIdent))
            QtInstanceWidget::setHelpId(*pAction, rHelpId);
    });
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

void QtInstanceMenu::insert_separator(int nPos, const OUString& rId)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        QAction* pAction = new QAction;
        insertAction(*pAction, rId, nPos);

        pAction->setSeparator(true);
    });
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

QMenu* QtInstanceMenu::getMenu() const { return m_pMenu; }

void QtInstanceMenu::setActionName(QAction& rAction, const OUString& rActionName)
{
    rAction.setProperty(PROPERTY_ACTION_NAME, toQString(rActionName));
}

void QtInstanceMenu::insertAction(QAction& rAction, const OUString& rId, int nPos)
{
    assert(GetQtInstance().IsMainThread() && "must be run in main thread");

    const int nCount = m_pMenu->actions().size();
    if (nPos == -1)
        nPos = nCount;

    QAction* pInsertBefore = nPos < nCount ? m_pMenu->actions().at(nPos) : nullptr;
    m_pMenu->insertAction(pInsertBefore, &rAction);

    rAction.setObjectName(toQString(rId));
}

QAction* QtInstanceMenu::getAction(const OUString& rIdent) const
{
    const QList<QAction*> aActions = m_pMenu->actions();
    for (QAction* pAction : aActions)
    {
        if (pAction && pAction->objectName() == toQString(rIdent))
            return pAction;
    }

    return nullptr;
}

void QtInstanceMenu::menuActionHovered(QAction* pAction) { m_pCurrentAction = pAction; }

void QtInstanceMenu::menuActionTriggered(QAction* pAction)
{
    SolarMutexGuard g;

    assert(pAction);
    signal_activate(toOUString(pAction->objectName()));
}

void QtInstanceMenu::showHelp()
{
    SolarMutexGuard aGuard;

    GetQtInstance().RunInMainThread([&] {
        if (!m_pCurrentAction)
            return;

        const OUString sHelpId = QtInstanceWidget::getHelpId(*m_pCurrentAction);
        if (sHelpId.isEmpty())
            return;

        if (Help* pHelp = Application::GetHelp())
            pHelp->Start(sHelpId);
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
