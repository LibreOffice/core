/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: container.hxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _COMPHELPER_CONTAINER_HXX_
#define _COMPHELPER_CONTAINER_HXX_

#include <vector>
#include "com/sun/star/uno/Reference.hxx"
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

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

    virtual ~IndexAccessIterator();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>    Next();

    virtual void Invalidate() { m_xCurrentObject = NULL; }

protected:
    virtual sal_Bool ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& /*rElement*/) { return sal_True; }
        // damit kann man bestimmte Elemente ausschliessen, die werden dann einfach
        // uebergangen
        // wenn hier sal_True zurueckkommt, wird dieses Element von Next zurueckgeliefert, man kann sich hier also auch
        // gleich ein paar zusaetzliche Angaben zu dem Element holen (deswegen ist die Methode auch nicht const)
    virtual sal_Bool ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& /*xContainer*/) const { return sal_True; }
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_CONTAINER_HXX_


