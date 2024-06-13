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
class QAbstractButton;
class QAction;
class QActionGroup;
class QButtonGroup;
class QMenu;
class QMenuBar;
class QPushButton;
class QtMenuItem;
class QtFrame;

/*
 * QtMenu can represent
 * (1) the top-level menu of a menubar, in which case 'mbMenuBar' is true and
 *     'mpQMenuBar' refers to the corresponding QMenuBar
 * (2) another kind of menu (like a PopupMenu), in which case the corresponding QMenu
 *     object is instantiated and owned by this QtMenu (held in 'mpOwnedQMenu').
 * (3) a "submenu" in an existing menu (like (1)), in which case the corresponding
 *     QMenu object is owned by the corresponding QtMenuItem.
 *
 * For (2) and (3), member 'mpQMenu' points to the corresponding QMenu object.
 */
class QtMenu : public QObject, public SalMenu
{
    Q_OBJECT
private:
    std::vector<QtMenuItem*> maItems;
    VclPtr<Menu> mpVCLMenu;
    QtMenu* mpParentSalMenu;
    QtFrame* mpFrame;
    bool mbMenuBar;
    QMenuBar* mpQMenuBar;
    // self-created QMenu that this QtMenu represents, if applicable (s. comment for class)
    std::unique_ptr<QMenu> mpOwnedQMenu;
    // pointer to QMenu owned by the corresponding QtMenuItem or self (-> mpOwnedQMenu)
    QMenu* mpQMenu;
    QButtonGroup* m_pButtonGroup;

    // help ID of currently/last selected item
    static OUString m_sCurrentHelpId;

    void DoFullMenuUpdate(Menu* pMenuBar);

    void InsertMenuItem(QtMenuItem* pSalMenuItem, unsigned nPos);

    void ReinitializeActionGroup(unsigned nPos);
    void ResetAllActionGroups();
    void UpdateActionGroupItem(const QtMenuItem* pSalMenuItem);
    bool validateQMenuBar() const;
    QPushButton* ImplAddMenuBarButton(const QIcon& rIcon, const QString& rToolTip, int nId);
    void ImplRemoveMenuBarButton(int nId);
    void connectHelpShortcut(QMenu* pMenu);
    // set slots that handle signals relevant for help menu
    void connectHelpSignalSlots(QMenu* pMenu, QtMenuItem* pSalMenuItem);

public:
    QtMenu(bool bMenuBar);

    virtual bool VisibleMenuBar() override; // must return TRUE to actually DISPLAY native menu bars

    virtual void InsertItem(SalMenuItem* pSalMenuItem, unsigned nPos) override;
    virtual void RemoveItem(unsigned nPos) override;
    virtual void SetSubMenu(SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos) override;
    virtual void SetFrame(const SalFrame* pFrame) override;
    virtual void ShowMenuBar(bool bVisible) override;
    virtual bool ShowNativePopupMenu(FloatingWindow* pWin, const tools::Rectangle& rRect,
                                     FloatWinPopupFlags nFlags) override;
    QtMenu* GetTopLevel();
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
    virtual bool AddMenuBarButton(const SalMenuButtonItem&) override;
    virtual void RemoveMenuBarButton(sal_uInt16 nId) override;
    virtual tools::Rectangle GetMenuBarButtonRectPixel(sal_uInt16 nId, SalFrame*) override;
    virtual int GetMenuBarHeight() const override;

    void SetMenu(Menu* pMenu) { mpVCLMenu = pMenu; }
    Menu* GetMenu() { return mpVCLMenu; }
    unsigned GetItemCount() const { return maItems.size(); }
    QtMenuItem* GetItemAtPos(unsigned nPos) { return maItems[nPos]; }

private slots:
    static void slotShowHelp();
    static void slotMenuHovered(QtMenuItem* pItem);
    static void slotMenuTriggered(QtMenuItem* pQItem);
    static void slotMenuAboutToShow(QtMenuItem* pQItem);
    static void slotMenuAboutToHide(QtMenuItem* pQItem);
    void slotCloseDocument();
    void slotMenuBarButtonClicked(QAbstractButton*);
    void slotShortcutF10();
};

class QtMenuItem : public SalMenuItem
{
public:
    QtMenuItem(const SalItemParams*);

    QAction* getAction() const;

    QtMenu* mpParentMenu; // The menu into which this menu item is inserted
    QtMenu* mpSubMenu; // Submenu of this item (if defined)
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
