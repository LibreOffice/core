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

#ifndef _FMSEARCH_HXX
#define _FMSEARCH_HXX

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <tools/string.hxx>
#include "svx/svxdllapi.h"

namespace com { namespace sun { namespace star {
    namespace util {
        class XNumberFormatsSupplier;
    }
}}}

// ===================================================================================================
// Hilfsmethoden

SVX_DLLPUBLIC sal_Bool IsSearchableControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xControl,
                                            OUString* pCurrentText = NULL);
    // check if the control has one of the interfaces we can use for searching
    // *pCurrentText will be filled with the current text of the control (as used when searching this control)

// ===================================================================================================
// Hilfsstrukturen

struct FmFoundRecordInformation
{
    ::com::sun::star::uno::Any      aPosition;  // bookmark of the record in which the text was found
    sal_Int16       nFieldPos;  // ditto : the relative position of the column (in the string name of the field list in the constructor)
    sal_Int16       nContext;   // the context in which was searched and found (if the current search knows several contexts)
};

// ===================================================================================================
// = struct FmSearchContext - information for the search in different contexts
// ===================================================================================================

struct FmSearchContext
{
    // [in]
    sal_Int16               nContext;       // the number of the context
    // [out]
    css::uno::Reference< css::sdbc::XResultSet>
                            xCursor;        // the iterator for the context
    OUString                strUsedFields;  // a list of field names separeted by ';'
    ::std::vector< css::uno::Reference< css::uno::XInterface > >
                            arrFields;      // the corresponding text interfaces for the fields in strUsedFields
    OUString                sFieldDisplayNames;     // if not empty : names to be displayed for the searchable fields (must have the same token count as strUsedFields !)
};

#endif // _FMSEARCH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
