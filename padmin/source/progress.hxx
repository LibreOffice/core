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
