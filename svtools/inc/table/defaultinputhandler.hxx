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

#ifndef INCLUDED_SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX
#define INCLUDED_SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX

#include <svtools/table/tableinputhandler.hxx>
#include <svtools/table/tabletypes.hxx>

#include <memory>


namespace svt { namespace table
{


    struct DefaultInputHandler_Impl;


    //= DefaultInputHandler

    class DefaultInputHandler : public ITableInputHandler
    {
    private:
        ::std::unique_ptr< DefaultInputHandler_Impl > m_pImpl;

    public:
        DefaultInputHandler();
        virtual ~DefaultInputHandler();

        virtual bool    MouseMove       ( ITableControl& _rControl, const MouseEvent& rMEvt ) override;
        virtual bool    MouseButtonDown ( ITableControl& _rControl, const MouseEvent& rMEvt ) override;
        virtual bool    MouseButtonUp   ( ITableControl& _rControl, const MouseEvent& rMEvt ) override;
        virtual bool    KeyInput        ( ITableControl& _rControl, const KeyEvent& rKEvt ) override;
        virtual bool    GetFocus        ( ITableControl& _rControl ) override;
        virtual bool    LoseFocus       ( ITableControl& _rControl ) override;
    };


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
