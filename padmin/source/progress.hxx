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

#ifndef _PAD_PROGRESS_HXX_
#define _PAD_PROGRESS_HXX_

#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/button.hxx"

#include "svtools/prgsbar.hxx"

namespace padmin {

    class ProgressDialog : public ModelessDialog
    {
        FixedText                       maOperation;
        FixedText                       maFilename;
        FixedText                       maProgressTxt;
        CancelButton                    maCancelButton;
        ProgressBar                     maProgressBar;

        int mnMax, mnMin;
        bool mbCanceled;

    public:
        ProgressDialog( Window*, sal_Bool bCancelable = sal_True, int nMin = 0, int nMax = 100 );
        ~ProgressDialog();

        DECL_LINK( ClickBtnHdl, Button* );

        void setValue( int nValue );
        void setRange( int nMin, int nMax ) { mnMin = nMin; mnMax = nMax; }
        void startOperation( const String& );
        void setFilename( const String& );

        bool isCanceled() { return mbCanceled; }
    };

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
