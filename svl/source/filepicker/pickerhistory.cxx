/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

