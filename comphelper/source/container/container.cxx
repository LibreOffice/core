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

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/container.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>


namespace comphelper
{


IndexAccessIterator::IndexAccessIterator(css::uno::Reference< css::uno::XInterface> const & xStartingPoint)
    :m_xStartingPoint(xStartingPoint)
    ,m_xCurrentObject(nullptr)
{
    OSL_ENSURE(m_xStartingPoint.is(), "IndexAccessIterator::IndexAccessIterator : no starting point !");
}

IndexAccessIterator::~IndexAccessIterator() {}


css::uno::Reference< css::uno::XInterface> const & IndexAccessIterator::Next()
{
    bool bCheckingStartingPoint = !m_xCurrentObject.is();
        // Is the current node the starting point?
    bool bAlreadyCheckedCurrent = m_xCurrentObject.is();
        // Have I already tested the current node through ShouldHandleElement?
    if (!m_xCurrentObject.is())
        m_xCurrentObject = m_xStartingPoint;

    css::uno::Reference< css::uno::XInterface> xSearchLoop( m_xCurrentObject);
    bool bHasMoreToSearch = true;
    bool bFoundSomething = false;
    while (!bFoundSomething && bHasMoreToSearch)
    {
        // Priming loop
        if (!bAlreadyCheckedCurrent && ShouldHandleElement(xSearchLoop))
        {
            m_xCurrentObject = xSearchLoop;
            bFoundSomething = true;
        }
        else
        {
            // First, check to see if there's a match below
            css::uno::Reference< css::container::XIndexAccess> xContainerAccess(xSearchLoop, css::uno::UNO_QUERY);
            if (xContainerAccess.is() && xContainerAccess->getCount() && ShouldStepInto(xContainerAccess))
            {
                css::uno::Any aElement(xContainerAccess->getByIndex(0));
                xSearchLoop = *o3tl::doAccess<css::uno::Reference<css::uno::XInterface>>(aElement);
                bCheckingStartingPoint = false;

                m_arrChildIndizies.push_back((sal_Int32)0);
            }
            else
            {   // otherwise, look above and to the right, if possible
                while (m_arrChildIndizies.size() > 0)
                {   // If the list isn't empty and there's nothing above
                    css::uno::Reference< css::container::XChild> xChild(xSearchLoop, css::uno::UNO_QUERY);
                    OSL_ENSURE(xChild.is(), "IndexAccessIterator::Next : a content has no appropriate interface !");

                    css::uno::Reference< css::uno::XInterface> xParent( xChild->getParent());
                    xContainerAccess.set(xParent, css::uno::UNO_QUERY);
                    OSL_ENSURE(xContainerAccess.is(), "IndexAccessIterator::Next : a content has an invalid parent !");

                    // Remove the index that SearchLoop had within this parent from my stack
                    sal_Int32 nOldSearchChildIndex = m_arrChildIndizies[m_arrChildIndizies.size() - 1];
                    m_arrChildIndizies.pop_back();

                    if (nOldSearchChildIndex < xContainerAccess->getCount() - 1)
                    {   // Move to the right in this row
                        ++nOldSearchChildIndex;
                        // and check the next child
                        css::uno::Any aElement(xContainerAccess->getByIndex(nOldSearchChildIndex));
                        xSearchLoop = *o3tl::doAccess<css::uno::Reference<css::uno::XInterface>>(aElement);
                        bCheckingStartingPoint = false;
                        // and update its position in the list.
                        m_arrChildIndizies.push_back((sal_Int32)nOldSearchChildIndex);

                        break;
                    }
                    // Finally, if there's nothing more to do in this row (to the right), we'll move on to the next row.
                    xSearchLoop = xParent;
                    bCheckingStartingPoint = false;
                }

                if (m_arrChildIndizies.empty() && !bCheckingStartingPoint)
                {   //This is the case if there is nothing to the right in the original search loop
                    bHasMoreToSearch = false;
                }
            }

            if (bHasMoreToSearch)
            {   // If there is still a node in the tree which can be tested
                if (ShouldHandleElement(xSearchLoop))
                {
                    m_xCurrentObject = xSearchLoop;
                    bFoundSomething = true;
                }
                else
                    if (bCheckingStartingPoint)
                        bHasMoreToSearch = false;
                bAlreadyCheckedCurrent = true;
            }
        }
    }

    if (!bFoundSomething)
    {
        OSL_ENSURE(m_arrChildIndizies.empty(), "IndexAccessIterator::Next : items left on stack ! how this ?");
        Invalidate();
    }

    return m_xCurrentObject;
}


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
