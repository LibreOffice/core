/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#ifndef UNX_GTK_GTKPRN_HXX_INCLUDED
#define UNX_GTK_GTKPRN_HXX_INCLUDED

#include "generic/genprn.h"

#include <boost/scoped_ptr.hpp>

struct GtkSalPrinter_Impl;


class VCL_DLLPUBLIC GtkSalPrinter : public PspSalPrinter
{
public:
    GtkSalPrinter(SalInfoPrinter* i_pInfoPrinter);

    using PspSalPrinter::StartJob;
    virtual sal_Bool StartJob(
            const OUString* i_pFileName, const OUString& i_rJobName,
            const OUString& i_rAppName, ImplJobSetup* io_pSetupData,
            vcl::PrinterController& io_rController);
    virtual sal_Bool EndJob();

private:
    bool impl_doJob(
            const OUString* i_pFileName, const OUString& i_rJobName,
            const OUString& i_rAppName, ImplJobSetup* io_pSetupData,
            int i_nCopies, bool i_bCollate, vcl::PrinterController& io_rController);

private:
    boost::scoped_ptr<GtkSalPrinter_Impl> m_pImpl;
};


class VCL_DLLPUBLIC GtkSalInfoPrinter : public PspSalInfoPrinter
{
public:
    sal_uLong GetCapabilities(const ImplJobSetup* i_pSetupData, sal_uInt16 i_nType);
};


#endif // UNX_GTK_GTKPRN_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
