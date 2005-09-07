/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lngopt.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:53:21 $
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

#ifndef _LINGUISTIC_LNGOPT_HHX_
#define _LINGUISTIC_LNGOPT_HHX_

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase5.hxx> // helper for implementations

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _SOLAR_H //autogen wg. INT16
#include <tools/solar.h>
#endif

#include <vcl/timer.hxx>
#include <svtools/itemprop.hxx>

#ifndef _LINGUISTIC_MISC_HXX_
#include "misc.hxx"
#endif
#ifndef _LINGUISTIC_DEFS_HXX_
#include "defs.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyChangeEvent;
    }
    namespace registry {
        class XRegistryKey;
    }
}}};



///////////////////////////////////////////////////////////////////////////
// LinguOptions
// This class represents all Linguistik relevant options.
//

class LinguOptions
{
    static SvtLinguOptions     *pData;
    static vos::ORefCount       aRefCount;  // number of objects of this class

    //! uses default assignment-operator

    BOOL LinguOptions::SetLocale_Impl( INT16 &rLanguage,
            ::com::sun::star::uno::Any &rOld,
            const ::com::sun::star::uno::Any &rVal);

public:
    LinguOptions();
    LinguOptions(const LinguOptions &rOpt);
    ~LinguOptions();

    BOOL            SetValue( ::com::sun::star::uno::Any &rOld,
                            const ::com::sun::star::uno::Any &rVal, INT32 nWID );
    void            GetValue( ::com::sun::star::uno::Any &rVal, INT32 nWID ) const;
    ::rtl::OUString GetName( INT32 nWID ) const;

    BOOL    IsSpellInAllLanguages() const   { return pData->bIsSpellInAllLanguages; }

    const ::com::sun::star::uno::Sequence< rtl::OUString >
            GetActiveDics() const   { return pData->aActiveDics; }

    const ::com::sun::star::uno::Sequence< rtl::OUString >
            GetActiveConvDics() const   { return pData->aActiveConvDics; }
};


///////////////////////////////////////////////////////////////////////////

// uses templates from <cppuhelper/interfacecontainer.h>


// helper function call class
struct PropHashType_Impl
{
    size_t operator()(const INT32 &s) const { return s; }
};

typedef cppu::OMultiTypeInterfaceContainerHelperVar
    <
        INT32,
        PropHashType_Impl,
        std::equal_to< INT32 >
    > OPropertyListenerContainerHelper;

///////////////////////////////////////////////////////////////////////////


class LinguProps :
    public cppu::WeakImplHelper5
    <
        com::sun::star::beans::XPropertySet,
        com::sun::star::beans::XFastPropertySet,
        com::sun::star::beans::XPropertyAccess,
        com::sun::star::lang::XComponent,
        com::sun::star::lang::XServiceInfo
    >
{
    ::cppu::OInterfaceContainerHelper           aEvtListeners;
    OPropertyListenerContainerHelper            aPropListeners;

    SfxItemPropertyMap                         *pMap;
    LinguOptions                                aOpt;

    BOOL                                        bDisposing;

    // disallow copy-constructor and assignment-operator for now
    LinguProps(const LinguProps &);
    LinguProps & operator = (const LinguProps &);

    void    launchEvent( const ::com::sun::star::beans::PropertyChangeEvent &rEvt ) const;

public:
    LinguProps();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setPropertyValue( const ::rtl::OUString& aPropertyName,
                const ::com::sun::star::uno::Any& aValue )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::beans::PropertyVetoException,
                  ::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL
        getPropertyValue( const ::rtl::OUString& PropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertyChangeListener >& rxListener )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertyChangeListener >& rxListener )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XVetoableChangeListener >& rxListener )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XVetoableChangeListener >& rxListener )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    // XFastPropertySet
    virtual void SAL_CALL
        setFastPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::beans::PropertyVetoException,
                  ::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL
        getFastPropertyValue( sal_Int32 nHandle )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue > SAL_CALL
        getPropertyValues()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setPropertyValues( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue >& aProps )
            throw(::com::sun::star::beans::UnknownPropertyException,
                  ::com::sun::star::beans::PropertyVetoException,
                  ::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::lang::WrappedTargetException,
                  ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL
        dispose()
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException);


    ////////////////////////////////////////////////////////////
    // Service specific part
    //

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw(::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();
};

inline ::rtl::OUString LinguProps::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.LinguProps" );
}

///////////////////////////////////////////////////////////////////////////

#endif

