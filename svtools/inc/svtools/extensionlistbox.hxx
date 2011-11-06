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

