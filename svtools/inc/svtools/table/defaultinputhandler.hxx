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

#ifndef SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX
#define SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX

#include <svtools/table/tableinputhandler.hxx>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    struct DefaultInputHandler_Impl;

    //====================================================================
    //= DefaultInputHandler
    //====================================================================
    class DefaultInputHandler : public ITableInputHandler
    {
            friend class TableDataWindow;
    private:
        DefaultInputHandler_Impl*   m_pImpl;
        bool                        m_bResize;

    public:
        DefaultInputHandler();
        ~DefaultInputHandler();

        virtual bool    MouseMove       ( IAbstractTableControl& _rControl, const MouseEvent& rMEvt );
        virtual bool    MouseButtonDown ( IAbstractTableControl& _rControl, const MouseEvent& rMEvt );
        virtual bool    MouseButtonUp   ( IAbstractTableControl& _rControl, const MouseEvent& rMEvt );
        virtual bool    KeyInput        ( IAbstractTableControl& _rControl, const KeyEvent& rKEvt );
        virtual bool    GetFocus        ( IAbstractTableControl& _rControl );
        virtual bool    LoseFocus       ( IAbstractTableControl& _rControl );
        virtual bool    RequestHelp     ( IAbstractTableControl& _rControl, const HelpEvent& rHEvt );
        virtual bool    Command         ( IAbstractTableControl& _rControl, const CommandEvent& rCEvt );
        virtual bool    PreNotify       ( IAbstractTableControl& _rControl, NotifyEvent& rNEvt );
        virtual bool    Notify          ( IAbstractTableControl& _rControl, NotifyEvent& rNEvt );
    };

//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX
