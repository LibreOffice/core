/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_PROPERTY_MAPPING_HXX
#define CHART2_PROPERTY_MAPPING_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/svtabbx.hxx>

#include <com/sun/star/chart2/XChartType.hpp>

namespace chart {

class PropertyMappingDlg : public ModalDialog
{
public:

    PropertyMappingDlg(Window* pParent, com::sun::star::uno::Reference<
            com::sun::star::chart2::XChartType > xChartType);

    OUString getSelectedEntry();

private:

    DECL_LINK( OkBtnHdl, void* );
    DECL_LINK( CancelBtnHdl, void* );

    SvTabListBox* mpMappingTable;
    Button* mpBtnOk;
    Button* mpBtnCancel;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
