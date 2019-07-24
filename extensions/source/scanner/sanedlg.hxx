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

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include "sane.hxx"

class ScanPreview;

class SaneDlg : public weld::GenericDialogController
{
private:
    weld::Window*          mpParent;
    Sane&                  mrSane;
    bool                   mbScanEnabled;

    Link<Sane&,void>       maOldLink;

    int             mnCurrentOption;
    int             mnCurrentElement;
    std::unique_ptr<double[]> mpRange;
    double          mfMin, mfMax;

    bool            doScan;

    std::unique_ptr<weld::Button> mxCancelButton;
    std::unique_ptr<weld::Button> mxDeviceInfoButton;
    std::unique_ptr<weld::Button> mxPreviewButton;
    std::unique_ptr<weld::Button> mxScanButton;
    std::unique_ptr<weld::Button> mxButtonOption;

    std::unique_ptr<weld::Label> mxOptionTitle;
    std::unique_ptr<weld::Label> mxOptionDescTxt;
    std::unique_ptr<weld::Label> mxVectorTxt;

    std::unique_ptr<weld::MetricSpinButton> mxLeftField;
    std::unique_ptr<weld::MetricSpinButton> mxTopField;
    std::unique_ptr<weld::MetricSpinButton> mxRightField;
    std::unique_ptr<weld::MetricSpinButton> mxBottomField;

    std::unique_ptr<weld::ComboBox> mxDeviceBox;
    std::unique_ptr<weld::ComboBox> mxReslBox;
    std::unique_ptr<weld::CheckButton> mxAdvancedBox;

    std::unique_ptr<weld::SpinButton> mxVectorBox;
    std::unique_ptr<weld::ComboBox> mxQuantumRangeBox;
    std::unique_ptr<weld::ComboBox> mxStringRangeBox;

    std::unique_ptr<weld::CheckButton> mxBoolCheckBox;

    std::unique_ptr<weld::Entry> mxStringEdit;
    std::unique_ptr<weld::Entry> mxNumericEdit;

    std::unique_ptr<weld::TreeView>  mxOptionBox;

    std::unique_ptr<ScanPreview> mxPreview;
    std::unique_ptr<weld::CustomWeld> mxPreviewWnd;

    DECL_LINK( ClickBtnHdl, weld::Button&, void );
    DECL_LINK( SelectHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( MetricValueModifyHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ValueModifyHdl, weld::ComboBox&, void );
    DECL_LINK( ReloadSaneOptionsHdl, Sane&, void );
    DECL_LINK( OptionsBoxSelectHdl, weld::TreeView&, void );

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

    // helper
    bool SetAdjustedNumericalValue( const char* pOption, double fValue, int nElement = 0 );
public:
    SaneDlg(weld::Window*, Sane&, bool);
    virtual ~SaneDlg() override;

    virtual short run() override;
    void UpdateScanArea( bool );
    bool getDoScan() { return doScan;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
