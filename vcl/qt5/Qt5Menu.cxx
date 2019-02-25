/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5Frame.hxx>
#include <Qt5MainWindow.hxx>
#include <Qt5Bitmap.hxx>
#include <Qt5Menu.hxx>
#include <Qt5Menu.moc>
#include <Qt5Instance.hxx>

#include <QtWidgets/QtWidgets>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <vcl/pngwrite.hxx>
#include <tools/stream.hxx>

Qt5Menu::Qt5Menu(bool bMenuBar)
    : mpVCLMenu(nullptr)
    , mpParentSalMenu(nullptr)
    , mpFrame(nullptr)
    , mbMenuBar(bMenuBar)
    , mpQMenuBar(nullptr)
    , mpQMenu(nullptr)
{
    connect(this, &Qt5Menu::setFrameSignal, this, &Qt5Menu::SetFrame, Qt::BlockingQueuedConnection);
}

bool Qt5Menu::VisibleMenuBar() { return true; }

void Qt5Menu::InsertMenuItem(Qt5MenuItem* pSalMenuItem, unsigned nPos)
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
                && (static_cast<size_t>(nPos) < static_cast<size_t>(mpQMenuBar->actions().size())))
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
    else if (mpQMenu)
    {
        if (pSalMenuItem->mpSubMenu)
        {
            // submenu
            QMenu* pQMenu = new QMenu(toQString(aText), nullptr);
            pSalMenuItem->mpMenu.reset(pQMenu);

            if ((nPos != MENU_APPEND)
                && (static_cast<size_t>(nPos) < static_cast<size_t>(mpQMenu->actions().size())))
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
                    && (static_cast<size_t>(nPos) < static_cast<size_t>(mpQMenu->actions().size())))
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
                    && (static_cast<size_t>(nPos) < static_cast<size_t>(mpQMenu->actions().size())))
                {
                    mpQMenu->insertAction(mpQMenu->actions()[nPos], pAction);
                }
                else
                {
                    mpQMenu->addAction(pAction);
                }

                ReinitializeActionGroup(nPos);

                UpdateActionGroupItem(pSalMenuItem);

                pAction->setShortcut(toQString(nAccelKey.GetName(GetFrame()->GetWindow())));

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

void Qt5Menu::ReinitializeActionGroup(unsigned nPos)
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

    Qt5MenuItem* pPrevItem = (nPos > 0) ? GetItemAtPos(nPos - 1) : nullptr;
    Qt5MenuItem* pCurrentItem = GetItemAtPos(nPos);
    Qt5MenuItem* pNextItem = (nPos < nCount - 1) ? GetItemAtPos(nPos + 1) : nullptr;

    if (pCurrentItem->mnType == MenuItemType::SEPARATOR)
    {
        pCurrentItem->mpActionGroup.reset();

        // if it's inserted into middle of existing group, split it into two groups:
        // first goes original group, after separator goes new group
        if (pPrevItem && pPrevItem->mpActionGroup && pNextItem && pNextItem->mpActionGroup
            && (pPrevItem->mpActionGroup == pNextItem->mpActionGroup))
        {
            std::shared_ptr<QActionGroup> pFirstActionGroup = pPrevItem->mpActionGroup;
            std::shared_ptr<QActionGroup> pSecondActionGroup(new QActionGroup(nullptr));
            pSecondActionGroup->setExclusive(true);

            auto actions = pFirstActionGroup->actions();

            for (unsigned idx = nPos + 1; idx < nCount; ++idx)
            {
                Qt5MenuItem* pModifiedItem = GetItemAtPos(idx);

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
                pCurrentItem->mpActionGroup.reset(new QActionGroup(nullptr));
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
                Qt5MenuItem* pModifiedItem = GetItemAtPos(idx);

                if ((!pModifiedItem) || (!pModifiedItem->mpActionGroup))
                {
                    break;
                }

                pModifiedItem->mpActionGroup = pCurrentItem->mpActionGroup;
            }
        }
    }
}

void Qt5Menu::ResetAllActionGroups()
{
    for (unsigned nItem = 0; nItem < GetItemCount(); ++nItem)
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nItem);
        pSalMenuItem->mpActionGroup.reset();
    }
}

void Qt5Menu::UpdateActionGroupItem(Qt5MenuItem* pSalMenuItem)
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

void Qt5Menu::InsertItem(SalMenuItem* pSalMenuItem, unsigned nPos)
{
    SolarMutexGuard aGuard;
    Qt5MenuItem* pItem = static_cast<Qt5MenuItem*>(pSalMenuItem);

    if (nPos == MENU_APPEND)
        maItems.push_back(pItem);
    else
        maItems.insert(maItems.begin() + nPos, pItem);

    pItem->mpParentMenu = this;

    InsertMenuItem(pItem, nPos);
}

void Qt5Menu::RemoveItem(unsigned nPos)
{
    SolarMutexGuard aGuard;

    if (nPos < maItems.size())
    {
        Qt5MenuItem* pItem = maItems[nPos];
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
}

void Qt5Menu::SetSubMenu(SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos)
{
    SolarMutexGuard aGuard;
    Qt5MenuItem* pItem = static_cast<Qt5MenuItem*>(pSalMenuItem);
    Qt5Menu* pQSubMenu = static_cast<Qt5Menu*>(pSubMenu);

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

void Qt5Menu::SetFrame(const SalFrame* pFrame)
{
    auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
    assert(pSalInst);
    if (!pSalInst->IsMainThread())
    {
        pSalInst->RunInMainThread([this, pFrame]() { SetFrame(pFrame); });
        return;
    }

    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    mpFrame = const_cast<Qt5Frame*>(static_cast<const Qt5Frame*>(pFrame));

    mpFrame->SetMenu(this);

    Qt5MainWindow* pMainWindow = mpFrame->GetTopLevelWindow();
    if (pMainWindow)
    {
        mpQMenuBar = pMainWindow->menuBar();
        if (mpQMenuBar)
            mpQMenuBar->clear();

        mpQMenu = nullptr;

        DoFullMenuUpdate(mpVCLMenu);
    }
}

void Qt5Menu::DoFullMenuUpdate(Menu* pMenuBar)
{
    // clear action groups since menu is rebuilt
    ResetAllActionGroups();

    for (sal_Int32 nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++)
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nItem);
        InsertMenuItem(pSalMenuItem, nItem);
        SetItemImage(nItem, pSalMenuItem, pSalMenuItem->maImage);

        if (pSalMenuItem->mpSubMenu != nullptr)
        {
            pMenuBar->HandleMenuActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
            pSalMenuItem->mpSubMenu->DoFullMenuUpdate(pMenuBar);
            pMenuBar->HandleMenuDeActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
        }
    }
}

void Qt5Menu::ShowItem(unsigned nPos, bool bShow)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
            pAction->setVisible(bShow);
        pSalMenuItem->mbVisible = bShow;
    }
}

void Qt5Menu::SetItemBits(unsigned nPos, MenuItemBits)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        UpdateActionGroupItem(pSalMenuItem);
    }
}

void Qt5Menu::CheckItem(unsigned nPos, bool bChecked)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
        {
            pAction->setCheckable(true);
            pAction->setChecked(bChecked);
        }
    }
}

void Qt5Menu::EnableItem(unsigned nPos, bool bEnable)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
            pAction->setEnabled(bEnable);
        pSalMenuItem->mbEnabled = bEnable;
    }
}

void Qt5Menu::SetItemText(unsigned, SalMenuItem* pItem, const OUString& rText)
{
    Qt5MenuItem* pSalMenuItem = static_cast<Qt5MenuItem*>(pItem);
    QAction* pAction = pSalMenuItem->getAction();
    if (pAction)
    {
        OUString aText(rText);
        NativeItemText(aText);
        pAction->setText(toQString(aText));
    }
}

void Qt5Menu::SetItemImage(unsigned, SalMenuItem* pItem, const Image& rImage)
{
    Qt5MenuItem* pSalMenuItem = static_cast<Qt5MenuItem*>(pItem);

    // Save new image to use it in DoFullMenuUpdate
    pSalMenuItem->maImage = rImage;

    QAction* pAction = pSalMenuItem->getAction();
    if (!pAction)
        return;

    QImage aImage;

    if (!!rImage)
    {
        SvMemoryStream aMemStm;
        vcl::PNGWriter aWriter(rImage.GetBitmapEx());
        aWriter.Write(aMemStm);

        if (!aImage.loadFromData(static_cast<const uchar*>(aMemStm.GetData()), aMemStm.TellEnd()))
        {
            return;
        }
    }

    pAction->setIcon(QPixmap::fromImage(aImage));
}

void Qt5Menu::SetAccelerator(unsigned, SalMenuItem* pItem, const vcl::KeyCode&,
                             const OUString& rText)
{
    Qt5MenuItem* pSalMenuItem = static_cast<Qt5MenuItem*>(pItem);
    QAction* pAction = pSalMenuItem->getAction();
    if (pAction)
        pAction->setShortcut(QKeySequence(toQString(rText), QKeySequence::PortableText));
}

void Qt5Menu::GetSystemMenuData(SystemMenuData*) {}

Qt5Menu* Qt5Menu::GetTopLevel()
{
    Qt5Menu* pMenu = this;
    while (pMenu->mpParentSalMenu)
        pMenu = pMenu->mpParentSalMenu;
    return pMenu;
}

const Qt5Frame* Qt5Menu::GetFrame() const
{
    SolarMutexGuard aGuard;
    const Qt5Menu* pMenu = this;
    while (pMenu && !pMenu->mpFrame)
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : nullptr;
}

void Qt5Menu::slotMenuTriggered(Qt5MenuItem* pQItem)
{
    if (pQItem)
    {
        Qt5Menu* pSalMenu = pQItem->mpParentMenu;
        Qt5Menu* pTopLevel = pSalMenu->GetTopLevel();

        Menu* pMenu = pSalMenu->GetMenu();
        auto mnId = pQItem->mnId;

        pTopLevel->GetMenu()->HandleMenuCommandEvent(pMenu, mnId);
    }
}

void Qt5Menu::slotMenuAboutToShow(Qt5MenuItem* pQItem)
{
    if (pQItem)
    {
        Qt5Menu* pSalMenu = pQItem->mpSubMenu;
        Qt5Menu* pTopLevel = pSalMenu->GetTopLevel();

        Menu* pMenu = pSalMenu->GetMenu();

        // following function may update the menu
        pTopLevel->GetMenu()->HandleMenuActivateEvent(pMenu);
    }
}

void Qt5Menu::slotMenuAboutToHide(Qt5MenuItem* pQItem)
{
    if (pQItem)
    {
        Qt5Menu* pSalMenu = pQItem->mpSubMenu;
        Qt5Menu* pTopLevel = pSalMenu->GetTopLevel();

        Menu* pMenu = pSalMenu->GetMenu();

        pTopLevel->GetMenu()->HandleMenuDeActivateEvent(pMenu);
    }
}

void Qt5Menu::NativeItemText(OUString& rItemText)
{
    // preserve literal '&'s in menu texts
    rItemText = rItemText.replaceAll("&", "&&");

    rItemText = rItemText.replace('~', '&');
}

Qt5MenuItem::Qt5MenuItem(const SalItemParams* pItemData)
    : mpParentMenu(nullptr)
    , mpSubMenu(nullptr)
    , mnId(pItemData->nId)
    , mnType(pItemData->eType)
    , mbVisible(true)
    , mbEnabled(true)
    , maImage(pItemData->aImage)
{
}

QAction* Qt5MenuItem::getAction() const
{
    if (mpMenu)
        return mpMenu->menuAction();
    if (mpAction)
        return mpAction.get();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
