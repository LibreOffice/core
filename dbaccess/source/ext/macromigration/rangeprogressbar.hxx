/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rangeprogressbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:07:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        ProgressBar::SetValue( (USHORT)( 100.0 * _nValue / m_nRange ) );
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
