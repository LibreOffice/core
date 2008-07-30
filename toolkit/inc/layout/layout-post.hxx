/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layout-post.hxx,v $
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

#ifndef _LAYOUT_POST_HXX
#define _LAYOUT_POST_HXX

#if ENABLE_LAYOUT

#undef AdvancedButton
#undef Box
#undef Button
#undef CancelButton
#undef CheckBox
#undef Container
#undef FixedImage
#undef FixedInfo
#undef FixedLine
#undef FixedText
#undef HelpButton
#undef HBox
#undef MetricField
#undef MoreButton
#undef MultiLineEdit
#undef OKButton
#undef ProgressBar
#undef PushButton
#undef RadioButton
#undef SfxModalDialog
#undef Table
#undef VBox

#undef Window

#undef SVX_RES
#define SVX_RES(i) ResId(i,DIALOG_MGR())

#undef SW_RES
#define SW_RES(i) ResId(i,SWDIALOG_MGR())

/* Allow re-inclusion for cxx file. */
#undef _LAYOUT_PRE_HXX

#endif /* ENABLE_LAYOUT */

#endif /* _LAYOUT_POST_HXX */
