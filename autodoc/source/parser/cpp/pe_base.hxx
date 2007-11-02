/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_base.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:52:24 $
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


#ifndef ADC_CPP_PE_BASE_HXX
#define ADC_CPP_PE_BASE_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>



namespace cpp
{

class PE_Type;

class PE_Base : public Cpp_PE
{
  public:
    enum E_State
    {
        startOfNext,
        inName,
        size_of_states
    };

    typedef ary::cpp::List_Bases        BaseList;
    typedef ary::cpp::S_Classes_Base    Base;

                        PE_Base(
                            Cpp_PE *            i_pParent );
                        ~PE_Base();

    const BaseList &    Result_BaseIds() const;

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    typedef SubPe< PE_Base, PE_Type >           SP_Type;
    typedef SubPeUse< PE_Base, PE_Type>         SPU_BaseName;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                SpReturn_BaseName();

    void                On_startOfNext_Identifier(const char *);
    void                On_startOfNext_public(const char *);
    void                On_startOfNext_protected(const char *);
    void                On_startOfNext_private(const char *);
    void                On_startOfNext_virtual(const char *);
    void                On_startOfNext_DoubleColon(const char *);

    void                On_inName_Identifier(const char *);
    void                On_inName_virtual(const char *);
    void                On_inName_SwBracket_Left(const char *);
    void                On_inName_DoubleColon(const char *);
    void                On_inName_Comma(const char *);

    Base &              CurObject();

    // DATA
    Dyn< PeStatusArray<PE_Base> >
                        pStati;

    Dyn<SP_Type>        pSpType;            /// till "{" incl.
    Dyn<SPU_BaseName>   pSpuBaseName;

    BaseList            aBaseIds;
};



// IMPLEMENTATION

inline const PE_Base::BaseList &
PE_Base::Result_BaseIds() const
    { return aBaseIds; }


inline PE_Base::Base &
PE_Base::CurObject()
    { return aBaseIds.back(); }





}   // namespace cpp
#endif
