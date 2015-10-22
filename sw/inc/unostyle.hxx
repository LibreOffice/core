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
#ifndef INCLUDED_SW_INC_UNOSTYLE_HXX
#define INCLUDED_SW_INC_UNOSTYLE_HXX

#include <svl/style.hxx>
#include <svl/lstner.hxx>
#include <unocoll.hxx>
#include <unomap.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <calbck.hxx>

#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/style/XAutoStyles.hpp>
#include <com/sun/star/style/XAutoStyle.hpp>

#include <istyleaccess.hxx>
#include <memory>

class SwDocShell;
class SfxItemPropertySet;
class SwXStyle;
class SwStyleProperties_Impl;
class SwAutoStylesEnumImpl;
class IStyleAccess;
class SfxItemSet;
typedef std::shared_ptr<SfxItemSet> SfxItemSet_Pointer_t;

class SwXStyleFamilies :  public cppu::WeakImplHelper
<
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::container::XNameAccess,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::style::XStyleLoader
>,
    public SwUnoCollection
{
    SwDocShell*         m_pDocShell;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   m_pxCharStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   m_pxParaStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   m_pxFrameStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   m_pxPageStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > *   m_pxNumberingStyles;
protected:
    virtual ~SwXStyleFamilies();
public:
    SwXStyleFamilies(SwDocShell& rDocShell);

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XStyleLoader
    virtual void SAL_CALL loadStylesFromURL(const OUString& rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aOptions)
        throw (::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getStyleLoaderOptions() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

class SwXStyleFamily : public cppu::WeakImplHelper
<
    ::com::sun::star::container::XNameContainer,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::container::XIndexAccess,
    ::com::sun::star::beans::XPropertySet
>,
    public SfxListener
{
    SfxStyleFamily              m_eFamily;
    SfxStyleSheetBasePool*      m_pBasePool;
    SwDocShell*                 m_pDocShell;

    SwXStyle*               _FindStyle(const OUString& rStyleName) const;
public:
    SwXStyleFamily(SwDocShell* pDocShell, sal_uInt16 nFamily);
    virtual ~SwXStyleFamily();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XNameContainer
    virtual void SAL_CALL insertByName(const OUString& Name, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL replaceByName(const OUString& Name, const ::com::sun::star::uno::Any& Element) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeByName(const OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

class SwXStyle : public cppu::WeakImplHelper
<
    ::com::sun::star::style::XStyle,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XMultiPropertySet,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::beans::XMultiPropertyStates
>,
        public SfxListener,
        public SwClient
{
    friend class SwXStyleFamily;
    SwDoc*                  m_pDoc;
    OUString                m_sStyleName;
    SfxStyleSheetBasePool*  m_pBasePool;
    SfxStyleFamily          m_eFamily;    // for Notify

    bool                    m_bIsDescriptor  : 1;
    bool                    m_bIsConditional : 1;
    OUString                m_sParentStyleName;
    SwStyleProperties_Impl* m_pPropertiesImpl;

    void    ApplyDescriptorProperties();
protected:
    void    Invalidate();

    const SfxStyleSheetBasePool*    GetBasePool() const {return m_pBasePool;}
    SfxStyleSheetBasePool*  GetBasePool() {return m_pBasePool;}

    void SetStyleName(const OUString& rSet){ m_sStyleName = rSet;}
    SwStyleProperties_Impl* GetPropImpl(){return m_pPropertiesImpl;}
    com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > mxStyleData;
    com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  mxStyleFamily;

    void SAL_CALL SetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL GetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
public:
    SwXStyle(SwDoc* pDoc, SfxStyleFamily eFam = SFX_STYLE_FAMILY_PARA, bool bConditional = false);
    SwXStyle(SfxStyleSheetBasePool& rPool, SfxStyleFamily eFam,
                                SwDoc*  pDoc,
                                const OUString& rStyleName);

    virtual ~SwXStyle();

    TYPEINFO_OVERRIDE();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XNamed
    virtual OUString SAL_CALL getName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setName(const OUString& Name_) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XStyle
    virtual sal_Bool SAL_CALL isUserDefined() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL isInUse() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getParentStyle() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setParentStyle(const OUString& aParentStyle) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    OUString            GetStyleName() const { return m_sStyleName;}
    SfxStyleFamily      GetFamily() const {return m_eFamily;}

    bool                IsDescriptor() const {return m_bIsDescriptor;}
    bool                IsConditional() const { return m_bIsConditional;}
    OUString            GetParentStyleName() const { return m_sParentStyleName;}
    void                SetDoc(SwDoc* pDc, SfxStyleSheetBasePool*   pPool)
                            {
                                m_bIsDescriptor = false; m_pDoc = pDc;
                                m_pBasePool = pPool;
                                StartListening(*m_pBasePool);
                            }
    SwDoc*                GetDoc() const { return m_pDoc; }
};

class SwXFrameStyle : public SwXStyle,
                        public com::sun::star::document::XEventsSupplier
{
public:
    SwXFrameStyle(SfxStyleSheetBasePool& rPool,
                                SwDoc*  pDoc,
                                const OUString& rStyleName) :
        SwXStyle(rPool, SFX_STYLE_FAMILY_FRAME, pDoc, rStyleName){}
    SwXFrameStyle( SwDoc *pDoc );
    virtual ~SwXFrameStyle();

    virtual void SAL_CALL acquire(  ) throw() override {SwXStyle::acquire();}
    virtual void SAL_CALL release(  ) throw() override {SwXStyle::release();}

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    friend class SwFrameStyleEventDescriptor;
};

class SwXPageStyle : public SwXStyle
{
protected:
    void SAL_CALL SetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL GetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

public:
    SwXPageStyle(SfxStyleSheetBasePool& rPool, SwDocShell* pDocSh, SfxStyleFamily eFam,
                                const OUString& rStyleName);
    SwXPageStyle(SwDocShell* pDocSh);
    virtual ~SwXPageStyle();

    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

};

// access to all automatic style families
class SwXAutoStyles :
    public cppu::WeakImplHelper< ::com::sun::star::style::XAutoStyles >,
    public SwUnoCollection
{
    SwDocShell *m_pDocShell;
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XAutoStyleFamily > m_xAutoCharStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XAutoStyleFamily > m_xAutoRubyStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XAutoStyleFamily > m_xAutoParaStyles;
    ::com::sun::star::uno::Reference< ::com::sun::star::style::XAutoStyleFamily > m_xAutoNotxtStyles;
    virtual ~SwXAutoStyles();

public:
    SwXAutoStyles(SwDocShell& rDocShell);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(const OUString& Name) throw( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL hasByName(const OUString& Name) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

};

// access to a family of automatic styles (character or paragraph or ...)
class SwXAutoStyleFamily : public cppu::WeakImplHelper< com::sun::star::style::XAutoStyleFamily >,
    public SwClient
{
    SwDocShell *m_pDocShell;
    IStyleAccess::SwAutoStyleFamily m_eFamily;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwXAutoStyleFamily(SwDocShell* pDocShell, IStyleAccess::SwAutoStyleFamily eFamily);
    virtual ~SwXAutoStyleFamily();

    //XAutoStyleFamily
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::style::XAutoStyle > SAL_CALL insertStyle( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Values ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

};

class SwXAutoStylesEnumerator : public cppu::WeakImplHelper< ::com::sun::star::container::XEnumeration >,
    public SwClient
{
    SwAutoStylesEnumImpl *m_pImpl;
public:
    SwXAutoStylesEnumerator( SwDoc* pDoc, IStyleAccess::SwAutoStyleFamily eFam );
    virtual ~SwXAutoStylesEnumerator();

    //XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
};

// an automatic style
class SwXAutoStyle : public cppu::WeakImplHelper
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::style::XAutoStyle
>,
    public SwClient
{
private:
    SfxItemSet_Pointer_t                mpSet;
    IStyleAccess::SwAutoStyleFamily     meFamily;
    SwDoc&                              mrDoc;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL GetPropertyValues_Impl( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:

    SwXAutoStyle( SwDoc* pDoc, SfxItemSet_Pointer_t pInitSet, IStyleAccess::SwAutoStyleFamily eFam );
    virtual ~SwXAutoStyle();

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // Special
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
