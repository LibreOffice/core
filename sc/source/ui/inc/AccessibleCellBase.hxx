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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECELLBASE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLECELLBASE_HXX

#include "AccessibleContextBase.hxx"
#include "global.hxx"
#include "address.hxx"
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <cppuhelper/implbase.hxx>

typedef cppu::ImplHelper < css::accessibility::XAccessibleValue >
                    ScAccessibleCellBaseImpl;

class ScAccessibleCellBase
    :   public ScAccessibleContextBase,
        public ScAccessibleCellBaseImpl
{
public:
    //=====  internal  ========================================================
    ScAccessibleCellBase(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScAddress& rCellAddress,
        sal_Int32 nIndex);
protected:
    virtual ~ScAccessibleCellBase() override;
public:

    virtual bool SAL_CALL isVisible()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    ///=====  XAccessibleComponent  ============================================

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

public:
    ///=====  XAccessibleValue  ================================================

    virtual css::uno::Any SAL_CALL
        getCurrentValue()
        throw (css::uno::RuntimeException,
               std::exception) override;

    virtual sal_Bool SAL_CALL
        setCurrentValue( const css::uno::Any& aNumber )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL
        getMaximumValue(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL
        getMinimumValue(  )
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    /// returns the possible types
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    ScAddress maCellAddress;

    ScDocument* mpDoc;

    sal_Int32 mnIndex;

private:
    virtual bool IsEditable(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);
protected:
    OUString SAL_CALL GetNote()
        throw (css::uno::RuntimeException);

    OUString SAL_CALL GetAllDisplayNote()
        throw (css::uno::RuntimeException, std::exception);
    OUString SAL_CALL getShadowAttrs()
        throw (css::uno::RuntimeException, std::exception);
    OUString SAL_CALL getBorderAttrs()
        throw (css::uno::RuntimeException, std::exception);
public:
    const ScAddress& GetCellAddress() const { return maCellAddress; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
