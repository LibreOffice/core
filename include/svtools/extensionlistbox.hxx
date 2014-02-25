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

#ifndef INCLUDED_SVTOOLS_EXTENSIONLISTBOX_HXX
#define INCLUDED_SVTOOLS_EXTENSIONLISTBOX_HXX

#include <vcl/ctrl.hxx>
#include <rtl/ustring.hxx>



namespace svt
{



/** This abstract class provides methods to implement an extension list box.
    This header is needed for the automatic test tool
*/
class IExtensionListBox: public Control
{
public:
    enum { ENTRY_NOTFOUND = -1 };

    IExtensionListBox( Window* pParent, WinBits nWinStyle = 0 ): Control( pParent, nWinStyle ){}

    /** @return  The count of the entries in the list box. */
    virtual sal_Int32 getItemCount() const = 0;

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns ENTRY_NOTFOUND */
    virtual sal_Int32 getSelIndex() const = 0;

    /** @return  The item name of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemName( sal_Int32 index ) const = 0;

    /** @return  The version string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemVersion( sal_Int32 index ) const = 0;

    /** @return  The description string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemDescription( sal_Int32 index ) const = 0;

    /** @return  The publisher string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemPublisher( sal_Int32 index ) const = 0;

    /** @return  The link behind the publisher text of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual OUString getItemPublisherLink( sal_Int32 index ) const = 0;

    /** The entry at the given position will be selected
        Index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual void select( sal_Int32 index ) = 0;

    /** The first found entry with the given name will be selected
        When there was no entry found with the name, the selection doesn't change.
        Please note that there might be more than one entry with the same
        name, because:
            1. the name is not unique
            2. one extension can be installed as user and shared extension.
    */
    virtual void select( const OUString & sName ) = 0;
};


} // namespace svt



#endif // INCLUDED_SVTOOLS_EXTENSIONLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
