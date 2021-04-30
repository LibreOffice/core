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
    namespace uno {
        class XAggregation;
    }
    namespace drawing {
        class XShape;
    }
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
    css::uno::Reference< css::uno::XAggregation >              mxShapeAgg;
    // cached pointers to avoid repeated queryAggregation calls:
    css::beans::XPropertySet*                                  pShapePropertySet;
    css::beans::XPropertyState*                                pShapePropertyState;
    css::uno::Reference< css::beans::XPropertySetInfo >        mxPropSetInfo;
    bool                                                       bIsTextShape;
    bool                                                       bIsNoteCaption;

    SdrObject* GetSdrObject() const noexcept;

    void                    GetShapePropertySet();
    void                    GetShapePropertyState();

friend class ShapeUnoEventAccessImpl;

public:
    static const SvEventDescription* GetSupportedMacroItems();

                            // ctor modifies xShape parameter
                            ScShapeObj( css::uno::Reference<
                                            css::drawing::XShape > & xShape );
    virtual                 ~ScShapeObj() override;

                            // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
                                const css::uno::Type & rType ) override;
    virtual void SAL_CALL   acquire() noexcept override;
    virtual void SAL_CALL   release() noexcept override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
                            getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

                            // XTextContent
    virtual void SAL_CALL   attach(const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getAnchor() override;

                            // XComponent
    virtual void SAL_CALL   dispose() override;
    virtual void SAL_CALL   addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;
    virtual void SAL_CALL   removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;

                            // XText
    virtual void SAL_CALL   insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange,
                                const css::uno::Reference< css::text::XTextContent >& xContent,
                                sal_Bool bAbsorb ) override;
    virtual void SAL_CALL   removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent ) override;

                            // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition ) override;
    virtual void SAL_CALL   insertString( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb ) override;
    virtual void SAL_CALL   insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb ) override;

                            // XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL   setString( const OUString& aString ) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent() override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& xParent ) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

                            // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override ;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
