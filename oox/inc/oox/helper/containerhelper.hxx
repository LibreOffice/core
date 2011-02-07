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
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

namespace rtl { class OUString; }

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace container { class XIndexContainer; }
    namespace container { class XNameAccess; }
    namespace container { class XNameContainer; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {

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
                        createIndexContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

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
                        createNameContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

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
    template< typename VectorType >
    static const typename VectorType::value_type*
                        getVectorElement( const VectorType& rVector, sal_Int32 nIndex );

    /** Returns the pointer to an existing element of the passed vector, or a
        null pointer, if the passed index is out of bounds. */
    template< typename VectorType >
    static typename VectorType::value_type*
                        getVectorElementAccess( VectorType& rVector, sal_Int32 nIndex );

    /** Returns the reference to an existing element of the passed vector, or
        the passed default value, if the passed index is out of bounds. */
    template< typename VectorType >
    static const typename VectorType::value_type&
                        getVectorElement( const VectorType& rVector, sal_Int32 nIndex, const typename VectorType::value_type& rDefault );

    /** Returns the reference to an existing element of the passed vector, or
        the passed default value, if the passed index is out of bounds. */
    template< typename VectorType >
    static typename VectorType::value_type&
                        getVectorElementAccess( VectorType& rVector, sal_Int32 nIndex, typename VectorType::value_type& rDefault );

    /** Returns the pointer to an existing element of the passed map, or a null
        pointer, if an element with the passed key does not exist. */
    template< typename MapType >
    static const typename MapType::mapped_type*
                        getMapElement( const MapType& rMap, const typename MapType::key_type& rKey );

    /** Returns the pointer to an existing element of the passed map, or a null
        pointer, if an element with the passed key does not exist. */
    template< typename MapType >
    static typename MapType::mapped_type*
                        getMapElementAccess( MapType& rMap, const typename MapType::key_type& rKey );

    /** Returns the reference to an existing element of the passed map, or the
        passed default value, if an element with the passed key does not exist. */
    template< typename MapType >
    static const typename MapType::mapped_type&
                        getMapElement( const MapType& rMap, const typename MapType::key_type& rKey, const typename MapType::mapped_type& rDefault );

    /** Returns the reference to an existing element of the passed map, or the
        passed default value, if an element with the passed key does not exist. */
    template< typename MapType >
    static typename MapType::mapped_type&
                        getMapElementAccess( MapType& rMap, const typename MapType::key_type& rKey, typename MapType::mapped_type& rDefault );

    // vector/map/matrix to UNO sequence --------------------------------------

    /** Creates a UNO sequence from a std::vector with copies of all elements.

        @param rVector  The vector to be converted to a sequence.

        @return  A com.sun.star.uno.Sequence object with copies of all objects
            contained in the passed vector.
     */
    template< typename VectorType >
    static ::com::sun::star::uno::Sequence< typename VectorType::value_type >
                            vectorToSequence( const VectorType& rVector );

    /** Creates a UNO sequence from a std::map with copies of all elements.

        @param rMap  The map to be converted to a sequence.

        @return  A com.sun.star.uno.Sequence object with copies of all objects
            contained in the passed map.
     */
    template< typename MapType >
    static ::com::sun::star::uno::Sequence< typename MapType::mapped_type >
                            mapToSequence( const MapType& rMap );

    /** Creates a UNO sequence of sequences from a matrix with copies of all elements.

        @param rMatrix  The matrix to be converted to a sequence of sequences.

        @return  A com.sun.star.uno.Sequence object containing
            com.sun.star.uno.Sequence objects with copies of all objects
            contained in the passed matrix.
     */
    template< typename MatrixType >
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< typename MatrixType::value_type > >
                            matrixToSequenceSequence( const MatrixType& rMatrix );
};

// ----------------------------------------------------------------------------

template< typename VectorType >
/*static*/ const typename VectorType::value_type* ContainerHelper::getVectorElement( const VectorType& rVector, sal_Int32 nIndex )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? &rVector[ static_cast< size_t >( nIndex ) ] : 0;
}

template< typename VectorType >
/*static*/ typename VectorType::value_type* ContainerHelper::getVectorElementAccess( VectorType& rVector, sal_Int32 nIndex )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? &rVector[ static_cast< size_t >( nIndex ) ] : 0;
}

template< typename VectorType >
/*static*/ const typename VectorType::value_type& ContainerHelper::getVectorElement( const VectorType& rVector, sal_Int32 nIndex, const typename VectorType::value_type& rDefault )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? rVector[ static_cast< size_t >( nIndex ) ] : rDefault;
}

template< typename VectorType >
/*static*/ typename VectorType::value_type& ContainerHelper::getVectorElementAccess( VectorType& rVector, sal_Int32 nIndex, typename VectorType::value_type& rDefault )
{
    return ((0 <= nIndex) && (static_cast< size_t >( nIndex ) < rVector.size())) ? rVector[ static_cast< size_t >( nIndex ) ] : rDefault;
}

template< typename MapType >
/*static*/ const typename MapType::mapped_type* ContainerHelper::getMapElement( const MapType& rMap, const typename MapType::key_type& rKey )
{
    typename MapType::const_iterator aIt = rMap.find( rKey );
    return (aIt == rMap.end()) ? 0 : &aIt->second;
}

template< typename MapType >
/*static*/ typename MapType::mapped_type* ContainerHelper::getMapElementAccess( MapType& rMap, const typename MapType::key_type& rKey )
{
    typename MapType::iterator aIt = rMap.find( rKey );
    return (aIt == rMap.end()) ? 0 : &aIt->second;
}

template< typename MapType >
/*static*/ const typename MapType::mapped_type& ContainerHelper::getMapElement( const MapType& rMap, const typename MapType::key_type& rKey, const typename MapType::mapped_type& rDefault )
{
    typename MapType::const_iterator aIt = rMap.find( rKey );
    return (aIt == rMap.end()) ? rDefault : aIt->second;
}

template< typename MapType >
/*static*/ typename MapType::mapped_type& ContainerHelper::getMapElementAccess( MapType& rMap, const typename MapType::key_type& rKey, typename MapType::mapped_type& rDefault )
{
    typename MapType::iterator aIt = rMap.find( rKey );
    return (aIt == rMap.end()) ? rDefault : aIt->second;
}

template< typename VectorType >
/*static*/ ::com::sun::star::uno::Sequence< typename VectorType::value_type > ContainerHelper::vectorToSequence( const VectorType& rVector )
{
    typedef typename VectorType::value_type ValueType;
    if( rVector.empty() )
        return ::com::sun::star::uno::Sequence< ValueType >();
    return ::com::sun::star::uno::Sequence< ValueType >( &rVector.front(), static_cast< sal_Int32 >( rVector.size() ) );
}

template< typename MapType >
/*static*/ ::com::sun::star::uno::Sequence< typename MapType::mapped_type > ContainerHelper::mapToSequence( const MapType& rMap )
{
    typedef typename MapType::mapped_type ValueType;
    if( rMap.empty() )
        return ::com::sun::star::uno::Sequence< ValueType >();
    ::com::sun::star::uno::Sequence< ValueType > aSeq( static_cast< sal_Int32 >( rMap.size() ) );
    sal_Int32 nIndex = 0;
    for( typename MapType::const_iterator aIt = rMap.begin(), aEnd = rMap.end(); aIt != aEnd; ++aIt, ++nIndex )
        aSeq[ nIndex ] = *aIt;
    return aSeq;
}

template< typename MatrixType >
/*static*/ ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< typename MatrixType::value_type > > ContainerHelper::matrixToSequenceSequence( const MatrixType& rMatrix )
{
    typedef typename MatrixType::value_type ValueType;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ValueType > > aSeq;
    if( !rMatrix.empty() )
    {
        aSeq.realloc( static_cast< sal_Int32 >( rMatrix.height() ) );
        for( size_t nRow = 0, nHeight = rMatrix.height(); nRow < nHeight; ++nRow )
            aSeq[ static_cast< sal_Int32 >( nRow ) ] =
                ::com::sun::star::uno::Sequence< ValueType >( &rMatrix.row_front( nRow ), static_cast< sal_Int32 >( rMatrix.width() ) );
    }
    return aSeq;
}

// ============================================================================

} // namespace oox

#endif
