/*************************************************************************
 *
 *  $RCSfile: container.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_CONTAINER_HXX_
#define _COMPHELPER_CONTAINER_HXX_

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
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
class IndexAccessIterator
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


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 29.09.00 09:01:05  fs
 ************************************************************************/

