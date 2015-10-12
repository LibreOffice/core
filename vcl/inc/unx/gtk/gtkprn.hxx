/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKPRN_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKPRN_HXX

#include "generic/genprn.h"

#include <memory>

struct GtkSalPrinter_Impl;

class VCL_DLLPUBLIC GtkSalPrinter : public PspSalPrinter
{
public:
    GtkSalPrinter(SalInfoPrinter* i_pInfoPrinter);

    using PspSalPrinter::StartJob;
    virtual bool StartJob(
            const OUString* i_pFileName, const OUString& i_rJobName,
            const OUString& i_rAppName, ImplJobSetup* io_pSetupData,
            vcl::PrinterController& io_rController) override;
    virtual bool EndJob() override;

private:
    bool impl_doJob(
            const OUString* i_pFileName, const OUString& i_rJobName,
            const OUString& i_rAppName, ImplJobSetup* io_pSetupData,
            int i_nCopies, bool i_bCollate, vcl::PrinterController& io_rController);

private:
    std::unique_ptr<GtkSalPrinter_Impl> m_xImpl;
};

class VCL_DLLPUBLIC GtkSalInfoPrinter : public PspSalInfoPrinter
{
public:
    sal_uLong GetCapabilities(const ImplJobSetup* i_pSetupData, PrinterCapType i_nType) override;
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKPRN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
