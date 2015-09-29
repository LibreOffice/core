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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLAYOUTMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLAYOUTMANAGER_HXX

#include <IDocumentLayoutAccess.hxx>
#include <boost/noncopyable.hpp>

class SwDoc;
class SwViewShell;
class SwLayouter;

namespace sw {

class DocumentLayoutManager : public IDocumentLayoutAccess,
                              public ::boost::noncopyable
{

public:

    DocumentLayoutManager( SwDoc& i_rSwdoc );

    virtual const SwViewShell *GetCurrentViewShell() const SAL_OVERRIDE;
    virtual SwViewShell *GetCurrentViewShell() SAL_OVERRIDE; //< It must be able to communicate to a SwViewShell.This is going to be removed later.
    virtual void SetCurrentViewShell( SwViewShell* pNew ) SAL_OVERRIDE;

    virtual const SwRootFrm *GetCurrentLayout() const SAL_OVERRIDE;
    virtual SwRootFrm *GetCurrentLayout() SAL_OVERRIDE;
    virtual bool HasLayout() const SAL_OVERRIDE;

    virtual const SwLayouter* GetLayouter() const SAL_OVERRIDE;
    virtual SwLayouter* GetLayouter() SAL_OVERRIDE;
    virtual void SetLayouter( SwLayouter* pNew ) SAL_OVERRIDE;

    virtual SwFrameFormat* MakeLayoutFormat( RndStdIds eRequest, const SfxItemSet* pSet ) SAL_OVERRIDE;
    virtual void DelLayoutFormat( SwFrameFormat *pFormat ) SAL_OVERRIDE;
    virtual SwFrameFormat* CopyLayoutFormat( const SwFrameFormat& rSrc, const SwFormatAnchor& rNewAnchor, bool bSetTextFlyAtt, bool bMakeFrms ) SAL_OVERRIDE;

    //Non Interface methods
    void ClearSwLayouterEntries();

    virtual ~DocumentLayoutManager();

private:

    SwDoc& m_rDoc;

    SwViewShell *mpCurrentView; //< SwDoc should get a new member mpCurrentView
    SwLayouter *mpLayouter; /**< ::com::sun::star::frame::Controller for complex layout formatting
                           like footnote/endnote in sections */
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
