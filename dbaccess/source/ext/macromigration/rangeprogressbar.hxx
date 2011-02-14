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

#ifndef DBACCESS_RANGEPROGRESSBAR_HXX
#define DBACCESS_RANGEPROGRESSBAR_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svtools/prgsbar.hxx>

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= RangeProgressBar
    //====================================================================
    /** a slight extension of the usual progress bar, which is able to remember a range
    */
    class RangeProgressBar : public ProgressBar
    {
    public:
        RangeProgressBar( Window* _pParent, WinBits nWinBits = WB_STDPROGRESSBAR )
            :ProgressBar( _pParent, nWinBits )
        {
        }

        RangeProgressBar( Window* _pParent, const ResId& rResId )
            :ProgressBar( _pParent, rResId )
        {
        }

        inline  void        SetRange( sal_uInt32 _nRange );
        inline  sal_uInt32  GetRange() const;

        inline  void        SetValue( sal_uInt32 _nValue );
        inline  sal_uInt32  GetValue() const;

    private:
        sal_uInt32  m_nRange;

    private:
        using ProgressBar::SetValue;
        using ProgressBar::GetValue;
    };

    //--------------------------------------------------------------------
    inline void RangeProgressBar::SetRange( sal_uInt32 _nRange )
    {
        m_nRange = _nRange;
        if ( !m_nRange )
            m_nRange = 100;
    }

    //--------------------------------------------------------------------
    inline sal_uInt32 RangeProgressBar::GetRange() const
    {
        return m_nRange;
    }

    //--------------------------------------------------------------------
    inline void RangeProgressBar::SetValue( sal_uInt32 _nValue )
    {
        ProgressBar::SetValue( (sal_uInt16)( 100.0 * _nValue / m_nRange ) );
    }

    //--------------------------------------------------------------------
    inline sal_uInt32 RangeProgressBar::GetValue() const
    {
        return (sal_uInt32)( ProgressBar::GetValue() / 100.0 * m_nRange );
    }


//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_RANGEPROGRESSBAR_HXX
