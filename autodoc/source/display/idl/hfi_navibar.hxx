/*************************************************************************
 *
 *  $RCSfile: hfi_navibar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 11:33:40 $
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

#ifndef ADC_DISPLAY_HFI_NAVIBAR_HXX
#define ADC_DISPLAY_HFI_NAVIBAR_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "hi_factory.hxx"
    // PARAMETERS

namespace ary
{
namespace idl
{
class CodeEntity;
}
}


class HF_NaviSubRow;

/** @resp
    Creates a navigation bar for an IDL HTML documentation page.
*/
class HF_IdlNavigationBar : public HtmlFactory_Idl
{
  public:
                        HF_IdlNavigationBar(
                            HtmlEnvironment_Idl &
                                                io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlNavigationBar();

    void                Produce_CeMainRow(
                            const client &      i_ce,
                            bool                i_bNoUsePage = false );
    void                Produce_CeXrefsMainRow(
                            const client &      i_ce );
    void                Produce_ModuleMainRow(
                            const client &      i_ce );
    void                Produce_IndexMainRow();

    /** Adds the subrow to the o_rOut argument of the constructor.
    */
    DYN HF_NaviSubRow & Add_SubRow();

  private:
    const ary::idl::CodeEntity *
                        pCe;
};

extern const String
    C_sLocalManualLinks;

#endif
