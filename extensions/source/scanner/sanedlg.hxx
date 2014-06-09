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
#ifndef INCLUDED_EXTENSIONS_SOURCE_SCANNER_SANEDLG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_SCANNER_SANEDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <svtools/treelistbox.hxx>

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
    bool            mbDragEnable;
    bool            mbIsDragging;
    bool            mbScanEnabled;
    bool            mbDragDrawn;
    DragDirection   meDragDirection;

    MapMode         maMapMode;

    Link            maOldLink;

    OKButton*       mpOKButton;
    CancelButton*   mpCancelButton;
    PushButton*     mpDeviceInfoButton;
    PushButton*     mpPreviewButton;
    PushButton*     mpScanButton;
    PushButton*     mpButtonOption;

    FixedText*      mpOptionTitle;
    FixedText*      mpOptionDescTxt;
    FixedText*      mpVectorTxt;

    MetricField*    mpLeftField;
    MetricField*    mpTopField;
    MetricField*    mpRightField;
    MetricField*    mpBottomField;

    ListBox*        mpDeviceBox;
    NumericBox*     mpReslBox;
    CheckBox*       mpAdvancedBox;

    NumericField*   mpVectorBox;
    ListBox*        mpQuantumRangeBox;
    ListBox*        mpStringRangeBox;

    CheckBox*       mpBoolCheckBox;

    Edit*           mpStringEdit;
    Edit*           mpNumericEdit;

    SvTreeListBox*  mpOptionBox;

    Window*         mpPreview;

    int             mnCurrentOption;
    int             mnCurrentElement;
    double*         mpRange;
    double          mfMin, mfMax;

    bool            doScan;

    DECL_LINK( ClickBtnHdl, Button* );
    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( ModifyHdl, Edit* );
    DECL_LINK( ReloadSaneOptionsHdl, Sane* );
    DECL_LINK( OptionsBoxSelectHdl, SvTreeListBox* );

    void SaveState();
    bool LoadState();

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
    void UpdateScanArea( bool );

    // helper
    bool SetAdjustedNumericalValue( const char* pOption, double fValue, int nElement = 0 );

    virtual void Paint( const Rectangle& ) SAL_OVERRIDE;
    virtual void MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
public:
    SaneDlg( Window*, Sane&, bool );
    virtual ~SaneDlg();

    virtual short Execute() SAL_OVERRIDE;
    bool getDoScan() { return doScan;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
