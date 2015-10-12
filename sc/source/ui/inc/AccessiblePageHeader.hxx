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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPAGEHEADER_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPAGEHEADER_HXX

#include "AccessibleContextBase.hxx"
#include <editeng/svxenum.hxx>

class ScPreviewShell;
class EditTextObject;
class ScAccessiblePageHeaderArea;
class ScPreviewShell;

class ScAccessiblePageHeader : public ScAccessibleContextBase
{
public:
    ScAccessiblePageHeader( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, bool bHeader, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePageHeader();

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;
     virtual void SAL_CALL disposing() override;

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   grabFocus() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    virtual OUString SAL_CALL createAccessibleDescription() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL createAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual Rectangle GetBoundingBoxOnScreen() const throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Rectangle GetBoundingBox() const throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ScPreviewShell*     mpViewShell;
    sal_Int32           mnIndex;
    bool                mbHeader;
    typedef std::vector< ScAccessiblePageHeaderArea* > ScHFAreas;
    ScHFAreas           maAreas;
    sal_Int32           mnChildCount;

    bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    void AddChild(const EditTextObject* pArea, sal_uInt32 nIndex, SvxAdjust eAdjust);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
