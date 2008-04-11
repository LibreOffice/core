/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implementationreference.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#define _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>

namespace comphelper
{

    /** Holds a uno::Reference alongside a C++ implementation pointer

        This template is useful to accomplish the following task: the
        client needs an implementation pointer to an object providing
        UNO interfaces. It is unsafe to simply store a C++ pointer,
        because of the automatic UNO lifetime control. It is
        inconvenient to always cast the UNO interface to the C++
        implementation, and what's more, it's mostly unclear to the
        casual code reader.

        Thus, this template nicely encapsulate the stated intention,
        by holding a uno::Reference internally, and providing simple
        C++ pointer semantics to the outside. As a differentiator to
        ::rtl::Reference, this template features a getRef() method,
        giving you friction-less access to the internal UNO interface,
        without extra querying.

        By the way, the pointer semantic of this template include
        transitive constness. That means, if this template's instance
        is const (e.g. because it is a member of a class which is
        accessed in a const method), the pointer returned is also
        const.

        As this template is geared towards fast, internal pointer
        access, validity of the UNO reference is _not_ checked for
        every pointer access. The client of this template is
        responsible to check that, whereever necessary, via the is()
        method.

        @tpl CppType
        The C++ type this class should mimick a pointer to (not the
        pointer type itself!).

        @tpl UnoType
        The UNO interface type of the object (a uno::Reference to this
        type is held internally).

        @tpl XIfType
        An unambiguous derivative of UnoType. This is defaulted to
        the second template parameter (UnoType), which should normally
        just work, since one typically has only single inheritance in
        UNO.<p>
        Alternatively, when using the
        ImplementationReference::createFromQuery() method to create an
        instance, this type can serve a different need: if the
        provided CppType only derives from XInterface (generally
        speaking, derives from a UNO interface above UnoType in the
        class hierarchy), then the default XIfType constitutes a
        possibly invalid downcast to UnoType. Setting XIfType equal to
        CppTypes's most derived UNO interface type then solves this
        problem (which is not as arcane as it seems to be. Just
        imagine you're providing a C++ abstract interface, which must
        provide UNO reference semantics. Naturally, you will derive
        this C++ interface only from XInterface, to reduce the number
        of ambiguous classes. Even more naturally, it is reasonable to
        have UnoType be something different from XInterface, governed
        by the usage of the C++ interface)

        @sample ImplementationReference< MyCppType, XMyInterface >

        @sample ImplementationReference< MyAbstractCppType, XMyInterface, XInterface >
        for an abstract C++ class

        @see ::rtl::Reference

     */
    template < class CppType,
               class UnoType,
               class XIfType=UnoType > class ImplementationReference
    {
    public:

        typedef UnoType UnoInterfaceType;
        typedef CppType ImplementationType;
        typedef XIfType UnambiguousXInterfaceType;

        /** Default-construct an ImplementationReference

            Uno reference will be invalid, implementation pointer will
            be NULL.
         */
        ImplementationReference() :
            mxRef(),
            mpImpl( NULL )
        {
        }

        /** Create an ImplementationReference from C++ pointer.

            This constructor does not perform an explicit
            QueryInterface on the provided implementation object, but
            constructs the UNO reference directly from the given
            pointer. This is the fastest, and most often the best way
            to create an ImplementationReference. If the conversion
            between the implementation object and the required UNO
            interface is ambiguous, provide the third template
            parameter with a type that can be unambiguously upcasted
            to the UNO interface (the second template parameter).

            There are cases, however, where performing a
            QueryInterface is the better, albeit slower choice. In
            these cases, createFromQuery() should be used.

            @param pImpl
            Pointer to the C++ implementation type

            @see createFromQuery()
        */
        explicit ImplementationReference( ImplementationType* pImpl ) :
            mxRef( static_cast<UnambiguousXInterfaceType*>(pImpl) ),
            mpImpl( pImpl )
        {
        }

        struct CreateFromQuery { };
        /** Create an ImplementationReference from C++ pointer

            @param pImpl
            The pointer to the C++ implementation type, which is
            queried for the template-parameterized UNO type.

            @param dummy
            Dummy parameter, to distinguish this contructor from the
            default unary one (which does not perform a
            QueryInterface)
         */
        ImplementationReference( ImplementationType* pImpl, CreateFromQuery ) :
            mxRef( static_cast<UnambiguousXInterfaceType*>(pImpl),
                   ::com::sun::star::uno::UNO_QUERY ),
            mpImpl( pImpl )
        {
        }

        /** Factory method to create an ImplementationReference from
            C++ pointer.

            This is a static version of the constructor which creates
            an instance of an implementation type which is explicitely
            queried for the ImplementationReference's
            template-parameterized UNO type.

            @sample
                mpRef = mpRef.createFromQuery( new ImplementationType );
        */
        static ImplementationReference createFromQuery( ImplementationType* pImpl )
        {
            return ImplementationReference( pImpl, CreateFromQuery() );
        }

        /** Query whether the pointer is still valid.

            Hands off also from the implementation pointer if this
            returns false!
         */
        bool is() const { return mxRef.is(); }

        /** Get a pointer to the implementation object

            Compatibility method to get an auto_ptr-compatible
            interface
         */
        ImplementationType*         get() { return mpImpl; }
        const ImplementationType*   get() const { return mpImpl; }

        /** Release all references

            Compatibility method to get an auto_ptr-compatible
            interface
         */
        void                        reset() { dispose(); }

        /** Release all references

            This method releases the UNO interface reference, and
            clears the C++ pointer to NULL.
         */
        void                        dispose() { mxRef = NULL; mpImpl=NULL; }

        ImplementationType*         operator->() { return mpImpl; }
        const ImplementationType*   operator->() const { return mpImpl; }

        ImplementationType&         operator*() { return *mpImpl; }
        const ImplementationType&   operator*() const { return *mpImpl; }

        /// Access to the underlying UNO reference, without extra querying
        ::com::sun::star::uno::Reference< UnoInterfaceType > getRef() { return mxRef; }

        /// Access to the underlying UNO reference, without extra querying
        const ::com::sun::star::uno::Reference< UnoInterfaceType >& getRef() const { return mxRef; }

        // default destructor, copy constructor and assignment will do
        // ~ImplementationReference();
        // ImplementationReference( const ImplementationReference& );
        // ImplementationReference& operator= ( const ImplementationReference& );

        /** Comparison operator

            Object identity is defined to be identity of the
            implementation pointers. This is in general invalid when
            comparing pointers to UNO objects (ambiguous class
            hierarchies, optimizations in the bridges, etc.), but okay
            for raw C++ pointers (which is what's compared herein).
        */
        bool operator==( const ImplementationReference& rhs ) const
        {
            return mpImpl == rhs.mpImpl;
        }

        /** less-than operator

            Object order is defined to be the ordering of the
            implementation pointers. This is in general invalid when
            comparing pointers to UNO objects (ambiguous class
            hierarchies, optimizations in the bridges, etc.), but okay
            for raw C++ pointers (which is what's used herein).

            This ordering complies with STL's strict weak ordering
            concept.
        */
        bool operator<( const ImplementationReference& rhs ) const
        {
            return mpImpl < rhs.mpImpl;
        }

    private:

        // the interface, hard reference to prevent object from vanishing
        ::com::sun::star::uno::Reference< UnoInterfaceType >    mxRef;

        // the c++ object, for our internal stuff
        ImplementationType*                                     mpImpl;

    };

}

#endif // _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
