/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impdialog.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:47:55 $
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

#ifndef IMPDIALOG_HXX
#define IMPDIALOG_HXX

#include "pdffilter.hxx"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <svtools/FilterConfigItem.hxx>

// ----------------
// - ImpPDFDialog -
// ----------------

class ResMgr;

class ImpPDFDialog : public ModalDialog
{
private:

    FixedLine                           maFlPages;
    RadioButton                         maRbAll;
    RadioButton                         maRbRange;
    RadioButton                         maRbSelection;
    Edit                                maEdPages;
    FixedLine                           maFlCompression;
    RadioButton                         maRbLosslessCompression;
    RadioButton                         maRbJPEGCompression;
    FixedText                           maFtQuality;
    MetricField                         maNfQuality;
    CheckBox                            maCbReduceImageResolution;
    ComboBox                            maCoReduceImageResolution;
    FixedLine                           maFlGeneral;
    CheckBox                            maCbTaggedPDF;
    CheckBox                            maCbExportNotes;
    CheckBox                            maCbTransitionEffects;
    FixedText                           maFtFormsFormat;
    ListBox                             maLbFormsFormat;

    OKButton                            maBtnOK;
    CancelButton                        maBtnCancel;
    HelpButton                          maBtnHelp;

    FilterConfigItem                    maConfigItem;
    Any                                 maSelection;
    sal_Bool                            mbIsPresentation;

                                        DECL_LINK( TogglePagesHdl, void* );
                                        DECL_LINK( ToggleCompressionHdl, void* );
                                        DECL_LINK( ToggleReduceImageResolutionHdl, void* );

public:

                                        ImpPDFDialog( Window* pParent, ResMgr& rResMgr,
                                                      Sequence< PropertyValue >& rFilterData,
                                                      const Reference< XComponent >& rDoc );
                                        ~ImpPDFDialog();

    Sequence< PropertyValue >           GetFilterData();
};

#endif // IMPDIALOG_HXX
