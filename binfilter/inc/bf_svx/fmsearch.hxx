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

#ifndef _FMSEARCH_HXX
#define _FMSEARCH_HXX

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>

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

#ifndef _SV_COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif


#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
FORWARD_DECLARE_INTERFACE(util,XNumberFormatsSupplier)
namespace binfilter {

//FORWARD_DECLARE_INTERFACE(uno,Reference)
//STRIP008 FORWARD_DECLARE_INTERFACE(util,XNumberFormatsSupplier)

// ===================================================================================================
// moegliche Rueckgabewerte fuer den Found-Handler
#define	FM_SEARCH_GETFOCUS_ASYNC	0x0001
    // setzt den Fokus auf den Dialog nicht sofort nach dem Aufruf des Found-Handlers, sondern postet sich selber dafuer
    // ein Ereignis

#define MAX_HISTORY_ENTRIES		50

// ===================================================================================================
// Hilfsmethoden

    // check if the control has one of the interfaces we can use for searching
    // *pCurrentText will be filled with the current text of the control (as used when searching this control)

// ===================================================================================================
// Hilfsstrukturen

struct FmFoundRecordInformation
{
    ::com::sun::star::uno::Any		aPosition;	// Bookmark des Datensatzes, in dem der Text gefunden wurde
    sal_Int16		nFieldPos;	// dito : die relative Position der Spalte (im Stringnamen in Feldliste im Constructor)
    sal_Int16		nContext;	// Kontext, in dem gesucht und gefunden wurde (falls die aktuelle Suche verschiedene solche kennt)
};

DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>, InterfaceArray);
    // TODO : use stl

// ===================================================================================================
// = struct FmSearchContext - Informationen fuer Suche in verschiedenen Kontexten
// ===================================================================================================

struct FmSearchContext
{
    // [in]
    sal_Int16					nContext;		// die Nummer des Kontextes
    // [out]
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>			xCursor;		// der Iterator fuer diesen Kontext
    String					strUsedFields;	// eine Liste von durch ';' getrennten Feldnamen
    InterfaceArray			arrFields;		// die korrespondierenden Text-Interfaces fuer die Felder in strUsedFields
    String					sFieldDisplayNames;		// if not empty : names to be displayed for the searchable fields (must have the same token count as strUsedFields !)
};

// ===================================================================================================
// = class FmSearchDialog - Dialog fuer Suchen in Formularen/Tabellen
// ===================================================================================================

namespace svxform {
    class FmSearchConfigItem;
}

class FmSearchEngine;
struct FmSearchProgress;


}//end of namespace binfilter
#endif // _FMSEARCH_HXX
