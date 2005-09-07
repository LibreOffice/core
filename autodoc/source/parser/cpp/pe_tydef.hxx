/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_tydef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:29:40 $
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

#ifndef ADC_CPP_PE_TYDEF_HXX
#define ADC_CPP_PE_TYDEF_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp {

class PE_Type;


class PE_Typedef : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expectName,
        afterName,
        size_of_states
    };
                        PE_Typedef(
                            Cpp_PE *        i_pParent );
                        ~PE_Typedef();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );
  private:
    typedef SubPe< PE_Typedef, PE_Type >        SP_Type;
    typedef SubPeUse< PE_Typedef, PE_Type>  SPU_Type;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                SpReturn_Type();

    void                On_start_typedef( const char * );
    void                On_expectName_Identifier( const char * );
    void                On_afterName_Semicolon( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Typedef> >
                        pStati;
    Dyn<SP_Type>        pSpType;
    Dyn<SPU_Type>       pSpuType;

    udmstri             sName;
    ary::Tid            nType;
};



// IMPLEMENTATION


}   // namespace cpp


#endif

