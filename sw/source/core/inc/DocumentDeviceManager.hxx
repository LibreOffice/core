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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDEVICEMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDEVICEMANAGER_HXX

#include <IDocumentDeviceAccess.hxx>
#include <vcl/vclptr.hxx>
#include <memory>

class SwDoc;
class SfxPrinter;
class VirtualDevice;
class OutputDevice;
class JobSetup;
class SwPrintData;

namespace sw {

class DocumentDeviceManager : public IDocumentDeviceAccess
{

public:

    DocumentDeviceManager( SwDoc& i_rSwdoc );

    SfxPrinter* getPrinter(/*[in]*/ bool bCreate ) const override;

    void setPrinter(/*[in]*/ SfxPrinter *pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged ) override;

    VirtualDevice* getVirtualDevice(/*[in]*/ bool bCreate ) const override;

    void setVirtualDevice(/*[in]*/ VirtualDevice* pVd ) override;

    OutputDevice* getReferenceDevice(/*[in]*/ bool bCreate ) const override;

    void setReferenceDeviceType(/*[in]*/ bool bNewVirtual, /*[in]*/ bool bNewHiRes ) override;

    const JobSetup* getJobsetup() const override;

    void setJobsetup(/*[in]*/ const JobSetup &rJobSetup ) override;

    const SwPrintData & getPrintData() const override;

    void setPrintData(/*[in]*/ const SwPrintData& rPrtData ) override;

    virtual ~DocumentDeviceManager() override;

private:

    DocumentDeviceManager(DocumentDeviceManager const&) = delete;
    DocumentDeviceManager& operator=(DocumentDeviceManager const&) = delete;

    VirtualDevice& CreateVirtualDevice_() const;
    SfxPrinter& CreatePrinter_() const;
    void PrtDataChanged(); /**< Printer or JobSetup altered.
                                Care has to be taken of the necessary
                                invalidations and notifications. */

    SwDoc& m_rDoc;
    VclPtr<SfxPrinter> mpPrt;
    VclPtr<VirtualDevice> mpVirDev;
    std::unique_ptr<SwPrintData> mpPrtData;
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
