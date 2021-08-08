/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mathml/iterator.hxx>

#include <stdlib.h>

/** The purpose of this iterator is to be able to iterate threw an infinite element tree
  * infinite -> as much as your memory can hold
  * No call-backs that will end up in out of stack
  */

namespace mathml
{
inline void deleteElement(SmMlElement* aSmMlElement, void*) { delete aSmMlElement; }

inline void cloneElement(SmMlElement* aSmMlElement, void* aData)
{
    // Prepare data
    SmMlElement* aNewSmMlElement = new SmMlElement(*aSmMlElement);
    SmMlElement* aCopyTree = *static_cast<SmMlElement**>(aData);

    // Append data
    aCopyTree->setSubElement(aCopyTree->getSubElementsCount(), aNewSmMlElement);

    // Prepare for following
    // If it has sub elements, then it will be the next
    if (aSmMlElement->getSubElementsCount() != 0)
        aCopyTree = aNewSmMlElement;
    else // Otherwise remounts up to where it should be
    {
        while (aSmMlElement->getParentElement() != nullptr)
        {
            // get parent
            SmMlElement* pParent = aSmMlElement->getParentElement();
            // was this the last branch ?
            if (aSmMlElement->getSubElementId() + 1 == pParent->getSubElementsCount()) // yes -> up
                aCopyTree = aCopyTree->getParentElement();
            else // no -> stay
                break;
            // Prepare for next round
            aSmMlElement = pParent;
        }
    }

    // Closing extras
    *static_cast<SmMlElement**>(aData) = aCopyTree;
}

void SmMlIteratorFree(SmMlElement* pMlElementTree)
{
    SmMlIteratorBottomToTop(pMlElementTree, deleteElement, nullptr);
}

SmMlElement* SmMlIteratorCopy(SmMlElement* pMlElementTree)
{
    SmMlElement* aDummyElement = new SmMlElement();
    SmMlIteratorTopToBottom(pMlElementTree, cloneElement, &aDummyElement);
    SmMlElement* aResultElement = aDummyElement->getSubElement(0);
    delete aDummyElement;
    return aResultElement;
}

} // end namespace mathml

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
