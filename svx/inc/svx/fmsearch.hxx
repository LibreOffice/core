/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmsearch.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _FMSEARCH_HXX
#define _FMSEARCH_HXX

#include <com/sun/star/sdbc/XResultSet.hpp>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/link.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <tools/string.hxx>
#include "svx/svxdllapi.h"

FORWARD_DECLARE_INTERFACE(util,XNumberFormatsSupplier)

// ===================================================================================================
// Hilfsmethoden

SVX_DLLPUBLIC sal_Bool IsSearchableControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xControl,
                                            ::rtl::OUString* pCurrentText = NULL);
    // check if the control has one of the interfaces we can use for searching
    // *pCurrentText will be filled with the current text of the control (as used when searching this control)

// ===================================================================================================
// Hilfsstrukturen

struct FmFoundRecordInformation
{
    ::com::sun::star::uno::Any      aPosition;  // Bookmark des Datensatzes, in dem der Text gefunden wurde
    sal_Int16       nFieldPos;  // dito : die relative Position der Spalte (im Stringnamen in Feldliste im Constructor)
    sal_Int16       nContext;   // Kontext, in dem gesucht und gefunden wurde (falls die aktuelle Suche verschiedene solche kennt)
};

// ===================================================================================================
// = struct FmSearchContext - Informationen fuer Suche in verschiedenen Kontexten
// ===================================================================================================

struct FmSearchContext
{
    // [in]
    sal_Int16                   nContext;       // die Nummer des Kontextes
    // [out]
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           xCursor;        // der Iterator fuer diesen Kontext
    String                  strUsedFields;  // eine Liste von durch ';' getrennten Feldnamen
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                            arrFields;      // die korrespondierenden Text-Interfaces fuer die Felder in strUsedFields
    String                  sFieldDisplayNames;     // if not empty : names to be displayed for the searchable fields (must have the same token count as strUsedFields !)
};

#endif // _FMSEARCH_HXX
