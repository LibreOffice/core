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

#ifndef VCL_MNEMONICENGINE_HXX
#define VCL_MNEMONICENGINE_HXX

#include "dllapi.h"

#include <sal/config.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <memory>

class KeyEvent;

//........................................................................
namespace vcl
{
//........................................................................

    //====================================================================
    //= IMnemonicEntryList
    //====================================================================
    /// callback for a MnemonicEngine
    class SAL_NO_VTABLE VCL_DLLPUBLIC IMnemonicEntryList
    {
    public:
        /** returns the first list entry for the mnemonic search

            @return
                a pointer which can be used to unuquely identify the entry.
                The MenomonicEngine itself does not use this value, it
                is only passed to other methods of this callback interface.

                If this value is <NULL/>, searching stops.
        */
        virtual const void* FirstSearchEntry( OUString& _rEntryText ) const = 0;

        /** returns the next list entry for the mnemonic search

            @return
                a pointer which can be used to unuquely identify the entry.
                The MenomonicEngine itself does not use this value, it
                is only passed to other methods of this callback interface.

                If this value is <NULL/>, searching stops.

                If this value is the same as returned by the previous call
                to <member>FirstSearchEntry</member> (i.e. you cycled
                around), then searching stops, too.
        */
        virtual const void* NextSearchEntry( const void* _pCurrentSearchEntry, OUString& _rEntryText ) const = 0;

        /** "selects" a given entry.

            Note: The semantics of "select" depends on your implementation. You
            might actually really select the entry (in the sense of a selected
            list box entry, for example), you might make it the current entry,
            if your implementation supports this - whatever.

            @param _pEntry
                the entry to select. This is the return value of a previous call
                to <member>FirstSearchEntry</member> or <member>NextSearchEntry</member>.
        */
        virtual void    SelectSearchEntry( const void* _pEntry ) = 0;

        /** "executes" the current search entry, i.e. the one returned
            in the previous <member>NextSearchEntry</member> call.

            Note: The semantics of "execute" depends on your implementation. You
            might even have a list of entries which cannot be executed at all.

            This method is called after <member>SelectSearchEntry</member>,
            if and only if the current entry's mnemonic is unambiguous.

            For instance, imagine a list which has two entries with the same mnemonic
            character, say "c". Now if the user presses <code>Alt-C</code>, the MnemonicEngine
            will call <member>SelectCurrentEntry</member> as soon as it encounters
            the first entry, but it'll never call <member>ExecuteSearchEntry</member>.

            If, however, "c" is a unique mnemonic character in your entry list, then the
            call of <member>SelectSearchEntry</member> will be followed by a
            call to <member>ExecuteSearchEntry</member>.

            This way, you can implement cyclic selection of entries: In
            <member>FirstSearchEntry</member>, return the entry which was previously
            selected, and in <member>NextSearchEntry</member>, interlly cycle around
            in your list. Then, multiple user inputs of <code>Alt-C</code> will
            cycle through all entries with the mnemonic being "c".

            @param _pEntry
                the entry to select. This is the return value of a previous call
                to <member>FirstSearchEntry</member> or <member>NextSearchEntry</member>.
        */
        virtual void    ExecuteSearchEntry( const void* _pEntry ) const = 0;

    protected:
        ~IMnemonicEntryList() {}
    };

    //====================================================================
    //= MnemonicEngine
    //====================================================================
    struct MnemonicEngine_Data;
    class VCL_DLLPUBLIC MnemonicEngine
    {
        ::std::auto_ptr< MnemonicEngine_Data >  m_pData;

    public:
        MnemonicEngine( IMnemonicEntryList& _rEntryList );
        ~MnemonicEngine();

        /** handles a key event

            If the key event denotes pressing an accelerator key, then the
            entry list is searched for a matching entry. If such an entry is
            found, <member>IMnemonicEntryList::SelectSearchEntry</member>
            is called.

            If the entry is the only one with the given mnemonic character, then
            also <member>IMnemonicEntryList::ExecuteSearchEntry</member>
            is called.

            @return
                if the key event has been handled, and should thus not be processed
                further.
        */
        bool    HandleKeyEvent( const KeyEvent& _rKEvt );
    };

//........................................................................
} // namespace vcl
//........................................................................

#endif // VCL_MNEMONICENGINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
