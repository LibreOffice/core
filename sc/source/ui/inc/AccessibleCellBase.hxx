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

#pragma once

#include "AccessibleContextBase.hxx"
#include <address.hxx>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

class ScAccessibleCellBase
    : public cppu::ImplInheritanceHelper<ScAccessibleContextBase,
                                         css::accessibility::XAccessibleValue>
{
public:
    //=====  internal  ========================================================
    ScAccessibleCellBase(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScAddress& rCellAddress,
        sal_Int64 nIndex);
protected:
    virtual ~ScAccessibleCellBase() override;
public:

    virtual bool isVisible() override;

    ///=====  XAccessibleComponent  ============================================

    virtual sal_Int32 SAL_CALL getForeground(  ) override;

    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return this objects index among the parents children.
    virtual sal_Int64 SAL_CALL
        getAccessibleIndexInParent() override;

protected:
    /// Return this object's description.
    virtual OUString
        createAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString
        createAccessibleName() override;

public:
    ///=====  XAccessibleValue  ================================================

    virtual css::uno::Any SAL_CALL
        getCurrentValue() override;

    virtual sal_Bool SAL_CALL
        setCurrentValue( const css::uno::Any& aNumber ) override;

    virtual css::uno::Any SAL_CALL
        getMaximumValue(  ) override;

    virtual css::uno::Any SAL_CALL
        getMinimumValue(  ) override;

    virtual css::uno::Any SAL_CALL
        getMinimumIncrement(  ) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

protected:
    ScAddress maCellAddress;

    ScDocument* mpDoc;

    sal_Int64 mnIndex;

private:
    virtual bool IsEditable(sal_Int64 nParentStates);
protected:
    /// @throw css::uno::RuntimeException
    OUString GetNote() const;

    /// @throw css::uno::RuntimeException
    OUString GetAllDisplayNote() const;
    /// @throw css::uno::RuntimeException
    OUString getShadowAttrs() const;
    /// @throw css::uno::RuntimeException
    OUString getBorderAttrs();
public:
    const ScAddress& GetCellAddress() const { return maCellAddress; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
