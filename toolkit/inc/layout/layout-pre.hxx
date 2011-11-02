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

#ifndef _LAYOUT_PRE_HXX
#define _LAYOUT_PRE_HXX

#if ENABLE_LAYOUT

///* Allow re-inclusion for next hxx file. */
#undef _LAYOUT_POST_HXX

#define Box layout::Box
#define Button layout::Button
#define CancelButton layout::CancelButton
#define Container layout::Container
#define Control layout::Control
#define Dialog layout::Dialog
#define Edit layout::Edit
#define FixedImage layout::FixedImage
#define FixedText layout::FixedText
#define HelpButton layout::HelpButton
#define IgnoreButton layout::IgnoreButton
#define ListBox layout::ListBox
#define MessBox layout::MessBox
#define MessageBox layout::MessageBox
#define MetricFormatter layout::MetricFormatter
#define NoButton layout::NoButton
#define NumericFormatter layout::NumericFormatter
#define PushButton layout::PushButton
#define RetryButton layout::RetryButton
#define SfxTabDialog layout::SfxTabDialog
#define SfxTabPage layout::SfxTabPage
#define SvxFontListBox layout::SvxFontListBox
#define SvxLanguageBox layout::SvxLanguageBox
#define TabControl layout::TabControl
#define TabPage layout::TabPage
#define Table layout::Table
#define YesButton layout::YesButton

#define ModalDialog Dialog
#define ModelessDialog Dialog
#define ScExpandedFixedText FixedText
#define SfxDialog Dialog
#define SfxModalDialog Dialog
#define SfxModelessDialog Dialog
#define TabDialog Dialog

#define Window ::Window

#endif /* ENABLE_LAYOUT */

#define LocalizedString String

#endif /* _LAYOUT_PRE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
