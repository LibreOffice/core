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

#include <cppuhelper/implbase.hxx>

#include <sfx2/objsh.hxx>

#include <flyenum.hxx>
#include <frmfmt.hxx>
#include <unotext.hxx>

class SdrObject;
class SwDoc;
class SwFormat;
class SwFlyFrameFormat;

class BaseFrameProperties_Impl;
class SwXFrame : public cppu::WeakImplHelper
<
    css::lang::XServiceInfo,
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::drawing::XShape,
    css::container::XNamed,
    css::lang::XUnoTunnel
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
    css::uno::Reference< css::beans::XPropertySet > mxStyleData;
    css::uno::Reference< css::container::XNameAccess >  mxStyleFamily;
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

    virtual ~SwXFrame();

    SwXFrame(FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet,
                SwDoc *pDoc ); //Descriptor-If
    SwXFrame(SwFrameFormat& rFrameFormat, FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet);

    template<class Interface, class Impl>
    static css::uno::Reference<Interface>
    CreateXFrame(SwDoc & rDoc, SwFrameFormat *const pFrameFormat);

public:
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;


    //XNamed
    virtual OUString SAL_CALL getName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setName(const OUString& Name_) throw( css::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

     //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

   //XShape
    virtual css::awt::Point SAL_CALL getPosition(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    //XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw( css::uno::RuntimeException, std::exception ) override;

    //Base implementation
    //XComponent
    virtual void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception);

    virtual css::uno::Reference< css::text::XTextRange >  SAL_CALL getAnchor() throw( css::uno::RuntimeException, std::exception );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    void attachToRange(const css::uno::Reference< css::text::XTextRange > & xTextRange)throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);
    void attach( const css::uno::Reference< css::text::XTextRange >& xTextRange ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);

    const SwFrameFormat* GetFrameFormat() const
    {
        return dynamic_cast<const SwFrameFormat*>( GetRegisteredIn()  );
    }
    SwFrameFormat* GetFrameFormat()
    {
        return dynamic_cast< SwFrameFormat*>( GetRegisteredIn() );
    }
    FlyCntType      GetFlyCntType()const {return eType;}

    bool IsDescriptor() const {return bIsDescriptor;}
    void            ResetDescriptor();
    //copy text from a given source PaM
    void            SetSelection(SwPaM& rCopySource);
    static SW_DLLPUBLIC SdrObject *GetOrCreateSdrObject(SwFlyFrameFormat &rFormat);
};

typedef cppu::WeakImplHelper
<
    css::text::XTextFrame,
    css::container::XEnumerationAccess,
    css::document::XEventsSupplier
>
SwXTextFrameBaseClass;

class SwXTextFrame : public SwXTextFrameBaseClass,
    public SwXText,
    public SwXFrame
{
protected:
    friend class SwXFrame; // just for CreateXFrame

    virtual const SwStartNode *GetStartNode() const override;

    virtual css::uno::Reference< css::text::XTextCursor >
        CreateCursor()
        throw (css::uno::RuntimeException) override;

    virtual ~SwXTextFrame();

    SwXTextFrame(SwDoc *pDoc);
    SwXTextFrame(SwFrameFormat& rFormat);

public:
    static SW_DLLPUBLIC css::uno::Reference<css::text::XTextFrame>
            CreateXTextFrame(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    // FIXME: EVIL HACK:  make available for SwXFrame::attachToRange
    void SetDoc(SwDoc *const pDoc) { SwXText::SetDoc(pDoc); };

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XTextFrame
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() throw( css::uno::RuntimeException, std::exception ) override;

    //XText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursor() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursorByRange(const css::uno::Reference< css::text::XTextRange > & aTextPosition) throw( css::uno::RuntimeException, std::exception ) override;

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() throw( css::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XTextContent
    virtual void SAL_CALL attach( const css::uno::Reference< css::text::XTextRange >& xTextRange ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XComponent
    virtual void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    //XPropertySet
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();
};

typedef cppu::WeakImplHelper
<
    css::text::XTextContent,
    css::document::XEventsSupplier
>
SwXTextGraphicObjectBaseClass;
class SwXTextGraphicObject : public SwXTextGraphicObjectBaseClass,
                            public SwXFrame
{
protected:
    friend class SwXFrame; // just for CreateXFrame

    virtual ~SwXTextGraphicObject();

    SwXTextGraphicObject( SwDoc *pDoc );
    SwXTextGraphicObject(SwFrameFormat& rFormat);

public:

    static css::uno::Reference<css::text::XTextContent>
        CreateXTextGraphicObject(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XTextContent
    virtual void SAL_CALL attach(const css::uno::Reference< css::text::XTextRange > & xTextRange) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange >  SAL_CALL getAnchor() throw( css::uno::RuntimeException, std::exception ) override;

    //XComponent
    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) throw(css::uno::RuntimeException, std::exception) override;
    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();
};

class SwOLENode;
typedef cppu::WeakImplHelper
<
    css::text::XTextContent,
    css::document::XEmbeddedObjectSupplier2,
    css::document::XEventsSupplier
>SwXTextEmbeddedObjectBaseClass;

class SwXTextEmbeddedObject : public SwXTextEmbeddedObjectBaseClass,
                                public SwXFrame
{
    css::uno::Reference<css::util::XModifyListener> m_xOLEListener;
protected:
    friend class SwXFrame; // just for CreateXFrame

    virtual ~SwXTextEmbeddedObject();

    SwXTextEmbeddedObject( SwDoc *pDoc );
    SwXTextEmbeddedObject(SwFrameFormat& rFormat);

public:

    static css::uno::Reference<css::text::XTextContent>
        CreateXTextEmbeddedObject(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XTextContent
    virtual void SAL_CALL attach(const css::uno::Reference< css::text::XTextRange > & xTextRange) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange >  SAL_CALL getAnchor() throw( css::uno::RuntimeException, std::exception ) override;

    //XComponent
    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;

    //XEmbeddedObjectSupplier2
    virtual css::uno::Reference< css::lang::XComponent >  SAL_CALL getEmbeddedObject() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::embed::XEmbeddedObject > SAL_CALL getExtendedControlOverEmbeddedObject() throw( css::uno::RuntimeException, std::exception ) override;
    virtual ::sal_Int64 SAL_CALL getAspect() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAspect( ::sal_Int64 _aspect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getReplacementGraphic() throw (css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) throw(css::uno::RuntimeException, std::exception) override;
    void * SAL_CALL operator new( size_t ) throw();
    void SAL_CALL operator delete( void * ) throw();
};

class SwXOLEListener : public cppu::WeakImplHelper
<
    css::util::XModifyListener
>,
    public SwClient
{
    css::uno::Reference< css::frame::XModel > xOLEModel;

    SwFormat*       GetFormat() const    {  return const_cast<SwFormat*>(static_cast<const SwFormat*>(GetRegisteredIn())); }
public:
    SwXOLEListener(SwFormat& rOLEFormat, css::uno::Reference< css::frame::XModel > xOLE);
    virtual ~SwXOLEListener();

// css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

// css::util::XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException, std::exception) override;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
