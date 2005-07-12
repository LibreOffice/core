/*************************************************************************
 *
 *  $RCSfile: propertysethelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 14:11:32 $
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

#ifndef __FRAMEWORK_CLASSES_PROPERTYSETHELPER_HXX_
#define __FRAMEWORK_CLASSES_PROPERTYSETHELPER_HXX_

#include <hash_map>
#include <vector>

//_________________________________________________________________________________________________________________
//  my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/transactionbase.hxx>
#include <macros/debug.hxx>
#include <general.h>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYEXISTEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyExistException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes

//_________________________________________________________________________________________________________________
//  namespace

namespace framework{

//_________________________________________________________________________________________________________________

/** supports the API XPropertySet and XPropertySetInfo.
 *
 *  It must be used as baseclass. The internal list of supported
 *  properties can be changed everytimes so dynamic property set's
 *  can be implemented.
 *
 *  Further the derived and this base class share the same lock.
 *  So it's possible to be threadsafe if it's needed.
*/
class PropertySetHelper : public css::beans::XPropertySet
                        , public css::beans::XPropertySetInfo
                        , public ThreadHelpBase
                        , public TransactionBase
{
    //-------------------------------------------------------------------------
    /* types */
    protected:

        typedef BaseHash< css::beans::Property > TPropInfoHash;

    //-------------------------------------------------------------------------
    /* member */
    protected:

        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        PropertySetHelper::TPropInfoHash m_lProps;

        ListenerHash m_lSimpleChangeListener;
        ListenerHash m_lVetoChangeListener;

        sal_Bool m_bReleaseLockOnCall;

    //-------------------------------------------------------------------------
    /* native interface */
    public:

        //---------------------------------------------------------------------
        /** initialize new instance of this helper.
         *
         *  @param  xSMGR
         *          points to an uno service manager, which is used internaly to create own
         *          needed uno services.
         */
        PropertySetHelper(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR             ,
                                ::vos::IMutex*                                          pSolarMutex       ,
                                sal_Bool                                                bReleaseLockOnCall);

        //---------------------------------------------------------------------
        /** free all needed memory.
         */
        virtual ~PropertySetHelper();

        //---------------------------------------------------------------------
        /** add a new property info to the set of supported ones.
         *
         *  @param  aProperty
         *          describes the new property.
         *
         *  @throw  [com::sun::star::beans::PropertyExistException]
         *          if a property with the same name already exists.
         *
         *  Note:   The consistence of the whole set of properties is not checked here.
         *          Means e.g. ... a handle which exists more then once is not detected.
         *          The owner of this class has to be sure, that every new property does
         *          not clash with any existing one.
         */
        virtual void SAL_CALL impl_addPropertyInfo(const css::beans::Property& aProperty)
            throw(css::beans::PropertyExistException,
                  css::uno::Exception               );

        //---------------------------------------------------------------------
        /** remove an existing property info from the set of supported ones.
         *
         *  @param  sProperty
         *          the name of the property.
         *
         *  @throw  [com::sun::star::beans::UnknownPropertyException]
         *          if no property with the specified name exists.
         */
        virtual void SAL_CALL impl_removePropertyInfo(const ::rtl::OUString& sProperty)
            throw(css::beans::UnknownPropertyException,
                  css::uno::Exception                 );

        //---------------------------------------------------------------------
        /** mark the object as "useable for working" or "dead".
         *
         *  This correspond to the lifetime handling implemented by the base class TransactionBase.
         *  There is no chance to reactive a "dead" object by calling impl_enablePropertySet()
         *  again!
         */
        virtual void SAL_CALL impl_enablePropertySet();
        virtual void SAL_CALL impl_disablePropertySet();

        //---------------------------------------------------------------------
        /**
         */
        virtual void SAL_CALL impl_setPropertyValue(const ::rtl::OUString& sProperty,
                                                          sal_Int32        nHandle  ,
                                                    const css::uno::Any&   aValue   ) = 0;

        virtual css::uno::Any SAL_CALL impl_getPropertyValue(const ::rtl::OUString& sProperty,
                                                                   sal_Int32        nHandle  ) = 0;

    //-------------------------------------------------------------------------
    /* uno interface */
    public:

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setPropertyValue(const ::rtl::OUString& sProperty,
                                               const css::uno::Any&   aValue   )
            throw(css::beans::UnknownPropertyException,
                  css::beans::PropertyVetoException   ,
                  css::lang::IllegalArgumentException ,
                  css::lang::WrappedTargetException   ,
                  css::uno::RuntimeException          );

        virtual css::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString& sProperty)
            throw(css::beans::UnknownPropertyException,
                  css::lang::WrappedTargetException   ,
                  css::uno::RuntimeException          );

        virtual void SAL_CALL addPropertyChangeListener(const ::rtl::OUString&                                            sProperty,
                                                        const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
            throw(css::beans::UnknownPropertyException,
                  css::lang::WrappedTargetException   ,
                  css::uno::RuntimeException          );

        virtual void SAL_CALL removePropertyChangeListener(const ::rtl::OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
            throw(css::beans::UnknownPropertyException,
                  css::lang::WrappedTargetException   ,
                  css::uno::RuntimeException          );

        virtual void SAL_CALL addVetoableChangeListener(const ::rtl::OUString&                                            sProperty,
                                                        const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
            throw(css::beans::UnknownPropertyException,
                  css::lang::WrappedTargetException   ,
                  css::uno::RuntimeException          );

        virtual void SAL_CALL removeVetoableChangeListener(const ::rtl::OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
            throw(css::beans::UnknownPropertyException,
                  css::lang::WrappedTargetException   ,
                  css::uno::RuntimeException          );

        // XPropertySetInfo
        virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties()
            throw(css::uno::RuntimeException);

        virtual css::beans::Property SAL_CALL getPropertyByName(const ::rtl::OUString& sName)
            throw(css::beans::UnknownPropertyException,
                  css::uno::RuntimeException          );

        virtual sal_Bool SAL_CALL hasPropertyByName(const ::rtl::OUString& sName)
            throw(css::uno::RuntimeException);

    //-------------------------------------------------------------------------
    /* internal helper */
    private:

        sal_Bool impl_existsVeto(const css::beans::PropertyChangeEvent& aEvent);

        void impl_notifyChangeListener(const css::beans::PropertyChangeEvent& aEvent);
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_PROPERTYSETHELPER_HXX_
