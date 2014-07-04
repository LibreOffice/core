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
#ifndef INCLUDED_SW_INC_UNOFRAME_HXX
#define INCLUDED_SW_INC_UNOFRAME_HXX

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase6.hxx>

#include <sfx2/objsh.hxx>

#include <flyenum.hxx>
#include <frmfmt.hxx>
#include <unotext.hxx>

class SdrObject;
class SwDoc;
class SwFmt;
class SwFlyFrmFmt;

class BaseFrameProperties_Impl;
class SwXFrame : public cppu::WeakImplHelper6
<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::drawing::XShape,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::lang::XUnoTunnel
>,
    public SwClient
{
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    const SfxItemPropertySet*       m_pPropSet;
    SwDoc*                          m_pDoc;

    const FlyCntType                eType;

    // Descriptor-interface
    BaseFrameProperties_Impl*       pProps;
    bool bIsDescriptor;
    OUString                        m_sName;

    SwPaM*                          m_pCopySource;

protected:
    com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > mxStyleData;
    com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  mxStyleFamily;
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;

    virtual ~SwXFrame();
public:
    SwXFrame(FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet,
                SwDoc *pDoc ); //Descriptor-If
    SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet);

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    TYPEINFO_OVERRIDE();

    //XNamed
    virtual OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL setName(const OUString& Name_) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

     //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

   //XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XShapeDescriptor
    virtual OUString SAL_CALL getShapeType(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //Base implementation
    //XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    void attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    SwFrmFmt*       GetFrmFmt() const
    {
        return PTR_CAST ( SwFrmFmt, GetRegisteredIn() );
    }
    FlyCntType      GetFlyCntType()const {return eType;}

    bool IsDescriptor() const {return bIsDescriptor;}
    void            ResetDescriptor();
    //copy text from a given source PaM
    void            SetSelection(SwPaM& rCopySource);
    static SW_DLLPUBLIC SdrObject *GetOrCreateSdrObject( SwFlyFrmFmt *pFmt );
};

typedef cppu::WeakImplHelper3
<
    ::com::sun::star::text::XTextFrame,
    ::com::sun::star::container::XEnumerationAccess,
    ::com::sun::star::document::XEventsSupplier
>
SwXTextFrameBaseClass;

class SwXTextFrame : public SwXTextFrameBaseClass,
    public SwXText,
    public SwXFrame
{
protected:
    virtual const SwStartNode *GetStartNode() const SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >
        CreateCursor()
        throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    virtual ~SwXTextFrame();
public:
    SwXTextFrame(SwDoc *pDoc);
    SwXTextFrame(SwFrmFmt& rFmt);

    // FIXME: EVIL HACK:  make available for SwXFrame::attachToRange
    void SetDoc(SwDoc *const pDoc) { SwXText::SetDoc(pDoc); };

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw() SAL_OVERRIDE;

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XTextFrame
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XTextContent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XPropertySet
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();
};

typedef cppu::WeakImplHelper2
<
    ::com::sun::star::text::XTextContent,
    ::com::sun::star::document::XEventsSupplier
>
SwXTextGraphicObjectBaseClass;
class SwXTextGraphicObject : public SwXTextGraphicObjectBaseClass,
                            public SwXFrame
{
protected:
    virtual ~SwXTextGraphicObject();
public:
    SwXTextGraphicObject( SwDoc *pDoc );
    SwXTextGraphicObject(SwFrmFmt& rFmt);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw() SAL_OVERRIDE;

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();
};

class SwOLENode;
typedef cppu::WeakImplHelper3
<
    ::com::sun::star::text::XTextContent,
    ::com::sun::star::document::XEmbeddedObjectSupplier2,
    ::com::sun::star::document::XEventsSupplier
>SwXTextEmbeddedObjectBaseClass;

class SwXTextEmbeddedObject : public SwXTextEmbeddedObjectBaseClass,
                                public SwXFrame
{
protected:
    virtual ~SwXTextEmbeddedObject();

public:
    SwXTextEmbeddedObject( SwDoc *pDoc );
    SwXTextEmbeddedObject(SwFrmFmt& rFmt);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw() SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw() SAL_OVERRIDE;

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    //XEmbeddedObjectSupplier2
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  SAL_CALL getEmbeddedObject(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > SAL_CALL getExtendedControlOverEmbeddedObject(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::sal_Int64 SAL_CALL getAspect() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setAspect( ::sal_Int64 _aspect ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getReplacementGraphic() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();
};

class SwXOLEListener : public cppu::WeakImplHelper1
<
    ::com::sun::star::util::XModifyListener
>,
    public SwClient
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xOLEModel;

    SwFmt*       GetFmt() const    {  return (SwFmt*)GetRegisteredIn(); }
public:
    SwXOLEListener(SwFmt& rOLEFmt, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xOLE);
    virtual ~SwXOLEListener();
    TYPEINFO_OVERRIDE();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
