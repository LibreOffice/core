/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:34:48 $
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
#ifndef _SD_PRINTDIALOG_HXX_
#define _SD_PRINTDIALOG_HXX_

#include <vcl/image.hxx>
#include <svtools/printdlg.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>

class SdOptionsPrintItem;

class SdPrintDialog : public PrintDialog
{
 public:
    static SdPrintDialog* Create( Window* pWindow, bool bImpress );
    virtual ~SdPrintDialog();

    void Init( const SdOptionsPrintItem* pPrintOpts );
    bool Fill( SdOptionsPrintItem* pPrintOpts );

private:
    SdPrintDialog( Window* pWindow, bool bImpress );

    void UpdateStates();
    void LoadPreviewImages();

    DECL_LINK( UpdateStatesHdl, void* );

    std::vector< boost::shared_ptr< Control > > mpControls;
    std::vector< boost::shared_ptr< Image > > mpPreviews;
    std::vector< boost::shared_ptr< Image > > mpPreviewsHC;
    bool mbImpress;
};

#endif // _SD_PRINTDIALOG_HXX_

