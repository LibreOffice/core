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


    /** Produces documentation by the CodeInfo accompanying
        ->i_ce.
    */
    void                Produce_fromCodeEntity(
                            const client &      i_ce ) const;

    /** Produces documentation by the CodeInfo accompanying
        a link or reference to a CodeEntity.

        @param i_rScopeGivingCe
        Gives the scope from which links are to be calculated.
    */
    void                Produce_fromReference(
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
