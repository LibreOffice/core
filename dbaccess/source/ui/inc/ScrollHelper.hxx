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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_SCROLLHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_SCROLLHELPER_HXX

#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>

namespace dbaui
{
    class OScrollHelper
    {
        Link<LinkParamNone*,void>  m_aUpScroll;
        Link<LinkParamNone*,void>  m_aDownScroll;
    public:
        /** default constructor
        */
        OScrollHelper();

        ~OScrollHelper();

        /** set the memthod which should be called when scrolling up
            @param  _rUpScroll
                the method to set
        */
        inline void setUpScrollMethod( const Link<LinkParamNone*,void>& _rUpScroll )
        {
            m_aUpScroll = _rUpScroll;
        }

        /** set the memthod which should be called when scrolling down
            @param  _rDownScroll
                the method to set
        */
        inline void setDownScrollMethod( const Link<LinkParamNone*,void>& _rDownScroll )
        {
            m_aDownScroll = _rDownScroll;
        }

        /** check if a scroll method has to be called
            @param  _rPoint
                the current selection point
            @param  _rOutputSize
                the output size of the window
        */
        void scroll(const Point& _rPoint, const Size& _rOutputSize);
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SCROLLHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
