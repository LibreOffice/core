/*************************************************************************
 *
 *  $RCSfile: SwXTextDefaults.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-03-21 14:50:43 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _SW_XTEXT_DEFAULTS_HXX
#define _SW_XTEXT_DEFAULTS_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

class SwXTextDefaults :  public cppu::WeakImplHelper2
                        <
                            com::sun::star::beans::XPropertyState,
                            com::sun::star::beans::XPropertySet
                        >
{
    SfxItemPropertySet  aPropSet;
    SwDoc * pDoc;
public:
    SwXTextDefaults ( SwDoc * pNewDoc );
    ~SwXTextDefaults ();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& rPropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& rPropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropertyNames )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& rPropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& rPropertyName )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};
#endif
