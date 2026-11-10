/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "swdllapi.h"
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/style/XStyleFamily.hpp>

#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/implbase.hxx>
#include <sal/types.h>
#include <svl/listener.hxx>

#include "flyenum.hxx"
#include "frmfmt.hxx"
#include "unotext.hxx"

#include <memory>
#include <mutex>

class SdrObject;
class SwDoc;
class SwFormat;
class SwUnoInternalPaM;
class SfxItemPropertySet;
class SwXOLEListener;
struct SwXParagraphEnumeration;
namespace com::sun::star::frame { class XModel; }

class BaseFrameProperties_Impl;
class SAL_DLLPUBLIC_RTTI SAL_LOPLUGIN_ANNOTATE("crosscast") SwXFrame : public cppu::WeakImplHelper
<
    css::lang::XServiceInfo,
    css::beans::XPropertySet,
    css::beans::XPropertyState,
    css::drawing::XShape,
    css::container::XNamed,
    css::text::XTextContent
>,
    public SvtListener
{
private:
    std::mutex m_Mutex; // just for OInterfaceContainerHelper4
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_EventListeners;
    SwFrameFormat* m_pFrameFormat;

    const SfxItemPropertySet*       m_pPropSet;
    SwDoc*                          m_pDoc;

    const FlyCntType                m_eType;

    // Descriptor-interface
    std::unique_ptr<BaseFrameProperties_Impl> m_pProps;
    bool m_bIsDescriptor;
    UIName                          m_sName;

    sal_Int64                       m_nDrawAspect;
    sal_Int64                       m_nVisibleAreaWidth;
    sal_Int64                       m_nVisibleAreaHeight;
    cpo::uno::Reference<SwXText> m_xParentText;
    cpo::uno::Reference< css::beans::XPropertySet > mxStyleData;
    cpo::uno::Reference< css::style::XStyleFamily >  mxStyleFamily;

    void DisposeInternal();

protected:
    virtual void Notify(const SfxHint&) override;

    virtual ~SwXFrame() override;

    SwXFrame(FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet,
                SwDoc *pDoc ); //Descriptor-If
    SwXFrame(SwFrameFormat& rFrameFormat, FlyCntType eSet,
                const SfxItemPropertySet*    pPropSet);

    template<class Impl>
    static rtl::Reference<Impl>
    CreateXFrame(SwDoc & rDoc, SwFrameFormat *const pFrameFormat);

public:

    //XNamed
    SW_DLLPUBLIC virtual OUString getName() override;
    SW_DLLPUBLIC virtual void setName(const OUString& Name_) override;

    //XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
    SW_DLLPUBLIC virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

     //XPropertyState
    virtual css::beans::PropertyState getPropertyState( const OUString& PropertyName ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyState > getPropertyStates( const cpo::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void setPropertyToDefault( const OUString& PropertyName ) override;
    virtual cpo::uno::Any getPropertyDefault( const OUString& aPropertyName ) override;

    //XShape
    virtual css::awt::Point getPosition(  ) override;
    virtual void setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size getSize(  ) override;
    virtual void setSize( const css::awt::Size& aSize ) override;

    //XShapeDescriptor
    virtual OUString getShapeType() override;

    //Base implementation
    //XComponent
    virtual void dispose() override;
    virtual void addEventListener(const cpo::uno::Reference<css::lang::XEventListener>& xListener) override;
    virtual void removeEventListener(const cpo::uno::Reference<css::lang::XEventListener>& xListener) override;

    // XTextContent
    virtual void attach(const cpo::uno::Reference<css::text::XTextRange>& xTextRange) override;
    virtual cpo::uno::Reference<css::text::XTextRange>  getAnchor() override;

    //XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    void attachToRange(cpo::uno::Reference<css::text::XTextRange> const& xTextRange,
            SwPaM const* pCopySource = nullptr);

    SW_DLLPUBLIC rtl::Reference< SwXTextRange > getSwAnchor();

    const SwFrameFormat* GetFrameFormat() const
        { return m_pFrameFormat; }
    SwFrameFormat* GetFrameFormat()
        { return m_pFrameFormat; }

    FlyCntType      GetFlyCntType()const {return m_eType;}

    bool IsDescriptor() const {return m_bIsDescriptor;}
    void            ResetDescriptor();
    static SdrObject *GetOrCreateSdrObject(SwFlyFrameFormat &rFormat);
};

typedef cppu::ImplInheritanceHelper
<
    SwXFrame,
    css::text::XTextFrame,
    css::container::XEnumerationAccess,
    css::document::XEventsSupplier
>
SwXTextFrameBaseClass;

class SAL_DLLPUBLIC_RTTI SwXTextFrame final : public SwXTextFrameBaseClass,
    public SwXText
{
    friend class SwXFrame; // just for CreateXFrame

    virtual const SwStartNode *GetStartNode() const override;

    virtual ~SwXTextFrame() override;

    SwXTextFrame(SwDoc *pDoc);
    SwXTextFrame(SwFrameFormat& rFormat);

public:
    static SW_DLLPUBLIC rtl::Reference<SwXTextFrame>
            CreateXTextFrame(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    // FIXME: EVIL HACK:  make available for SwXFrame::attachToRange
    using SwXText::SetDoc;

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;
    virtual SW_DLLPUBLIC void acquire(  ) noexcept override;
    virtual SW_DLLPUBLIC void release(  ) noexcept override;

    //XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId(  ) override;

    //XTextFrame
    virtual SW_DLLPUBLIC cpo::uno::Reference< css::text::XText >  getText() override;

    //XText
    virtual rtl::Reference< SwXTextCursor > createXTextCursor() override;
    virtual rtl::Reference< SwXTextCursor > createXTextCursorByRange(
            const ::cpo::uno::Reference< ::css::text::XTextRange >& aTextPosition ) override;

    //XEnumerationAccess - was: XParagraphEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >  createEnumeration() override;

    //XElementAccess
    virtual cpo::uno::Type getElementType(  ) override;
    virtual bool hasElements(  ) override;

    //XTextContent
    virtual void attach( const cpo::uno::Reference< css::text::XTextRange >& xTextRange ) override;
    virtual cpo::uno::Reference< css::text::XTextRange > getAnchor(  ) override final;

    //XComponent
    virtual void dispose(  ) override;
    virtual void addEventListener( const cpo::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const cpo::uno::Reference< css::lang::XEventListener >& aListener ) override;

    //XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XEventsSupplier
    virtual cpo::uno::Reference< css::container::XNameReplace > getEvents(  ) override;

    //XPropertySet
    virtual SW_DLLPUBLIC cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    using SwXFrame::setPropertyValue;

    SW_DLLPUBLIC rtl::Reference< SwXParagraphEnumeration > createSwEnumeration();

private:
    rtl::Reference< SwXTextCursor > createXTextCursorByRangeImpl(SwFrameFormat& rFormat, SwUnoInternalPaM& rPam);
};

typedef cppu::ImplInheritanceHelper
<   SwXFrame,
    css::document::XEventsSupplier
>
SwXTextGraphicObjectBaseClass;
class SW_DLLPUBLIC SwXTextGraphicObject final : public SwXTextGraphicObjectBaseClass
{
    friend class SwXFrame; // just for CreateXFrame

    virtual ~SwXTextGraphicObject() override;

    SwXTextGraphicObject( SwDoc *pDoc );
    SwXTextGraphicObject(SwFrameFormat& rFormat);

public:

    static rtl::Reference<SwXTextGraphicObject>
        CreateXTextGraphicObject(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    //XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XEventsSupplier
    virtual cpo::uno::Reference< css::container::XNameReplace > getEvents(  ) override;
};

typedef cppu::ImplInheritanceHelper
<   SwXFrame,
    css::document::XEmbeddedObjectSupplier2,
    css::document::XEventsSupplier
> SwXTextEmbeddedObjectBaseClass;

class SW_DLLPUBLIC SwXTextEmbeddedObject final : public SwXTextEmbeddedObjectBaseClass
{
    rtl::Reference<SwXOLEListener> m_xOLEListener;

    friend class SwXFrame; // just for CreateXFrame

    virtual ~SwXTextEmbeddedObject() override;

    SwXTextEmbeddedObject( SwDoc *pDoc );
    SwXTextEmbeddedObject(SwFrameFormat& rFormat);

public:

    static rtl::Reference<SwXTextEmbeddedObject>
        CreateXTextEmbeddedObject(SwDoc & rDoc, SwFrameFormat * pFrameFormat);

    //XEmbeddedObjectSupplier2
    virtual cpo::uno::Reference< css::lang::XComponent >  getEmbeddedObject() override;
    virtual cpo::uno::Reference< css::embed::XEmbeddedObject > getExtendedControlOverEmbeddedObject() override;
    virtual ::sal_Int64 getAspect() override;
    virtual void setAspect( ::sal_Int64 _aspect ) override;
    virtual cpo::uno::Reference< css::graphic::XGraphic > getReplacementGraphic() override;

    //XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XEventsSupplier
    virtual cpo::uno::Reference< css::container::XNameReplace > getEvents(  ) override;
};

class SwXOLEListener final : public cppu::WeakImplHelper<css::util::XModifyListener>, public SvtListener
{
    SwFormat* m_pOLEFormat;
    cpo::uno::Reference<css::frame::XModel> m_xOLEModel;

public:
    SwXOLEListener(SwFormat& rOLEFormat, cpo::uno::Reference< css::frame::XModel > xOLE);
    virtual ~SwXOLEListener() override;

// css::lang::XEventListener
    virtual void disposing( const css::lang::EventObject& Source ) override;

// css::util::XModifyListener
    virtual void modified( const css::lang::EventObject& aEvent ) override;

    virtual void Notify( const SfxHint& ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
