/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibilityoptions_const.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:17:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef INCLUDE_CONFIGITEMS_ACCESSIBILITYOPTIONS_CONST_HXX
#define INCLUDE_CONFIGITEMS_ACCESSIBILITYOPTIONS_CONST_HXX

#include <rtl/ustring.hxx>

namespace
{
    static const ::rtl::OUString s_sAccessibility           = ::rtl::OUString::createFromAscii("org.openoffice.Office.Common/Accessibility");
    static const ::rtl::OUString s_sAutoDetectSystemHC      = ::rtl::OUString::createFromAscii("AutoDetectSystemHC");
    static const ::rtl::OUString s_sIsForPagePreviews       = ::rtl::OUString::createFromAscii("IsForPagePreviews");
    static const ::rtl::OUString s_sIsHelpTipsDisappear     = ::rtl::OUString::createFromAscii("IsHelpTipsDisappear");
    static const ::rtl::OUString s_sHelpTipSeconds          = ::rtl::OUString::createFromAscii("HelpTipSeconds");
    static const ::rtl::OUString s_sIsAllowAnimatedGraphics = ::rtl::OUString::createFromAscii("IsAllowAnimatedGraphics");
    static const ::rtl::OUString s_sIsAllowAnimatedText     = ::rtl::OUString::createFromAscii("IsAllowAnimatedText");
    static const ::rtl::OUString s_sIsAutomaticFontColor    = ::rtl::OUString::createFromAscii("IsAutomaticFontColor");
    static const ::rtl::OUString s_sIsSystemFont            = ::rtl::OUString::createFromAscii("IsSystemFont");
    static const ::rtl::OUString s_sIsSelectionInReadonly   = ::rtl::OUString::createFromAscii("IsSelectionInReadonly");
}

#endif //  INCLUDE_CONFIGITEMS_ACCESSIBILITYOPTIONS_CONST_HXX
