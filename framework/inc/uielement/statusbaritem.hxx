/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_STATUSBARITEM_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_STATUSBARITEM_HXX

#include <com/sun/star/ui/XStatusbarItem.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <vcl/vclptr.hxx>

class StatusBar;

namespace framework
{

struct AddonStatusbarItemData;

typedef cppu::WeakComponentImplHelper< com::sun::star::ui::XStatusbarItem > StatusbarItem_Base;

class StatusbarItem : protected cppu::BaseMutex,
                      public StatusbarItem_Base
{
public:
    explicit StatusbarItem(
                            StatusBar              *pStatusBar,
                            AddonStatusbarItemData *pItemData,
                            sal_uInt16              nId,
                            const rtl::OUString&   aCommand );
    virtual ~StatusbarItem();

    void SAL_CALL disposing() override;

    // com::sun::star::ui::XStatusbarItem Attributes
    virtual ::rtl::OUString SAL_CALL getCommand() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_uInt16 SAL_CALL getItemId() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_uInt32 SAL_CALL getWidth() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_uInt16 SAL_CALL getStyle() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getOffset() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getItemRect() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setText( const rtl::OUString& rText ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getHelpText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHelpText( const rtl::OUString& rHelpText ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getQuickHelpText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setQuickHelpText( const rtl::OUString& rQuickHelpText ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::rtl::OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAccessibleName( const rtl::OUString& rAccessibleName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getVisible() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // com::sun::star::ui::XStatusbarItem Methods
    virtual void SAL_CALL repaint(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    VclPtr<StatusBar>       m_pStatusBar;
    AddonStatusbarItemData *m_pItemData;
    sal_uInt16              m_nId;
    sal_uInt16              m_nStyle;
    rtl::OUString           m_aCommand;

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
