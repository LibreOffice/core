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
class QPushButton;
class QMenu;
class QMenuBar;
class Qt5MenuItem;
class Qt5Frame;

/*
 * Qt5Menu can represent
 * (1) the top-level menu of a menubar, in which case 'mbMenuBar' is true and
 *     'mpQMenuBar' refers to the corresponding QMenuBar
 * (2) another kind of menu (like a PopupMenu), in which case the corresponding QMenu
 *     object is instantiated and owned by this Qt5Menu (held in 'mpOwnedQMenu').
 * (3) a "submenu" in an existing menu (like (1)), in which case the corresponding
 *     QMenu object is owned by the corresponding Qt5MenuItem.
 *
 * For (2) and (3), member 'mpQMenu' points to the corresponding QMenu object.
 */
class Qt5Menu : public QObject, public SalMenu
{
    Q_OBJECT
private:
    std::vector<Qt5MenuItem*> maItems;
    VclPtr<Menu> mpVCLMenu;
    Qt5Menu* mpParentSalMenu;
    Qt5Frame* mpFrame;
    const bool m_bMenuBar;
    QMenuBar* mpQMenuBar;
    // self-created QMenu that this Qt5Menu represents, if applicable (s. comment for class)
    std::unique_ptr<QMenu> mpOwnedQMenu;
    // pointer to QMenu owned by the corresponding Qt5MenuItem or self (-> mpOwnedQMenu)
    QMenu* mpQMenu;
    QPushButton* mpCloseButton;
    QMetaObject::Connection maCloseButtonConnection;

    void DoFullMenuUpdate(Menu* pMenuBar);
    static void NativeItemText(OUString& rItemText);

    void InsertMenuItem(Qt5MenuItem* pSalMenuItem, unsigned nPos);

    void ReinitializeActionGroup(unsigned nPos);
    void ResetAllActionGroups();
    void UpdateActionGroupItem(const Qt5MenuItem* pSalMenuItem);

public:
    Qt5Menu(bool bMenuBar);

    virtual bool VisibleMenuBar() override; // must return TRUE to actually DISPLAY native menu bars

    virtual void InsertItem(SalMenuItem* pSalMenuItem, unsigned nPos) override;
    virtual void RemoveItem(unsigned nPos) override;
    virtual void SetSubMenu(SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos) override;
    virtual void SetFrame(const SalFrame* pFrame) override;
    const Qt5Frame* GetFrame() const;
    virtual void ShowMenuBar(bool bVisible) override;
    virtual bool ShowNativePopupMenu(FloatingWindow* pWin, const tools::Rectangle& rRect,
                                     FloatWinPopupFlags nFlags) override;
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
    virtual void ShowCloseButton(bool bShow) override;
    int GetMenuBarHeight() const override;

    void SetMenu(Menu* pMenu) { mpVCLMenu = pMenu; }
    Menu* GetMenu() { return mpVCLMenu; }
    unsigned GetItemCount() const { return maItems.size(); }
    Qt5MenuItem* GetItemAtPos(unsigned nPos) { return maItems[nPos]; }

private slots:
    static void slotMenuTriggered(Qt5MenuItem* pQItem);
    static void slotMenuAboutToShow(Qt5MenuItem* pQItem);
    static void slotMenuAboutToHide(Qt5MenuItem* pQItem);
    void slotCloseDocument();
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
