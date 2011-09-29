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
 *   Copyright (C) 2011 SUSE, Inc.
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

#ifndef _SV_GENERIC_INST_H
#define _SV_GENERIC_INST_H

#include <vcl/sv.h>
#include <osl/thread.hxx>
#include <vclpluginapi.h>
#include <salinst.hxx>

/*
 * Abstract generic class to build vclplugin's instance classes from
 */
class ImplDevFontList;
class VCL_DLLPUBLIC GenericInstance : public SalInstance
{
protected:
    bool mbPrinterInit;

public:
    GenericInstance() : mbPrinterInit( false ) {}
    virtual ~GenericInstance() {}

    virtual SalInfoPrinter*     CreateInfoPrinter      ( SalPrinterQueueInfo* pQueueInfo,
                                                         ImplJobSetup* pSetupData );
    virtual void                DestroyInfoPrinter     ( SalInfoPrinter* pPrinter );
    virtual SalPrinter*         CreatePrinter          ( SalInfoPrinter* pInfoPrinter );
    virtual void                DestroyPrinter         ( SalPrinter* pPrinter );

    virtual void                GetPrinterQueueInfo    ( ImplPrnQueueList* pList );
    virtual void                GetPrinterQueueState   ( SalPrinterQueueInfo* pInfo );
    virtual void                DeletePrinterQueueInfo ( SalPrinterQueueInfo* pInfo );
    virtual String              GetDefaultPrinter();
    virtual void                PostPrintersChanged() = 0;

    virtual void updatePrinterUpdate();
    virtual void jobStartedPrinterUpdate();
    virtual void jobEndedPrinterUpdate();

    bool isPrinterInit() const { return mbPrinterInit; }
    static void RegisterFontSubstitutors( ImplDevFontList* pList );
};

#endif // _SV_GENERIC_INST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
