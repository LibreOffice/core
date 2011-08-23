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

#ifndef OOX_HELPER_CONTAINERHELPER_HXX
#define OOX_HELPER_CONTAINERHELPER_HXX

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

namespace rtl { class OUString; }

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace container { class XIndexContainer; }
    namespace container { class XNameAccess; }
    namespace container { class XNameContainer; }
    namespace lang { class XMultiServiceFactory; }
} } }

namespace oox {

// ============================================================================

/** Template for a vector of ref-counted objects with additional accessor functions.

    An instance of the class RefVector< Type > stores elements of the type
    ::boost::shared_ptr< Type >. The new accessor functions has() and get()
    work correctly for indexes out of the current range, there is no need to
    check the passed index before.
 */
template< typename ObjType >
class RefVector : public ::std::vector< ::boost::shared_ptr< ObjType > >
{
public:
    typedef ::std::vector< ::boost::shared_ptr< ObjType > > container_type;
    typedef typename container_type::value_type             value_type;
    typedef typename container_type::size_type              size_type;

public:
    /** Returns true, if the object with the passed index exists. Returns
        false, if the vector element exists but is an empty reference. */
    inline bool         has( sal_Int32 nIndex ) const
                        {
                            const value_type* pxRef = getRef( nIndex );
                            return pxRef && pxRef->get();
                        }

    /** Returns a reference to the object with the passed index, or 0 on error. */
    inline value_type   get( sal_Int32 nIndex ) const
                        {
                            if( const value_type* pxRef = getRef( nIndex ) ) return *pxRef;
                            return value_type();
                        }

    /** Returns the index of the last element, or -1, if the vector is empty.
        Does *not* check whether the last element is an empty reference. */
    inline sal_Int32    getLastIndex() const { return static_cast< sal_Int32 >( this->size() ) - 1; }

    /** Calls the passed functor for every contained object, automatically
        skips all elements that are empty references. */
    template< typename FunctorType >
    inline void         forEach( FunctorType aFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctor< FunctorType >( aFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType >
    inline void         forEachMem( FuncType pFunc ) const
                        {
                            forEach( ::boost::bind( pFunc, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType >
    inline void         forEachMem( FuncType pFunc, ParamType aParam ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2, aParam3 ) );
                        }

    /** Calls the passed functor for every contained object. Passes the index as
        first argument and the object reference as second argument to rFunctor. */
    template< typename FunctorType >
    inline void         forEachWithIndex( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctorWithIndex< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the vector index to the member function. */
    template< typename FuncType >
    inline void         forEachMemWithIndex( FuncType pFunc ) const
                        {
                            forEachWithIndex( ::boost::bind( pFunc, _2, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the vector index as first argument to the member function. */
    template< typename FuncType, typename ParamType >
    inline void         forEachMemWithIndex( FuncType pFunc, ParamType aParam ) const
                        {
                            forEachWithIndex( ::boost::bind( pFunc, _2, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the vector index as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    inline void         forEachMemWithIndex( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEachWithIndex( ::boost::bind( pFunc, _2, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the vector index as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    inline void         forEachMemWithIndex( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEachWithIndex( ::boost::bind( pFunc, _2, _1, aParam1, aParam2, aParam3 ) );
                        }

    /** Searches for an element by using the passed functor that takes a
        constant reference of the object type (const ObjType&). */
    template< typename FunctorType >
    inline value_type   findIf( const FunctorType& rFunctor ) const
                        {
                            typename container_type::const_iterator aIt = ::std::find_if( this->begin(), this->end(), FindFunctor< FunctorType >( rFunctor ) );
                            return (aIt == this->end()) ? value_type() : *aIt;
                        }

private:
    template< typename FunctorType >
    struct ForEachFunctor
    {
        FunctorType         maFunctor;
        inline explicit     ForEachFunctor( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        inline void         operator()( const value_type& rxValue ) { if( rxValue.get() ) maFunctor( *rxValue ); }
    };

    template< typename FunctorType >
    struct ForEachFunctorWithIndex
    {
        FunctorType         maFunctor;
        sal_Int32           mnIndex;
        inline explicit     ForEachFunctorWithIndex( const FunctorType& rFunctor ) : maFunctor( rFunctor ), mnIndex( 0 ) {}
        inline void         operator()( const value_type& rxValue ) { if( rxValue.get() ) maFunctor( mnIndex, *rxValue ); ++mnIndex; }
    };
    
    template< typename FunctorType >
    struct FindFunctor
    {
        FunctorType         maFunctor;
        inline explicit     FindFunctor( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        inline bool         operator()( const value_type& rxValue ) { return rxValue.get() && maFunctor( *rxValue ); }
    };

    inline const value_type* getRef( sal_Int32 nIndex ) const
                        {
                            return ((0 <= nIndex) && (static_cast< size_type >( nIndex ) < this->size())) ?
                                &(*this)[ static_cast< size_type >( nIndex ) ] : 0;
                        }
};

// ============================================================================

/** Template for a map of ref-counted objects with additional accessor functions.

    An instance of the class RefMap< Type > stores elements of the type
    ::boost::shared_ptr< Type >. The new accessor functions has() and get()
    work correctly for nonexisting keys, there is no need to check the passed
    key before.
 */
template< typename KeyType, typename ObjType, typename CompType = ::std::less< KeyType > >
class RefMap : public ::std::map< KeyType, ::boost::shared_ptr< ObjType >, CompType >
{
public:
    typedef ::std::map< KeyType, ::boost::shared_ptr< ObjType >, CompType > container_type;
    typedef typename container_type::key_type                               key_type;
    typedef typename container_type::mapped_type                            mapped_type;
    typedef typename container_type::value_type                             value_type;
    typedef typename container_type::key_compare                            key_compare;

public:
    /** Returns true, if the object accossiated to the passed key exists.
        Returns false, if the key exists but points to an empty reference. */
    inline bool         has( key_type nKey ) const
                        {
                            const mapped_type* pxRef = getRef( nKey );
                            return pxRef && pxRef->get();
                        }

    /** Returns a reference to the object accossiated to the passed key, or an
        empty reference on error. */
    inline mapped_type  get( key_type nKey ) const
                        {
                            if( const mapped_type* pxRef = getRef( nKey ) ) return *pxRef;
                            return mapped_type();
                        }

    /** Calls the passed functor for every contained object, automatically
        skips all elements that are empty references. */
    template< typename FunctorType >
    inline void         forEach( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctor< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType >
    inline void         forEachMem( FuncType pFunc ) const
                        {
                            forEach( ::boost::bind( pFunc, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType >
    inline void         forEachMem( FuncType pFunc, ParamType aParam ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2, aParam3 ) );
                        }
    /** Calls the passed functor for every contained object. Passes the key as
        first argument and the object reference as second argument to rFunctor. */
    template< typename FunctorType >
    inline void         forEachWithKey( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctorWithKey< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as argument to the member function. */
    template< typename FuncType >
    inline void         forEachMemWithKey( FuncType pFunc ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType >
    inline void         forEachMemWithKey( FuncType pFunc, ParamType aParam ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    inline void         forEachMemWithKey( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    inline void         forEachMemWithKey( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam1, aParam2, aParam3 ) );
                        }

private:
    template< typename FunctorType >
    struct ForEachFunctor
    {
        FunctorType         maFunctor;
        inline explicit     ForEachFunctor( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        inline void         operator()( const value_type& rValue ) { if( rValue.second.get() ) maFunctor( *rValue.second ); }
    };

    template< typename FunctorType >
    struct ForEachFunctorWithKey
    {
        FunctorType         maFunctor;
        inline explicit     ForEachFunctorWithKey( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        inline void         operator()( const value_type& rValue ) { if( rValue.second.get() ) maFunctor( rValue.first, *rValue.second ); }
    };

    inline const mapped_type* getRef( key_type nKey ) const
                        {
                            typename container_type::const_iterator aIt = find( nKey );
                            return (aIt == this->end()) ? 0 : &aIt->second;
                        }
};

// ============================================================================

/** Template for a 2-dimensional array of objects.

    This class template provides a similar interface to the ::std::vector
    template.
 */
template< typename Type >
class Matrix
{
public:
    typedef ::std::vector< Type >                       container_type;
    typedef typename container_type::value_type         value_type;
    typedef typename container_type::pointer            pointer;
    typedef typename container_type::reference          reference;
    typedef typename container_type::const_reference    const_reference;
    typedef typename container_type::size_type          size_type;
    typedef typename container_type::iterator           iterator;
    typedef typename container_type::const_iterator     const_iterator;

    inline explicit     Matrix() : mnWidth( 0 ) {}
    inline explicit     Matrix( size_type nWidth, size_type nHeight ) { this->resize( nWidth, nHeight ); }
    inline explicit     Matrix( size_type nWidth, size_type nHeight, const_reference rData ) { this->resize( nWidth, nHeight, rData ); }

    inline size_type    capacity() const { return maData.capacity(); }
    inline bool         empty() const { return maData.empty(); }
    inline size_type    size() const { return maData.size(); }
    inline size_type    width() const { return mnWidth; }
    inline size_type    height() const { return this->empty() ? 0 : (this->size() / this->width()); }
    inline bool         has( size_type nX, size_type nY ) const { return (nX < this->width()) && (nY < this->height()); }

    inline void         reserve( size_type nWidth, size_type nHeight ) { maData.reserve( nWidth * nHeight ); }
    inline void         clear() { this->resize( 0, 0 ); }
    inline void         resize( size_type nWidth, size_type nHeight ) { mnWidth = nWidth; maData.resize( nWidth * nHeight ); }
    inline void         resize( size_type nWidth, size_type nHeight, const_reference rData ) { mnWidth = nWidth; maData.resize( nWidth * nHeight, rData ); }

    inline iterator     at( size_type nX, size_type nY ) { return maData.begin() + mnWidth * nY + nX; }
    inline const_iterator at( size_type nX, size_type nY ) const { return maData.begin() + mnWidth * nY + nX; }

    inline reference    operator()( size_type nX, size_type nY ) { return *this->at( nX, nY ); }
    inline const_reference operator()( size_type nX, size_type nY ) const { return *this->at( nX, nY ); }

    inline iterator     begin() { return maData.begin(); }
    inline const_iterator begin() const { return maData.begin(); }
    inline iterator     end() { return maData.end(); }
    inline const_iterator end() const { return maData.end(); }

    inline reference    front() { return maData.front(); }
    inline const_reference front() const { return maData.front(); }
    inline reference    back() { return maData.back(); }
    inline const_reference back() const { return maData.back(); }

    inline iterator     row_begin( size_type nY ) { return this->at( 0, nY ); }
    inline const_iterator row_begin( size_type nY ) const { return this->at( 0, nY ); }
    inline iterator     row_end( size_type nY ) { return this->at( mnWidth, nY ); }
    inline const_iterator row_end( size_type nY ) const { return this->at( mnWidth, nY ); }

    inline reference    row_front( size_type nY ) { return (*this)( 0, nY ); }
    inline const_reference row_front( size_type nY ) const { return (*this)( 0, nY ); }
    inline reference    row_back( size_type nY ) { return (*this)( mnWidth - 1, nY ); }
    inline const_reference row_back( size_type nY ) const { return (*this)( mnWidth - 1, nY ); }

    inline void         swap( Matrix& rMatrix ) { maData.swap( rMatrix.maData ); }

private:
    container_type      maData;
    size_type           mnWidth;
};

// ============================================================================

/** Static helper functions for improved API container handling. */
class ContainerHelper
{
public:
    // com.sun.star.container.XIndexContainer ---------------------------------

    /** Creates a new index container object from scratch. */
    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >
                        createIndexContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    /** Inserts an object into an indexed container.

        @param rxIndexContainer  com.sun.star.container.XIndexContainer
            interface of the indexed container.

        @param nIndex  Insertion index for the object.

        @param rObject  The object to be inserted.

        @return  True = object successfully inserted.
     */
    static bool         insertByIndex(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rxIndexContainer,
                            sal_Int32 nIndex,
                            const ::com::sun::star::uno::Any& rObject );

    // com.sun.star.container.XNameContainer ----------------------------------

    /** Creates a new name container object from scratch. */
    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        createNameContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    /** Returns a name that is not used in the passed name container.

        @param rxNameAccess  com.sun.star.container.XNameAccess interface of
            the name container.

        @param rSuggestedName  Suggested name for the object.

        @return  An unused name. Will be equal to the suggested name, if not
            contained, otherwise a numerical index will be appended.
     */
    static ::rtl::OUString getUnusedName(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& rxNameAccess,
                            const ::rtl::OUString& rSuggestedName,
                            sal_Unicode cSeparator,
                            sal_Int32 nFirstIndexToAppend = 1 );

    /** Inserts an object into a name container.

        @param rxNameContainer  com.sun.star.container.XNameContainer interface
            of the name container.

        @param rName  Exact name for the object.

        @param rObject  The object to be inserted.

        @return  True = object successfully inserted.
     */
    static bool         insertByName(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxNameContainer,
                            const ::rtl::OUString& rName,
                            const ::com::sun::star::uno::Any& rObject,
                            bool bReplaceOldExisting = true );

    /** Inserts an object into a name container.

        The function will use an unused name to insert the object, based on the
        suggested object name. It is possible to specify whether the existing
        object or the new inserted object will be renamed, if the container
        already has an object with the name suggested for the new object.

        @param rxNameContainer  com.sun.star.container.XNameContainer interface
            of the name container.

        @param rSuggestedName  Suggested name for the object.

        @param rObject  The object to be inserted.

        @param bRenameOldExisting  Specifies behaviour if an object with the
            suggested name already exists. If false (default), the new object
            will be inserted with a name not yet extant in the container (this
            is done by appending a numerical index to the suggested name). If
            true, the existing object will be removed and inserted with an
            unused name, and the new object will be inserted with the suggested
            name.

        @return  The final name the object is inserted with. Will always be
            equal to the suggested name, if parameter bRenameOldExisting is
            true.
     */
    static ::rtl::OUString insertByUnusedName(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxNameContainer,
                            const ::rtl::OUString& rSuggestedName,
                            sal_Unicode cSeparator,
                            const ::com::sun::star::uno::Any& rObject,
                            bool bRenameOldExisting = false );

    // std::vector and std::map element access --------------------------------

    /** Returns the pointer to an existing element of the passed vector, or a
        null pointer, if the passed index is out of bounds. */
    template< typename Type >
    static const Type*  getVectorElement( const ::std::vector< Type >& rVector, sal_Int32 nIndex );

    /** Returns the pointer to an existing element of the passed vector, or a
        null pointer, if the passed index is out of bounds. */
    template< typename Type >
    static Type*        getVectorElement( ::std::vector< Type >& rVector, sal_Int32 nIndex );

    /** Returns the reference to an existing element of the passed vector, or
        the passed default value, if the passed index is out of bounds. */
    template< typename Type >
    static const Type&  getVectorElement( const ::std::vector< Type >& rVector, sal_Int32 nIndex, const Type& rDefault );

    /** Returns the reference to an existing element of the passed vector, or
        the passed default value, if the passed index is out of bounds. */
    template< typename Type >
    static Type&        getVectorElement( ::std::vector< Type >& rVector, sal_Int32 nIndex, Type& rDefault );

    /** Returns the pointer to an existing element of the passed map, or a null
        pointer, if an element with the passed key does not exist. */
    template< typename Type, typename KeyType >
    static const Type*  getMapElement( const ::std::map< KeyType, Type >& rMap, KeyType nKey );

    /** Returns the pointer to an existing element of the passed map, or a null
        pointer, if an element with the passed key does not exist. */
    template< typename Type, typename KeyType >
    static Type*        getMapElement( ::std::map< KeyType, Type >& rMap, KeyType nKey );

    /** Returns the reference to an existing element of the passed map, or the
        passed default value, if an element with the passed key does not exist. */
    template< typename Type, typename KeyType >
    static const Type&  getMapElement( const ::std::map< KeyType, Type >& rMap, KeyType nKey, const Type& rDefault );

    /** Returns the reference to an existing element of the passed map, or the
        passed default value, if an element with the passed key does not exist. */
    template< typename Type, typename KeyType >
    static Type&        getMapElement( ::std::map< KeyType, Type >& rMap, KeyType nKey, Type& rDefault );

    // vector/matrix to Sequence ----------------------------------------------

    /** Creates a UNO sequence from a std::vector with copies of all elements.

        @param rVector  The vector to be converted to a sequence.

        @return  A com.sun.star.uno.Sequence object with copies of all objects
            contained in the passed vector.
     */
    template< typename Type >
    static ::com::sun::star::uno::Sequence< Type >
                            vectorToSequence( const ::std::vector< Type >& rVector );

    /** Creates a UNO sequence of sequences from a matrix with copies of all elements.

        @param rMatrix  The matrix to be converted to a sequence of sequences.

        @return  A com.sun.star.uno.Sequence object containing
            com.sun.star.uno.Sequence objects with copies of all objects
            contained in the passed matrix.
     */
    template< typename Type >
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< Type > >
                            matrixToSequenceSequence( const Matrix< Type >& rMatrix );
};

// ----------------------------------------------------------------------------

template< typename Type >
const Type* ContainerHelper::getVectorElement( const ::std::vector< Type >& rVector, sal_Int32 nIndex )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? &rVector[ static_cast< size_t >( nIndex ) ] : 0;
}

template< typename Type >
Type* ContainerHelper::getVectorElement( ::std::vector< Type >& rVector, sal_Int32 nIndex )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? &rVector[ static_cast< size_t >( nIndex ) ] : 0;
}

template< typename Type >
const Type& ContainerHelper::getVectorElement( const ::std::vector< Type >& rVector, sal_Int32 nIndex, const Type& rDefault )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? rVector[ static_cast< size_t >( nIndex ) ] : rDefault;
}

template< typename Type >
Type& ContainerHelper::getVectorElement( ::std::vector< Type >& rVector, sal_Int32 nIndex, Type& rDefault )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? rVector[ static_cast< size_t >( nIndex ) ] : rDefault;
}

template< typename Type, typename KeyType >
const Type* ContainerHelper::getMapElement( const ::std::map< KeyType, Type >& rMap, KeyType nKey )
{
    typename ::std::map< KeyType, Type >::const_iterator aIt = rMap.find( nKey );
    return (aIt == rMap.end()) ? 0 : &aIt->second;
}

template< typename Type, typename KeyType >
Type* ContainerHelper::getMapElement( ::std::map< KeyType, Type >& rMap, KeyType nKey )
{
    typename ::std::map< KeyType, Type >::iterator aIt = rMap.find( nKey );
    return (aIt == rMap.end()) ? 0 : &aIt->second;
}

template< typename Type, typename KeyType >
const Type& ContainerHelper::getMapElement( const ::std::map< KeyType, Type >& rMap, KeyType nKey, const Type& rDefault )
{
    typename ::std::map< KeyType, Type >::const_iterator aIt = rMap.find( nKey );
    return (aIt == rMap.end()) ? rDefault : aIt->second;
}

template< typename Type, typename KeyType >
Type& ContainerHelper::getMapElement( ::std::map< KeyType, Type >& rMap, KeyType nKey, Type& rDefault )
{
    typename ::std::map< KeyType, Type >::iterator aIt = rMap.find( nKey );
    return (aIt == rMap.end()) ? rDefault : aIt->second;
}

template< typename Type >
::com::sun::star::uno::Sequence< Type > ContainerHelper::vectorToSequence( const ::std::vector< Type >& rVector )
{
    if( rVector.empty() )
        return ::com::sun::star::uno::Sequence< Type >();
    return ::com::sun::star::uno::Sequence< Type >( &rVector.front(), static_cast< sal_Int32 >( rVector.size() ) );
}

template< typename Type >
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< Type > > ContainerHelper::matrixToSequenceSequence( const Matrix< Type >& rMatrix )
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< Type > > aSeq;
    if( !rMatrix.empty() )
    {
        aSeq.realloc( static_cast< sal_Int32 >( rMatrix.height() ) );
        for( size_t nRow = 0, nHeight = rMatrix.height(); nRow < nHeight; ++nRow )
            aSeq[ static_cast< sal_Int32 >( nRow ) ] =
                ::com::sun::star::uno::Sequence< Type >( &rMatrix.row_front( nRow ), static_cast< sal_Int32 >( rMatrix.width() ) );
    }
    return aSeq;
}

// ============================================================================

/** This helper manages named objects in a container, which is created on demand.
 */
class ObjectContainer
{
public:
    explicit            ObjectContainer(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const ::rtl::OUString& rServiceName );
                        ~ObjectContainer();

    /** Returns true, if the object with the passed name exists in the container. */
    bool                hasObject( const ::rtl::OUString& rObjName ) const;

    /** Returns the object with the passed name from the container. */
    ::com::sun::star::uno::Any getObject( const ::rtl::OUString& rObjName ) const;

    /** Inserts the passed object into the container, returns its final name. */
    ::rtl::OUString     insertObject(
                            const ::rtl::OUString& rObjName,
                            const ::com::sun::star::uno::Any& rObj,
                            bool bInsertByUnusedName );

private:
    void                createContainer() const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        mxFactory;              /// Factory to create the container.
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        mxContainer;            /// Container for the objects.
    ::rtl::OUString     maServiceName;          /// Service name to create the container.
    sal_Int32           mnIndex;                /// Index to create unique identifiers.
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
