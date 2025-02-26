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

#include "thumbnailviewacc.hxx"
#include "thumbnailviewitemacc.hxx"

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/thumbnailview.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <mutex>

using namespace ::com::sun::star;

ThumbnailViewItemAcc::ThumbnailViewItemAcc(ThumbnailViewItem* pThumbnailViewItem)
    : mpThumbnailViewItem(pThumbnailViewItem)
{
}

ThumbnailViewItemAcc::~ThumbnailViewItemAcc()
{
}

void ThumbnailViewItemAcc::ThumbnailViewItemDestroyed()
{
    std::scoped_lock aGuard( maMutex );
    mpThumbnailViewItem = nullptr;
}

uno::Reference< accessibility::XAccessibleContext > SAL_CALL ThumbnailViewItemAcc::getAccessibleContext()
{
    return this;
}

sal_Int64 SAL_CALL ThumbnailViewItemAcc::getAccessibleChildCount()
{
    return 0;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleChild( sal_Int64 )
{
    throw lang::IndexOutOfBoundsException();
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleParent()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xRet;

    if (mpThumbnailViewItem)
        xRet = mpThumbnailViewItem->mrParent.getAccessible();

    return xRet;
}

sal_Int64 SAL_CALL ThumbnailViewItemAcc::getAccessibleIndexInParent()
{
    const SolarMutexGuard aSolarGuard;
    // The index defaults to -1 to indicate the child does not belong to its
    // parent.
    sal_Int64 nIndexInParent = -1;

    if (mpThumbnailViewItem)
    {
        bool bDone = false;

        sal_uInt16 nCount = mpThumbnailViewItem->mrParent.ImplGetVisibleItemCount();
        ThumbnailViewItem* pItem;
        for (sal_uInt16 i=0; i<nCount && !bDone; i++)
        {
            // Guard the retrieval of the i-th child with a try/catch block
            // just in case the number of children changes in the meantime.
            try
            {
                pItem = mpThumbnailViewItem->mrParent.ImplGetVisibleItem(i);
            }
            catch (const lang::IndexOutOfBoundsException&)
            {
                pItem = nullptr;
            }

            if (pItem && pItem == mpThumbnailViewItem)
            {
                nIndexInParent = i;
                bDone = true;
            }
        }
    }

    return nIndexInParent;
}

sal_Int16 SAL_CALL ThumbnailViewItemAcc::getAccessibleRole()
{
    return accessibility::AccessibleRole::LIST_ITEM;
}

OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleDescription()
{
    return OUString();
}

OUString SAL_CALL ThumbnailViewItemAcc::getAccessibleName()
{
    const SolarMutexGuard aSolarGuard;
    OUString aRet;

    if (mpThumbnailViewItem)
    {
        aRet = mpThumbnailViewItem->maTitle;

        if( aRet.isEmpty() )
        {
            aRet = "Item " + OUString::number(static_cast<sal_Int32>(mpThumbnailViewItem->mnId));
        }
    }

    return aRet;
}

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL ThumbnailViewItemAcc::getAccessibleRelationSet()
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

sal_Int64 SAL_CALL ThumbnailViewItemAcc::getAccessibleStateSet()
{
    const SolarMutexGuard aSolarGuard;
    sal_Int64 nStateSet = 0;

    if (mpThumbnailViewItem)
    {
        nStateSet |= accessibility::AccessibleStateType::ENABLED;
        nStateSet |= accessibility::AccessibleStateType::SENSITIVE;
        nStateSet |= accessibility::AccessibleStateType::SHOWING;
        nStateSet |= accessibility::AccessibleStateType::VISIBLE;
        nStateSet |= accessibility::AccessibleStateType::TRANSIENT;
        nStateSet |= accessibility::AccessibleStateType::SELECTABLE;
        nStateSet |= accessibility::AccessibleStateType::FOCUSABLE;

        if (mpThumbnailViewItem->isSelected())
        {
            nStateSet |= accessibility::AccessibleStateType::SELECTED;
            if (mpThumbnailViewItem->mrParent.HasChildFocus())
                nStateSet |= accessibility::AccessibleStateType::FOCUSED;
        }
    }

    return nStateSet;
}

lang::Locale SAL_CALL ThumbnailViewItemAcc::getLocale()
{
    const SolarMutexGuard aSolarGuard;
    uno::Reference< accessibility::XAccessible >    xParent( getAccessibleParent() );
    lang::Locale                                    aRet( u""_ustr, u""_ustr, u""_ustr );

    if( xParent.is() )
    {
        uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );

        if( xParentContext.is() )
            aRet = xParentContext->getLocale();
    }

    return aRet;
}

uno::Reference< accessibility::XAccessible > SAL_CALL ThumbnailViewItemAcc::getAccessibleAtPoint( const awt::Point& )
{
    uno::Reference< accessibility::XAccessible > xRet;
    return xRet;
}

awt::Rectangle ThumbnailViewItemAcc::implGetBounds()
{
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle      aRet;

    if (mpThumbnailViewItem)
    {
        tools::Rectangle aRect(mpThumbnailViewItem->getDrawArea());
        tools::Rectangle   aParentRect;

        // get position of the accessible parent in screen coordinates
        uno::Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            uno::Reference<XAccessibleComponent> xParentComponent(xParent->getAccessibleContext(), uno::UNO_QUERY);
            if (xParentComponent.is())
            {
                awt::Size aParentSize = xParentComponent->getSize();
                aParentRect = tools::Rectangle(0, 0, aParentSize.Width, aParentSize.Height);
            }
        }

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

void SAL_CALL ThumbnailViewItemAcc::grabFocus()
{
    // nothing to do
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getForeground(  )
{
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL ThumbnailViewItemAcc::getBackground(  )
{
    return static_cast<sal_Int32>(Application::GetSettings().GetStyleSettings().GetWindowColor());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
