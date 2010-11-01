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

#ifndef _SVTOOLS_EXTENSIONLISTBOX_HXX
#define _SVTOOLS_EXTENSIONLISTBOX_HXX

#include <vcl/ctrl.hxx>
#include <rtl/ustring.hxx>

// ============================================================================

namespace svt
{

#define EXTENSION_LISTBOX_ENTRY_NOTFOUND (sal_Int32) 0xFFFFFFFF

// ============================================================================

/** This abstract class provides methods to implement an extension list box.
    This header is needed for the automatic test tool
*/
class IExtensionListBox: public Control
{
public:
    IExtensionListBox( Window* pParent, WinBits nWinStyle = 0 ): Control( pParent, nWinStyle ){}

    /** @return  The count of the entries in the list box. */
    virtual sal_Int32 getItemCount() const = 0;

    /** @return  The index of the first selected entry in the list box.
        When nothing is selected, which is the case when getItemCount returns '0',
        then this function returns EXTENSION_LISTBOX_ENTRY_NOTFOUND */
    virtual sal_Int32 getSelIndex() const = 0;

    /** @return  The item name of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemName( sal_Int32 index ) const = 0;

    /** @return  The version string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemVersion( sal_Int32 index ) const = 0;

    /** @return  The description string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemDescription( sal_Int32 index ) const = 0;

    /** @return  The publisher string of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemPublisher( sal_Int32 index ) const = 0;

    /** @return  The link behind the publisher text of the entry with the given index
        The index starts with 0.
        Throws an com::sun::star::lang::IllegalArgumentException, when the position is invalid. */
    virtual ::rtl::OUString getItemPublisherLink( sal_Int32 index ) const = 0;

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
    virtual void select( const ::rtl::OUString & sName ) = 0;
};
// ============================================================================

} // namespace svt

// ============================================================================

#endif // _SVTOOLS_EXTENSIONLISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
