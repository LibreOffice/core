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

QMenu* Qt5Menu::InsertMenuItem(Qt5MenuItem* pSalMenuItem)
{
    QMenu* pQMenu = mpQMenu;
    sal_uInt16 nId = pSalMenuItem->mnId;
    OUString aText = mpVCLMenu->GetItemText(nId);
    NativeItemText(aText);
    vcl::KeyCode nAccelKey = mpVCLMenu->GetAccelKey(nId);
    bool bChecked = mpVCLMenu->IsItemChecked(nId);
    MenuItemBits itemBits = mpVCLMenu->GetItemBits(nId);

    if (mbMenuBar && mpQMenuBar)
        // top-level menu
        pQMenu = mpQMenuBar->addMenu(toQString(aText));
    else
    {
        if (pSalMenuItem->mpSubMenu)
        {
            // submenu
            pQMenu = pQMenu->addMenu(toQString(aText));
            mpQActionGroup = new QActionGroup(pQMenu);
        }
        else
        {
            if (pSalMenuItem->mnType == MenuItemType::SEPARATOR)
                pQMenu->addSeparator();
            else
            {
                // leaf menu
                QAction* pAction = pQMenu->addAction(toQString(aText));
                pSalMenuItem->mpAction = pAction;
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

        DoFullMenuUpdate(mpVCLMenu);
    }
}

void Qt5Menu::DoFullMenuUpdate(Menu* pMenuBar, QMenu* pParentMenu)
{
    mpQMenu = pParentMenu;

    if (mbMenuBar && mpQMenuBar)
        mpQMenuBar->clear();
    mpQActionGroup = nullptr;

    for (sal_Int32 nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++)
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nItem);
        QMenu* pQMenu = InsertMenuItem(pSalMenuItem);

        if (pSalMenuItem->mpSubMenu != nullptr)
        {
            pMenuBar->HandleMenuActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
            pSalMenuItem->mpSubMenu->DoFullMenuUpdate(pMenuBar, pQMenu);
            pMenuBar->HandleMenuDeActivateEvent(pSalMenuItem->mpSubMenu->GetMenu());
        }
    }
}

void Qt5Menu::ShowItem(unsigned nPos, bool bShow)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        if (pSalMenuItem->mpAction)
            pSalMenuItem->mpAction->setVisible(bShow);
        pSalMenuItem->mbVisible = bShow;
    }
}

void Qt5Menu::CheckItem(unsigned nPos, bool bChecked)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        if (pSalMenuItem->mpAction)
            pSalMenuItem->mpAction->setChecked(bChecked);
    }
}

void Qt5Menu::EnableItem(unsigned nPos, bool bEnable)
{
    if (nPos < maItems.size())
    {
        Qt5MenuItem* pSalMenuItem = GetItemAtPos(nPos);
        if (pSalMenuItem->mpAction)
            pSalMenuItem->mpAction->setEnabled(bEnable);
        pSalMenuItem->mbEnabled = bEnable;
    }
}

void Qt5Menu::SetItemText(unsigned, SalMenuItem* pItem, const OUString& rText)
{
    Qt5MenuItem* pSalMenuItem = static_cast<Qt5MenuItem*>(pItem);
    if (pSalMenuItem->mpAction)
        pSalMenuItem->mpAction->setText(toQString(rText));
}

void Qt5Menu::SetItemImage(unsigned, SalMenuItem* pItem, const Image& rImage)
{
    BitmapEx aBmpEx(rImage.GetBitmapEx());
    Bitmap aBmp(aBmpEx.GetBitmap());

    if (!aBmp || !aBmp.ImplGetSalBitmap())
        return;

    // simple case, no transparency
    Qt5MenuItem* pSalMenuItem = static_cast<Qt5MenuItem*>(pItem);
    if (pSalMenuItem->mpAction)
        pSalMenuItem->mpAction->setIcon(QPixmap::fromImage(
            *static_cast<Qt5Bitmap*>(aBmp.ImplGetSalBitmap().get())->GetQImage()));
}

void Qt5Menu::SetAccelerator(unsigned, SalMenuItem* pItem, const vcl::KeyCode&,
                             const OUString& rText)
{
    Qt5MenuItem* pSalMenuItem = static_cast<Qt5MenuItem*>(pItem);
    if (pSalMenuItem->mpAction)
        pSalMenuItem->mpAction->setShortcut(
            QKeySequence(toQString(rText), QKeySequence::PortableText));
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

        // it is possible that dispatcher of a menu entry was cleared since
        // initial activation (happens e.g. when extension comes w/ its own menus
        // and dispatchers) so re-activate the entry before dispatching the command
        pTopLevel->GetMenu()->HandleMenuActivateEvent(pSalMenu->GetMenu());
        pTopLevel->GetMenu()->HandleMenuCommandEvent(pSalMenu->GetMenu(), pQItem->mnId);
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
    , mpAction(nullptr)
    , mnId(pItemData->nId)
    , mnType(pItemData->eType)
    , mbVisible(true)
    , mbEnabled(true)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
