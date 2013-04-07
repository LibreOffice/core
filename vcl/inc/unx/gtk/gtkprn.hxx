/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 * Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com> ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
