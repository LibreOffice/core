/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: container.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:29:14 $
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

#ifndef _COMPHELPER_CONTAINER_HXX_
#define _COMPHELPER_CONTAINER_HXX_

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

#define NOTIFY_LISTENERS(listeners, XIface, method, evt)                \
    if (listeners.getLength())                                          \
    {                                                                   \
        ::cppu::OInterfaceIteratorHelper aIter(listeners);              \
        while (aIter.hasMoreElements())                                 \
            reinterpret_cast< XIface* >(aIter.next())->method(evt);     \
    }

//========================================================================
//= IndexAccessIterator
//========================================================================
/** ein Iterator, der von einem XIndexAccess ausgehend alle Elemente durchiteriert (pre-order)
*/
class COMPHELPER_DLLPUBLIC IndexAccessIterator
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    m_xStartingPoint;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    m_xCurrentObject;
        // das aktuelle Objekt
    ::std::vector<sal_Int32>        m_arrChildIndizies;
        // ich bewege mich eigentlich durch einen Baum, dummerweise haben dessen
        // Elemente aber kein GetNextSibling, also muss ich mir merken, wo die Childs
        // innerhalb ihres Parents sitzen (das ist sozusagen der Pfad von der Wurzel
        // zu m_xCurrentObject

    ::rtl::OUString     m_ustrProperty;
        // der Name der gesuchten property

public:
    IndexAccessIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    Next();

    virtual void Invalidate() { m_xCurrentObject = NULL; }

protected:
    virtual sal_Bool ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& rElement) { return sal_True; }
        // damit kann man bestimmte Elemente ausschliessen, die werden dann einfach
        // uebergangen
        // wenn hier sal_True zurueckkommt, wird dieses Element von Next zurueckgeliefert, man kann sich hier also auch
        // gleich ein paar zusaetzliche Angaben zu dem Element holen (deswegen ist die Methode auch nicht const)
    virtual sal_Bool ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xContainer) const { return sal_True; }
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_CONTAINER_HXX_


