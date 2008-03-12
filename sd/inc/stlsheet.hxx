/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stlsheet.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:22:55 $
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

#ifndef _SD_STLSHEET_HXX
#define _SD_STLSHEET_HXX

#include <rtl/ref.hxx>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/basemutex.hxx>

#include <svtools/style.hxx>

#include <svx/unoipset.hxx>

#include <boost/scoped_ptr.hpp>

class ModifyListenerForewarder;

typedef cppu::ImplInheritanceHelper5< SfxUnoStyleSheet,
                                    ::com::sun::star::beans::XPropertySet,
                                    ::com::sun::star::lang::XServiceInfo,
                                    ::com::sun::star::beans::XPropertyState,
                                    ::com::sun::star::util::XModifyBroadcaster,
                                    ::com::sun::star::lang::XComponent > SdStyleSheetBase ;

class SdStyleSheet : public SdStyleSheetBase, private ::cppu::BaseMutex
{
public:
    SdStyleSheet( const rtl::OUString& rDisplayName, SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily, USHORT nMask );
    SdStyleSheet( const SdStyleSheet& );

    virtual BOOL        SetParent (const String& rParentName);
    virtual SfxItemSet& GetItemSet();
    virtual BOOL        IsUsed() const;
    virtual BOOL        HasFollowSupport() const;
    virtual BOOL        HasParentSupport() const;
    virtual BOOL        HasClearParentSupport() const;
    virtual BOOL        SetName( const UniString& );
    virtual void        SetHelpId( const String& r, ULONG nId );

    void        AdjustToFontHeight(SfxItemSet& rSet, BOOL bOnlyMissingItems = TRUE);

    SdStyleSheet* GetRealStyleSheet() const;
    SdStyleSheet* GetPseudoStyleSheet() const;

    void SetApiName( const ::rtl::OUString& rApiName );
    rtl::OUString GetApiName() const { return msApiName; }

    static rtl::OUString GetFamilyString( SfxStyleFamily eFamily );

    static SdStyleSheet* CreateEmptyUserStyle( SfxStyleSheetBasePool& rPool, SfxStyleFamily eFamily );

    // XInterface
    virtual void SAL_CALL release(  ) throw ();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XStyle
    virtual sal_Bool SAL_CALL isUserDefined(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isInUse(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getParentStyle(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParentStyle( const ::rtl::OUString& aParentStyle ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    void notifyModifyListener();

protected:
    const SfxItemPropertyMap* getPropertyMapEntry( const ::rtl::OUString& rPropertyName ) const throw();

    virtual void Load (SvStream& rIn, USHORT nVersion);
    virtual void Store(SvStream& rOut);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual             ~SdStyleSheet();

    void throwIfDisposed() throw (::com::sun::star::uno::RuntimeException);

    virtual void disposing();

    rtl::OUString   msApiName;
    rtl::Reference< SfxStyleSheetBasePool > mxPool;

    /** boradcast helper for events */
    ::cppu::OBroadcastHelper mrBHelper;

    boost::scoped_ptr< ModifyListenerForewarder > mpModifyListenerForewarder;

private:
    SdStyleSheet& operator=( const SdStyleSheet& ); // not implemented
};

typedef rtl::Reference< SdStyleSheet > SdStyleSheetRef;
typedef std::vector< SdStyleSheetRef > SdStyleSheetVector;

#endif     // _SD_STLSHEET_HXX



