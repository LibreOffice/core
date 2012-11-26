/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    OSectionView( SdrModel& rModel, OReportSection* _pSectionWindow, OReportWindow* pEditor );
    virtual ~OSectionView();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void handleSelectionChange();
    virtual void MakeVisibleAtView( const basegfx::B2DRange& rRange, Window& rWin );

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
