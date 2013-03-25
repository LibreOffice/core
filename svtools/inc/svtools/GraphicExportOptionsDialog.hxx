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

#ifndef _GRAPHICEXPORTOPTIONSDIALOG_HXX_
#define _GRAPHICEXPORTOPTIONSDIALOG_HXX_

#include <vcl/fltcall.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/scrbar.hxx>

#include <svtools/DocumentToGraphicRenderer.hxx>

#include "svtools/svtdllapi.h"

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::beans;

class SVT_DLLPUBLIC GraphicExportOptionsDialog : public ModalDialog
{
private:
    const Reference<XComponent>& mxSourceDocument;

    NumericField*    mpWidth;
    NumericField*    mpHeight;
    ComboBox*        mpResolution;

    Size        mSize100mm;
    double      mResolution;

    DocumentToGraphicRenderer mRenderer;

    sal_Int32   mCurrentPage;

    void initialize();
    void updateWidth();
    void updateHeight();
    void updateResolution();

    double getViewWidthInch();
    double getViewHeightInch();

    DECL_LINK( widthModifiedHandle,         void* );
    DECL_LINK( heightModifiedHandle,        void* );
    DECL_LINK( resolutionModifiedHandle,    void* );

public:
    GraphicExportOptionsDialog( Window* pWindow, const Reference<XComponent>& rxSourceDocument );
    ~GraphicExportOptionsDialog();

    Sequence<PropertyValue> getFilterData();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
