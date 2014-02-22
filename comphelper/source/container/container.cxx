/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/container.hxx>
#include <osl/diagnose.h>


namespace comphelper
{



IndexAccessIterator::IndexAccessIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint)
    :m_xStartingPoint(xStartingPoint)
    ,m_xCurrentObject(NULL)
{
    OSL_ENSURE(m_xStartingPoint.is(), "IndexAccessIterator::IndexAccessIterator : no starting point !");
}

IndexAccessIterator::~IndexAccessIterator() {}


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> IndexAccessIterator::Next()
{
    bool bCheckingStartingPoint = !m_xCurrentObject.is();
        
    bool bAlreadyCheckedCurrent = m_xCurrentObject.is();
        
    if (!m_xCurrentObject.is())
        m_xCurrentObject = m_xStartingPoint;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xSearchLoop( m_xCurrentObject);
    bool bHasMoreToSearch = true;
    bool bFoundSomething = false;
    while (!bFoundSomething && bHasMoreToSearch)
    {
        
        if (!bAlreadyCheckedCurrent && ShouldHandleElement(xSearchLoop))
        {
            m_xCurrentObject = xSearchLoop;
            bFoundSomething = true;
        }
        else
        {
            
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainerAccess(xSearchLoop, ::com::sun::star::uno::UNO_QUERY);
            if (xContainerAccess.is() && xContainerAccess->getCount() && ShouldStepInto(xContainerAccess))
            {
                ::com::sun::star::uno::Any aElement(xContainerAccess->getByIndex(0));
                xSearchLoop = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue();
                bCheckingStartingPoint = false;

                m_arrChildIndizies.push_back((sal_Int32)0);
            }
            else
            {   
                while (m_arrChildIndizies.size() > 0)
                {   
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xSearchLoop, ::com::sun::star::uno::UNO_QUERY);
                    OSL_ENSURE(xChild.is(), "IndexAccessIterator::Next : a content has no approriate interface !");

                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xParent( xChild->getParent());
                    xContainerAccess = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xParent, ::com::sun::star::uno::UNO_QUERY);
                    OSL_ENSURE(xContainerAccess.is(), "IndexAccessIterator::Next : a content has an invalid parent !");

                    
                    sal_Int32 nOldSearchChildIndex = m_arrChildIndizies[m_arrChildIndizies.size() - 1];
                    m_arrChildIndizies.pop_back();

                    if (nOldSearchChildIndex < xContainerAccess->getCount() - 1)
                    {   
                        ++nOldSearchChildIndex;
                        
                        ::com::sun::star::uno::Any aElement(xContainerAccess->getByIndex(nOldSearchChildIndex));
                        xSearchLoop = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*) aElement.getValue();
                        bCheckingStartingPoint = false;
                        
                        m_arrChildIndizies.push_back((sal_Int32)nOldSearchChildIndex);

                        break;
                    }
                    
                    xSearchLoop = xParent;
                    bCheckingStartingPoint = false;
                }

                if (m_arrChildIndizies.empty() && !bCheckingStartingPoint)
                {   
                    bHasMoreToSearch = false;
                }
            }

            if (bHasMoreToSearch)
            {   
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


}   



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
