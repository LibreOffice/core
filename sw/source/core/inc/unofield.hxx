/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SW_UNOFIELD_HXX
#define SW_UNOFIELD_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>

#include <tools/string.hxx>

#include <calbck.hxx>
#include <unobaseclass.hxx>


class SwFieldType;
class SwDoc;
class SwFmtFld;
class SwField;
class SwSetExpField;
class SwTextAPIObject;

class SwXFieldMaster : public cppu::WeakImplHelper4
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::lang::XComponent
>,
    public SwClient
{

private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    sal_uInt16                  nResTypeId;
    SwDoc*                      m_pDoc;

    sal_Bool                    m_bIsDescriptor;

    String                      sParam1;    //Content / Database / NumberingSeparator
    String                      sParam2;    // -    /DataTablename
    String                      sParam3;    // -    /DataFieldName
    String                      sParam4;
    String                      sParam5;    // -    /DataBaseURL
    String                      sParam6;    // -    /DataBaseResource
    double                      fParam1;    // Value / -
    sal_Int8                    nParam1;    // ChapterNumberingLevel
    sal_Bool                    bParam1;    // IsExpression
    sal_Int32                   nParam2;

protected:
    virtual ~SwXFieldMaster();
    //SwClient
virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:

    SwXFieldMaster(SwDoc* pDoc, sal_uInt16 nResId);
    SwXFieldMaster(SwFieldType& rType, SwDoc* pDoc);


    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    static OUString GetProgrammaticName(const SwFieldType& rType, SwDoc& rDoc);
    static OUString LocalizeFormula(const SwSetExpField& rFld, const OUString& rFormula, sal_Bool bQuery);

    SwFieldType* GetFldType(sal_Bool bDontCreate = sal_False) const;
    SwDoc*      GetDoc() {return m_pDoc;}

    sal_Bool        IsDescriptor()const  {return m_bIsDescriptor;}
};

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

private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    const SwFmtFld*             pFmtFld;
    SwDoc*                      m_pDoc;
    SwTextAPIObject*            m_pTextObject;

    sal_Bool                        m_bIsDescriptor;
    SwClient                        m_aFieldTypeClient; // required to access field master of not yet inserted fields
    sal_Bool                        m_bCallUpdate;
    sal_uInt16                      m_nServiceId;
    SwFieldProperties_Impl*     m_pProps;
    String                      m_sTypeName;


    //SwFieldType* GetFldType() const { return (SwFieldType*)GetRegisteredIn(); }
    SwDoc*       GetDoc() {return m_pDoc;}
protected:
    virtual ~SwXTextField();
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwXTextField(sal_uInt16 nServiceId, SwDoc* pDoc=0);
    SwXTextField(const SwFmtFld& rFmt, SwDoc* pDoc);


    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XDependentTextField
    virtual void SAL_CALL attachTextFieldMaster(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xFieldMaster) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getTextFieldMaster(void) throw( ::com::sun::star::uno::RuntimeException );

    //XTextField
    virtual OUString SAL_CALL getPresentation(sal_Bool bShowCommand) throw( ::com::sun::star::uno::RuntimeException );

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XUpdatable
    virtual void SAL_CALL update(  ) throw (::com::sun::star::uno::RuntimeException);

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    const SwField*      GetField() const;
    const SwFmtFld*     GetFldFmt(){return GetField() ? pFmtFld : 0; };

    void Invalidate();

    /// @return an SwXTextField, either an already existing one or a new one
    static SwXTextField* CreateSwXTextField(SwDoc & rDoc, SwFmtFld const& rFmt);
    sal_uInt16 GetServiceId();
};

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
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
public:
    SwXFieldEnumeration(SwDoc* pDoc);

    //XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(void) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
