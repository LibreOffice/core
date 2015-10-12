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

class ScanPreview;

class SaneDlg : public ModalDialog
{
private:
    Sane&                  mrSane;
    bool                   mbScanEnabled;

    Link<Sane&,void>       maOldLink;

    VclPtr<OKButton>       mpOKButton;
    VclPtr<CancelButton>   mpCancelButton;
    VclPtr<PushButton>     mpDeviceInfoButton;
    VclPtr<PushButton>     mpPreviewButton;
    VclPtr<PushButton>     mpScanButton;
    VclPtr<PushButton>     mpButtonOption;

    VclPtr<FixedText>      mpOptionTitle;
    VclPtr<FixedText>      mpOptionDescTxt;
    VclPtr<FixedText>      mpVectorTxt;

    VclPtr<MetricField>    mpLeftField;
    VclPtr<MetricField>    mpTopField;
    VclPtr<MetricField>    mpRightField;
    VclPtr<MetricField>    mpBottomField;

    VclPtr<ListBox>        mpDeviceBox;
    VclPtr<NumericBox>     mpReslBox;
    VclPtr<CheckBox>       mpAdvancedBox;

    VclPtr<NumericField>   mpVectorBox;
    VclPtr<ListBox>        mpQuantumRangeBox;
    VclPtr<ListBox>        mpStringRangeBox;

    VclPtr<CheckBox>       mpBoolCheckBox;

    VclPtr<Edit>           mpStringEdit;
    VclPtr<Edit>           mpNumericEdit;

    VclPtr<SvTreeListBox>  mpOptionBox;

    VclPtr<ScanPreview>    mpPreview;

    int             mnCurrentOption;
    int             mnCurrentElement;
    double*         mpRange;
    double          mfMin, mfMax;

    bool            doScan;

    DECL_LINK_TYPED( ClickBtnHdl, Button*, void );
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
    DECL_LINK( ModifyHdl, Edit* );
    DECL_LINK_TYPED( ReloadSaneOptionsHdl, Sane&, void );
    DECL_LINK_TYPED( OptionsBoxSelectHdl, SvTreeListBox*, void );

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
    SaneDlg( vcl::Window*, Sane&, bool );
    virtual ~SaneDlg();
    virtual void dispose() override;

    virtual short Execute() override;
    void UpdateScanArea( bool );
    bool getDoScan() { return doScan;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
