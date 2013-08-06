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
        virtual StringEntryIdentifier   CurrentEntry( OUString& _out_entryText ) const = 0;

        /** returns the next entry in the list.

            The implementation is expected to wrap around. That is, if the given entry denotes the last
            entry in the list, then NextEntry should return the first entry.
        */
        virtual StringEntryIdentifier   NextEntry( StringEntryIdentifier _currentEntry, OUString& _out_entryText ) const = 0;

        /** selects a given entry
        */
        virtual void                    SelectEntry( StringEntryIdentifier _entry ) = 0;

    protected:
        ~ISearchableStringList() {}
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
