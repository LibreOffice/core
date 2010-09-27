/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef VCL_QUICKSELECTIONENGINE_HXX
#define VCL_QUICKSELECTIONENGINE_HXX

#include "dllapi.h"

#include <tools/string.hxx>

#include <memory>

class KeyEvent;

//........................................................................
namespace vcl
{
//........................................................................

    typedef const void* StringEntryIdentifier;

    //====================================================================
    //= ISearchableStringList
    //====================================================================
    // TODO: consolidate this with ::vcl::IMnemonicEntryList
    class SAL_NO_VTABLE VCL_DLLPUBLIC ISearchableStringList
    {
    public:
        /** returns the current entry in the list of searchable strings.

            Search operations will start with this entry.
        */
        virtual StringEntryIdentifier   CurrentEntry( String& _out_entryText ) const = 0;

        /** returns the next entry in the list.

            The implementation is expected to wrap around. That is, if the given entry denotes the last
            entry in the list, then NextEntry should return the first entry.
        */
        virtual StringEntryIdentifier   NextEntry( StringEntryIdentifier _currentEntry, String& _out_entryText ) const = 0;

        /** selects a given entry
        */
        virtual void                    SelectEntry( StringEntryIdentifier _entry ) = 0;
    };

    //====================================================================
    //= QuickSelectionEngine
    //====================================================================
    struct QuickSelectionEngine_Data;
    class VCL_DLLPUBLIC QuickSelectionEngine
    {
    public:
        QuickSelectionEngine( ISearchableStringList& _entryList );
        ~QuickSelectionEngine();

        bool    HandleKeyEvent( const KeyEvent& _rKEvt );
        void    Reset();

    private:
        ::std::auto_ptr< QuickSelectionEngine_Data >    m_pData;

    private:
        QuickSelectionEngine();                                         // never implemented
        QuickSelectionEngine( const QuickSelectionEngine& );            // never implemented
        QuickSelectionEngine& operator=( const QuickSelectionEngine& ); // never implemented
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_QUICKSELECTIONENGINE_HXX
