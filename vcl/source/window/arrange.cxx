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

#include "vcl/arrange.hxx"

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

WindowArranger* RowOrColumn::getChild( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].m_pChild.get() : NULL;
}

Window* RowOrColumn::getWindow( size_t i_nIndex ) const
{
    return i_nIndex < m_aElements.size() ? m_aElements[i_nIndex].m_pElement : NULL;
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
    if( ! io_rSizes.empty() )
    {
        // distribute extra space evenly among elements
        size_t nElements = io_rSizes.size();
        long nDelta = i_nExtraWidth / nElements;
        for( size_t i = 0; i < nElements; i++ )
        {
            io_rSizes[i].Width() += nDelta;
            i_nExtraWidth -= nDelta;
        }
        // add the last pixels to the last row element
        if( i_nExtraWidth > 0 && nElements > 0 )
            io_rSizes.back().Width() += i_nExtraWidth;
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

void RowOrColumn::addWindow( Window* i_pWindow, sal_Int32 i_nIndex )
{
    if( i_nIndex < 0 || size_t(i_nIndex) >= m_aElements.size() )
        m_aElements.push_back( WindowArranger::Element( i_pWindow, NULL ) );
    else
    {
        std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
        while( i_nIndex-- )
            ++it;
        m_aElements.insert( it, WindowArranger::Element( i_pWindow, NULL ) );
    }
}

void RowOrColumn::addChild( WindowArranger* i_pChild, sal_Int32 i_nIndex )
{
    if( i_nIndex < 0 || size_t(i_nIndex) >= m_aElements.size() )
        m_aElements.push_back( WindowArranger::Element( NULL, i_pChild ) );
    else
    {
        std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
        while( i_nIndex-- )
            ++it;
        m_aElements.insert( it, WindowArranger::Element( NULL, i_pChild ) );
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

void RowOrColumn::remove( WindowArranger* i_pChild )
{
    if( i_pChild )
    {
        for( std::vector< WindowArranger::Element >::iterator it = m_aElements.begin();
            it != m_aElements.end(); ++it )
        {
            if( it->m_pChild.get() == i_pChild )
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

void Indenter::setWindow( Window* i_pWindow )
{
    OSL_VERIFY( (m_aElement.m_pElement == 0 && m_aElement.m_pChild == 0) || i_pWindow == 0 );
    OSL_VERIFY( i_pWindow == 0 || i_pWindow->GetParent() == m_pParentWindow );
    m_aElement.m_pElement = i_pWindow;
}

void Indenter::setChild( WindowArranger* i_pChild )
{
    OSL_VERIFY( (m_aElement.m_pElement == 0 && m_aElement.m_pChild == 0 ) || i_pChild == 0 );
    m_aElement.m_pChild.reset( i_pChild );
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
