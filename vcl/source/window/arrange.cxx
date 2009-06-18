/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accel.hxx,v $
 * $Revision: 1.3 $
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

#include "precompiled_vcl.hxx"

#include "vcl/arrange.hxx"
#include "vcl/edit.hxx"

#include "osl/diagnose.h"

using namespace vcl;

// ----------------------------------------
// vcl::WindowArranger
//-----------------------------------------

WindowArranger::~WindowArranger()
{}

void WindowArranger::setParent( WindowArranger* i_pParent )
{
    OSL_VERIFY( i_pParent->m_pParentWindow == m_pParentWindow || m_pParentWindow == NULL );

    m_pParentArranger = i_pParent;
    m_pParentWindow = i_pParent->m_pParentWindow;
    setParentWindow( m_pParentWindow );
}

sal_Int32 WindowArranger::Element::getExpandPriority() const
{
    sal_Int32 nPrio = m_nExpandPriority;
    if( m_pChild )
    {
        size_t nElements = m_pChild->countElements();
        for( size_t i = 0; i < nElements; i++ )
        {
            sal_Int32 nCPrio = m_pChild->getExpandPriority( i );
            if( nCPrio > nPrio )
                nPrio = nCPrio;
        }
    }
    return nPrio;
}

// ----------------------------------------
// vcl::RowOrColumn
//-----------------------------------------

RowOrColumn::~RowOrColumn()
{
    for( std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
         it != m_aElements.end(); ++it )
    {
        it->deleteChild();
    }
}

boost::shared_ptr<WindowArranger> RowOrColumn::getChild( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].m_pChild : boost::shared_ptr<WindowArranger>();
}

Window* RowOrColumn::getWindow( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].m_pElement : NULL;
}

sal_Int32 RowOrColumn::getExpandPriority( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].getExpandPriority() : 0;
}

Size RowOrColumn::getOptimalSize( WindowSizeType i_eType ) const
{
    Size aRet( 0, 0 );
    if( ! m_aElements.empty() )
    {
        if( m_bColumn )
            aRet.Height() -= m_nBorderWidth;
        else
            aRet.Width() -= m_nBorderWidth;
    }

    for( std::vector< WindowArranger::Element >::const_iterator it = m_aElements.begin();
         it != m_aElements.end(); ++it )
    {
        // get the size of type of the managed element
        Size aElementSize( it->m_pElement
                           ? it->m_pElement->GetOptimalSize( i_eType )
                           : it->m_pChild
                             ? it->m_pChild->getOptimalSize( i_eType )
                             : Size() );
        if( m_bColumn )
        {
            // add the distance between elements
            aRet.Height() += m_nBorderWidth;
            // check if the width needs adjustment
            if( aRet.Width() < aElementSize.Width() )
                aRet.Width() = aElementSize.Width();
            aRet.Height() += aElementSize.Height();
        }
        else
        {
            // add the distance between elements
            aRet.Width() += m_nBorderWidth;
            // check if the height needs adjustment
            if( aRet.Height() < aElementSize.Height() )
                aRet.Height() = aElementSize.Height();
            aRet.Width() += aElementSize.Width();
        }
    }

    // add the outer border
    aRet.Width() += 2*m_nOuterBorder;
    aRet.Height() += 2*m_nOuterBorder;

    return aRet;
}

void RowOrColumn::distributeRowWidth( std::vector<Size>& io_rSizes, long /*i_nUsedWidth*/, long i_nExtraWidth )
{
    if( ! io_rSizes.empty() && io_rSizes.size() == m_aElements.size() )
    {
        // find all elements with the highest expand priority
        size_t nElements = m_aElements.size();
        std::vector< size_t > aIndices;
        sal_Int32 nHighPrio = 0;
        for( size_t i = 0; i < nElements; i++ )
        {
            sal_Int32 nCurPrio = m_aElements[ i ].getExpandPriority();
            if( nCurPrio > nHighPrio )
            {
                aIndices.clear();
                nHighPrio = nCurPrio;
            }
            if( nCurPrio == nHighPrio )
                aIndices.push_back( i );
        }

        // distribute extra space evenly among elements
        nElements = aIndices.size();
        if( nElements > 0 ) // sanity check
        {
            long nDelta = i_nExtraWidth / nElements;
            for( size_t i = 0; i < nElements; i++ )
            {
                io_rSizes[ aIndices[i] ].Width() += nDelta;
                i_nExtraWidth -= nDelta;
            }
            // add the last pixels to the last row element
            if( i_nExtraWidth > 0 && nElements > 0 )
                io_rSizes[aIndices.back()].Width() += i_nExtraWidth;
        }
    }
}

void RowOrColumn::resize()
{
    // check if we can get optimal size, else fallback to minimal size
    Size aOptSize( getOptimalSize( WINDOWSIZE_PREFERRED ) );
    WindowSizeType eType = WINDOWSIZE_PREFERRED;
    if( m_bColumn )
    {
        if( aOptSize.Height() > m_aManagedArea.GetHeight() )
            eType = WINDOWSIZE_MINIMUM;
    }
    else
    {
        if( aOptSize.Width() > m_aManagedArea.GetWidth() )
            eType = WINDOWSIZE_MINIMUM;
    }

    size_t nElements = m_aElements.size();
    // get all element sizes for sizing
    std::vector<Size> aElementSizes( nElements );
    long nUsedWidth = 2*m_nOuterBorder - (nElements ? m_nBorderWidth : 0);
    for( size_t i = 0; i < nElements; i++ )
    {
        aElementSizes[i] = m_aElements[i].m_pElement
                           ? m_aElements[i].m_pElement->GetOptimalSize( eType )
                           : m_aElements[i].m_pChild
                             ? m_aElements[i].m_pChild->getOptimalSize( eType )
                             : Size();
        if( m_bColumn )
        {
            aElementSizes[i].Width() = m_aManagedArea.GetWidth() - 2* m_nOuterBorder;
            nUsedWidth += aElementSizes[i].Height() + m_nBorderWidth;
        }
        else
        {
            aElementSizes[i].Height() = m_aManagedArea.GetHeight() - 2* m_nOuterBorder;
            nUsedWidth += aElementSizes[i].Width() + m_nBorderWidth;
        }
    }

    long nExtraWidth = m_aManagedArea.GetWidth() - nUsedWidth;
    if( nExtraWidth > 0 )
        if( ! m_bColumn )
            distributeRowWidth( aElementSizes, nUsedWidth, nExtraWidth );

    // get starting position
    Point aElementPos( m_aManagedArea.TopLeft() );
    // outer border
    aElementPos.X() += m_nOuterBorder;
    aElementPos.Y() += m_nOuterBorder;

    // position managed windows
    for( size_t i = 0; i < nElements; i++ )
    {
        // get the size of type of the managed element

        if( m_aElements[i].m_pElement ) // this actually is a window
        {
            m_aElements[i].m_pElement->SetPosSizePixel( aElementPos, aElementSizes[i] );
        }
        else if( m_aElements[i].m_pChild )
        {
            m_aElements[i].m_pChild->setManagedArea( Rectangle( aElementPos, aElementSizes[i] ) );
        }
        if( m_bColumn )
            aElementPos.Y() += m_nBorderWidth + aElementSizes[i].Height();
        else
            aElementPos.X() += m_nBorderWidth + aElementSizes[i].Width();
    }
}

void RowOrColumn::setParentWindow( Window* i_pNewParent )
{
    m_pParentWindow = i_pNewParent;
    for( std::vector< WindowArranger::Element >::const_iterator it = m_aElements.begin();
         it != m_aElements.end(); ++it )
    {
        #if OSL_DEBUG_LEVEL > 0
        if( it->m_pElement )
        {
            OSL_VERIFY( it->m_pElement->GetParent() == i_pNewParent );
        }
        #endif
        if( it->m_pChild )
            it->m_pChild->setParentWindow( i_pNewParent );
    }
}

void RowOrColumn::addWindow( Window* i_pWindow, sal_Int32 i_nExpandPrio, sal_Int32 i_nIndex )
{
    if( i_nIndex < 0 || size_t(i_nIndex) >= m_aElements.size() )
        m_aElements.push_back( WindowArranger::Element( i_pWindow, boost::shared_ptr<WindowArranger>(), i_nExpandPrio ) );
    else
    {
        std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
        while( i_nIndex-- )
            ++it;
        m_aElements.insert( it, WindowArranger::Element( i_pWindow, boost::shared_ptr<WindowArranger>(), i_nExpandPrio ) );
    }
}

void RowOrColumn::addChild( boost::shared_ptr<WindowArranger> const & i_pChild, sal_Int32 i_nExpandPrio, sal_Int32 i_nIndex )
{
    if( i_nIndex < 0 || size_t(i_nIndex) >= m_aElements.size() )
        m_aElements.push_back( WindowArranger::Element( NULL, i_pChild, i_nExpandPrio ) );
    else
    {
        std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
        while( i_nIndex-- )
            ++it;
        m_aElements.insert( it, WindowArranger::Element( NULL, i_pChild, i_nExpandPrio ) );
    }
}

void RowOrColumn::remove( Window* i_pWindow )
{
    if( i_pWindow )
    {
        for( std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
        {
            if( it->m_pElement == i_pWindow )
            {
                m_aElements.erase( it );
                return;
            }
        }
    }
}

void RowOrColumn::remove( boost::shared_ptr<WindowArranger> const & i_pChild )
{
    if( i_pChild )
    {
        for( std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
        {
            if( it->m_pChild == i_pChild )
            {
                m_aElements.erase( it );
                return;
            }
        }
    }
}

// ----------------------------------------
// vcl::Indenter
//-----------------------------------------

Indenter::~Indenter()
{
    m_aElement.deleteChild();
}

Size Indenter::getOptimalSize( WindowSizeType i_eType ) const
{
    Size aSize( m_aElement.m_pElement
                ? m_aElement.m_pElement->GetOptimalSize( i_eType )
                : m_aElement.m_pChild
                  ? m_aElement.m_pChild->getOptimalSize( i_eType )
                  : Size() );
    aSize.Width()  += 2*m_nOuterBorder + m_nIndent;
    aSize.Height() += 2*m_nOuterBorder;
    return aSize;
}

void Indenter::resize()
{
    Point aPt( m_aManagedArea.TopLeft() );
    aPt.X() += m_nOuterBorder + m_nIndent;
    aPt.Y() += m_nOuterBorder;
    Size aSz( m_aManagedArea.GetSize() );
    aSz.Width()  -= 2*m_nOuterBorder - m_nIndent;
    aSz.Height() -= 2*m_nOuterBorder;
    if( m_aElement.m_pElement )
        m_aElement.m_pElement->SetPosSizePixel( aPt, aSz );
    else if( m_aElement.m_pChild )
        m_aElement.m_pChild->setManagedArea( Rectangle( aPt, aSz ) );
}

void Indenter::setWindow( Window* i_pWindow, sal_Int32 i_nExpandPrio )
{
    OSL_VERIFY( (m_aElement.m_pElement == 0 && m_aElement.m_pChild == 0) || i_pWindow == 0 );
    OSL_VERIFY( i_pWindow == 0 || i_pWindow->GetParent() == m_pParentWindow );
    m_aElement.m_pElement = i_pWindow;
    m_aElement.m_nExpandPriority = i_nExpandPrio;
}

void Indenter::setChild( boost::shared_ptr<WindowArranger> const & i_pChild, sal_Int32 i_nExpandPrio )
{
    OSL_VERIFY( (m_aElement.m_pElement == 0 && m_aElement.m_pChild == 0 ) || i_pChild == 0 );
    m_aElement.m_pChild = i_pChild;
    m_aElement.m_nExpandPriority = i_nExpandPrio;
}

void Indenter::setParentWindow( Window* i_pNewParent )
{
    m_pParentWindow = i_pNewParent;
    #if OSL_DEBUG_LEVEL > 0
    if( m_aElement.m_pElement )
    {
        OSL_VERIFY( m_aElement.m_pElement->GetParent() == i_pNewParent );
    }
    #endif
    if( m_aElement.m_pChild )
        m_aElement.m_pChild->setParentWindow( i_pNewParent );
}

// ----------------------------------------
// vcl::MatrixArranger
//-----------------------------------------
MatrixArranger::~MatrixArranger()
{
}

Size MatrixArranger::getOptimalSize( WindowSizeType i_eType, std::vector<long>& o_rColumnWidths, std::vector<long>& o_rRowHeights ) const
{
    Size aMatrixSize( 2*m_nOuterBorder, 2*m_nOuterBorder );

    // first find out the current number of rows and columns
    sal_uInt32 nRows = 0, nColumns = 0;
    for( std::vector< MatrixElement >::const_iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
    {
        if( it->m_nX >= nColumns )
            nColumns = it->m_nX+1;
        if( it->m_nY >= nRows )
            nRows = it->m_nY+1;
    }

    // now allocate row and column depth vectors
    o_rColumnWidths = std::vector< long >( nColumns, 0 );
    o_rRowHeights   = std::vector< long >( nRows, 0 );

    // get sizes an allocate them into rows/columns
    for( std::vector< MatrixElement >::const_iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
    {
        Size aSize;
        if( it->m_pElement )
            aSize = it->m_pElement->GetOptimalSize( i_eType );
        else if( it->m_pChild )
            aSize = it->m_pChild->getOptimalSize( i_eType );
        if( aSize.Width() > o_rColumnWidths[ it->m_nX ] )
            o_rColumnWidths[ it->m_nX ] = aSize.Width();
        if( aSize.Height() > o_rRowHeights[ it->m_nY ] )
            o_rRowHeights[ it->m_nY ] = aSize.Height();
    }

    // add up sizes
    for( sal_uInt32 i = 0; i < nColumns; i++ )
        aMatrixSize.Width() += o_rColumnWidths[i] + m_nBorderX;
    if( nColumns > 0 )
        aMatrixSize.Width() -= m_nBorderX;

    for( sal_uInt32 i = 0; i < nRows; i++ )
        aMatrixSize.Height() += o_rRowHeights[i] + m_nBorderY;
    if( nRows > 0 )
        aMatrixSize.Height() -= m_nBorderY;

    return aMatrixSize;
}

Size MatrixArranger::getOptimalSize( WindowSizeType i_eType ) const
{
    std::vector<long> aColumnWidths, aRowHeights;
    return getOptimalSize( i_eType, aColumnWidths, aRowHeights );
}

void MatrixArranger::resize()
{
    // assure that we have at least one row and column
    if( m_aElements.empty() )
        return;

    // check if we can get optimal size, else fallback to minimal size
    std::vector<long> aColumnWidths, aRowHeights;
    Size aOptSize( getOptimalSize( WINDOWSIZE_PREFERRED, aColumnWidths, aRowHeights ) );
    if( aOptSize.Height() > m_aManagedArea.GetHeight() ||
        aOptSize.Width() > m_aManagedArea.GetWidth() )
    {
        std::vector<long> aMinColumnWidths, aMinRowHeights;
        getOptimalSize( WINDOWSIZE_MINIMUM, aMinColumnWidths, aMinRowHeights );
        if( aOptSize.Height() > m_aManagedArea.GetHeight() )
            aRowHeights = aMinRowHeights;
        if( aOptSize.Width() > m_aManagedArea.GetWidth() )
            aColumnWidths = aMinColumnWidths;
    }

    // FIXME: distribute extra space available

    // prepare offsets
    std::vector<long> aColumnX( aColumnWidths.size() );
    aColumnX[0] = m_aManagedArea.Left() + m_nOuterBorder;
    for( size_t i = 1; i < aColumnX.size(); i++ )
        aColumnX[i] = aColumnX[i-1] + aColumnWidths[i-1] + m_nBorderX;

    std::vector<long> aRowY( aRowHeights.size() );
    aRowY[0] = m_aManagedArea.Top() + m_nOuterBorder;
    for( size_t i = 1; i < aRowY.size(); i++ )
        aRowY[i] = aRowY[i-1] + aRowHeights[i-1] + m_nBorderY;

    // now iterate over the elements and assign their positions
    for( std::vector< MatrixElement >::iterator it = m_aElements.begin();
         it != m_aElements.end(); ++it )
    {
        Point aCellPos( aColumnX[it->m_nX], aRowY[it->m_nY] );
        Size  aCellSize( aColumnWidths[it->m_nX], aRowHeights[it->m_nY] );
        if( it->m_pElement )
            it->m_pElement->SetPosSizePixel( aCellPos, aCellSize );
        else if( it->m_pChild )
            it->m_pChild->setManagedArea( Rectangle( aCellPos, aCellSize ) );
    }
}

void MatrixArranger::setParentWindow( Window* i_pNewParent )
{
    m_pParentWindow = i_pNewParent;
    for( std::vector< MatrixElement >::const_iterator it = m_aElements.begin();
         it != m_aElements.end(); ++it )
    {
        #if OSL_DEBUG_LEVEL > 0
        if( it->m_pElement )
        {
            OSL_VERIFY( it->m_pElement->GetParent() == i_pNewParent );
        }
        #endif
        if( it->m_pChild )
            it->m_pChild->setParentWindow( i_pNewParent );
    }
}

boost::shared_ptr<WindowArranger> MatrixArranger::getChild( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].m_pChild : boost::shared_ptr<WindowArranger>();
}

Window* MatrixArranger::getWindow( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].m_pElement : NULL;
}

sal_Int32 MatrixArranger::getExpandPriority( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].getExpandPriority() : 0;
}

void MatrixArranger::addWindow( Window* i_pWindow, sal_uInt32 i_nX, sal_uInt32 i_nY, sal_Int32 i_nExpandPrio )
{
    sal_uInt64 nMapValue = getMap( i_nX, i_nY );
    std::map< sal_uInt64, size_t >::const_iterator it = m_aMatrixMap.find( nMapValue );
    if( it == m_aMatrixMap.end() )
    {
        m_aMatrixMap[ nMapValue ] = m_aElements.size();
        m_aElements.push_back( MatrixElement( i_pWindow, i_nX, i_nY, boost::shared_ptr<WindowArranger>(), i_nExpandPrio ) );
    }
    else
    {
        MatrixElement& rEle( m_aElements[ it->second ] );
        rEle.m_pElement = i_pWindow;
        rEle.m_pChild.reset();
        rEle.m_nExpandPriority = i_nExpandPrio;
        rEle.m_nX = i_nX;
        rEle.m_nY = i_nY;
    }
}

void MatrixArranger::remove( Window* i_pWindow )
{
    if( i_pWindow )
    {
        for( std::vector< MatrixElement >::iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
        {
            if( it->m_pElement == i_pWindow )
            {
                m_aMatrixMap.erase( getMap( it->m_nX, it->m_nY ) );
                m_aElements.erase( it );
                return;
            }
        }
    }
}

void MatrixArranger::addChild( boost::shared_ptr<WindowArranger> const &i_pChild, sal_uInt32 i_nX, sal_uInt32 i_nY, sal_Int32 i_nExpandPrio )
{
    sal_uInt64 nMapValue = getMap( i_nX, i_nY );
    std::map< sal_uInt64, size_t >::const_iterator it = m_aMatrixMap.find( nMapValue );
    if( it == m_aMatrixMap.end() )
    {
        m_aMatrixMap[ nMapValue ] = m_aElements.size();
        m_aElements.push_back( MatrixElement( NULL, i_nX, i_nY, i_pChild, i_nExpandPrio ) );
    }
    else
    {
        MatrixElement& rEle( m_aElements[ it->second ] );
        rEle.m_pElement = 0;
        rEle.m_pChild = i_pChild;
        rEle.m_nExpandPriority = i_nExpandPrio;
        rEle.m_nX = i_nX;
        rEle.m_nY = i_nY;
    }
}

void MatrixArranger::remove( boost::shared_ptr<WindowArranger> const &i_pChild )
{
    if( i_pChild )
    {
        for( std::vector< MatrixElement >::iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
        {
            if( it->m_pChild == i_pChild )
            {
                m_aMatrixMap.erase( getMap( it->m_nX, it->m_nY ) );
                m_aElements.erase( it );
                return;
            }
        }
    }
}

