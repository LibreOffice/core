/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layout-pre.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef _LAYOUT_PRE_HXX
#define _LAYOUT_PRE_HXX

#if ENABLE_LAYOUT

//#if !LAYOUT
//#define LAYOUT 1
//#endif
//#define LAYOUT_NS layout

// TWBN, but find that SfxModalDialog and FixedLine are already defined, eg compiling
// dlgfact.cxx
//typedef layout::Dialog SfxModalDialog;
//typedef layout::FixedLine FixedLine;

#define AdvancedButton layout::AdvancedButton
#define Box layout::Box
#define Button layout::Button
#define CancelButton layout::CancelButton
#define CheckBox layout::CheckBox
#define Container layout::Container
#define FixedImage layout::FixedImage
#define FixedInfo layout::FixedInfo
#define FixedLine layout::FixedLine
#define FixedText layout::FixedText
#define HBox layout::HBox
#define HelpButton layout::HelpButton
#define MetricField layout::MetricField
#define MoreButton layout::MoreButton
#define MultiLineEdit layout::MultiLineEdit
#define OKButton layout::OKButton
#define ProgressBar layout::ProgressBar
#define PushButton layout::PushButton
#define RadioButton layout::RadioButton
#define SfxModalDialog layout::Dialog
#define Table layout::Table
#define VBox layout::VBox

/* FIXME: why are we defaulting to layout::Window?
   /home/janneke/vc/ooo-build/build/hack/sw/source/ui/dialog/wordcountdialog.cxx:87: error: no matching function for call to 'layout::Dialog::Dialog(Window*&, const char [14], const char [7])'
   ../../../../layout/inc/layout/layout.hxx:304: note: candidates are: layout::Dialog::Dialog(layout::Window*, const char*, const char*, sal_uInt32)
   ../../../../layout/inc/layout/layout.hxx:300: note:                 layout::Dialog::Dialog(const layout::Dialog&)
*/

#define Window ::Window

#undef SVX_RES
#define SVX_RES(x) #x
#undef SW_RES
#define SW_RES(x) #x

/* Hmm.  This hack makes zoom.cxx, wordcountdialog.cxx diffs smaller
 * but is not scalable. */
#ifdef _LAYOUT_POST_HXX

#ifdef _SVX_RECOVER_CXX
#undef SfxModalDialog
// 3rd parameter must match ID in <modaldialog> "RID_SVXDLG_RECOVER", localize.sdf
#define SfxModalDialog( pParent, SVX_RES_RID ) layout::Dialog( pParent, "recover.xml", "RID_SVXDLG_RECOVER" )
//#define _SVX_RECOVER_HRC
#endif /* _SVX_RECOVER_CXX */

#ifdef SW_WORDCOUNTDIALOG_HXX
#undef SfxModalDialog
// 3rd parameter must match ID in <modaldialog> "DLG_WORDCOUNT", localize.sdf
#define SfxModalDialog( pParent, SW_RES_RID ) layout::Dialog( pParent, "wordcount.xml", "DLG_WORDCOUNT" )
#define SW_WORDCOUNTDIALOG_HRC
#endif /* SW_WORDCOUNTDIALOG_HXX */

#ifdef _SVX_ZOOM_CXX
#undef SfxModalDialog
// 3rd parameter must match ID in <modaldialog> "RID_SVXDLG_ZOOM", localize.sdf
#define SfxModalDialog( pParent, SVX_RES_RID ) layout::Dialog( pParent, "zoom.xml", "RID_SVXDLG_ZOOM" )
#define _SVX_ZOOM_HRC
#endif /* _SVX_ZOOM_CXX */

#endif /* _LAYOUT_POST_HXX */

#else /* !ENABLE_LAYOUT */

#define LAYOUT_PRE_POST

#endif /* !ENABLE_LAYOUT */

#endif /* _LAYOUT_PRE_HXX */
