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
    , mpQActionGroup(nullptr)
{
    connect(this, &Qt5Menu::setFrameSignal, this, &Qt5Menu::SetFrame, Qt::BlockingQueuedConnection);
}

Qt5Menu::~Qt5Menu() { maItems.clear(); }

bool Qt5Menu::VisibleMenuBar() { return true; }

QMenu* Qt5Menu::InsertMenuItem(Qt5MenuItem* pSalMenuItem, unsigned nPos)
{
    QMenu* pQMenu = mpQMenu;
    sal_uInt16 nId = pSalMenuItem->mnId;
    OUString aText = mpVCLMenu->GetItemText(nId);
    NativeItemText(aText);
    vcl::KeyCode nAccelKey = mpVCLMenu->GetAccelKey(nId);
    bool bChecked = mpVCLMenu->IsItemChecked(nId);
    MenuItemBits itemBits = mpVCLMenu->GetItemBits(nId);

    pSalMenuItem->mpAction.reset();
    pSalMenuItem->mpMenu.reset();

    if (mbMenuBar)
    {
        // top-level menu
        if (mpQMenuBar)
        {
            pQMenu = new QMenu(toQString(aText), nullptr);
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

            connect(pQMenu, &QMenu::aboutToShow, this,
                    [pSalMenuItem] { slotMenuAboutToShow(pSalMenuItem); });
            connect(pQMenu, &QMenu::aboutToHide, this,
                    [pSalMenuItem] { slotMenuAboutToHide(pSalMenuItem); });
        }
    }
    else if (pQMenu)
    {
        if (pSalMenuItem->mpSubMenu)
        {
            // submenu
            QMenu* pTempQMenu = new QMenu(toQString(aText), nullptr);
            pSalMenuItem->mpMenu.reset(pTempQMenu);

            if ((nPos != MENU_APPEND)
                && (static_cast<size_t>(nPos) < static_cast<size_t>(pQMenu->actions().size())))
            {
                pQMenu->insertMenu(pQMenu->actions()[nPos], pTempQMenu);
            }
            else
            {
                pQMenu->addMenu(pTempQMenu);
            }

            pQMenu = pTempQMenu;
            mpQActionGroup = new QActionGroup(pQMenu);

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
                    && (static_cast<size_t>(nPos) < static_cast<size_t>(pQMenu->actions().size())))
                {
                    pQMenu->insertAction(pQMenu->actions()[nPos], pAction);
                }
                else
                {
                    pQMenu->addAction(pAction);
                }
            }
            else
            {
                // leaf menu
                QAction* pAction = new QAction(toQString(aText), nullptr);
                pSalMenuItem->mpAction.reset(pAction);

                if ((nPos != MENU_APPEND)
                    && (static_cast<size_t>(nPos) < static_cast<size_t>(pQMenu->actions().size())))
                {
                    pQMenu->insertAction(pQMenu->actions()[nPos], pAction);
                }
                else
                {
                    pQMenu->addAction(pAction);
                }

                pAction->setShortcut(toQString(nAccelKey.GetName(GetFrame()->GetWindow())));

                if (itemBits & MenuItemBits::CHECKABLE)
                {
                    pAction->setCheckable(true);
                    pAction->setChecked(bChecked);
                }
                else if (itemBits & MenuItemBits::RADIOCHECK)
                {
                    pAction->setCheckable(true);
                    if (!mpQActionGroup)
                    {
                        mpQActionGroup = new QActionGroup(pQMenu);
                        mpQActionGroup->setExclusive(true);
                    }
                    // NOTE: QActionGroup support may need improvement
                    // if menu item is added not to the end of menu,
                    // it may be needed to add new item to QActionGroup different from last created one for this menu
                    mpQActionGroup->addAction(pAction);
                    pAction->setChecked(bChecked);
                }

                pAction->setEnabled(pSalMenuItem->mbEnabled);
                pAction->setVisible(pSalMenuItem->mbVisible);

                connect(pAction, &QAction::triggered, this,
                        [pSalMenuItem] { slotMenuTriggered(pSalMenuItem); });
            }
        }
    }

    return pQMenu;
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
    if (qApp->thread() != QThread::currentThread())
    {
        SolarMutexReleaser aReleaser;
        return Q_EMIT setFrameSignal(pFrame);
    }

    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    mpFrame = const_cast<Qt5Frame*>(static_cast<const Qt5Frame*>(pFrame));

    mpFrame->SetMenu(this);

    Qt5MainWindow* pMainWindow = mpFrame->GetTopLevelWindow();
    if (pMainWindow)
    {
        mpQMenuBar = pMainWindow->menuBar();
        mpQMenu = nullptr;
        mpQActionGroup = nullptr;

        DoFullMenuUpdate(mpVCLMenu);
    }
}

void Qt5Menu::DoFullMenuUpdate(Menu* pMenuBar)
{
    if (mbMenuBar && mpQMenuBar)
        mpQMenuBar->clear();

    for (sal_Int32 nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++)
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nItem);
        QMenu* pQMenu = InsertMenuItem(pSalMenuItem, MENU_APPEND);
        SetItemImage(nItem, pSalMenuItem, pSalMenuItem->maImage);

        if (pSalMenuItem->mpSubMenu != nullptr)
        {
            // correct parent menu and action group before calling HandleMenuActivateEvent
            pSalMenuItem->mpSubMenu->mpQMenu = pQMenu;
            pSalMenuItem->mpSubMenu->mpQActionGroup = nullptr;

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

void Qt5Menu::CheckItem(unsigned nPos, bool bChecked)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        QAction* pAction = pSalMenuItem->getAction();
        if (pAction)
            pAction->setChecked(bChecked);
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
