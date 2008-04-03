/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progress.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 17:17:53 $
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
        BOOL mbCanceled;

    public:
        ProgressDialog( Window*, BOOL bCancelable = TRUE, int nMin = 0, int nMax = 100 );
        ~ProgressDialog();

        DECL_LINK( ClickBtnHdl, Button* );

        void setValue( int nValue );
        void setRange( int nMin, int nMax ) { mnMin = nMin; mnMax = nMax; }
        void startOperation( const String& );
        void setFilename( const String& );

        BOOL isCanceled() { return mbCanceled; }
    };

} // namespace

#endif
