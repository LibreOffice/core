/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _UNOFRAME_HXX
#define _UNOFRAME_HXX

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
#include <unoevtlstnr.hxx>
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
    SwEventListenerContainer        aLstnrCntnr;
    const SfxItemPropertySet*       m_pPropSet;
    SwDoc*                          m_pDoc;

    const FlyCntType                eType;

    // Descriptor-interface
    BaseFrameProperties_Impl*       pProps;
    bool bIsDescriptor;
    String                          sName;

    SwPaM*                          m_pCopySource;

protected:
    com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > mxStyleData;
    com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  mxStyleFamily;
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

    virtual ~SwXFrame();
public:
    SwXFrame(FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet,
                SwDoc *pDoc ); //Descriptor-If
    SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet);


    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    TYPEINFO();

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

     //XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

   //XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    //XShapeDescriptor
    virtual rtl::OUString SAL_CALL getShapeType(void) throw( ::com::sun::star::uno::RuntimeException );

    //Base implementation
    //XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

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
    virtual const SwStartNode *GetStartNode() const;

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >
        CreateCursor()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ~SwXTextFrame();
public:
    SwXTextFrame(SwDoc *pDoc);
    SwXTextFrame(SwFrmFmt& rFmt);

    // FIXME: EVIL HACK:  make available for SwXFrame::attachToRange
    void SetDoc(SwDoc *const pDoc) { SwXText::SetDoc(pDoc); };

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTextFrame
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >  SAL_CALL getText(void) throw( ::com::sun::star::uno::RuntimeException );

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

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


    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);
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


    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >  SAL_CALL getAnchor(void) throw( ::com::sun::star::uno::RuntimeException );

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XEmbeddedObjectSupplier2
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  SAL_CALL getEmbeddedObject(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > SAL_CALL getExtendedControlOverEmbeddedObject(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::sal_Int64 SAL_CALL getAspect() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAspect( ::sal_Int64 _aspect ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getReplacementGraphic() throw (::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);
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
    ~SwXOLEListener();
    TYPEINFO();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
