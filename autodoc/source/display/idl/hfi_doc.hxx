/*************************************************************************
 *
 *  $RCSfile: hfi_doc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:24:50 $
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

#ifndef ADC_DISPLAY_HFI_DOC_HXX
#define ADC_DISPLAY_HFI_DOC_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS


class HF_DocEntryList;


class HF_IdlDocu : public HtmlFactory_Idl
{
  public:
                        HF_IdlDocu(
                            Environment &       io_rEnv,
                            HF_DocEntryList &   o_rOut );
    virtual             ~HF_IdlDocu();


    /** @Deprecated
        Use on of:
            ->Produce_byCesOwnDocu()
            ->Produce_byDocu4Reference()
    */
    void                Produce_byData(
                            const client &      i_ce,
                            const ce_info *     i_doc = 0 ) const;

    /** Produces documentation by the CodeInfo accompanying
        ->i_ce.
    */
    void                Produce_byCesOwnDocu(
                            const client &      i_ce ) const;
    /** Produces documentation by the CodeInfo accompanying
        a link or reference to a CodeEntity.

        @param i_rScopeGivingCe
        Gives the scope from which links are to be calculated.
    */
    void                Produce_byDocu4Reference(
                            const ce_info &     i_rDocuForReference,
                            const client &      i_rScopeGivingCe ) const;

  private:
    // Locals
    /** Produces documentation.

        @param i_rScopeGivingCe
        Gives the scope from which links are to be calculated.
    */
    void                Produce_byDocuAndScope(
                            const ce_info &     i_rDocu,
                            const client *      i_pClient,      /// May be 0.
                            const client &      i_rScopeGivingCe ) const;

    // DATA
    HF_DocEntryList &   rOut;
};


#endif
