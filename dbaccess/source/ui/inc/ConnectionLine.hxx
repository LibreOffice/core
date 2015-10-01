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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_CONNECTIONLINE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_CONNECTIONLINE_HXX

#include <tools/gen.hxx>
#include "ConnectionLineData.hxx"
#include <vcl/vclptr.hxx>

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
        VclPtr<OTableConnection>       m_pTabConn;
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
        bool                RecalcLine();
        void                Draw( OutputDevice* pOutDev );
        bool                CheckHit( const Point& rMousePos ) const;

        bool                IsValid() const;

        Rectangle           GetSourceTextPos() const;
        Rectangle           GetDestTextPos() const;

        OConnectionLineDataRef  GetData() const { return m_pData; }

        Point               getMidPoint() const;
    };

}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_CONNECTIONLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
