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

#pragma once

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase1.hxx>

namespace com::sun::star {
    namespace drawing {
        class XShape;
    }
}
namespace cpo::uno {
    class XAggregation;
}
class SdrObject;
struct SvEventDescription;
class  ScMacroInfo;

//  object which aggregates all svx shape objects,
//  to add own properties

typedef ::cppu::WeakImplHelper <   css::beans::XPropertySet
                                ,   css::beans::XPropertyState
                                ,   css::text::XTextContent
                                ,   css::document::XEventsSupplier
                                ,   css::lang::XServiceInfo
                                >   ScShapeObj_Base;
typedef ::cppu::ImplHelper1     <   css::text::XText
                                >   ScShapeObj_TextBase;
typedef ::cppu::ImplHelper1     <   css::container::XChild
                                >   ScShapeObj_ChildBase;
class ScShapeObj final : public ScShapeObj_Base
                    ,public ScShapeObj_TextBase
                    ,public ScShapeObj_ChildBase
{
private:
    friend ScMacroInfo* ScShapeObj_getShapeHyperMacroInfo( const ScShapeObj* pShape, bool bCreate );
    cpo::uno::Reference< cpo::uno::XAggregation >              mxShapeAgg;
    // cached pointers to avoid repeated queryAggregation calls:
    css::beans::XPropertySet*                                  pShapePropertySet;
    css::beans::XPropertyState*                                pShapePropertyState;
    cpo::uno::Reference< css::beans::XPropertySetInfo >        mxPropSetInfo;
    bool                                                       bIsTextShape;
    bool                                                       bIsNoteCaption;

    SdrObject* GetSdrObject() const noexcept;

    void                    GetShapePropertySet();
    void                    GetShapePropertyState();

friend class ShapeUnoEventAccessImpl;

public:
    static const SvEventDescription* GetSupportedMacroItems();

                            // ctor modifies xShape parameter
                            ScShapeObj( cpo::uno::Reference<
                                            css::drawing::XShape > & xShape );
    virtual                 ~ScShapeObj() override;

                            // XInterface
    virtual cpo::uno::Any queryInterface(
                                const cpo::uno::Type & rType ) override;
    virtual void   acquire() noexcept override;
    virtual void   release() noexcept override;

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XPropertyState
    virtual css::beans::PropertyState getPropertyState( const OUString& PropertyName ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyState >
                            getPropertyStates( const cpo::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void   setPropertyToDefault( const OUString& PropertyName ) override;
    virtual cpo::uno::Any getPropertyDefault( const OUString& aPropertyName ) override;

                            // XTextContent
    virtual void   attach(const cpo::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getAnchor() override;

                            // XComponent
    virtual void   dispose() override;
    virtual void   addEventListener(const cpo::uno::Reference< css::lang::XEventListener > & aListener) override;
    virtual void   removeEventListener(const cpo::uno::Reference< css::lang::XEventListener > & aListener) override;

                            // XText
    virtual void   insertTextContent( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                const cpo::uno::Reference< css::text::XTextContent >& xContent,
                                bool bAbsorb ) override;
    virtual void   removeTextContent( const cpo::uno::Reference< css::text::XTextContent >& xContent ) override;

                            // XSimpleText
    virtual cpo::uno::Reference< css::text::XTextCursor >
                            createTextCursor() override;
    virtual cpo::uno::Reference< css::text::XTextCursor >
                            createTextCursorByRange( const cpo::uno::Reference< css::text::XTextRange >& aTextPosition ) override;
    virtual void   insertString( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, bool bAbsorb ) override;
    virtual void   insertControlCharacter( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, bool bAbsorb ) override;

                            // XTextRange
    virtual cpo::uno::Reference< css::text::XText >
                            getText() override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getStart() override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getEnd() override;
    virtual OUString getString() override;
    virtual void   setString( const OUString& aString ) override;

    // XChild
    virtual cpo::uno::Reference< cpo::uno::XInterface > getParent() override;
    virtual void setParent( const cpo::uno::Reference< cpo::uno::XInterface >& xParent ) override;

                            // XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

                            // XEventsSupplier
    virtual cpo::uno::Reference< css::container::XNameReplace > getEvents() override;

                            // XServiceInfo
    virtual OUString getImplementationName(  ) override ;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
