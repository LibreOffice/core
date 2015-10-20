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

#ifndef INCLUDED_EDITENG_ACCESSIBLEPARAMANAGER_HXX
#define INCLUDED_EDITENG_ACCESSIBLEPARAMANAGER_HXX

#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <editeng/editengdllapi.h>

class SvxEditSourceAdapter;

namespace accessibility
{
    class AccessibleEditableTextPara;

    /** Helper class for WeakCppRef

        This class is returned by WeakChild::get() and contains a hard
        reference and a reference to the c++ object. This combination
        prevents the c++ object from destruction during usage. Hold
        this object only as long as absolutely necessary, prevents
        referenced object from vanishing otherwise
    */
    template < class UnoType, class CppType > class HardCppRef
    {
    public:

        typedef UnoType UnoInterfaceType;
        typedef CppType InterfaceType;

        HardCppRef( const css::uno::WeakReference< UnoInterfaceType >& xRef, InterfaceType* rImpl ) :
            mxRef( xRef ),
            mpImpl( rImpl )
        {
        }

        /** Query whether the reference is still valid.

            Hands off also from the implementation pointer if this
            returns sal_False!
         */
        bool is() const { return mxRef.is(); }
        InterfaceType* operator->() const { return mpImpl; }
        InterfaceType& operator*() const { return *mpImpl; }
        css::uno::Reference< UnoInterfaceType >& getRef() { return mxRef; }
        const css::uno::Reference< UnoInterfaceType >& getRef() const { return mxRef; }

        // default copy constructor and assignment will do
        // HardCppRef( const HardCppRef& );
        // HardCppRef& operator= ( const HardCppRef& );

    private:

        // the interface, hard reference to prevent object from vanishing
        css::uno::Reference< UnoInterfaceType >                 mxRef;

        // the c++ object, for our internal stuff
        InterfaceType*                                          mpImpl;

    };

    /** Helper class for weak object references plus implementation

        This class combines a weak reference (to facilitate automatic
        object disposal if user drops last reference) and hard
        reference to the c++ class (for fast access and bypassing of
        the UNO interface)
    */
    template < class UnoType, class CppType > class WeakCppRef
    {
    public:

        typedef UnoType UnoInterfaceType;
        typedef CppType InterfaceType;
        typedef HardCppRef< UnoInterfaceType, InterfaceType >  HardRefType;

        WeakCppRef() : maWeakRef(), maUnsafeRef( NULL ) {}
        WeakCppRef( InterfaceType& rImpl ) :
            maWeakRef( css::uno::Reference< UnoInterfaceType >( rImpl, css::uno::UNO_QUERY ) ),
            maUnsafeRef( &rImpl )
        {
        }

        WeakCppRef( HardRefType& rImpl ) :
            maWeakRef( rImpl.getRef() ),
            maUnsafeRef( rImpl.operator->() )
        {
        }

        // get object with c++ object and hard reference (which
        // prevents the c++ object from destruction during use)
        HardRefType get() const { return HardRefType( maWeakRef, maUnsafeRef ); }

        // default copy constructor and assignment will do
        // WeakCppRef( const WeakCppRef& );
        // WeakCppRef& operator= ( const WeakCppRef& );

    private:

        // the interface, hold weakly
        css::uno::WeakReference< UnoInterfaceType >    maWeakRef;

        // hard ref to c++ class, _only_ valid if maWeakRef.is() is true
        InterfaceType*                                 maUnsafeRef;
    };


    /** This class manages the paragraphs of an AccessibleTextHelper

        To facilitate automatic deletion of paragraphs no longer used,
        this class uses the WeakCppRef helper to hold the objects weakly.
     */
    class EDITENG_DLLPUBLIC AccessibleParaManager
    {
    public:
        typedef WeakCppRef < css::accessibility::XAccessible, AccessibleEditableTextPara > WeakPara;
        typedef ::std::pair< WeakPara, css::awt::Rectangle > WeakChild;
        typedef ::std::pair< css::uno::Reference<
            css::accessibility::XAccessible > , css::awt::Rectangle > Child;
        typedef ::std::vector< WeakChild > VectorOfChildren;
        typedef ::std::vector< sal_Int16 > VectorOfStates;

        AccessibleParaManager();
        ~AccessibleParaManager();

        /** Sets a vector of additional accessible states.

            The states are passed to every created child object
            (text paragraph). The state values are defined in
            css::accessibility::AccessibleStateType.
         */
        void SetAdditionalChildStates( const VectorOfStates& rChildStates );

        /** Set the number of paragraphs

            @param nNumPara
            The total number of paragraphs the EditEngine currently
            has (_not_ the number of currently visible children)
         */
        void SetNum( sal_Int32 nNumParas );

        /** Get the number of paragraphs currently possible */
        sal_Int32 GetNum() const;

        // iterators
        VectorOfChildren::iterator begin();
        VectorOfChildren::iterator end();

        /// Set focus to given child
        void SetFocus( sal_Int32 nChild );

        void FireEvent( sal_Int32 nPara,
                        const sal_Int16 nEventId,
                        const css::uno::Any& rNewValue = css::uno::Any(),
                        const css::uno::Any& rOldValue = css::uno::Any() ) const;

        static bool IsReferencable( WeakPara::HardRefType aChild );
        bool IsReferencable( sal_Int32 nChild ) const;
        static void ShutdownPara( const WeakChild& rChild );

        Child CreateChild( sal_Int32                                                                                        nChild,
                           const css::uno::Reference< css::accessibility::XAccessible >& xFrontEnd,
                           SvxEditSourceAdapter&                                                                            rEditSource,
                           sal_Int32                                                                                       nParagraphIndex );

        WeakChild GetChild( sal_Int32 nParagraphIndex ) const;

        // forwarder to all paragraphs
        /// Make all children active and editable (or off)
        void SetActive( bool bActive = true );
        /// Set state of all children
        void SetState( const sal_Int16 nStateId );
        /// Unset state of all children
        void UnSetState( const sal_Int16 nStateId );
        /// Set offset to edit engine for all children
        void SetEEOffset        ( const Point& rOffset );
        /// Dispose all living children
        void Dispose            ();

        // forwarder to given paragraphs

        /** Release the given range of paragraphs

            All ranges have the meaning [start,end), similar to STL

            @param nStartPara
            Index of paragraph to start with releasing

            @param nEndPara
            Index of first paragraph to stop with releasing
         */
        void Release( sal_Int32 nStartPara, sal_Int32 nEndPara );

        /** Fire event for the given range of paragraphs

            All ranges have the meaning [start,end), similar to STL

            @param nStartPara
            Index of paragraph to start with event firing

            @param nEndPara
            Index of first paragraph to stop with event firing
         */
        void FireEvent( sal_Int32 nStartPara,
                        sal_Int32 nEndPara,
                        const sal_Int16 nEventId,
                        const css::uno::Any& rNewValue = css::uno::Any(),
                        const css::uno::Any& rOldValue = css::uno::Any() ) const;

        /** Functor adapter for ForEach template

            Adapts giving functor such that only the paragraph objects
            are accessed and the fact that our children are held
            weakly is hidden

            The functor must provide the following method:
            void operator() ( AccessibleEditablePara& )

        */
        template < typename Functor > class WeakChildAdapter : public ::std::unary_function< const WeakChild&, void >
        {
        public:
            WeakChildAdapter( Functor& rFunctor ) : mrFunctor(rFunctor) {}
            void operator()( const WeakChild& rPara )
            {
                // retrieve hard reference from weak one
                WeakPara::HardRefType aHardRef( rPara.first.get() );

                if( aHardRef.is() )
                    mrFunctor( *aHardRef );
            }

        private:
            Functor& mrFunctor;
        };

        /** Adapter for unary member functions

            Since STL's binder don't work with const& arguments (and
            BOOST's neither, at least on MSVC), have to provide our
            own adapter for unary member functions.

            Create with pointer to member function of
            AccessibleEditableTextPara and the corresponding argument.
         */
        template < typename Argument > class MemFunAdapter : public ::std::unary_function< const WeakChild&, void >
        {
        public:
            typedef void (::accessibility::AccessibleEditableTextPara::*FunctionPointer)( Argument );

            MemFunAdapter( FunctionPointer aFunPtr, Argument aArg ) : maFunPtr(aFunPtr), maArg(aArg) {}
            void operator()( const WeakChild& rPara )
            {
                // retrieve hard reference from weak one
                WeakPara::HardRefType aHardRef( rPara.first.get() );

                if( aHardRef.is() )
                    (*aHardRef.*maFunPtr)( maArg );
            }

        private:
            FunctionPointer maFunPtr;
            Argument maArg;
        };

    private:
        /// Set state on given child
        void SetState( sal_Int32 nChild, const sal_Int16 nStateId );
        /// Unset state on given child
        void UnSetState( sal_Int32 nChild, const sal_Int16 nStateId );
        /// Init child with default state (as stored in previous SetFocus and SetActive calls)
        void InitChild( AccessibleEditableTextPara&     rChild,
                        SvxEditSourceAdapter&           rEditSource,
                        sal_Int32                       nChild,
                        sal_Int32                       nParagraphIndex ) const;

        // vector the size of the paragraph number of the underlying EditEngine
        VectorOfChildren maChildren;

        /// Additional states that will be set at every created child object.
        VectorOfStates maChildStates;

        // cache EE offset for child creation
        Point maEEOffset;

        // which child currently has the focus (-1 for none)
        sal_Int32 mnFocusedChild;

        // whether children are active and editable
        bool mbActive;
    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
