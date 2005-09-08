/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: container.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:44:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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


