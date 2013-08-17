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

#ifndef DBACCESS_RANGEPROGRESSBAR_HXX
#define DBACCESS_RANGEPROGRESSBAR_HXX

#include <vcl/prgsbar.hxx>

namespace dbmm
{

    // RangeProgressBar
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

    inline void RangeProgressBar::SetRange( sal_uInt32 _nRange )
    {
        m_nRange = _nRange;
        if ( !m_nRange )
            m_nRange = 100;
    }

    inline sal_uInt32 RangeProgressBar::GetRange() const
    {
        return m_nRange;
    }

    inline void RangeProgressBar::SetValue( sal_uInt32 _nValue )
    {
        ProgressBar::SetValue( (sal_uInt16)( 100.0 * _nValue / m_nRange ) );
    }

    inline sal_uInt32 RangeProgressBar::GetValue() const
    {
        return (sal_uInt32)( ProgressBar::GetValue() / 100.0 * m_nRange );
    }

} // namespace dbmm

#endif // DBACCESS_RANGEPROGRESSBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
