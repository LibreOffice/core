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

#include <svl/pickerhistoryaccess.hxx>
#include <cppuhelper/weakref.hxx>
#include <vector>

namespace svt
{
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

            // then push_back the picker
            _rHistory.push_back( InterfaceAdapter( _rxPicker ) );
        }
    }

    void addFolderPicker( const Reference< XInterface >& _rxPicker )
    {
        implPushBackPicker( getFolderPickerHistory(), _rxPicker );
    }

    void addFilePicker( const Reference< XInterface >& _rxPicker )
    {
        implPushBackPicker( getFilePickerHistory(), _rxPicker );
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
