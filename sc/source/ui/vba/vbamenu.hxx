/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef SC_VBA_MENU_HXX
#define SC_VBA_MENU_HXX

#include <ooo/vba/excel/XMenu.hpp>
#include <ooo/vba/XCommandBarControl.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XMenu > Menu_BASE;

class ScVbaMenu : public Menu_BASE
{
private:
    css::uno::Reference< ov::XCommandBarControl > m_xCommandBarControl;

public:
    ScVbaMenu( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< ov::XCommandBarControl >& xCommandBarControl ) throw( css::uno::RuntimeException );

    virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL MenuItems( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif//SC_VBA_MENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
