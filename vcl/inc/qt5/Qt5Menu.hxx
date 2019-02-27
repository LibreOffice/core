/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <salmenu.hxx>

#include <QtCore/QObject>

#include <memory>

class MenuItemList;
class QAction;
class QActionGroup;
class QMenu;
class QMenuBar;
class Qt5MenuItem;
class Qt5Frame;

class Qt5Menu : public QObject, public SalMenu
{
    Q_OBJECT
private:
    std::vector<Qt5MenuItem*> maItems;
    VclPtr<Menu> mpVCLMenu;
    Qt5Menu* mpParentSalMenu;
    Qt5Frame* mpFrame;
    bool mbMenuBar;
    QMenuBar* mpQMenuBar;
    QMenu* mpQMenu;

    void DoFullMenuUpdate(Menu* pMenuBar);
    static void NativeItemText(OUString& rItemText);

    void InsertMenuItem(Qt5MenuItem* pSalMenuItem, unsigned nPos);

    void ReinitializeActionGroup(unsigned nPos);
    void ResetAllActionGroups();
    void UpdateActionGroupItem(Qt5MenuItem* pSalMenuItem);

public:
    Qt5Menu(bool bMenuBar);

    virtual bool VisibleMenuBar() override; // must return TRUE to actually DISPLAY native menu bars

    virtual void InsertItem(SalMenuItem* pSalMenuItem, unsigned nPos) override;
    virtual void RemoveItem(unsigned nPos) override;
    virtual void SetSubMenu(SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos) override;
    virtual void SetFrame(const SalFrame* pFrame) override;
    const Qt5Frame* GetFrame() const;
    Qt5Menu* GetTopLevel();
    virtual void SetItemBits(unsigned nPos, MenuItemBits nBits) override;
    virtual void CheckItem(unsigned nPos, bool bCheck) override;
    virtual void EnableItem(unsigned nPos, bool bEnable) override;
    virtual void ShowItem(unsigned nPos, bool bShow) override;
    virtual void SetItemText(unsigned nPos, SalMenuItem* pSalMenuItem,
                             const OUString& rText) override;
    virtual void SetItemImage(unsigned nPos, SalMenuItem* pSalMenuItem,
                              const Image& rImage) override;
    virtual void SetAccelerator(unsigned nPos, SalMenuItem* pSalMenuItem,
                                const vcl::KeyCode& rKeyCode, const OUString& rKeyName) override;
    virtual void GetSystemMenuData(SystemMenuData* pData) override;

    void SetMenu(Menu* pMenu) { mpVCLMenu = pMenu; }
    Menu* GetMenu() { return mpVCLMenu; }
    unsigned GetItemCount() { return maItems.size(); }
    Qt5MenuItem* GetItemAtPos(unsigned nPos) { return maItems[nPos]; }

Q_SIGNALS:
    void setFrameSignal(const SalFrame* pFrame);

private slots:
    static void slotMenuTriggered(Qt5MenuItem* pQItem);
    static void slotMenuAboutToShow(Qt5MenuItem* pQItem);
    static void slotMenuAboutToHide(Qt5MenuItem* pQItem);
};

class Qt5MenuItem : public SalMenuItem
{
public:
    Qt5MenuItem(const SalItemParams*);

    QAction* getAction() const;

    Qt5Menu* mpParentMenu; // The menu into which this menu item is inserted
    Qt5Menu* mpSubMenu; // Submenu of this item (if defined)
    std::unique_ptr<QAction> mpAction; // action corresponding to this item
    std::unique_ptr<QMenu> mpMenu; // menu corresponding to this item
    std::shared_ptr<QActionGroup> mpActionGroup; // empty if it's a separator element
    sal_uInt16 mnId; // Item ID
    MenuItemType mnType; // Item type
    bool mbVisible; // Item visibility.
    bool mbEnabled; // Item active.
    Image maImage; // Item image
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
