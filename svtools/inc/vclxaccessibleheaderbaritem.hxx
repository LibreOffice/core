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
#ifndef INCLUDED_SVTOOLS_INC_VCLXACCESSIBLEHEADERBARITEM_HXX
#define INCLUDED_SVTOOLS_INC_VCLXACCESSIBLEHEADERBARITEM_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase2.hxx>

#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

class HeaderBar;
class VCLExternalSolarLock;
class VclSimpleEvent;
class VclWindowEvent;

namespace utl {
class AccessibleStateSetHelper;
}


//    ----------------------------------------------------
//    class VCLXAccessibleHeaderBarItem
//    ----------------------------------------------------

typedef ::comphelper::OAccessibleExtendedComponentHelper    AccessibleExtendedComponentHelper_BASE;

typedef ::cppu::ImplHelper2<
::com::sun::star::accessibility::XAccessible,
::com::sun::star::lang::XServiceInfo > VCLXAccessibleHeaderBarItem_BASE;

class VCLXAccessibleHeaderBarItem :    public AccessibleExtendedComponentHelper_BASE,
                        public VCLXAccessibleHeaderBarItem_BASE
{
private:
    VCLExternalSolarLock*    m_pExternalLock;
    VclPtr<HeaderBar>        m_pHeadBar;
    sal_Int32                m_nIndexInParent;

protected:

    void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) override;

    // XComponent
    virtual void SAL_CALL    disposing() override;

public:
    VCLXAccessibleHeaderBarItem( HeaderBar*    pHeadBar, sal_Int32 _nIndexInParent );
    virtual ~VCLXAccessibleHeaderBarItem();

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override {};
    virtual sal_Int32 SAL_CALL getForeground() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(    ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SVTOOLS_INC_VCLXACCESSIBLEHEADERBARITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
