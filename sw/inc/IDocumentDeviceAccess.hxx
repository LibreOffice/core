/*************************************************************************
 *
 *  $RCSfile: IDocumentDeviceAccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:12:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

 #ifndef IDOCUMENTDEVICEACCESS_HXX_INCLUDED
 #define IDOCUMENTDEVICEACCESS_HXX_INCLUDED

 class SfxPrinter;
 class JobSetup;
 struct SwPrintData;
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
    virtual SwPrintData* getPrintData() const = 0;

    /** Sets the PrintData

       @param rPrtData
       [in] the new PrintData.
     */
    virtual void setPrintData(/*[in]*/ const SwPrintData& rPrtData) = 0;

 protected:
    virtual ~IDocumentDeviceAccess() {};
 };

 #endif // IDOCUMENTDEVICEACCESS_HXX_INCLUDED
