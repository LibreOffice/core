/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlsourcedlg.hxx"
#include "xmlsourcedlg.hrc"

#include "scresid.hxx"

ScXMLSourceDlg::ScXMLSourceDlg(Window* pParent) :
    ModalDialog(pParent, ScResId(RID_SCDLG_XML_SOURCE))
{
    FreeResource();
}

ScXMLSourceDlg::~ScXMLSourceDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
