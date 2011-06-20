/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

 #ifndef IDOCUMENTDEVICEACCESS_HXX_INCLUDED
 #define IDOCUMENTDEVICEACCESS_HXX_INCLUDED

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
    virtual SfxPrinter* getPrinter(/*[in]*/ bool bCreate ) const = 0;

    /** Set the printer at the document.

       @param pP
       [in] the new printer or 0.

       @param bDeleteOld
       [in] if set, the old printer will be deleted.

       @param bCallPrtDataChanged
       [in] if set, triggers all necessary invalidations.

       @returns
       the printer set at the document.
    */
    virtual void setPrinter(/*[in]*/ SfxPrinter* pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged ) = 0;

    /** Return the virtual device set at the document.

       @param bCreate
       [in] if set, a virtual device will be created if not already done.

       @returns
       the virtual device set at the document.
    */
    virtual VirtualDevice* getVirtualDevice(/*[in]*/ bool bCreate ) const = 0;

    /** Sets the current virtual device

       @param pVd
       [in] the new virtual device.

       @param bDeleteOld
       [in] if set, the old virtual device will be deleted.

       @param bCallVirDevDataChanged
       [in] if set, triggers all necessary invalidations.
     */
    virtual void setVirtualDevice(/*[in]*/ VirtualDevice* pVd,/*[in]*/ bool bDeleteOld, /*[in]*/ bool bCallVirDevDataChanged ) = 0;

    /** Returns the current reference device

       @param bCreate
       [in] if set, the reference device will be created if not already done.

       @returns
       the current reference device
     */
    virtual OutputDevice* getReferenceDevice(/*[in]*/ bool bCreate ) const = 0;

    /** Sets the type of the reference device used for formatting the document

       @param bNewVirtual
       [in] if set, the reference device will be a virtual device, otherwise
            the printer is used for formatting the document

       @param bNewHiRes
       [in] if set, the virtual device will be the HiRes virtual device
     */
    virtual void setReferenceDeviceType(/*[in]*/ bool bNewVirtual, /*[in]*/ bool bNewHiRes ) = 0;

    /** Returns the Jobsetup

       @returns
       the current Jobsetup
     */
    virtual const JobSetup* getJobsetup() const = 0;

    /** Sets the Jobsetup

       @param rJobSetup
       [in] the new Jobsetup.
     */
    virtual void  setJobsetup(/*[in]*/ const JobSetup& rJobSetup ) = 0;

    /** Returns the PrintData

       @returns
       the current PrintData
     */
    virtual const SwPrintData & getPrintData() const = 0;

    /** Sets the PrintData

       @param rPrtData
       [in] the new PrintData.
     */
    virtual void setPrintData(/*[in]*/ const SwPrintData& rPrtData) = 0;

 protected:
    virtual ~IDocumentDeviceAccess() {};
 };

 #endif // IDOCUMENTDEVICEACCESS_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
