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


#include "tablegeometry.hxx"
#include "tablecontrol_impl.hxx"


namespace svt { namespace table
{



    //= TableRowGeometry


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


    bool TableRowGeometry::impl_isValidRow( RowPos const i_row ) const
    {
        return m_bAllowVirtualRows || ( i_row < m_rControl.m_pModel->getRowCount() );
    }


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


    //= TableColumnGeometry


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


    bool TableColumnGeometry::impl_isValidColumn( ColPos const i_column ) const
    {
        return m_bAllowVirtualColumns || ( i_column < ColPos( m_rControl.m_aColumnWidths.size() ) );
    }


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


} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
