        /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_method.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:36:41 $
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

#ifndef ADC_DISPLAY_HFI_METHOD_HXX
#define ADC_DISPLAY_HFI_METHOD_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_param.hxx>
#include <ary/doc/d_oldidldocu.hxx>
typedef ary::doc::OldIdlDocu CodeInfo;
#include "hfi_linklist.hxx"



namespace csi
{
    namespace idl
    {
        class Parameter;
    }
}

class HF_FunctionDeclaration;

class HF_IdlMethod : public HtmlFactory_Idl
{
  public:
    typedef ary::StdConstIterator<ary::idl::Parameter>  param_list;

                        HF_IdlMethod(
                            Environment &       io_rEnv,
                            Xml::Element &      o_cell );
    virtual             ~HF_IdlMethod();

    void                Produce_byData(
                            const String &      i_sName,
                            type_id             i_nReturnType,
                            param_list &        i_rParams,
                            type_list &         i_rExceptions,
                            bool                i_bOneway,
                            bool                i_bEllipse,
                            const client &      i_ce ) const;
  private:
    void                write_Declaration(
                            const String &      i_sName,
                            type_id             i_nReturnType,
                            param_list &        i_rParams,
                            type_list &         i_rExceptions,
                            bool                i_bOneway,
                            bool                i_bEllipse ) const;
//    void                write_Param(
//                            HF_IdlTypeText &    o_type,
//                            Xml::Element &      o_names,
//                            const ary::idl::Parameter &
//                                                i_param ) const;

    Xml::Element *      write_Param(
                            HF_FunctionDeclaration &
                                                o_decl,
                            const ary::idl::Parameter &
                                                i_param ) const;
    void                enter_ContentCell() const;
    void                leave_ContentCell() const;
};



// IMPLEMENTATION



#endif


