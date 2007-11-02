/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_interface.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:35:54 $
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

#ifndef ADC_DISPLAY_HFI_INTERFACE_HXX
#define ADC_DISPLAY_HFI_INTERFACE_HXX


// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>
#include <ary/doc/d_oldidldocu.hxx>


class HF_NaviSubRow;
class HF_SubTitleTable;
class HF_IdlBaseNode;

class HF_IdlInterface : public HtmlFactory_Idl
{
  public:
                        HF_IdlInterface(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlInterface();

    void                Produce_byData(
                            const client &      i_ce ) const;
    void                Display_BaseNode(
                            const HF_IdlBaseNode &
                                                i_rNode ) const;
  private:
    // Locals
    DYN HF_NaviSubRow & make_Navibar(
                            const client &      i_ce ) const;

    virtual void        produce_MemberDetails(
                            HF_SubTitleTable &  o_table,
                            const client &      ce ) const;
    void                produce_BaseHierarchy(
                            Xml::Element &      o_screen,
                            const client &      i_ce,
                            const String &      i_sLabel ) const;

    // Locals
    enum E_CurProducedMembers
    {
        mem_none,
        mem_Functions,
        mem_Attributes
    };

    // DATA
    mutable E_CurProducedMembers
                        eCurProducedMembers;
};



// IMPLEMENTATION

extern const String
    C_sCePrefix_Interface;



#endif
