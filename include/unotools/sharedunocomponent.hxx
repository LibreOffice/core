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

#ifndef INCLUDED_UNOTOOLS_SHAREDUNOCOMPONENT_HXX
#define INCLUDED_UNOTOOLS_SHAREDUNOCOMPONENT_HXX

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include <memory>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XComponent;
    }
} } }

namespace utl
{

    //= DisposableComponent

    /** is a class which controls lifetime of an UNO component via ->XComponent::dispose

        You'll usually never use this class directly, but only as parameter for a
        ->SharedUNOComponent class.
    */
    class UNOTOOLS_DLLPUBLIC DisposableComponent
    {
        css::uno::Reference< css::lang::XComponent >  m_xComponent;

    public:
        /** constructs a ->DisposableComponent instance

        @param _rxComponent
            the component whose life time should be controlled by the instance. Must not be <NULL/>.
        */
        DisposableComponent( const css::uno::Reference< css::uno::XInterface >& _rxComponent );

        /** disposes the component represented by the instance

            The component is queried for ->XComponent(which <em>must</em> be supported),
            and ->XComponent::dispose is invoked. A failure of this invocation (e.g. a thrown
            exception) is silenced in release builds, and reported in debug builds.
        */
        ~DisposableComponent();

    private:
        DisposableComponent( const DisposableComponent& ) = delete;
        DisposableComponent& operator=( const DisposableComponent& ) = delete;
    };

    //= CloseableComponent

    class CloseableComponentImpl;
    /** is a class which controls lifetime of an UNO component via ->XCloseable::close

        You'll usually never use this class directly, but only as parameter for a
        ->SharedUNOComponent class.
    */
    class UNOTOOLS_DLLPUBLIC CloseableComponent
    {
    private:
        /** Our IMPL class.
        */
        ::rtl::Reference< CloseableComponentImpl >  m_pImpl;

    public:
        /** constructs a ->CloseableComponent instance

        @param _rxComponent
            the component whose life time should be controlled by the instance. Must not be <NULL/>.
        */
        CloseableComponent( const css::uno::Reference< css::uno::XInterface >& _rxComponent );

        /** destroys resources associated with this instance, and disposes the component

            The component is queried for ->XCloseable (which <em>must</em> be supported),
            and ->XCloseable::close is invoked, with delivering the ownership.
            If the invocation fails with a ->CloseVetoException, this is ignored, since in
            this case the vetoing instance took the ownership.

            Any other failure will be reported in a debug version via assertion mechanisms,
            and silenced in release builds.
        */
        ~CloseableComponent();

    private:
        CloseableComponent( const CloseableComponent& ) = delete;
        CloseableComponent& operator=( const CloseableComponent& ) = delete;
    };

    //= SharedUNOComponent

    /** is a helper class for sharing ownership of a UNO component

        If you need to share an UNO component, which normally needs a dedicated owner,
        and is lifetime controlled by an explicit disposal action (not necessarily ->XComponent::dispose,
        but <em>any</em> explicit method call, after which the object is considered
        to be disposed), between different classes, ->SharedUNOComponent is what you need.

        Instead of passing around a <code>Reference&lt; XFoo &gt;</code>, and bothering
        with ownership and disposal, you just use a <code>SharedUNOComponent&lt; XFoo &gt;</code>.
        This instance can be passed around, including copying, and in nearly all respects behaves
        like the original <code>Reference&lt; XFoo &gt;</code>. However, when the last
        ->SharedUNOComponent referencing a certain <code>Reference&lt; XFoo &gt;</code> dies, it
        will automatically get rid of the object held by this reference.

    @param INTERFACE
        the UNO interface type as which the component should be held

    @param COMPONENT_HOLDER
        a class which can be used to represent and dispose a UNO component.
        The class must support (maybe explicit only) construction from a
        <code>Reference&lt; INTERFACE &gt;</code>, and destruction. Upon destruction,
        the class must dispose (by any suitable means) the component instance it was
        constructed with.
    */
    template < class INTERFACE, class COMPONENT = DisposableComponent >
    class SharedUNOComponent
    {
    private:
        typedef COMPONENT                           Component;
        typedef std::shared_ptr<Component>          ComponentPointer;

    private:
        ComponentPointer                            m_xComponent;
        css::uno::Reference< INTERFACE >            m_xTypedComponent;

    public:
        enum AssignmentMode
        {
            TakeOwnership,
            NoTakeOwnership
        };

    public:
        inline  SharedUNOComponent()
        {
        }

        explicit inline  SharedUNOComponent( const css::uno::Reference< INTERFACE >& _rxComponent, AssignmentMode eMode = TakeOwnership )
        {
            reset( _rxComponent, eMode );
        }

        inline SharedUNOComponent( const css::uno::BaseReference & _rRef, css::uno::UnoReference_QueryThrow _queryThrow )
        {
            set( _rRef, _queryThrow );
        }

//        SharedUNOComponent& operator=( const css::uno::Reference< INTERFACE >& _rxComponent );
        // This operator is intentionally not implemented. There is no canonic ownership after this operator
        // would have been applied: Should the SharedUNOComponent have the ownership of the component,
        // or shouldn't it? Hard to guess, and probably wrong in 50 percent of all cases, anyway. So,
        // instead of tempting clients of this class to use such a dangerous operator, we do
        // not offer it at all. If you need to assign a Reference< INTERFACE > to your SharedUNOComponent,
        // use the ->reset method.

        /** assigns a new component, and releases the old one
        */
        void reset( const css::uno::Reference< INTERFACE >& _rxComponent, AssignmentMode _eMode = TakeOwnership );

        inline bool set( const css::uno::BaseReference& _rRef, css::uno::UnoReference_Query _query );

        inline void set( const css::uno::BaseReference & _rRef, css::uno::UnoReference_QueryThrow _queryThrow );

        inline void set( const css::uno::Reference< INTERFACE >& _rRef, css::uno::UnoReference_SetThrow _setThrow );
        inline void set( const SharedUNOComponent& _rComp, css::uno::UnoReference_SetThrow _setThrow );

        INTERFACE* SAL_CALL operator->() const;

        operator const css::uno::Reference< INTERFACE >&() const
        {
            return m_xTypedComponent;
        }

        const css::uno::Reference< INTERFACE >& getTyped() const
        {
            return m_xTypedComponent;
        }

        bool is() const
        {
            return m_xTypedComponent.is();
        }

        void clear()
        {
            m_xComponent.reset();
            m_xTypedComponent.clear();
        }
    };

    template < class INTERFACE, class COMPONENT >
    INTERFACE* SAL_CALL SharedUNOComponent< INTERFACE, COMPONENT >::operator->() const
    {
        return m_xTypedComponent.operator->();
    }

    // assignments
    template < class INTERFACE, class COMPONENT >
    void SharedUNOComponent< INTERFACE, COMPONENT >::reset( const css::uno::Reference< INTERFACE >& _rxComponent, AssignmentMode _eMode )
    {
        m_xComponent.reset(_eMode == TakeOwnership ? new COMPONENT( _rxComponent ) : nullptr);
        m_xTypedComponent = _rxComponent;
    }

    // comparison operators

    template < class INTERFACE, class COMPONENT >
    bool operator==( const SharedUNOComponent< INTERFACE, COMPONENT >& _rLHS, const css::uno::Reference< INTERFACE >& _rRHS )
    {
        return _rLHS.getTyped() == _rRHS;
    }

    template < class INTERFACE, class COMPONENT >
    inline css::uno::Any SAL_CALL makeAny( const SharedUNOComponent< INTERFACE, COMPONENT >& value )
    {
        return makeAny( value.getTyped() );
    }

    template < class INTERFACE, class COMPONENT >
    void SharedUNOComponent< INTERFACE, COMPONENT >::set( const css::uno::BaseReference & _rRef, css::uno::UnoReference_QueryThrow _queryThrow )
    {
        reset( css::uno::Reference< INTERFACE >( _rRef, _queryThrow ), TakeOwnership );
    }

    template < class INTERFACE, class COMPONENT >
    void SharedUNOComponent< INTERFACE, COMPONENT >::set( const css::uno::Reference< INTERFACE >& _rRef, css::uno::UnoReference_SetThrow _setThrow )
    {
        reset( css::uno::Reference< INTERFACE >( _rRef, _setThrow ), TakeOwnership );
    }

    template < class INTERFACE, class COMPONENT >
    void SharedUNOComponent< INTERFACE, COMPONENT >::set( const SharedUNOComponent& _rComp, css::uno::UnoReference_SetThrow _setThrow )
    {
        *this = _rComp;
        // provoke an exception in case the component is NULL
        m_xTypedComponent.set( m_xTypedComponent, _setThrow );
    }

    template < class INTERFACE, class COMPONENT >
    bool SharedUNOComponent< INTERFACE, COMPONENT >::set( const css::uno::BaseReference& _rRef, css::uno::UnoReference_Query _query )
    {
        reset( css::uno::Reference< INTERFACE >( _rRef, _query ) );
        return is();
    }

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_SHAREDUNOCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
