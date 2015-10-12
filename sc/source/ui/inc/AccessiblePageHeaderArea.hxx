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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPAGEHEADERAREA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPAGEHEADERAREA_HXX

#include "AccessibleContextBase.hxx"
#include <editeng/svxenum.hxx>

class EditTextObject;
namespace accessibility
{
    class AccessibleTextHelper;
}
class ScPreviewShell;

class ScAccessiblePageHeaderArea
    :   public ScAccessibleContextBase
{
public:
    //=====  internal  ========================================================
    ScAccessiblePageHeaderArea(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell,
        const EditTextObject* pEditObj,
        bool bHeader,
        SvxAdjust eAdjust);
protected:
    virtual ~ScAccessiblePageHeaderArea();
public:
    const EditTextObject* GetEditTextObject() const { return mpEditObj; }

    using ScAccessibleContextBase::disposing;
    virtual void SAL_CALL disposing() override;

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount()
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext and Accessible service.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    virtual OUString SAL_CALL createAccessibleDescription() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL createAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual Rectangle GetBoundingBoxOnScreen() const throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Rectangle GetBoundingBox() const throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    EditTextObject* mpEditObj;
    accessibility::AccessibleTextHelper* mpTextHelper;
    ScPreviewShell* mpViewShell;
    bool            mbHeader;
    SvxAdjust       meAdjust;

    void CreateTextHelper();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
