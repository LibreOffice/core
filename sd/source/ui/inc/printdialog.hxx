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

