/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifndef _DLGEOS2_HXX_
#define _DLGEOS2_HXX_
#include <svtools/fltcall.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen bei Vektorformaten
|*
\************************************************************************/

class FilterConfigItem;
class ResMgr;

class DlgExportEMET : public ModalDialog
{
private:

    FltCallDialogParameter& rFltCallPara;

    OKButton			aBtnOK;
    CancelButton		aBtnCancel;
    HelpButton			aBtnHelp;

    RadioButton			aRbOriginal;
    RadioButton			aRbSize;
    FixedLine			aGrpMode;

    FixedText			aFtSizeX;
    MetricField			aMtfSizeX;
    FixedText			aFtSizeY;
    MetricField			aMtfSizeY;
    FixedLine           aGrpSize;

    FilterConfigItem*	pConfigItem;
    ResMgr*				pMgr;

    DECL_LINK( OK, void* p );
    DECL_LINK( ClickRbOriginal,void* p );
    DECL_LINK( ClickRbSize,void* p );

public:
            DlgExportEMET( FltCallDialogParameter& rPara );
            ~DlgExportEMET();
};


#endif // _DLGEMET_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
