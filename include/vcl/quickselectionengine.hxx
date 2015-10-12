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

#ifndef INCLUDED_VCL_QUICKSELECTIONENGINE_HXX
#define INCLUDED_VCL_QUICKSELECTIONENGINE_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <memory>

class KeyEvent;


namespace vcl
{


    typedef const void* StringEntryIdentifier;


    //= ISearchableStringList

    // TODO: consolidate this with vcl::IMnemonicEntryList
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


    //= QuickSelectionEngine

    struct QuickSelectionEngine_Data;
    class VCL_DLLPUBLIC QuickSelectionEngine
    {
    public:
        QuickSelectionEngine( ISearchableStringList& _entryList );
        ~QuickSelectionEngine();

        bool    HandleKeyEvent( const KeyEvent& _rKEvt );
        void    Reset();
        void SetEnabled( const bool& b );

    private:
        ::std::unique_ptr< QuickSelectionEngine_Data >    m_pData;
        bool bEnabled;

    private:
        QuickSelectionEngine( const QuickSelectionEngine& ) = delete;
        QuickSelectionEngine& operator=( const QuickSelectionEngine& ) = delete;
    };


} // namespace vcl


#endif // INCLUDED_VCL_QUICKSELECTIONENGINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
