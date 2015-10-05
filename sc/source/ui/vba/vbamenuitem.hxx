/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAMENUITEM_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAMENUITEM_HXX

#include <ooo/vba/excel/XMenuItem.hpp>
#include <ooo/vba/XCommandBarControl.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XMenuItem > MenuItem_BASE;

class ScVbaMenuItem : public MenuItem_BASE
{
private:
    css::uno::Reference< ov::XCommandBarControl > m_xCommandBarControl;

public:
    ScVbaMenuItem( const css::uno::Reference< ov::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< ov::XCommandBarControl >& rCommandBarControl ) throw( css::uno::RuntimeException );

    virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getOnAction() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setOnAction( const OUString& _onaction ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAMENUITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
