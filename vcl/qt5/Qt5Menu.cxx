/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5Frame.hxx>
#include <Qt5Menu.hxx>
#include <Qt5Menu.moc>

#include <QtWidgets/QtWidgets>

#include <vcl/svapp.hxx>

Qt5Menu::Qt5Menu(bool bMenuBar)
    : mpVCLMenu(nullptr)
    , mpParentSalMenu(nullptr)
    , mpFrame(nullptr)
    , mbMenuBar(bMenuBar)
{
}

Qt5Menu::~Qt5Menu() { maItems.clear(); }

bool Qt5Menu::VisibleMenuBar() { return false; }

void Qt5Menu::InsertItem(SalMenuItem* pSalMenuItem, unsigned nPos)
{
    SolarMutexGuard aGuard;
    Qt5MenuItem* pItem = static_cast<Qt5MenuItem*>(pSalMenuItem);

    if (nPos == MENU_APPEND)
        maItems.push_back(pItem);
    else
        maItems.insert(maItems.begin() + nPos, pItem);

    pItem->mpParentMenu = this;
}

void Qt5Menu::RemoveItem(unsigned nPos)
{
    SolarMutexGuard aGuard;
    maItems.erase(maItems.begin() + nPos);
}

void Qt5Menu::SetSubMenu(SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned)
{
    SolarMutexGuard aGuard;
    Qt5MenuItem* pItem = static_cast<Qt5MenuItem*>(pSalMenuItem);
    Qt5Menu* pQSubMenu = static_cast<Qt5Menu*>(pSubMenu);

    if (pQSubMenu == nullptr)
        return;

    pQSubMenu->mpParentSalMenu = this;
    pItem->mpSubMenu = pQSubMenu;
}

void Qt5Menu::SetFrame(const SalFrame* pFrame)
{
    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    mpFrame = const_cast<Qt5Frame*>(static_cast<const Qt5Frame*>(pFrame));

    mpFrame->SetMenu(this);

    QWidget* pWidget = mpFrame->GetQWidget();
    QMainWindow* pMainWindow = dynamic_cast<QMainWindow*>(pWidget);
    if (pMainWindow)
        mpQMenuBar = pMainWindow->menuBar();

    DoFullMenuUpdate(mpVCLMenu);
}

void Qt5Menu::DoFullMenuUpdate(Menu* pMenuBar, QMenu* pParentMenu)
{
    Menu* pVCLMenu = mpVCLMenu;

    if (mbMenuBar && mpQMenuBar)
        mpQMenuBar->clear();
    QActionGroup* pQAG = nullptr;

    for (sal_Int32 nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++)
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nItem);
        sal_uInt16 nId = pSalMenuItem->mnId;
        OUString aText = pVCLMenu->GetItemText(nId);
        QMenu* pQMenu = pParentMenu;
        NativeItemText(aText);
        vcl::KeyCode nAccelKey = pVCLMenu->GetAccelKey(nId);
        bool bChecked = pVCLMenu->IsItemChecked(nId);
        MenuItemBits itemBits = pVCLMenu->GetItemBits(nId);

        if (mbMenuBar && mpQMenuBar)
            // top-level menu
            pQMenu = mpQMenuBar->addMenu(toQString(aText));
        else
        {
            if (pSalMenuItem->mpSubMenu)
            {
                // submenu
                pQMenu = pQMenu->addMenu(toQString(aText));
                pQAG = new QActionGroup(pQMenu);
            }
            else
            {
                if (pSalMenuItem->mnType == MenuItemType::SEPARATOR)
                    pQMenu->addSeparator();
                else
                {
                    // leaf menu
                    QAction* pAction = pQMenu->addAction(toQString(aText));
                    pAction->setShortcut(toQString(nAccelKey.GetName(GetFrame()->GetWindow())));

                    if (itemBits & MenuItemBits::CHECKABLE)
                    {
                        pAction->setCheckable(true);
                        pAction->setChecked(bChecked);
                    }
                    else if (itemBits & MenuItemBits::RADIOCHECK)
                    {
                        pAction->setCheckable(true);
                        if (!pQAG)
                        {
                            pQAG = new QActionGroup(pQMenu);
                            pQAG->setExclusive(true);
                        }
                        pQAG->addAction(pAction);
                    }

                    connect(pAction, &QAction::triggered, this,
                            [this, pSalMenuItem] { DispatchCommand(pSalMenuItem); });
                }
            }
        }

        if (pSalMenuItem->mpSubMenu != nullptr)
        {
            pMenuBar->HandleMenuActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
            pSalMenuItem->mpSubMenu->DoFullMenuUpdate(pMenuBar, pQMenu);
            pMenuBar->HandleMenuDeActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
        }
    }
}

void Qt5Menu::ShowItem(unsigned, bool) {}

void Qt5Menu::CheckItem(unsigned, bool) {}

void Qt5Menu::EnableItem(unsigned, bool) {}

void Qt5Menu::SetItemText(unsigned, SalMenuItem*, const rtl::OUString&) {}

void Qt5Menu::SetItemImage(unsigned, SalMenuItem*, const Image&) {}

void Qt5Menu::SetAccelerator(unsigned, SalMenuItem*, const vcl::KeyCode&, const OUString&) {}

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

void Qt5Menu::DispatchCommand(Qt5MenuItem* pQItem)
{
    if (pQItem)
    {
        Qt5Menu* pSalMenu = pQItem->mpParentMenu;
        Qt5Menu* pTopLevel = pSalMenu->GetTopLevel();
        pTopLevel->GetMenu()->HandleMenuCommandEvent(pSalMenu->GetMenu(), pQItem->mnId);
        SAL_WARN("vcl.qt5", "menu triggered " << pQItem->mnId);
    }
}

void Qt5Menu::NativeItemText(OUString& rItemText) { rItemText = rItemText.replace('~', '&'); }

Qt5MenuItem::Qt5MenuItem(const SalItemParams* pItemData)
    : mnId(pItemData->nId)
    , mnType(pItemData->eType)
    , mpVCLMenu(pItemData->pMenu)
    , mpParentMenu(nullptr)
    , mpSubMenu(nullptr)
{
}

Qt5MenuItem::~Qt5MenuItem() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
