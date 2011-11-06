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


