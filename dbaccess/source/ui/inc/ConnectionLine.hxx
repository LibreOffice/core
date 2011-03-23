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
#ifndef DBAUI_CONNECTIONLINE_HXX
#define DBAUI_CONNECTIONLINE_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include "ConnectionLineData.hxx"
#include <functional>

class OutputDevice;
namespace dbaui
{

    //==================================================================
    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine
    //==================================================================

    /*
        the class OConnectionLine represents the graphical line between the to two windows
    **/
    class OConnectionLineData;
    class OTableConnection;
    class OTableWindow;
    class OConnectionLine
    {
        OTableConnection*       m_pTabConn;
        OConnectionLineDataRef  m_pData;

        Point                   m_aSourceConnPos,
                                m_aDestConnPos;
        Point                   m_aSourceDescrLinePos,
                                m_aDestDescrLinePos;
    public:
        OConnectionLine( OTableConnection* pConn, OConnectionLineDataRef pLineData );
        OConnectionLine( const OConnectionLine& rLine );
        virtual ~OConnectionLine();

        OConnectionLine& operator=( const OConnectionLine& rLine );

        Rectangle           GetBoundingRect();
        sal_Bool                RecalcLine();
        void                Draw( OutputDevice* pOutDev );
        bool                CheckHit( const Point& rMousePos ) const;
        String              GetSourceFieldName() const { return m_pData->GetSourceFieldName(); }
        String              GetDestFieldName() const { return m_pData->GetDestFieldName(); }

        sal_Bool                IsValid() const;

        Rectangle           GetSourceTextPos() const;
        Rectangle           GetDestTextPos() const;

        OConnectionLineDataRef  GetData() const { return m_pData; }

        Point               getMidPoint() const;
    };
    /// unary_function Functor object for class OConnectionLine returntype is void
    /// draws a connectionline object on outputdevice
    struct TConnectionLineDrawFunctor : ::std::unary_function<OConnectionLine*,void>
    {
        OutputDevice* pDevice;
        TConnectionLineDrawFunctor(OutputDevice* _pDevice)
        {
            pDevice = _pDevice;
        }
        inline void operator()(OConnectionLine* _pLine)
        {
            _pLine->Draw(pDevice);
        }
    };
    /// binary_function Functor object for class OConnectionLine returntype is bool
    /// checks if the point is on connectionline
    struct TConnectionLineCheckHitFunctor : ::std::binary_function<OConnectionLine*,Point,bool>
    {
        inline bool operator()(const OConnectionLine* lhs,const Point& rhs) const
        {
            return lhs->CheckHit(rhs);
        }
    };

}
#endif // DBAUI_CONNECTIONLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
