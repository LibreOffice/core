/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pickerhistory.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:49:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef SVTOOLS_PICKERHISTORY_HXX
#include "pickerhistory.hxx"
#endif
#ifndef SVTOOLS_PICKERHISTORYACCESS_HXX
#include "pickerhistoryaccess.hxx"
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
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

