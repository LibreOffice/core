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

#ifndef INCLUDED_SC_INC_SHAPEUNO_HXX
#define INCLUDED_SC_INC_SHAPEUNO_HXX

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <cppuhelper/implbase.hxx>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XAggregation;
    }
    namespace drawing {
        class XShape;
    }
}}}

class SdrObject;
struct SvEventDescription;
class ShapeUnoEventAccessImpl;
class  ScMacroInfo;

//  object which aggregates all svx shape objects,
//  to add own properties

typedef ::cppu::WeakImplHelper <   css::beans::XPropertySet
                                ,   css::beans::XPropertyState
                                ,   css::text::XTextContent
                                ,   css::document::XEventsSupplier
                                ,   css::lang::XServiceInfo
                                >   ScShapeObj_Base;
typedef ::cppu::ImplHelper      <   css::text::XText
                                >   ScShapeObj_TextBase;
typedef ::cppu::ImplHelper      <   css::container::XChild
                                >   ScShapeObj_ChildBase;
class ScShapeObj    :public ScShapeObj_Base
                    ,public ScShapeObj_TextBase
                    ,public ScShapeObj_ChildBase
{
private:
    friend ScMacroInfo* ScShapeObj_getShapeHyperMacroInfo( ScShapeObj* pShape, bool bCreate );
    css::uno::Reference< css::uno::XAggregation >              mxShapeAgg;
    // cached pointers to avoid repeated queryAggregation calls:
    css::beans::XPropertySet*                                  pShapePropertySet;
    css::beans::XPropertyState*                                pShapePropertyState;
    css::uno::Reference< css::beans::XPropertySetInfo >        mxPropSetInfo;
    bool                                                       bIsTextShape;
    bool                                                       bIsNoteCaption;
    bool                                                       bInitializedNotifier;

    SdrObject* GetSdrObject() const throw();

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
                                const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException,
                                    std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException,
                                    std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
                            getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName )
                                throw (css::beans::UnknownPropertyException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XTextContent
    virtual void SAL_CALL   attach(const css::uno::Reference< css::text::XTextRange > & xTextRange)
                                throw( css::lang::IllegalArgumentException,
                                        css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getAnchor()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;

                            // XComponent
    virtual void SAL_CALL   dispose() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener)
                                throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener)
                                throw( css::uno::RuntimeException, std::exception ) override;

                            // XText
    virtual void SAL_CALL   insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange,
                                const css::uno::Reference< css::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(css::lang::IllegalArgumentException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent )
                                    throw(css::container::NoSuchElementException,
                                            css::uno::RuntimeException, std::exception) override;

                            // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertString( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(css::lang::IllegalArgumentException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setString( const OUString& aString )
                                    throw(css::uno::RuntimeException, std::exception) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent()
                            throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& xParent )
                            throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw (css::uno::RuntimeException, std::exception) override ;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
                                throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
