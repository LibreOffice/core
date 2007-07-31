/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmsearch.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-07-31 13:55:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FMSEARCH_HXX
#define _FMSEARCH_HXX

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

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
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
