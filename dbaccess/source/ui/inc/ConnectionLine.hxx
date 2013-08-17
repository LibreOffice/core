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
#ifndef DBAUI_CONNECTIONLINE_HXX
#define DBAUI_CONNECTIONLINE_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include "ConnectionLineData.hxx"
#include <functional>

class OutputDevice;
namespace dbaui
{

    // ConnData     ---------->*    ConnLineData
    //    ^1                            ^1
    //    |                             |
    //  Conn        ---------->*    ConnLine

    /*
        the class OConnectionLine represents the graphical line between the to two windows
    **/
    class OTableConnection;
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
