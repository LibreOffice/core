/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_typedef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:48:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADC_DISPLAY_HFI_TYPEDEF_HXX
#define ADC_DISPLAY_HFI_TYPEDEF_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS


class HF_IdlTypedef : public HtmlFactory_Idl
{
  public:
                        HF_IdlTypedef(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlTypedef();

    void                Produce_byData(
                            const client &      ce ) const;
  private:
    void                make_Navibar(
                            const client &      ce ) const;
};



// IMPLEMENTATION


const String
    C_sCePrefix_Typedef("typedef");

#endif


