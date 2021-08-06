/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <element.hxx>
#include <stdlib.h>

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
        if (pCurrent->getSubElement(0) != nullptr)
            pCurrent = pCurrent->getSubElement(0);
        else
            break;
    }

    do
    {
        // Fetch next element
        size_t nId = pCurrent->getSubElementId();
        // We are back to the top.
        if (pCurrent->getParent() == nullptr)
            break;
        // If this was the last, then turn back to the parent
        if (nId + 1 == pCurrent->getParent()->getSubElementsCount())
            pCurrent = pCurrent->getParent();
        else // If not, next is the one near it
        {
            // It could have sub elements
            if (pCurrent->getParent()->getSubElement(nId + 1) == nullptr)
                break;
            // Fetch the deepest element
            while (pCurrent->getSubElementsCount() != 0)
            {
                if (pCurrent->getSubElement(0) != nullptr)
                    pCurrent = pCurrent->getSubElement(0);
                else
                    break;
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
        if (pCurrent->getSubElement(0) != nullptr)
        {
            pCurrent = pCurrent->getSubElement(0);
            aRunType(pCurrent, aData);
        }
        else
            break;
    }

    do
    {
        // Fetch next element
        size_t nId = pCurrent->getSubElementId();
        // We are back to the top.
        if (pCurrent->getParent() == nullptr)
            break;
        // If this was the last, then turn back to the parent
        if (nId + 1 == pCurrent->getParent()->getSubElementsCount())
            pCurrent = pCurrent->getParent();
        else // If not, next is the one near it
        {
            // It could have sub elements
            if (pCurrent->getParent()->getSubElement(nId + 1) == nullptr)
                break;
            // Fetch the deepest element
            while (pCurrent->getSubElementsCount() != 0)
            {
                if (pCurrent->getSubElement(0) != nullptr)
                {
                    pCurrent = pCurrent->getSubElement(0);
                    aRunType(pCurrent, aData);
                }
                else
                    break;
            }
        }

    } while (pCurrent != nullptr);
}

inline void deleteElement(SmMlElement* aSmMlElement, void*) { delete aSmMlElement; }

inline void cloneElement(SmMlElement* aSmMlElement, void* aData)
{
    // Prepare data
    SmMlElement* aNewSmMlElement = new SmMlElement(aSmMlElement->getMlElementType());
    SmMlElement* aCopyTree = static_cast<SmMlElement**>(aData)[0];

    // Copy data
    for (size_t i = 0; i < aSmMlElement->getAttributeCount(); ++i)
        aNewSmMlElement->setAttribute(aSmMlElement->getAttribute[i]);
    aCopyTree->setSubElement(aNewSmMlElement, aCopyTree->getSubElementsCount());

    // Prepare for following
    // If it has sub elements, then it will be the next
    if (aSmMlElement->getSubElementsCount() != 0)
        *aCopyTree = aNewSmMlElement;
    else // Otherwise remounts up to where it should be
    {
        while (aSmMlElement->getParent() != nullptr)
        {
            // get parent
            SmMlElement pParent = aSmMlElement->getParent();
            // was this the last branch ?
            if (aSmMlElement->getSubElementId() + 1 == pParent->getSubElementsCount()) // yes -> up
            {
                aCopyTree = aCopyTree->getParent();
            }
            else // no -> stay
                break;
            // Prepare for next round
            aSmMlElement = pParent;
        }
    }

    // Closing extras
    *static_cast<SmMlElement**>(aData)[0] = aCopyTree;
}

inline void SmMlIteratorFree(SmMlElement* pMlElementTree)
{
    SmMlIteratorBottomToTop(pMlElementTree, deleteElement, nullptr);
}

inline void SmMlIteratorCopy(SmMlElement* pMlElementTree)
{
    SmMlElement* aDummyElement = new SmMlElement[1];
    SmMlIteratorTopToBottom(pMlElementTree, cloneElement, &aDummyElement);
    SmMlElement* aResultElement = aDummyElement->getSubElement(0);
    delete aDummyElement;
    return aResultElement;
}

} // end namespace mathml

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
