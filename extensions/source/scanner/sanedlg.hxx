/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sanedlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:37:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVT_SANEDLG_HXX
#define _SVT_SANEDLG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
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
    BOOL            mbDragEnable;
    BOOL            mbIsDragging;
    int             mnDragMode;
    BOOL            mbDragDrawn;
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
    BOOL LoadState();

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
    void UpdateScanArea( BOOL );

    // helper
    BOOL SetAdjustedNumericalValue( const char* pOption, double fValue, int nElement = 0 );

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
