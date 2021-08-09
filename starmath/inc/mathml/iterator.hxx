/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "element.hxx"

/** The purpose of this iterator is to be able to iterate threw an infinite element tree
  * infinite -> as much as your memory can hold
  * No call-backs that will end up in out of stack
  */

namespace mathml
{
template <typename runType>
void SmMlIteratorBottomToTop(SmMlElement* pMlElementTree, runType aRunType, void* aData)
{
    if (pMlElementTree == nullptr)
        return;

    SmMlElement* pCurrent;

    // Fetch the deepest element
    pCurrent = pMlElementTree;
    while (pCurrent->getSubElementsCount() != 0)
    {
        if (pCurrent->getSubElement(0) == nullptr)
            break;
        pCurrent = pCurrent->getSubElement(0);
    }

    do
    {
        // Fetch next element
        size_t nId = pCurrent->getSubElementId();
        // We are back to the top.
        if (pCurrent->getParentElement() == nullptr)
            break;
        // If this was the last, then turn back to the parent
        if (nId + 1 == pCurrent->getParentElement()->getSubElementsCount())
            pCurrent = pCurrent->getParentElement();
        else // If not, next is the one near it
        {
            // It could have sub elements
            if (pCurrent->getParentElement()->getSubElement(nId + 1) == nullptr)
                break;
            pCurrent = pCurrent->getParentElement()->getSubElement(nId + 1);
            // Fetch the deepest element
            while (pCurrent->getSubElementsCount() != 0)
            {
                if (pCurrent->getSubElement(0) == nullptr)
                    break;
                pCurrent = pCurrent->getSubElement(0);
            }
        }

        // Just in case of, but should be forbidden
        if (pCurrent != nullptr)
            aRunType(pCurrent, aData);

    } while (pCurrent != nullptr);
}

template <typename runType>
void SmMlIteratorTopToBottom(SmMlElement* pMlElementTree, runType aRunType, void* aData)
{
    if (pMlElementTree == nullptr)
        return;

    SmMlElement* pCurrent;

    // Fetch the deepest element
    pCurrent = pMlElementTree;
    aRunType(pCurrent, aData);
    while (pCurrent->getSubElementsCount() != 0)
    {
        if (pCurrent->getSubElement(0) == nullptr)
            break;
        pCurrent = pCurrent->getSubElement(0);
        aRunType(pCurrent, aData);
    }

    do
    {
        // Fetch next element
        size_t nId = pCurrent->getSubElementId();
        // We are back to the top.
        if (pCurrent->getParentElement() == nullptr)
            break;
        // If this was the last, then turn back to the parent
        if (nId + 1 == pCurrent->getParentElement()->getSubElementsCount())
            pCurrent = pCurrent->getParentElement();
        else // If not, next is the one near it
        {
            // It could have sub elements
            if (pCurrent->getParentElement()->getSubElement(nId + 1) == nullptr)
                break;
            pCurrent = pCurrent->getParentElement()->getSubElement(nId + 1);
            aRunType(pCurrent, aData);
            // Fetch the deepest element
            while (pCurrent->getSubElementsCount() != 0)
            {
                if (pCurrent->getSubElement(0) == nullptr)
                    break;
                pCurrent = pCurrent->getSubElement(0);
                aRunType(pCurrent, aData);
            }
        }

    } while (pCurrent != nullptr);
}

void SmMlIteratorFree(SmMlElement* pMlElementTree);

SmMlElement* SmMlIteratorCopy(SmMlElement* pMlElementTree);

} // end namespace mathml

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
