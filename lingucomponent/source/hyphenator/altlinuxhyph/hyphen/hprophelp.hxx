/*************************************************************************
 *
 *  $RCSfile: hprophelp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:34:42 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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


#ifndef _LINGU2_PROPHELP_HXX_
#define _LINGU2_PROPHELP_HXX_

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase2.hxx> // helper for implementations

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif

#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

namespace com { namespace sun { namespace star { namespace beans {
    class   XPropertySet;
}}}};

namespace com { namespace sun { namespace star { namespace linguistic2 {
    struct  LinguServiceEvent;
}}}};


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

///////////////////////////////////////////////////////////////////////////
// PropertyChgHelper
// virtual base class for all XPropertyChangeListener members of the
// various lingu services.
// Only propertyChange needs to be implemented.

class PropertyChgHelper :
    public cppu::WeakImplHelper2
    <
        XPropertyChangeListener,
        XLinguServiceEventBroadcaster
    >
{
    Sequence< OUString >                aPropNames;
    Reference< XInterface >             xMyEvtObj;
    ::cppu::OInterfaceContainerHelper   aLngSvcEvtListeners;
    Reference< XPropertySet >           xPropSet;

    // disallow use of copy-constructor and assignment-operator
    PropertyChgHelper( const PropertyChgHelper & );
    PropertyChgHelper & operator = ( const PropertyChgHelper & );

public:
    PropertyChgHelper(
            const Reference< XInterface > &rxSource,
            Reference< XPropertySet > &rxPropSet,
            const char *pPropNames[], USHORT nPropCount );
    virtual ~PropertyChgHelper();

    // XEventListener
    virtual void SAL_CALL
        disposing( const EventObject& rSource )
            throw(RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const PropertyChangeEvent& rEvt )
            throw(RuntimeException) = 0;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL
        addLinguServiceEventListener(
                const Reference< XLinguServiceEventListener >& rxListener )
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        removeLinguServiceEventListener(
                const Reference< XLinguServiceEventListener >& rxListener )
            throw(RuntimeException);

    // non UNO functions
    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void    LaunchEvent( const LinguServiceEvent& rEvt );

    const Sequence< OUString > &
            GetPropNames() const    { return aPropNames; }
    const Reference< XPropertySet > &
            GetPropSet() const      { return xPropSet; }
    const Reference< XInterface > &
            GetEvtObj() const       { return xMyEvtObj; }

};


///////////////////////////////////////////////////////////////////////////

class PropertyHelper_Hyphen :
    public PropertyChgHelper
{
    // default values
        INT16 nHyphMinLeading;
        INT16 nHyphMinTrailing;
        INT16 nHyphMinWordLength;

    // return values, will be set to default value or current temporary value
        INT16 nResHyphMinLeading;
        INT16 nResHyphMinTrailing;
        INT16 nResHyphMinWordLength;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Hyphen( const PropertyHelper_Hyphen & );
    PropertyHelper_Hyphen & operator = ( const PropertyHelper_Hyphen & );

protected:
    // PropertyChgHelper
    virtual void    SetDefault();

public:
    PropertyHelper_Hyphen(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > &rxPropSet);
    virtual ~PropertyHelper_Hyphen();

    virtual void    SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals );

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw(::com::sun::star::uno::RuntimeException);

    INT16   GetMinLeading() const               { return nHyphMinLeading; }
    INT16   GetMinTrailing() const              { return nHyphMinTrailing; }
    INT16   GetMinWordLength() const            { return nHyphMinWordLength; }
};

///////////////////////////////////////////////////////////////////////////

#endif
