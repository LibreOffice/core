/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtMenu.hxx>
#include <QtMenu.moc>

#include <QtFrame.hxx>
#include <QtInstance.hxx>
#include <QtMainWindow.hxx>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtWidgets/QActionGroup>
#else
#include <QtGui/QActionGroup>
#endif

#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>

#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <strings.hrc>
#include <bitmaps.hlst>

#include <vcl/toolkit/floatwin.hxx>
#include <window.h>

QtMenu::QtMenu(bool bMenuBar)
    : mpVCLMenu(nullptr)
    , mpParentSalMenu(nullptr)
    , mpFrame(nullptr)
    , mbMenuBar(bMenuBar)
    , mpQMenuBar(nullptr)
    , mpQMenu(nullptr)
    , mpCloseButton(nullptr)
{
}

bool QtMenu::VisibleMenuBar() { return true; }

void QtMenu::InsertMenuItem(QtMenuItem* pSalMenuItem, unsigned nPos)
{
    sal_uInt16 nId = pSalMenuItem->mnId;
    OUString aText = mpVCLMenu->GetItemText(nId);
    NativeItemText(aText);
    vcl::KeyCode nAccelKey = mpVCLMenu->GetAccelKey(nId);

    pSalMenuItem->mpAction.reset();
    pSalMenuItem->mpMenu.reset();

    if (mbMenuBar)
    {
        // top-level menu
        if (mpQMenuBar)
        {
            QMenu* pQMenu = new QMenu(toQString(aText), nullptr);
            pSalMenuItem->mpMenu.reset(pQMenu);

            if ((nPos != MENU_APPEND)
                && (static_cast<size_t>(nPos) < o3tl::make_unsigned(mpQMenuBar->actions().size())))
            {
                mpQMenuBar->insertMenu(mpQMenuBar->actions()[nPos], pQMenu);
            }
            else
            {
                mpQMenuBar->addMenu(pQMenu);
            }

            // correct parent menu for generated menu
            if (pSalMenuItem->mpSubMenu)
            {
                pSalMenuItem->mpSubMenu->mpQMenu = pQMenu;
            }

            connect(pQMenu, &QMenu::aboutToShow, this,
                    [pSalMenuItem] { slotMenuAboutToShow(pSalMenuItem); });
            connect(pQMenu, &QMenu::aboutToHide, this,
                    [pSalMenuItem] { slotMenuAboutToHide(pSalMenuItem); });
        }
    }
    else
    {
        if (!mpQMenu)
        {
            // no QMenu set, instantiate own one
            mpOwnedQMenu.reset(new QMenu);
            mpQMenu = mpOwnedQMenu.get();
        }

        if (pSalMenuItem->mpSubMenu)
        {
            // submenu
            QMenu* pQMenu = new QMenu(toQString(aText), nullptr);
            pSalMenuItem->mpMenu.reset(pQMenu);

            if ((nPos != MENU_APPEND)
                && (static_cast<size_t>(nPos) < o3tl::make_unsigned(mpQMenu->actions().size())))
            {
                mpQMenu->insertMenu(mpQMenu->actions()[nPos], pQMenu);
            }
            else
            {
                mpQMenu->addMenu(pQMenu);
            }

            // correct parent menu for generated menu
            pSalMenuItem->mpSubMenu->mpQMenu = pQMenu;

            ReinitializeActionGroup(nPos);

            // clear all action groups since menu is recreated
            pSalMenuItem->mpSubMenu->ResetAllActionGroups();

            connect(pQMenu, &QMenu::aboutToShow, this,
                    [pSalMenuItem] { slotMenuAboutToShow(pSalMenuItem); });
            connect(pQMenu, &QMenu::aboutToHide, this,
                    [pSalMenuItem] { slotMenuAboutToHide(pSalMenuItem); });
        }
        else
        {
            if (pSalMenuItem->mnType == MenuItemType::SEPARATOR)
            {
                QAction* pAction = new QAction(nullptr);
                pSalMenuItem->mpAction.reset(pAction);
                pAction->setSeparator(true);

                if ((nPos != MENU_APPEND)
                    && (static_cast<size_t>(nPos) < o3tl::make_unsigned(mpQMenu->actions().size())))
                {
                    mpQMenu->insertAction(mpQMenu->actions()[nPos], pAction);
                }
                else
                {
                    mpQMenu->addAction(pAction);
                }

                ReinitializeActionGroup(nPos);
            }
            else
            {
                // leaf menu
                QAction* pAction = new QAction(toQString(aText), nullptr);
                pSalMenuItem->mpAction.reset(pAction);

                if ((nPos != MENU_APPEND)
                    && (static_cast<size_t>(nPos) < o3tl::make_unsigned(mpQMenu->actions().size())))
                {
                    mpQMenu->insertAction(mpQMenu->actions()[nPos], pAction);
                }
                else
                {
                    mpQMenu->addAction(pAction);
                }

                ReinitializeActionGroup(nPos);

                UpdateActionGroupItem(pSalMenuItem);

                pAction->setShortcut(toQString(nAccelKey.GetName()));

                connect(pAction, &QAction::triggered, this,
                        [pSalMenuItem] { slotMenuTriggered(pSalMenuItem); });
            }
        }
    }

    QAction* pAction = pSalMenuItem->getAction();
    if (pAction)
    {
        pAction->setEnabled(pSalMenuItem->mbEnabled);
        pAction->setVisible(pSalMenuItem->mbVisible);
    }
}

void QtMenu::ReinitializeActionGroup(unsigned nPos)
{
    const unsigned nCount = GetItemCount();

    if (nCount == 0)
    {
        return;
    }

    if (nPos == MENU_APPEND)
    {
        nPos = nCount - 1;
    }
    else if (nPos >= nCount)
    {
        return;
    }

    QtMenuItem* pPrevItem = (nPos > 0) ? GetItemAtPos(nPos - 1) : nullptr;
    QtMenuItem* pCurrentItem = GetItemAtPos(nPos);
    QtMenuItem* pNextItem = (nPos < nCount - 1) ? GetItemAtPos(nPos + 1) : nullptr;

    if (pCurrentItem->mnType == MenuItemType::SEPARATOR)
    {
        pCurrentItem->mpActionGroup.reset();

        // if it's inserted into middle of existing group, split it into two groups:
        // first goes original group, after separator goes new group
        if (pPrevItem && pPrevItem->mpActionGroup && pNextItem && pNextItem->mpActionGroup
            && (pPrevItem->mpActionGroup == pNextItem->mpActionGroup))
        {
            std::shared_ptr<QActionGroup> pFirstActionGroup = pPrevItem->mpActionGroup;
            auto pSecondActionGroup = std::make_shared<QActionGroup>(nullptr);
            pSecondActionGroup->setExclusive(true);

            auto actions = pFirstActionGroup->actions();

            for (unsigned idx = nPos + 1; idx < nCount; ++idx)
            {
                QtMenuItem* pModifiedItem = GetItemAtPos(idx);

                if ((!pModifiedItem) || (!pModifiedItem->mpActionGroup))
                {
                    break;
                }

                pModifiedItem->mpActionGroup = pSecondActionGroup;
                auto action = pModifiedItem->getAction();

                if (actions.contains(action))
                {
                    pFirstActionGroup->removeAction(action);
                    pSecondActionGroup->addAction(action);
                }
            }
        }
    }
    else
    {
        if (!pCurrentItem->mpActionGroup)
        {
            // unless element is inserted between two separators, or a separator and an end of vector, use neighbouring group since it's shared
            if (pPrevItem && pPrevItem->mpActionGroup)
            {
                pCurrentItem->mpActionGroup = pPrevItem->mpActionGroup;
            }
            else if (pNextItem && pNextItem->mpActionGroup)
            {
                pCurrentItem->mpActionGroup = pNextItem->mpActionGroup;
            }
            else
            {
                pCurrentItem->mpActionGroup = std::make_shared<QActionGroup>(nullptr);
                pCurrentItem->mpActionGroup->setExclusive(true);
            }
        }

        // if there's also a different group after this element, merge it
        if (pNextItem && pNextItem->mpActionGroup
            && (pCurrentItem->mpActionGroup != pNextItem->mpActionGroup))
        {
            auto pFirstCheckedAction = pCurrentItem->mpActionGroup->checkedAction();
            auto pSecondCheckedAction = pNextItem->mpActionGroup->checkedAction();
            auto actions = pNextItem->mpActionGroup->actions();

            // first move all actions from second group to first one, and if first group already has checked action,
            // and second group also has a checked action, uncheck action from second group
            for (auto action : actions)
            {
                pNextItem->mpActionGroup->removeAction(action);

                if (pFirstCheckedAction && pSecondCheckedAction && (action == pSecondCheckedAction))
                {
                    action->setChecked(false);
                }

                pCurrentItem->mpActionGroup->addAction(action);
            }

            // now replace all pointers to second group with pointers to first group
            for (unsigned idx = nPos + 1; idx < nCount; ++idx)
            {
                QtMenuItem* pModifiedItem = GetItemAtPos(idx);

                if ((!pModifiedItem) || (!pModifiedItem->mpActionGroup))
                {
                    break;
                }

                pModifiedItem->mpActionGroup = pCurrentItem->mpActionGroup;
            }
        }
    }
}

void QtMenu::ResetAllActionGroups()
{
    for (unsigned nItem = 0; nItem < GetItemCount(); ++nItem)
    {
        QtMenuItem* pSalMenuItem = GetItemAtPos(nItem);
        pSalMenuItem->mpActionGroup.reset();
    }
}

void QtMenu::UpdateActionGroupItem(const QtMenuItem* pSalMenuItem)
{
    QAction* pAction = pSalMenuItem->getAction();
    if (!pAction)
        return;

    bool bChecked = mpVCLMenu->IsItemChecked(pSalMenuItem->mnId);
    MenuItemBits itemBits = mpVCLMenu->GetItemBits(pSalMenuItem->mnId);

    if (itemBits & MenuItemBits::RADIOCHECK)
    {
        pAction->setCheckable(true);

        if (pSalMenuItem->mpActionGroup)
        {
            pSalMenuItem->mpActionGroup->addAction(pAction);
        }

        pAction->setChecked(bChecked);
    }
    else
    {
        pAction->setActionGroup(nullptr);

        if (itemBits & MenuItemBits::CHECKABLE)
        {
            pAction->setCheckable(true);
            pAction->setChecked(bChecked);
        }
        else
        {
            pAction->setChecked(false);
            pAction->setCheckable(false);
        }
    }
}

void QtMenu::InsertItem(SalMenuItem* pSalMenuItem, unsigned nPos)
{
    SolarMutexGuard aGuard;
    QtMenuItem* pItem = static_cast<QtMenuItem*>(pSalMenuItem);

    if (nPos == MENU_APPEND)
        maItems.push_back(pItem);
    else
        maItems.insert(maItems.begin() + nPos, pItem);

    pItem->mpParentMenu = this;

    InsertMenuItem(pItem, nPos);
}

void QtMenu::RemoveItem(unsigned nPos)
{
    SolarMutexGuard aGuard;

    if (nPos >= maItems.size())
        return;

    QtMenuItem* pItem = maItems[nPos];
    pItem->mpAction.reset();
    pItem->mpMenu.reset();

    maItems.erase(maItems.begin() + nPos);

    // Recalculate action groups if necessary:
    // if separator between two QActionGroups was removed,
    // it may be needed to merge them
    if (nPos > 0)
    {
        ReinitializeActionGroup(nPos - 1);
    }
}

void QtMenu::SetSubMenu(SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos)
{
    SolarMutexGuard aGuard;
    QtMenuItem* pItem = static_cast<QtMenuItem*>(pSalMenuItem);
    QtMenu* pQSubMenu = static_cast<QtMenu*>(pSubMenu);

    pItem->mpSubMenu = pQSubMenu;
    // at this point the pointer to parent menu may be outdated, update it too
    pItem->mpParentMenu = this;

    if (pQSubMenu != nullptr)
    {
        pQSubMenu->mpParentSalMenu = this;
        pQSubMenu->mpQMenu = pItem->mpMenu.get();
    }

    // if it's not a menu bar item, then convert it to corresponding item if type if necessary.
    // If submenu is present and it's an action, convert it to menu.
    // If submenu is not present and it's a menu, convert it to action.
    // It may be fine to proceed in any case, but by skipping other cases
    // amount of unneeded actions taken should be reduced.
    if (pItem->mpParentMenu->mbMenuBar || (pQSubMenu && pItem->mpMenu)
        || ((!pQSubMenu) && pItem->mpAction))
    {
        return;
    }

    InsertMenuItem(pItem, nPos);
}

void QtMenu::SetFrame(const SalFrame* pFrame)
{
    auto* pSalInst(GetQtInstance());
    assert(pSalInst);
    if (!pSalInst->IsMainThread())
    {
        pSalInst->RunInMainThread([this, pFrame]() { SetFrame(pFrame); });
        return;
    }

    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    mpFrame = const_cast<QtFrame*>(static_cast<const QtFrame*>(pFrame));

    mpFrame->SetMenu(this);

    QtMainWindow* pMainWindow = mpFrame->GetTopLevelWindow();
    if (!pMainWindow)
        return;

    mpQMenuBar = pMainWindow->menuBar();
    if (mpQMenuBar)
    {
        mpQMenuBar->clear();
        QPushButton* pButton
            = static_cast<QPushButton*>(mpQMenuBar->cornerWidget(Qt::TopRightCorner));
        if (pButton && ((mpCloseButton != pButton) || !maCloseButtonConnection))
        {
            maCloseButtonConnection
                = connect(pButton, &QPushButton::clicked, this, &QtMenu::slotCloseDocument);
            mpCloseButton = pButton;
        }
    }

    mpQMenu = nullptr;

    DoFullMenuUpdate(mpVCLMenu);
}

void QtMenu::DoFullMenuUpdate(Menu* pMenuBar)
{
    // clear action groups since menu is rebuilt
    ResetAllActionGroups();
    ShowCloseButton(false);

    for (sal_Int32 nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++)
    {
        QtMenuItem* pSalMenuItem = GetItemAtPos(nItem);
        InsertMenuItem(pSalMenuItem, nItem);
        SetItemImage(nItem, pSalMenuItem, pSalMenuItem->maImage);
        const bool bShowDisabled
            = bool(pMenuBar->GetMenuFlags() & MenuFlags::AlwaysShowDisabledEntries)
              || !bool(pMenuBar->GetMenuFlags() & MenuFlags::HideDisabledEntries);
        const bool bVisible = bShowDisabled || mpVCLMenu->IsItemEnabled(pSalMenuItem->mnId);
        pSalMenuItem->getAction()->setVisible(bVisible);

        if (pSalMenuItem->mpSubMenu != nullptr)
        {
            pMenuBar->HandleMenuActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
            pSalMenuItem->mpSubMenu->DoFullMenuUpdate(pMenuBar);
            pMenuBar->HandleMenuDeActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
        }
    }
}

void QtMenu::ShowItem(unsigned nPos, bool bShow)
{
    if (nPos < maItems.size())
    {
        QtMenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
            pAction->setVisible(bShow);
        pSalMenuItem->mbVisible = bShow;
    }
}

void QtMenu::SetItemBits(unsigned nPos, MenuItemBits)
{
    if (nPos < maItems.size())
    {
        QtMenuItem* pSalMenuItem = GetItemAtPos(nPos);
        UpdateActionGroupItem(pSalMenuItem);
    }
}

void QtMenu::CheckItem(unsigned nPos, bool bChecked)
{
    if (nPos < maItems.size())
    {
        QtMenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
        {
            pAction->setCheckable(true);
            pAction->setChecked(bChecked);
        }
    }
}

void QtMenu::EnableItem(unsigned nPos, bool bEnable)
{
    if (nPos < maItems.size())
    {
        QtMenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
            pAction->setEnabled(bEnable);
        pSalMenuItem->mbEnabled = bEnable;
    }
}

void QtMenu::SetItemText(unsigned, SalMenuItem* pItem, const OUString& rText)
{
    QtMenuItem* pSalMenuItem = static_cast<QtMenuItem*>(pItem);
    QAction* pAction = pSalMenuItem->getAction();
    if (pAction)
    {
        OUString aText(rText);
        NativeItemText(aText);
        pAction->setText(toQString(aText));
    }
}

void QtMenu::SetItemImage(unsigned, SalMenuItem* pItem, const Image& rImage)
{
    QtMenuItem* pSalMenuItem = static_cast<QtMenuItem*>(pItem);

    // Save new image to use it in DoFullMenuUpdate
    pSalMenuItem->maImage = rImage;

    QAction* pAction = pSalMenuItem->getAction();
    if (!pAction)
        return;

    pAction->setIcon(QPixmap::fromImage(toQImage(rImage)));
}

void QtMenu::SetAccelerator(unsigned, SalMenuItem* pItem, const vcl::KeyCode&,
                            const OUString& rText)
{
    QtMenuItem* pSalMenuItem = static_cast<QtMenuItem*>(pItem);
    QAction* pAction = pSalMenuItem->getAction();
    if (pAction)
        pAction->setShortcut(QKeySequence(toQString(rText), QKeySequence::PortableText));
}

void QtMenu::GetSystemMenuData(SystemMenuData*) {}

QtMenu* QtMenu::GetTopLevel()
{
    QtMenu* pMenu = this;
    while (pMenu->mpParentSalMenu)
        pMenu = pMenu->mpParentSalMenu;
    return pMenu;
}

void QtMenu::ShowMenuBar(bool bVisible)
{
    if (mpQMenuBar)
        mpQMenuBar->setVisible(bVisible);
}

const QtFrame* QtMenu::GetFrame() const
{
    SolarMutexGuard aGuard;
    const QtMenu* pMenu = this;
    while (pMenu && !pMenu->mpFrame)
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : nullptr;
}

void QtMenu::slotMenuTriggered(QtMenuItem* pQItem)
{
    if (!pQItem)
        return;

    QtMenu* pSalMenu = pQItem->mpParentMenu;
    QtMenu* pTopLevel = pSalMenu->GetTopLevel();

    Menu* pMenu = pSalMenu->GetMenu();
    auto mnId = pQItem->mnId;

    // HACK to allow HandleMenuCommandEvent to "not-set" the checked button
    // LO expects a signal before an item state change, so reset the check item
    if (pQItem->mpAction->isCheckable()
        && (!pQItem->mpActionGroup || pQItem->mpActionGroup->actions().size() <= 1))
        pQItem->mpAction->setChecked(!pQItem->mpAction->isChecked());
    pTopLevel->GetMenu()->HandleMenuCommandEvent(pMenu, mnId);
}

void QtMenu::slotMenuAboutToShow(QtMenuItem* pQItem)
{
    if (pQItem)
    {
        QtMenu* pSalMenu = pQItem->mpSubMenu;
        QtMenu* pTopLevel = pSalMenu->GetTopLevel();

        Menu* pMenu = pSalMenu->GetMenu();

        // following function may update the menu
        pTopLevel->GetMenu()->HandleMenuActivateEvent(pMenu);
    }
}

void QtMenu::slotMenuAboutToHide(QtMenuItem* pQItem)
{
    if (pQItem)
    {
        QtMenu* pSalMenu = pQItem->mpSubMenu;
        QtMenu* pTopLevel = pSalMenu->GetTopLevel();

        Menu* pMenu = pSalMenu->GetMenu();

        pTopLevel->GetMenu()->HandleMenuDeActivateEvent(pMenu);
    }
}

void QtMenu::NativeItemText(OUString& rItemText)
{
    // preserve literal '&'s in menu texts
    rItemText = rItemText.replaceAll("&", "&&");

    rItemText = rItemText.replace('~', '&');
}

void QtMenu::slotCloseDocument()
{
    MenuBar* pVclMenuBar = static_cast<MenuBar*>(mpVCLMenu.get());
    if (pVclMenuBar)
        Application::PostUserEvent(pVclMenuBar->GetCloseButtonClickHdl());
}

void QtMenu::ShowCloseButton(bool bShow)
{
    if (!mpQMenuBar)
        return;

    QPushButton* pButton = static_cast<QPushButton*>(mpQMenuBar->cornerWidget(Qt::TopRightCorner));
    if (!pButton)
    {
        QIcon aIcon;
        if (QIcon::hasThemeIcon("window-close-symbolic"))
            aIcon = QIcon::fromTheme("window-close-symbolic");
        else
            aIcon = QIcon(
                QPixmap::fromImage(toQImage(Image(StockImage::Yes, SV_RESID_BITMAP_CLOSEDOC))));
        pButton = new QPushButton(mpQMenuBar);
        pButton->setIcon(aIcon);
        pButton->setFlat(true);
        pButton->setFocusPolicy(Qt::NoFocus);
        pButton->setToolTip(toQString(VclResId(SV_HELPTEXT_CLOSEDOCUMENT)));
        mpQMenuBar->setCornerWidget(pButton, Qt::TopRightCorner);
        maCloseButtonConnection
            = connect(pButton, &QPushButton::clicked, this, &QtMenu::slotCloseDocument);
        mpCloseButton = pButton;
    }

    if (bShow)
        pButton->show();
    else
        pButton->hide();
}

bool QtMenu::ShowNativePopupMenu(FloatingWindow*, const tools::Rectangle&,
                                 FloatWinPopupFlags nFlags)
{
    assert(mpQMenu);
    DoFullMenuUpdate(mpVCLMenu);
    mpQMenu->setTearOffEnabled(bool(nFlags & FloatWinPopupFlags::AllowTearOff));

    const QPoint aPos = QCursor::pos();
    mpQMenu->exec(aPos);

    return true;
}

QtMenuItem::QtMenuItem(const SalItemParams* pItemData)
    : mpParentMenu(nullptr)
    , mpSubMenu(nullptr)
    , mnId(pItemData->nId)
    , mnType(pItemData->eType)
    , mbVisible(true)
    , mbEnabled(true)
    , maImage(pItemData->aImage)
{
}

QAction* QtMenuItem::getAction() const
{
    if (mpMenu)
        return mpMenu->menuAction();
    if (mpAction)
        return mpAction.get();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
