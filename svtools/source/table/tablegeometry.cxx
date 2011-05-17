/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "tablegeometry.hxx"
#include "tablecontrol_impl.hxx"

#include <tools/debug.hxx>

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    //==================================================================================================================
    //= TableRowGeometry
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TableRowGeometry::TableRowGeometry( TableControl_Impl const & _rControl, Rectangle const & _rBoundaries,
            RowPos const _nRow, bool const i_allowVirtualRows )
        :TableGeometry( _rControl, _rBoundaries )
        ,m_nRowPos( _nRow )
        ,m_bAllowVirtualRows( i_allowVirtualRows )
    {
        if ( m_nRowPos == ROW_COL_HEADERS )
        {
            m_aRect.Top() = 0;
            m_aRect.Bottom() = m_rControl.m_nColHeaderHeightPixel - 1;
        }
        else
        {
            impl_initRect();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableRowGeometry::impl_initRect()
    {
        if ( ( m_nRowPos >= m_rControl.m_nTopRow ) && impl_isValidRow( m_nRowPos ) )
        {
            m_aRect.Top() = m_rControl.m_nColHeaderHeightPixel + ( m_nRowPos - m_rControl.m_nTopRow ) * m_rControl.m_nRowHeightPixel;
            m_aRect.Bottom() = m_aRect.Top() + m_rControl.m_nRowHeightPixel - 1;
        }
        else
            m_aRect.SetEmpty();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableRowGeometry::impl_isValidRow( RowPos const i_row ) const
    {
        return m_bAllowVirtualRows || ( i_row < m_rControl.m_pModel->getRowCount() );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableRowGeometry::moveDown()
    {
        if ( m_nRowPos == ROW_COL_HEADERS )
        {
            m_nRowPos = m_rControl.m_nTopRow;
            impl_initRect();
        }
        else
        {
            if ( impl_isValidRow( ++m_nRowPos ) )
                m_aRect.Move( 0, m_rControl.m_nRowHeightPixel );
            else
                m_aRect.SetEmpty();
        }
        return isValid();
    }

    //==================================================================================================================
    //= TableColumnGeometry
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    TableColumnGeometry::TableColumnGeometry( TableControl_Impl const & _rControl, Rectangle const & _rBoundaries,
            ColPos const _nCol, bool const i_allowVirtualColumns )
        :TableGeometry( _rControl, _rBoundaries )
        ,m_nColPos( _nCol )
        ,m_bAllowVirtualColumns( i_allowVirtualColumns )
    {
        if ( m_nColPos == COL_ROW_HEADERS )
        {
            m_aRect.Left() = 0;
            m_aRect.Right() = m_rControl.m_nRowHeaderWidthPixel - 1;
        }
        else
        {
            impl_initRect();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableColumnGeometry::impl_initRect()
    {
        ColPos nLeftColumn = m_rControl.m_nLeftColumn;
        if ( ( m_nColPos >= nLeftColumn ) && impl_isValidColumn( m_nColPos ) )
        {
            m_aRect.Left() = m_rControl.m_nRowHeaderWidthPixel;
            // TODO: take into account any possibly frozen columns

            for ( ColPos col = nLeftColumn; col < m_nColPos; ++col )
                m_aRect.Left() += m_rControl.m_aColumnWidths[ col ].getWidth();
            m_aRect.Right() = m_aRect.Left() + m_rControl.m_aColumnWidths[ m_nColPos ].getWidth() - 1;
        }
        else
            m_aRect.SetEmpty();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableColumnGeometry::impl_isValidColumn( ColPos const i_column ) const
    {
        return m_bAllowVirtualColumns || ( i_column < ColPos( m_rControl.m_aColumnWidths.size() ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableColumnGeometry::moveRight()
    {
        if ( m_nColPos == COL_ROW_HEADERS )
        {
            m_nColPos = m_rControl.m_nLeftColumn;
            impl_initRect();
        }
        else
        {
            if ( impl_isValidColumn( ++m_nColPos ) )
            {
                m_aRect.Left() = m_aRect.Right() + 1;
                m_aRect.Right() += m_rControl.m_aColumnWidths[ m_nColPos ].getWidth();
            }
            else
                m_aRect.SetEmpty();
        }

        return isValid();
    }

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
