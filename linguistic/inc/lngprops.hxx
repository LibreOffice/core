/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lngprops.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:46:35 $
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

#ifndef _LINGUISTIC_LNGPROPS_HHX_
#define _LINGUISTIC_LNGPROPS_HHX_

#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif

// maximal number of suggestions to be returned in spelling  context-menu
// (may not include results added by looking up user dictionaries)
#define UPN_MAX_NUMBER_OF_SUGGESTIONS       "MaxNumberOfSuggestions"

// WIDs for property names
//!! Don't change values! They are used as the property handles in
//!! the service description
#define WID_IS_GERMAN_PRE_REFORM            UPH_IS_GERMAN_PRE_REFORM
#define WID_IS_USE_DICTIONARY_LIST          UPH_IS_USE_DICTIONARY_LIST
#define WID_IS_IGNORE_CONTROL_CHARACTERS    UPH_IS_IGNORE_CONTROL_CHARACTERS
#define WID_IS_SPELL_UPPER_CASE             UPH_IS_SPELL_UPPER_CASE
#define WID_IS_SPELL_WITH_DIGITS            UPH_IS_SPELL_WITH_DIGITS
#define WID_IS_SPELL_CAPITALIZATION         UPH_IS_SPELL_CAPITALIZATION
#define WID_HYPH_MIN_LEADING                UPH_HYPH_MIN_LEADING
#define WID_HYPH_MIN_TRAILING               UPH_HYPH_MIN_TRAILING
#define WID_HYPH_MIN_WORD_LENGTH            UPH_HYPH_MIN_WORD_LENGTH
#define WID_DEFAULT_LOCALE                  UPH_DEFAULT_LOCALE
#define WID_IS_SPELL_AUTO                   UPH_IS_SPELL_AUTO
#define WID_IS_SPELL_HIDE                   UPH_IS_SPELL_HIDE
#define WID_IS_SPELL_IN_ALL_LANGUAGES       UPH_IS_SPELL_IN_ALL_LANGUAGES
#define WID_IS_SPELL_SPECIAL                UPH_IS_SPELL_SPECIAL
#define WID_IS_HYPH_AUTO                    UPH_IS_HYPH_AUTO
#define WID_IS_HYPH_SPECIAL                 UPH_IS_HYPH_SPECIAL
#define WID_IS_WRAP_REVERSE                 UPH_IS_WRAP_REVERSE
#define WID_DEFAULT_LANGUAGE                UPH_DEFAULT_LANGUAGE
#define WID_DEFAULT_LOCALE_CJK              UPH_DEFAULT_LOCALE_CJK
#define WID_DEFAULT_LOCALE_CTL              UPH_DEFAULT_LOCALE_CTL

#endif

