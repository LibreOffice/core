/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/container.hxx>
#include <osl/diagnose.h>

//.........................................................................
namespace comphelper
{
//.........................................................................

//==============================================================================
IndexAccessIterator::IndexAccessIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint)
    :m_xStartingPoint(xStartingPoint)
    ,m_xCurrentObject(NULL)
{
    OSL_ENSURE(m_xStartingPoint.is(), "IndexAccessIterator::IndexAccessIterator : no starting point !");
}

IndexAccessIterator::~IndexAccessIterator() {}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> IndexAccessIterator::Next()
{
    sal_Bool bCheckingStartingPoint = !m_xCurrentObject.is();
        // ist die aktuelle Node der Anfangspunkt ?
    sal_Bool bAlreadyCheckedCurrent = m_xCurrentObject.is();
        // habe ich die aktuelle Node schon mal mittels ShouldHandleElement testen ?
    if (!m_xCurrentObject.is())
        m_xCurrentObject = m_xStartingPoint;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xSearchLoop( m_xCurrentObject);
    sal_Bool bHasMoreToSearch = sal_True;
    sal_Bool bFoundSomething = sal_False;
    while (!bFoundSomething && bHasMoreToSearch)
    {
        // pre-order-traversierung
        if (!bAlreadyCheckedCurrent && ShouldHandleElement(xSearchLoop))
        {
            m_xCurrentObject = xSearchLoop;
            bFoundSomething = sal_True;
        }
        else
        {
            // zuerst absteigen, wenn moeglich
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainerAccess(xSearchLoop, ::com::sun::star::uno::UNO_QUERY);
            if (xContainerAccess.is() && xContainerAccess->getCount() && ShouldStepInto(xContainerAccess))
            {   // zum ersten Child
                ::com::sun::star::uno::Any aElement(xContainerAccess->getByIndex(0));
                xSearchLoop = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue();
                bCheckingStartingPoint = sal_False;

                m_arrChildIndizies.push_back((sal_Int32)0);
            }
            else
            {
                // dann nach oben und nach rechts, wenn moeglich
                while (m_arrChildIndizies.size() > 0)
                {   // (mein Stack ist nich leer, also kann ich noch nach oben gehen)
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xSearchLoop, ::com::sun::star::uno::UNO_QUERY);
                    OSL_ENSURE(xChild.is(), "IndexAccessIterator::Next : a content has no approriate interface !");

                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xParent( xChild->getParent());
                    xContainerAccess = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xParent, ::com::sun::star::uno::UNO_QUERY);
                    OSL_ENSURE(xContainerAccess.is(), "IndexAccessIterator::Next : a content has an invalid parent !");

                    // den Index, den SearchLoop in diesem Parent hatte, von meinem 'Stack'
                    sal_Int32 nOldSearchChildIndex = m_arrChildIndizies[m_arrChildIndizies.size() - 1];
                    m_arrChildIndizies.pop_back();

                    if (nOldSearchChildIndex < xContainerAccess->getCount() - 1)
                    {   // auf dieser Ebene geht es noch nach rechts
                        ++nOldSearchChildIndex;
                        // also das naechste Child
                        ::com::sun::star::uno::Any aElement(xContainerAccess->getByIndex(nOldSearchChildIndex));
                        xSearchLoop = *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*) aElement.getValue();
                        bCheckingStartingPoint = sal_False;
                        // und dessen Position auf den 'Stack'
                        m_arrChildIndizies.push_back((sal_Int32)nOldSearchChildIndex);

                        break;
                    }
                    // hierher komme ich, wenn es auf der aktuellen Ebene nicht nach rechts geht, dann mache ich eine darueber weiter
                    xSearchLoop = xParent;
                    bCheckingStartingPoint = sal_False;
                }

                if ((m_arrChildIndizies.size() == 0) && !bCheckingStartingPoint)
                {   // das ist genau dann der Fall, wenn ich keinen rechten Nachbarn fuer irgendeinen der direkten Vorfahren des
                    // urspruenglichen xSearchLoop gefunden habe
                    bHasMoreToSearch = sal_False;
                }
            }

            if (bHasMoreToSearch)
            {   // ich habe in xSearchLoop jetzt ein Interface eines 'Knotens' meines 'Baumes', den ich noch abtesten kann
                if (ShouldHandleElement(xSearchLoop))
                {
                    m_xCurrentObject = xSearchLoop;
                    bFoundSomething = sal_True;
                }
                else
                    if (bCheckingStartingPoint)
                        // ich bin noch am Anfang, konnte nicht absteigen, und habe an diesem Anfang nix gefunden -> nix mehr zu tun
                        bHasMoreToSearch = sal_False;
                bAlreadyCheckedCurrent = sal_True;
            }
        }
    }

    if (!bFoundSomething)
    {
        OSL_ENSURE(m_arrChildIndizies.size() == 0, "IndexAccessIterator::Next : items left on stack ! how this ?");
        Invalidate();
    }

    return m_xCurrentObject;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................


