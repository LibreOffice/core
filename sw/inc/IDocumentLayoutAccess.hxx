/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_INC_IDOCUMENTLAYOUTACCESS_HXX
#define INCLUDED_SW_INC_IDOCUMENTLAYOUTACCESS_HXX

#include <swtypes.hxx>

class SwViewShell;
class SwRootFrame;
class SwFrameFormat;
class SfxItemSet;
class SwLayouter;
class SwFormatAnchor;

/** Provides access to the layout of a document.
*/
class IDocumentLayoutAccess
{
public:

    /** Returns the layout set at the document.
    */
    virtual const SwViewShell* GetCurrentViewShell() const = 0;
    virtual       SwViewShell* GetCurrentViewShell() = 0;
    virtual const SwRootFrame* GetCurrentLayout() const = 0;
    virtual       SwRootFrame* GetCurrentLayout() = 0;
    virtual bool HasLayout() const = 0;

    /** !!!The old layout must be deleted!!!
    */
    virtual void SetCurrentViewShell( SwViewShell* pNew ) = 0;

    /**
    */
    virtual SwFrameFormat* MakeLayoutFormat( RndStdIds eRequest, const SfxItemSet* pSet ) = 0;

    /**
    */
    virtual SwLayouter* GetLayouter() = 0;
    virtual const SwLayouter* GetLayouter() const = 0;
    virtual void SetLayouter( SwLayouter* pNew ) = 0;

    /**
    */
    virtual void DelLayoutFormat( SwFrameFormat *pFormat ) = 0;

    /**
    */
    virtual SwFrameFormat* CopyLayoutFormat( const SwFrameFormat& rSrc, const SwFormatAnchor& rNewAnchor,
                                     bool bSetTextFlyAtt, bool bMakeFrames ) = 0;

protected:

    virtual ~IDocumentLayoutAccess() {};
 };

 #endif // INCLUDED_SW_INC_IDOCUMENTLAYOUTACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
