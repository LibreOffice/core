/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _UNOFIELD_HXX
#define _UNOFIELD_HXX

#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XUPDATABLE_HPP_
#include <com/sun/star/util/XUpdatable.hpp>
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif
#ifndef _REFRESH_LISTENER_CONTAINER_HXX_
#include <RefreshListenerContainer.hxx>
#endif
class String; 
namespace binfilter {

class SwFieldType;
class SwDoc;
class SwFmtFld;
class SwField;
class SwSetExpField;

/* -----------------04.12.98 12:49-------------------
 *
 * --------------------------------------------------*/
class SwXFieldMaster : public cppu::WeakImplHelper4
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::lang::XComponent
>,
    public SwClient
{
    SwEventListenerContainer	aLstnrCntnr;
    sal_uInt16                  nResTypeId;
    SwDoc* 						m_pDoc;

    sal_Bool                    m_bIsDescriptor;

    String 						sParam1;	//Content / Database / NumberingSeparator
    String 						sParam2;    // -	/DataTablename
    String 						sParam3;    // -	/DataFieldName
    double						fParam1;	// Value / -
    sal_Int8					nParam1;	// ChapterNumberingLevel
    sal_Bool					bParam1;    // IsExpression
    sal_Int32                   nParam2;

protected:
    virtual ~SwXFieldMaster();
public:

    SwXFieldMaster(SwDoc* pDoc, sal_uInt16 nResId);
    SwXFieldMaster(SwFieldType& rType, SwDoc* pDoc);
    

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    static ::rtl::OUString GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc);
    static ::rtl::OUString LocalizeFormula(const SwSetExpField& rFld, const ::rtl::OUString& rFormula, sal_Bool bQuery);

    SwFieldType* GetFldType(sal_Bool bDontCreate = sal_False) const;
    SwDoc*		GetDoc() {return m_pDoc;}

    sal_Bool		IsDescriptor()const  {return m_bIsDescriptor;}
    sal_uInt16 		GetResId() const {return nResTypeId;}
};

/* -----------------04.12.98 12:49-------------------
 *
 * --------------------------------------------------*/
struct SwFieldProperties_Impl;
class SwXTextField : public cppu::WeakImplHelper5
<
    ::com::sun::star::text::XDependentTextField,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::util::XUpdatable
>,
            public SwClient
{
    SwEventListenerContainer	aLstnrCntnr;
    const SwFmtFld* 			pFmtFld;
    SwDoc* 						m_pDoc;

    sal_Bool 						m_bIsDescriptor;
    sal_Bool                        m_bCallUpdate;
    sal_uInt16                      m_nServiceId;
    SwFieldProperties_Impl* 	m_pProps;
    String 						m_sTypeName;


    //SwFieldType* GetFldType() const { return (SwFieldType*)GetRegisteredIn(); }
    SwDoc* 		 GetDoc() {return m_pDoc;}
protected:
    virtual ~SwXTextField();
public:
    SwXTextField(sal_uInt16 nServiceId);
    SwXTextField(const SwFmtFld& rFmt, SwDoc* pDoc);
    

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XDependentTextField
    virtual void SAL_CALL attachTextFieldMaster(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xFieldMaster) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getTextFieldMaster(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextField
    virtual ::rtl::OUString SAL_CALL getPresentation(sal_Bool bShowCommand) throw( ::com::sun::star::uno::RuntimeException );

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XUpdatable
    virtual void SAL_CALL update(  ) throw (::com::sun::star::uno::RuntimeException);

    //SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    const SwField*  	GetField() const;
    const SwFmtFld* 	GetFldFmt(){return GetField() ? pFmtFld : 0; };
    void Invalidate();
};
/* -----------------21.12.98 10:26-------------------
 *
 * --------------------------------------------------*/
typedef
cppu::WeakImplHelper2
<
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::lang::XServiceInfo
>
SwXTextFieldMastersBaseClass;
class SwXTextFieldMasters : public SwXTextFieldMastersBaseClass,
    public SwUnoCollection
{
protected:
    virtual ~SwXTextFieldMasters();
public:
    SwXTextFieldMasters(SwDoc* pDoc);
    

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const ::rtl::OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& Name) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );


    static sal_Bool getInstanceName(const SwFieldType& rFldType, String& rName);
};

/* -----------------21.12.98 10:26-------------------
 *
 * --------------------------------------------------*/
typedef
cppu::WeakImplHelper3
<
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::util::XRefreshable
>
SwXTextFieldTypesBaseClass;
class SwXTextFieldTypes : public SwXTextFieldTypesBaseClass,
        public SwUnoCollection
{
    SwRefreshListenerContainer      aRefreshCont;
protected:
    virtual ~SwXTextFieldTypes();
public:
    SwXTextFieldTypes(SwDoc* pDoc);
    

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XRefreshable
    virtual void SAL_CALL refresh(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XRefreshListener >& l ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // SwUnoCollection
    virtual void    Invalidate();
};

/* -----------------21.12.98 14:49-------------------
 *
 * --------------------------------------------------*/
class SwXFieldEnumeration : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XEnumeration,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextField > > aItems;
    sal_Int32       nNextIndex;     // index of next element to be returned

    SwDoc* pDoc;

protected:
    virtual ~SwXFieldEnumeration();
public:
    SwXFieldEnumeration(SwDoc* pDoc);

    //XEnumeration
    virtual BOOL SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
    virtual void 		Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
};
} //namespace binfilter
#endif



