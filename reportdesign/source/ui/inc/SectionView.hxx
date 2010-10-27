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

#ifndef _REPORT_SECTIONVIEW_HXX
#define _REPORT_SECTIONVIEW_HXX


#include <svx/svdview.hxx>
namespace rptui
{
class OReportWindow;
class OReportSection;

//============================================================================
// OSectionView
//============================================================================

class OSectionView : public SdrView
{
private:
    OReportWindow*      m_pReportWindow;
    OReportSection*     m_pSectionWindow;

    void ObjectRemovedInAliveMode( const SdrObject* pObject );
    OSectionView(const OSectionView&);
    void operator =(const OSectionView&);
public:
    TYPEINFO();

    OSectionView( SdrModel* pModel, OReportSection* _pSectionWindow, OReportWindow* pEditor );
    virtual ~OSectionView();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void MarkListHasChanged();
    virtual void MakeVisible( const Rectangle& rRect, Window& rWin );

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
#endif //_REPORT_SECTIONVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
