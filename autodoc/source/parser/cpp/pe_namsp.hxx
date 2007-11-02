/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_namsp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:56:14 $
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

#ifndef ADC_CPP_PE_NAMSP_HXX
#define ADC_CPP_PE_NAMSP_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_pe.hxx>
    // PARAMETERS

namespace ary
{
namespace cpp
{
class Namespace;
}
}


namespace cpp
{


class PE_Namespace : public Cpp_PE
{
  public:
    enum E_State
    {
        start,
        gotName,
        expectSemicolon,        /// after namespace assignment
        size_of_states
    };
                        PE_Namespace(
                            Cpp_PE *            i_pParent );
                        ~PE_Namespace();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    ary::cpp::Namespace *
                        Result_OpenedNamespace() const;
  private:
    void                Setup_StatusFunctions();

    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                On_start_Identifier(const char * i_sText);
    void                On_start_SwBracket_Left(const char * i_sText);
    void                On_gotName_SwBracket_Left(const char * i_sText);
    void                On_gotName_Assign(const char * i_sText);
    void                On_expectSemicolon_Semicolon(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Namespace> >
                        pStati;

    String              sLocalName;
    bool                bPush;
};




}   // namespace cpp
#endif

