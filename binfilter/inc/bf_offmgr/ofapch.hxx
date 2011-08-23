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

#ifndef _OFAPCH_HXX
#define _OFAPCH_HXX

#define _SVSTDARR_STRINGSISORTDTOR
#define _STD_VAR_ARRAYS
#define _INCL_EXPTRAY
#define _INCL_SFXNEWS

#ifndef OS2
// initialized data in header!
#include <sysdep.hxx>
#endif
#include <svgen.hxx>
#include <tlgen.hxx>
#include <sfx.hxx>
#include <sfxsh.hxx>
#include <sfxdoc.hxx>
#include <sfxdlg.hxx>
#include <sfxview.hxx>
#include <sfxiiter.hxx>
#include <svstdarr.hxx>
#include <svmem.hxx>
#define ITEMID_SPELLCHECK SID_ATTR_SPELL
#define ITEMID_HYPHENREGION 	SID_ATTR_HYPHENREGION
#ifndef OS2
#include <options.hxx>
#endif
namespace binfilter {

} //namespace binfilter
#endif
