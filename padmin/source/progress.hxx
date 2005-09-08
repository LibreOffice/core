/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progress.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:28:28 $
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

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _PRGSBAR_HXX
#include <prgsbar.hxx>
#endif

namespace padmin {

    class ProgressDialog : public ModelessDialog
    {
        FixedText                       maOperation;
        FixedText                       maFilename;
        FixedText                       maProgressTxt;
        CancelButton                    maCancelButton;
        ::padmin::ProgressBar           maProgressBar;

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
