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
#include <com/sun/star/document/XEventsSupplier.hpp>

#include <cppuhelper/implbase.hxx>
#include <svl/listener.hxx>

#include "flyenum.hxx"
#include "frmfmt.hxx"
#include "unotext.hxx"

#include <memory>

class SdrObject;
class SwDoc;
class SwFormat;
class SfxItemPropertySet;
namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }

class BaseFrameProperties_Impl;
class SwXFrame : public cppu::WeakImplHelper
<
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel,
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::drawing::XShape,
    css::container::XNamed,
    css::text::XTextContent
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
    std::unique_ptr<BaseFrameProperties_Impl> m_pProps;
    bool bIsDescriptor;
    OUString                        m_sName;

    std::unique_ptr<SwPaM>          m_pCopySource;

    sal_Int64                       m_nDrawAspect;
    sal_Int64                       m_nVisibleAreaWidth;
    sal_Int64                       m_nVisibleAreaHeight;

protected:
    css::uno::Reference< css::beans::XPropertySet > mxStyleData;
    css::uno::Reference< css::container::XNameAccess >  mxStyleFamily;
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

    virtual ~SwXFrame() override;

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
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;


    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& Name_) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

     //XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    //XShape
    virtual css::awt::Point SAL_CALL getPosition(  ) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) override;

    //XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() override;

    //Base implementation
    //XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference<css::lang::XEventListener>& xListener) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference<css::lang::XEventListener>& xListener) override;

    // XTextContent
    virtual void SAL_CALL attach(const css::uno::Reference<css::text::XTextRange>& xTextRange) override;
    virtual css::uno::Reference<css::text::XTextRange>  SAL_CALL getAnchor() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    void attachToRange(const css::uno::Reference< css::text::XTextRange > & xTextRange);

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

class SwXTextFrame final : public SwXTextFrameBaseClass,
    public SwXText,
    public SwXFrame
{
    friend class SwXFrame; // just for CreateXFrame

    virtual const SwStartNode *GetStartNode() const override;

    virtual css::uno::Reference< css::text::XTextCursor >
        CreateCursor() override;

    virtual ~SwXTextFrame() override;

    SwXTextFrame(SwDoc *pDoc);
    SwXTextFrame(SwFrameFormat& rFormat);

public:
    static SW_DLLPUBLIC css::uno::Reference<css::text::XTextFrame>
            CreateXTextFrame(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    // FIXME: EVIL HACK:  make available for SwXFrame::attachToRange
    using SwXText::SetDoc;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    //XTextFrame
    virtual css::uno::Reference< css::text::XText >  SAL_CALL getText() override;

    //XText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursorByRange(const css::uno::Reference< css::text::XTextRange > & aTextPosition) override;

    //XEnumerationAccess - frueher XParagraphEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XTextContent
    virtual void SAL_CALL attach( const css::uno::Reference< css::text::XTextRange >& xTextRange ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor(  ) override;

    //XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XPropertySet
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
};

typedef cppu::ImplInheritanceHelper
<   SwXFrame,
    css::document::XEventsSupplier
>
SwXTextGraphicObjectBaseClass;
class SwXTextGraphicObject final : public SwXTextGraphicObjectBaseClass
{
    friend class SwXFrame; // just for CreateXFrame

    virtual ~SwXTextGraphicObject() override;

    SwXTextGraphicObject( SwDoc *pDoc );
    SwXTextGraphicObject(SwFrameFormat& rFormat);

public:

    static css::uno::Reference<css::text::XTextContent>
        CreateXTextGraphicObject(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;
};

typedef cppu::ImplInheritanceHelper
<   SwXFrame,
    css::document::XEmbeddedObjectSupplier2,
    css::document::XEventsSupplier
> SwXTextEmbeddedObjectBaseClass;

class SwXTextEmbeddedObject final : public SwXTextEmbeddedObjectBaseClass
{
    css::uno::Reference<css::util::XModifyListener> m_xOLEListener;

    friend class SwXFrame; // just for CreateXFrame

    virtual ~SwXTextEmbeddedObject() override;

    SwXTextEmbeddedObject( SwDoc *pDoc );
    SwXTextEmbeddedObject(SwFrameFormat& rFormat);

public:

    static css::uno::Reference<css::text::XTextContent>
        CreateXTextEmbeddedObject(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    //XEmbeddedObjectSupplier2
    virtual css::uno::Reference< css::lang::XComponent >  SAL_CALL getEmbeddedObject() override;
    virtual css::uno::Reference< css::embed::XEmbeddedObject > SAL_CALL getExtendedControlOverEmbeddedObject() override;
    virtual ::sal_Int64 SAL_CALL getAspect() override;
    virtual void SAL_CALL setAspect( ::sal_Int64 _aspect ) override;
    virtual css::uno::Reference< css::graphic::XGraphic > SAL_CALL getReplacementGraphic() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents(  ) override;
};

class SwXOLEListener : public cppu::WeakImplHelper<css::util::XModifyListener>, public SvtListener
{
    SwFormat* m_pOLEFormat;
    css::uno::Reference<css::frame::XModel> m_xOLEModel;

public:
    SwXOLEListener(SwFormat& rOLEFormat, css::uno::Reference< css::frame::XModel > const & xOLE);
    virtual ~SwXOLEListener() override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

// css::util::XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

    virtual void Notify( const SfxHint& ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
