/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): Generated on 2006-09-01 17:49:30.436716

#ifdef PRECOMPILED_HEADERS
#include "com/sun/star/accessibility/AccessibleEventId.hpp"
#include "com/sun/star/accessibility/AccessibleRole.hpp"
#include "com/sun/star/accessibility/AccessibleStateType.hpp"
#include "com/sun/star/awt/XDialog.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/document/MacroExecMode.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XLayoutManager.hpp"
#include "com/sun/star/resource/XStringResourcePersistence.hpp"
#include "com/sun/star/script/XLibraryContainer.hpp"
#include "com/sun/star/script/XLibraryContainerPassword.hpp"
#include "com/sun/star/script/XLibraryContainer2.hpp"
#include "com/sun/star/task/XStatusIndicator.hpp"
#include "com/sun/star/task/XStatusIndicatorFactory.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp"
#include "com/sun/star/util/XCloneable.hpp"

#include "comphelper/types.hxx"
#include "comphelper/processfactory.hxx"

#include "toolkit/awt/vclxfont.hxx"
#include "toolkit/helper/externallock.hxx"
#include "toolkit/helper/convert.hxx"
#include "toolkit/helper/vclunohelper.hxx"

#include "tools/debug.hxx"
#include "tools/diagnose_ex.h"
#include "tools/urlobj.hxx"

#include "vcl/wrkwin.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/menu.hxx"
#include "vcl/status.hxx"
#include "vcl/sound.hxx"
#include "vcl/svapp.hxx"
#include "vcl/scrbar.hxx"

#include "svl/svarray.hxx"
#include "svl/itempool.hxx"
#include "svl/itemset.hxx"
#include "svl/aeitem.hxx"
#include "svl/stritem.hxx"
#include "svl/whiter.hxx"
#include "svtools/langtab.hxx"
#include "svtools/texteng.hxx"
#include "svtools/textview.hxx"
#include "svtools/xtextedt.hxx"
#include "svtools/txtattr.hxx"
#include "svtools/textwindowpeer.hxx"
#include "svl/urihelper.hxx"

#define _BASIC_TEXTPORTIONS
#include "basic/sbdef.hxx"
#include "basic/sbstar.hxx"
#include "basic/sbmeth.hxx"
#include "basic/sbmod.hxx"
#include "basic/basmgr.hxx"
#include "basic/sbuno.hxx"
#include "basic/basrdll.hxx"
#include "basic/sbx.hxx"

#include "sfx2/app.hxx"
#include "sfx2/objface.hxx"
#include "sfx2/printer.hxx"
#include "sfx2/viewsh.hxx"
#include "sfx2/event.hxx"
#include "sfx2/bindings.hxx"
#include "sfx2/request.hxx"
#include "sfx2/childwin.hxx"
#include "sfx2/viewfrm.hxx"
#include "sfx2/dispatch.hxx"
#include "sfx2/objsh.hxx"
#include "sfx2/genlink.hxx"
#include "sfx2/dispatch.hxx"
#include "sfx2/minfitem.hxx"
#include "sfx2/dinfdlg.hxx"
#include "sfx2/docfac.hxx"
#include "sfx2/progress.hxx"
#include "sfx2/docfile.hxx"
#include "sfx2/docfac.hxx"


#define ITEMID_FONT             0
#define ITEMID_POSTURE          0
#define ITEMID_WEIGHT           0
#define ITEMID_SHADOWED         0
#define ITEMID_CONTOUR          0
#define ITEMID_CROSSEDOUT       0
#define ITEMID_UNDERLINE        0
#define ITEMID_FONTHEIGHT       0
#define ITEMID_COLOR            0
#define ITEMID_ADJUST           0
#define ITEMID_LINESPACING      0
#define ITEMID_TABSTOP          0
#define ITEMID_LRSPACE          0
#define ITEMID_ULSPACE          0
#define ITEMID_SEARCH           0
#define ITEMID_SEARCH           0
#define ITEMID_SIZE             0
#define ITEMID_FONTLIST     0
#define ITEMID_AUTOKERN     0
#define ITEMID_WORDLINEMODE 0
#define ITEMID_PROPSIZE     0
#define ITEMID_CHARSETCOLOR 0
#define ITEMID_KERNING      0
#define ITEMID_CASEMAP      0
#define ITEMID_ESCAPEMENT   0
#define ITEMID_LANGUAGE     0
#define ITEMID_NOLINEBREAK  0
#define ITEMID_NOHYPHENHERE 0
#define ITEMID_BLINK        0
#define ITEMID_PAPERBIN     0
#define ITEMID_PRINT        0
#define ITEMID_OPAQUE       0
#define ITEMID_PROTECT      0
#define ITEMID_SHADOW       0
#define ITEMID_BOX          0
#define ITEMID_BOXINFO      0
#define ITEMID_FMTBREAK     0
#define ITEMID_FMTKEEP      0
#define ITEMID_LINE         0
#define ITEMID_BRUSH        0

#include "svl/srchitem.hxx"
#include "editeng/flstitem.hxx"
#include "editeng/fontitem.hxx"
#include "editeng/postitem.hxx"
#include "editeng/wghtitem.hxx"
#include "editeng/fhgtitem.hxx"
#include "editeng/fwdtitem.hxx"
#include "editeng/udlnitem.hxx"
#include "editeng/crsditem.hxx"
#include "editeng/shdditem.hxx"
#include "editeng/akrnitem.hxx"
#include "editeng/wrlmitem.hxx"
#include "editeng/cntritem.hxx"
#include "editeng/prszitem.hxx"
#include "editeng/colritem.hxx"
#include "editeng/cscoitem.hxx"
#include "editeng/kernitem.hxx"

#include "editeng/cmapitem.hxx"
#include "editeng/escpitem.hxx"
#include "editeng/langitem.hxx"
#include "editeng/nlbkitem.hxx"
#include "editeng/nhypitem.hxx"
#include "editeng/lcolitem.hxx"
#include "editeng/blnkitem.hxx"

#include "editeng/pbinitem.hxx"
#include "editeng/sizeitem.hxx"
#include "editeng/lrspitem.hxx"
#include "editeng/ulspitem.hxx"
#include "editeng/prntitem.hxx"
#include "editeng/opaqitem.hxx"
#include "editeng/protitem.hxx"
#include "editeng/shaditem.hxx"
#include "editeng/boxitem.hxx"
#include "editeng/brkitem.hxx"
#include "editeng/keepitem.hxx"
#include "editeng/bolnitem.hxx"
#include "editeng/brshitem.hxx"
#include "svx/xmlsecctrl.hxx"
#include "editeng/unolingu.hxx"

#include "xmlscript/xmldlg_imexp.hxx"

#include "unotools/charclass.hxx"
#include "unotools/accessiblestatesethelper.hxx"
#include "unotools/accessiblerelationsethelper.hxx"
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
