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
#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_LABIMP_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_LABIMP_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XWordCursor.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XSentenceCursor.hpp>
#include <com/sun/star/awt/XFileDialog.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XProgressMonitor.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XMessageBox.hpp>
#include <com/sun/star/awt/XTextEditField.hpp>
#include <com/sun/star/awt/Style.hpp>
#include <com/sun/star/awt/XTimeField.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XSpinField.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XImageButton.hpp>
#include <com/sun/star/awt/XFixedText.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/XRadioButton.hpp>
#include <com/sun/star/awt/XCurrencyField.hpp>
#include <com/sun/star/awt/XPatternField.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XDateField.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XProgressBar.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <svtools/treelistbox.hxx>
#include <label.hxx>
#include <labimg.hxx>
#include <vector>

#define GETFLDVAL(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SETFLDVAL(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwLabRec
{
public:
    SwLabRec(): lHDist(0), lVDist(0), lWidth(0), lHeight(0), lLeft(0), lUpper(0), lPWidth(0), lPHeight(0), nCols(0), nRows(0), bCont(false) {}

    void SetFromItem( const SwLabItem& rItem );
    void FillItem( SwLabItem& rItem ) const;

    OUString        aMake;
    OUString        aType;
    long            lHDist;
    long            lVDist;
    long            lWidth;
    long            lHeight;
    long            lLeft;
    long            lUpper;
    long            lPWidth;
    long            lPHeight;
    sal_Int32       nCols;
    sal_Int32       nRows;
    sal_Bool        bCont;
};

class SwLabRecs : public std::vector<SwLabRec*> {
public:
    ~SwLabRecs()
    {
        for(const_iterator it = begin(); it != end(); ++it)
            delete *it;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
