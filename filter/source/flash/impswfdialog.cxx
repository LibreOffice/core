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


#include "impswfdialog.hxx"
#include <tools/solar.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


ImpSWFDialog::ImpSWFDialog(weld::Window* pParent, Sequence< PropertyValue >& rFilterData)
    : GenericDialogController(pParent, "filter/ui/impswfdialog.ui", "ImpSWFDialog")
    , maConfigItem("Office.Common/Filter/Flash/Export/", &rFilterData)
    , mxNumFldQuality(m_xBuilder->weld_spin_button("quality"))
    , mxCheckExportAll(m_xBuilder->weld_check_button("exportall"))
    , mxCheckExportBackgrounds(m_xBuilder->weld_check_button("exportbackgrounds"))
    , mxCheckExportBackgroundObjects(m_xBuilder->weld_check_button("exportbackgroundobjects"))
    , mxCheckExportSlideContents(m_xBuilder->weld_check_button("exportslidecontents"))
    , mxCheckExportSound(m_xBuilder->weld_check_button("exportsound"))
    , mxCheckExportOLEAsJPEG(m_xBuilder->weld_check_button("exportoleasjpeg"))
    , mxCheckExportMultipleFiles(m_xBuilder->weld_check_button("exportmultiplefiles"))
{
    const sal_uLong nCompressMode = maConfigItem.ReadInt32( "CompressMode", 75 );
    mxNumFldQuality->set_value(nCompressMode);

    mxCheckExportAll->set_active(true);
    mxCheckExportSlideContents->set_active(true);
    mxCheckExportSound->set_active(true);

    mxCheckExportAll->connect_toggled(LINK(this, ImpSWFDialog, OnToggleCheckbox));

    mxCheckExportBackgrounds->set_sensitive(false);
    mxCheckExportBackgroundObjects->set_sensitive(false);
    mxCheckExportSlideContents->set_sensitive(false);
}

ImpSWFDialog::~ImpSWFDialog()
{
}

Sequence< PropertyValue > ImpSWFDialog::GetFilterData()
{
    sal_Int32 nCompressMode = static_cast<sal_Int32>(mxNumFldQuality->get_value());
    maConfigItem.WriteInt32( "CompressMode" , nCompressMode );
    maConfigItem.WriteBool( "ExportAll", mxCheckExportAll->get_active() );
    maConfigItem.WriteBool( "ExportBackgrounds", mxCheckExportBackgrounds->get_active() );
    maConfigItem.WriteBool( "ExportBackgroundObjects", mxCheckExportBackgroundObjects->get_active() );
    maConfigItem.WriteBool( "ExportSlideContents", mxCheckExportSlideContents->get_active() );
    maConfigItem.WriteBool( "ExportSound", mxCheckExportSound->get_active() );
    maConfigItem.WriteBool( "ExportOLEAsJPEG", mxCheckExportOLEAsJPEG->get_active() );
    maConfigItem.WriteBool( "ExportMultipleFiles", mxCheckExportMultipleFiles->get_active() );

    Sequence< PropertyValue > aRet( maConfigItem.GetFilterData() );

    return aRet;
}

IMPL_LINK_NOARG(ImpSWFDialog, OnToggleCheckbox, weld::ToggleButton&, void)
{
    mxCheckExportBackgrounds->set_sensitive(!mxCheckExportBackgrounds->get_sensitive());
    mxCheckExportBackgroundObjects->set_sensitive(!mxCheckExportBackgroundObjects->get_sensitive());
    mxCheckExportSlideContents->set_sensitive(!mxCheckExportSlideContents->get_sensitive());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
