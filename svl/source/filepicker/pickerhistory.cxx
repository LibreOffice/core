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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <svl/pickerhistory.hxx>
#include <svl/pickerhistoryaccess.hxx>
#include <cppuhelper/weakref.hxx>
#include <vector>

//.........................................................................
namespace svt
{
//.........................................................................
    using namespace ::com::sun::star::uno;

    namespace
    {
        typedef ::com::sun::star::uno::WeakReference< XInterface >  InterfaceAdapter;
        typedef ::std::vector< InterfaceAdapter  >                  InterfaceArray;

        // ----------------------------------------------------------------
        InterfaceArray& getFolderPickerHistory()
        {
            static InterfaceArray s_aHistory;
            return s_aHistory;
        }

        // ----------------------------------------------------------------
        InterfaceArray& getFilePickerHistory()
        {
            static InterfaceArray s_aHistory;
            return s_aHistory;
        }

        // ----------------------------------------------------------------
        void implPushBackPicker( InterfaceArray& _rHistory, const Reference< XInterface >& _rxPicker )
        {
            if ( !_rxPicker.is() )
                return;

            //=============================================================
            // first, check which of the objects we hold in s_aHistory can be removed
            {
                InterfaceArray aCleanedHistory;
                for (   InterfaceArray::const_iterator aLoop = _rHistory.begin();
                        aLoop != _rHistory.end();
                        ++aLoop
                    )
                {
                    Reference< XInterface > xCurrent( aLoop->get() );
                    if ( xCurrent.is() )
                    {
                        if ( aCleanedHistory.empty() )
                            // make some room, assume that all interfaces (from here on) are valie
                            aCleanedHistory.reserve( _rHistory.size() - ( aLoop - _rHistory.begin() ) );
                        aCleanedHistory.push_back( InterfaceAdapter( xCurrent ) );
                    }
                }
                _rHistory.swap( aCleanedHistory );
            }

            //=============================================================
            // then push_back the picker
            _rHistory.push_back( InterfaceAdapter( _rxPicker ) );
        }

        //-----------------------------------------------------------------
        Reference< XInterface > implGetTopMostPicker( const InterfaceArray& _rHistory )
        {
            Reference< XInterface > xTopMostAlive;

            //=============================================================
            // search the first picker which is still alive ...
            for (   InterfaceArray::const_reverse_iterator aLoop = _rHistory.rbegin();
                    ( aLoop != _rHistory.rend() ) && !xTopMostAlive.is();
                    ++aLoop
                )
            {
                xTopMostAlive = aLoop->get();
            }

            return xTopMostAlive;
        }
    }

    //---------------------------------------------------------------------
    Reference< XInterface > GetTopMostFolderPicker( )
    {
        return implGetTopMostPicker( getFolderPickerHistory() );
    }

    //---------------------------------------------------------------------
    Reference< XInterface > GetTopMostFilePicker( )
    {
        return implGetTopMostPicker( getFilePickerHistory() );
    }

    //---------------------------------------------------------------------
    void addFolderPicker( const Reference< XInterface >& _rxPicker )
    {
        implPushBackPicker( getFolderPickerHistory(), _rxPicker );
    }

    //---------------------------------------------------------------------
    void addFilePicker( const Reference< XInterface >& _rxPicker )
    {
        implPushBackPicker( getFilePickerHistory(), _rxPicker );
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
