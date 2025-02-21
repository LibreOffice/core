/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <gtk/gtkcssprovider.h>
#include <gtk/gtk.h>
#include <vcl/settings.hxx>

namespace CustomTheme
{
#if GTK_CHECK_VERSION(4, 0, 0)
void ApplyCustomTheme(GdkDisplay* pGdkDisplay, GtkCssProvider** pCustomThemeProvider);
#else
void ApplyCustomTheme(GdkScreen* pScreen, GtkCssProvider** pCustomThemeProvider);
#endif

OString CreateStyleString();
}
