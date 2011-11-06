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


#ifndef _SVT_SANEDLG_HXX
#define _SVT_SANEDLG_HXX

#include <vcl/dialog.hxx>
#include <tools/config.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <svtools/svtreebx.hxx>

#include <sane.hxx>

class SaneDlg : public ModalDialog
{
private:
    enum DragDirection { TopLeft, Top, TopRight, Right, BottomRight, Bottom,
                         BottomLeft, Left };

    Sane&           mrSane;
    Bitmap          maPreviewBitmap;
    Rectangle       maPreviewRect;
    Point           maTopLeft, maBottomRight;
    Point           maMinTopLeft, maMaxBottomRight;
    sal_Bool            mbDragEnable;
    sal_Bool            mbIsDragging;
    int             mnDragMode;
    sal_Bool            mbDragDrawn;
    DragDirection   meDragDirection;

    MapMode         maMapMode;

    Link            maOldLink;

    OKButton        maOKButton;
    CancelButton    maCancelButton;
    PushButton      maDeviceInfoButton;
    PushButton      maPreviewButton;
    PushButton      maButtonOption;

    FixedText       maOptionsTxt;
    FixedText       maOptionTitle;
    FixedText       maOptionDescTxt;
    FixedText       maVectorTxt;

    FixedText       maScanLeftTxt;
    MetricField     maLeftField;
    FixedText       maScanTopTxt;
    MetricField     maTopField;
    FixedText       maRightTxt;
    MetricField     maRightField;
    FixedText       maBottomTxt;
    MetricField     maBottomField;

    FixedText       maDeviceBoxTxt;
    ListBox         maDeviceBox;
    FixedText       maReslTxt;
    NumericBox      maReslBox;
    FixedText       maAdvancedTxt;
    CheckBox        maAdvancedBox;

    NumericField    maVectorBox;
    ListBox         maQuantumRangeBox;
    ListBox         maStringRangeBox;

    FixedLine       maPreviewBox;
    FixedLine       maAreaBox;

    CheckBox        maBoolCheckBox;

    Edit            maStringEdit;
    Edit            maNumericEdit;

    SvTreeListBox   maOptionBox;

    int             mnCurrentOption;
    int             mnCurrentElement;
    double*         mpRange;
    double          mfMin, mfMax;

    DECL_LINK( ClickBtnHdl, Button* );
    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( ModifyHdl, Edit* );
    DECL_LINK( ReloadSaneOptionsHdl, Sane* );
    DECL_LINK( OptionsBoxSelectHdl, SvTreeListBox* );

    void SaveState();
    sal_Bool LoadState();

    void InitDevices();
    void InitFields();
    void AcquirePreview();
    void DisableOption();
    void EstablishBoolOption();
    void EstablishStringOption();
    void EstablishStringRange();
    void EstablishQuantumRange();
    void EstablishNumericOption();
    void EstablishButtonOption();

    void DrawRectangles( Point&, Point& );
    void DrawDrag();
    Point GetPixelPos( const Point& );
    Point GetLogicPos( const Point& );
    void UpdateScanArea( sal_Bool );

    // helper
    sal_Bool SetAdjustedNumericalValue( const char* pOption, double fValue, int nElement = 0 );

    virtual void Paint( const Rectangle& );
    virtual void MouseMove( const MouseEvent& rMEvt );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    virtual void MouseButtonUp( const MouseEvent& rMEvt );
public:
    SaneDlg( Window*, Sane& );
    ~SaneDlg();

    virtual short Execute();
};


#endif
