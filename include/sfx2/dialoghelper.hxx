/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_DIALOGHELPER_HXX
#define INCLUDED_SFX2_DIALOGHELPER_HXX

#include <sfx2/dllapi.h>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>

class DateTime;
class LocaleDataWrapper;
namespace vcl { class Window; }
class VclBuilderContainer;

//when two tab pages both have the same basic layout with a preview on the
//right, get both of their non-preview areas to request the same size so that
//the preview appears in the same place in each one so flipping between tabs
//isn't distracting as it jumps around

//there has to be a "maingrid" container which contains all the widgets
//except for the preview widget
void SFX2_DLLPUBLIC setPreviewsToSamePlace(vcl::Window *pParent, VclBuilderContainer *pPage);

Size SFX2_DLLPUBLIC getParagraphPreviewOptimalSize(const vcl::Window *pReference);

Size SFX2_DLLPUBLIC getDrawPreviewOptimalSize(const vcl::Window *pReference);

Size SFX2_DLLPUBLIC getDrawListBoxOptimalSize(const vcl::Window *pReference);

Size SFX2_DLLPUBLIC getPreviewStripSize(const vcl::Window *pReference);

Size SFX2_DLLPUBLIC getPreviewOptionsSize(const vcl::Window *pReference);

OUString SFX2_DLLPUBLIC getWidestTime(const LocaleDataWrapper& rWrapper);

OUString SFX2_DLLPUBLIC formatTime(const DateTime& rDateTime, const LocaleDataWrapper& rWrapper);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
