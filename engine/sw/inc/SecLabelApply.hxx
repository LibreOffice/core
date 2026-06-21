/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_SECLABELAPPLY_HXX
#define INCLUDED_SW_INC_SECLABELAPPLY_HXX

#include "swdllapi.h"
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <string_view>

namespace com::sun::star::frame
{
class XModel;
}

namespace sw::seclabel
{
/// Append a STANAG customXml part (the 4778 binding) and its itemProps to the
/// document's InteropGrabBag, so the DOCX export writes them as
/// customXml/item*.xml + customXml/itemProps*.xml on the next save.
SW_DLLPUBLIC void storeLabelPart(const css::uno::Reference<css::frame::XModel>& xModel,
                                 std::u16string_view rBindingXml,
                                 std::u16string_view rItemPropsXml);

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_SECLABELAPPLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
