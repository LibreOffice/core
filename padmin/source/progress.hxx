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
        sal_Bool mbCanceled;

    public:
        ProgressDialog( Window*, sal_Bool bCancelable = sal_True, int nMin = 0, int nMax = 100 );
        ~ProgressDialog();

        DECL_LINK( ClickBtnHdl, Button* );

        void setValue( int nValue );
        void setRange( int nMin, int nMax ) { mnMin = nMin; mnMax = nMax; }
        void startOperation( const String& );
        void setFilename( const String& );

        sal_Bool isCanceled() { return mbCanceled; }
    };

} // namespace

#endif
