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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCCELL_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCCELL_HXX

#include "acccontext.hxx"
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <accselectionhelper.hxx>

class SwCellFrm;
class SwAccessibleTable;
class SwFrameFormat;

class SwAccessibleCell : public SwAccessibleContext,
                  ::com::sun::star::accessibility::XAccessibleValue,
                  ::com::sun::star::accessibility::XAccessibleSelection,
                    public  ::com::sun::star::accessibility::XAccessibleExtendedAttributes
{
    // Implementation for XAccessibleSelection interface
    SwAccessibleSelectionHelper aSelectionHelper;
    bool    bIsSelected;    // protected by base class mutex

    bool    IsSelected();

    bool _InvalidateMyCursorPos();
    bool _InvalidateChildrenCursorPos( const SwFrm *pFrm );

    rtl::Reference<SwAccessibleTable> m_pAccTable;

protected:
    // Set states for getAccessibleStateSet.
    // This derived class additionally sets SELECTABLE(1) and SELECTED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet ) override;

    virtual void _InvalidateCursorPos() override;

    virtual ~SwAccessibleCell();

public:
    SwAccessibleCell( SwAccessibleMap* pInitMap, const SwCellFrm *pCellFrm );

    virtual bool HasCursor() override;   // required by map to remember that object

    // XAccessibleContext

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo

    // Returns an identifier for the implementation of this object.
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // Return whether the specified service is supported by this class.
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // Returns a list of all supported services.  In this case that is just
    // the AccessibleContext service.
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void Dispose( bool bRecursive = false ) override;

    virtual void InvalidatePosOrSize( const SwRect& rFrm ) override;

    // XInterface

    // (XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleValue).

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleValue

    // XAccessibleExtendedAttributes
    ::com::sun::star::uno::Any SAL_CALL getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override ;
private:
    SwFrameFormat* GetTableBoxFormat() const;

public:
    virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue( )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL setCurrentValue(
        const ::com::sun::star::uno::Any& aNumber )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    // XAccessibleComponent
    sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
