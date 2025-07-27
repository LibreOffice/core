/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_DOCUMENTTABBARINTEGRATION_HXX
#define INCLUDED_SFX2_DOCUMENTTABBARINTEGRATION_HXX

#include <sfx2/dllapi.h>

class SfxViewFrame;
class DocumentTabBar;

namespace sfx2 {

/**
 * Initialize the global document tab bar system.
 * Should be called during application startup.
 */
SFX2_DLLPUBLIC void InitializeDocumentTabBar();

/**
 * Dispose the global document tab bar system.
 * Should be called during application shutdown.
 */
SFX2_DLLPUBLIC void DisposeDocumentTabBar();

/**
 * Register a view frame with the tab bar system.
 * Creates a tab for the document associated with the view frame.
 */
SFX2_DLLPUBLIC void RegisterViewFrameForTabs(SfxViewFrame* pViewFrame);

/**
 * Unregister a view frame from the tab bar system.
 * Removes the tab for the document associated with the view frame.
 */
SFX2_DLLPUBLIC void UnregisterViewFrameFromTabs(SfxViewFrame* pViewFrame);

/**
 * Set the currently active view frame.
 * Updates the active tab in the tab bar.
 */
SFX2_DLLPUBLIC void SetCurrentTabViewFrame(SfxViewFrame* pViewFrame);

/**
 * Get access to the global document tab bar widget.
 * Returns nullptr if document tabbing is not enabled.
 */
SFX2_DLLPUBLIC DocumentTabBar* GetDocumentTabBar();

/**
 * Check if document tabbing is currently enabled.
 */
SFX2_DLLPUBLIC bool IsDocumentTabbingEnabled();

/**
 * Register a DocumentTabBar widget with the global manager.
 * Each LibreOffice window should register its tab bar for synchronization.
 */
SFX2_DLLPUBLIC void RegisterDocumentTabBar(DocumentTabBar* pTabBar);

/**
 * Unregister a DocumentTabBar widget from the global manager.
 * Should be called when the tab bar is being disposed.
 */
SFX2_DLLPUBLIC void UnregisterDocumentTabBar(DocumentTabBar* pTabBar);

} // namespace sfx2

#endif // INCLUDED_SFX2_DOCUMENTTABBARINTEGRATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */