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

#ifndef INCLUDED_SW_INC_IDOCUMENTDEVICEACCESS_HXX
#define INCLUDED_SW_INC_IDOCUMENTDEVICEACCESS_HXX

class SfxPrinter;
class JobSetup;
class SwPrintData;
class VirtualDevice;
class OutputDevice;

/** Provides access to the formatting devices of a document
 */
class IDocumentDeviceAccess
{
public:
    /** Return the printer set at the document.

       @param bCreate
       [in] if set, a printer will be created if not already done.

       @returns
       the printer set at the document.
    */
    virtual SfxPrinter* getPrinter(/*[in]*/ bool bCreate) const = 0;

    /** Set the printer at the document.

       @param pP
       [in] the new printer or 0.

       @param bDeleteOld
       [in] if set, the old printer will be deleted.

       @param bCallPrtDataChanged
       [in] if set, triggers all necessary invalidations.
    */
    virtual void setPrinter(/*[in]*/ SfxPrinter* pP, /*[in]*/ bool bDeleteOld,
                            /*[in]*/ bool bCallPrtDataChanged)
        = 0;

    /** Return the virtual device set at the document.

       @param bCreate
       [in] if set, a virtual device will be created if not already done.

       @returns
       the virtual device set at the document.
    */
    virtual VirtualDevice* getVirtualDevice(/*[in]*/ bool bCreate) const = 0;

    /** Sets the current virtual device

       @param pVd
       [in] the new virtual device.

       The old virtual device will be deleted.
       Triggers all necessary invalidations.
     */
    virtual void setVirtualDevice(/*[in]*/ VirtualDevice* pVd) = 0;

    /** Returns the current reference device

       @param bCreate
       [in] if set, the reference device will be created if not already done.

       @returns
       the current reference device
     */
    virtual OutputDevice* getReferenceDevice(/*[in]*/ bool bCreate) const = 0;

    /** Sets the type of the reference device used for formatting the document

       @param bNewVirtual
       [in] if set, the reference device will be a virtual device, otherwise
            the printer is used for formatting the document

       @param bNewHiRes
       [in] if set, the virtual device will be the HiRes virtual device
     */
    virtual void setReferenceDeviceType(/*[in]*/ bool bNewVirtual, /*[in]*/ bool bNewHiRes) = 0;

    /** Returns the Jobsetup

       @returns
       the current Jobsetup
     */
    virtual const JobSetup* getJobsetup() const = 0;

    /** Sets the Jobsetup

       @param rJobSetup
       [in] the new Jobsetup.
     */
    virtual void setJobsetup(/*[in]*/ const JobSetup& rJobSetup) = 0;

    /** Returns the PrintData

       @returns
       the current PrintData
     */
    virtual const SwPrintData& getPrintData() const = 0;

    /** Sets the PrintData

       @param rPrtData
       [in] the new PrintData.
     */
    virtual void setPrintData(/*[in]*/ const SwPrintData& rPrtData) = 0;

protected:
    virtual ~IDocumentDeviceAccess(){};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
