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

#include "svtools/table/tableinputhandler.hxx"
#include "svtools/table/tabletypes.hxx"

#include <boost/scoped_ptr.hpp>

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    struct DefaultInputHandler_Impl;

    //==================================================================================================================
    //= DefaultInputHandler
    //==================================================================================================================
    class DefaultInputHandler : public ITableInputHandler
    {
    private:
        ::boost::scoped_ptr< DefaultInputHandler_Impl > m_pImpl;

    public:
        DefaultInputHandler();
        ~DefaultInputHandler();

        virtual bool    MouseMove       ( ITableControl& _rControl, const MouseEvent& rMEvt );
        virtual bool    MouseButtonDown ( ITableControl& _rControl, const MouseEvent& rMEvt );
        virtual bool    MouseButtonUp   ( ITableControl& _rControl, const MouseEvent& rMEvt );
        virtual bool    KeyInput        ( ITableControl& _rControl, const KeyEvent& rKEvt );
        virtual bool    GetFocus        ( ITableControl& _rControl );
        virtual bool    LoseFocus       ( ITableControl& _rControl );
        virtual bool    RequestHelp     ( ITableControl& _rControl, const HelpEvent& rHEvt );
        virtual bool    Command         ( ITableControl& _rControl, const CommandEvent& rCEvt );
        virtual bool    PreNotify       ( ITableControl& _rControl, NotifyEvent& rNEvt );
        virtual bool    Notify          ( ITableControl& _rControl, NotifyEvent& rNEvt );
    };

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................

#endif // SVTOOLS_INC_TABLE_DEFAULTINPUTHANDLER_HXX
