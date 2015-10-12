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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_SECTIONVIEW_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_SECTIONVIEW_HXX


#include <svx/svdview.hxx>
namespace rptui
{
class OReportWindow;
class OReportSection;


// OSectionView


class OSectionView : public SdrView
{
private:
    VclPtr<OReportWindow>      m_pReportWindow;
    VclPtr<OReportSection>     m_pSectionWindow;

    void ObjectRemovedInAliveMode( const SdrObject* pObject );
    OSectionView(const OSectionView&) = delete;
    void operator =(const OSectionView&) = delete;
public:
    TYPEINFO_OVERRIDE();

    OSectionView( SdrModel* pModel, OReportSection* _pSectionWindow, OReportWindow* pEditor );
    virtual ~OSectionView();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
    virtual void MarkListHasChanged() SAL_OVERRIDE;
    virtual void MakeVisible( const Rectangle& rRect, vcl::Window& rWin ) SAL_OVERRIDE;

    inline OReportSection*  getReportSection() const { return m_pSectionWindow; }

    // switch the marked objects to the given layer.
    void SetMarkedToLayer( SdrLayerID nLayerNo );

    // return true when only shapes are marked, otherwise false.
    bool OnlyShapesMarked() const;

    /* returns the common layer id of the marked objects, otherwise -1 will be returned.
    */
    short GetLayerIdOfMarkedObjects() const;

    // returns true if objects at Drag & Drop is resize not move
    bool IsDragResize() const;
};
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_SECTIONVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
