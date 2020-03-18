/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_LOK_HXX
#define INCLUDED_COMPHELPER_LOK_HXX

#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>

class LanguageTag;

// Interface between the LibreOfficeKit implementation called by LibreOfficeKit clients and other
// LibreOffice code.

namespace comphelper
{

namespace LibreOfficeKit
{

// Functions to be called only from the LibreOfficeKit implementation in desktop, not from other
// places in LibreOffice code.

COMPHELPER_DLLPUBLIC void setActive(bool bActive = true);

// Note that currently it is undefined behaviour to call both setMobilePhone() and setTablet(). This
// will be remedied in the future. For now, just make sure you call just either for a view, and just
// once, with the bool parameter as true. In the future, this will probably be changed into using an
// enum for the kind of the view, that can be DESKTOP, MOBILEPHONE, or TABLET.

// Tell that LOK view is on a mobile phone (regardless what its pixel resolution is, whether its form factor is "phablet" or not)
COMPHELPER_DLLPUBLIC void setMobilePhone(int nViewId, bool bIsMobilePhone);

// Tell that LOK view is on a tablet
COMPHELPER_DLLPUBLIC void setTablet(int nViewId, bool bIsTablet);

enum class statusIndicatorCallbackType { Start, SetValue, Finish };

COMPHELPER_DLLPUBLIC void setStatusIndicatorCallback(void (*callback)(void *data, statusIndicatorCallbackType type, int percent), void *data);


// Functions that can be called from arbitrary places in LibreOffice.

// Check whether the code is running as invoked through LibreOfficeKit.
COMPHELPER_DLLPUBLIC bool isActive();

// Check whether we are serving to a mobile phone
COMPHELPER_DLLPUBLIC bool isMobilePhone(int nViewId);

// Check whether we are serving to a tablet
COMPHELPER_DLLPUBLIC bool isTablet(int nViewId);

/// Shift the coordinates before rendering each bitmap.
/// Used by Calc to render each tile separately.
/// This should be unnecessary (and removed) once Calc
/// moves to using 100MM Unit.
COMPHELPER_DLLPUBLIC void setLocalRendering(bool bLocalRendering = true);
COMPHELPER_DLLPUBLIC bool isLocalRendering();

/// Check whether clients want a part number in an invalidation payload.
COMPHELPER_DLLPUBLIC bool isPartInInvalidation();
/// Set whether clients want a part number in an invalidation payload.
COMPHELPER_DLLPUBLIC void setPartInInvalidation(bool bPartInInvalidation);

/// Check if we are doing tiled painting.
COMPHELPER_DLLPUBLIC bool isTiledPainting();
/// Set if we are doing tiled painting.
COMPHELPER_DLLPUBLIC void setTiledPainting(bool bTiledPainting);
/// Check if we are painting the dialog.
COMPHELPER_DLLPUBLIC bool isDialogPainting();
/// Set if we are painting the dialog.
COMPHELPER_DLLPUBLIC void setDialogPainting(bool bDialogPainting);
/// Set the DPI scale for rendering for HiDPI displays.
COMPHELPER_DLLPUBLIC void setDPIScale(double fDPIScale);
/// Get the DPI scale for rendering for HiDPI displays.
COMPHELPER_DLLPUBLIC double getDPIScale();
/// Set if we want no annotations rendering
COMPHELPER_DLLPUBLIC void setTiledAnnotations(bool bTiledAnnotations);
/// Check if annotations rendering is turned off
COMPHELPER_DLLPUBLIC bool isTiledAnnotations();
/// Set if we want range based header data
COMPHELPER_DLLPUBLIC void setRangeHeaders(bool bTiledAnnotations);
/// Check if range based header data is enabled
COMPHELPER_DLLPUBLIC bool isRangeHeaders();

enum Compat : sal_uInt32
{
    none = 0,
    scNoGridBackground = 1,
};
/// Set compatibility flags
COMPHELPER_DLLPUBLIC void setCompatFlag(Compat flag);
/// Get compatibility flags
COMPHELPER_DLLPUBLIC bool isCompatFlagSet(Compat flag);


/// Check whether clients want viewId in visible cursor invalidation payload.
COMPHELPER_DLLPUBLIC bool isViewIdForVisCursorInvalidation();
/// Set whether clients want viewId in visible cursor invalidation payload.
COMPHELPER_DLLPUBLIC void setViewIdForVisCursorInvalidation(bool bViewIdForVisCursorInvalidation);

/// Update the current LOK's language.
COMPHELPER_DLLPUBLIC void setLanguageTag(const LanguageTag& languageTag);
/// Get the current LOK's language.
COMPHELPER_DLLPUBLIC const LanguageTag& getLanguageTag();
/// If the language name should be used for this LOK instance.
COMPHELPER_DLLPUBLIC bool isWhitelistedLanguage(const OUString& lang);

// Status indicator handling. Even if in theory there could be several status indicators active at
// the same time, in practice there is only one at a time, so we don't handle any identification of
// status indicator in this API.
COMPHELPER_DLLPUBLIC void statusIndicatorStart();
COMPHELPER_DLLPUBLIC void statusIndicatorSetValue(int percent);
COMPHELPER_DLLPUBLIC void statusIndicatorFinish();

}
}

#endif // INCLUDED_COMPHELPER_LOK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
