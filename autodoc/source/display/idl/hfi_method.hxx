        /*************************************************************************
 *
 *  $RCSfile: hfi_method.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:27:15 $
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

#ifndef ADC_DISPLAY_HFI_METHOD_HXX
#define ADC_DISPLAY_HFI_METHOD_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_param.hxx>
#include <ary_i/codeinf2.hxx>
typedef ary::info::CodeInformation CodeInfo;
#include "hfi_linklist.hxx"



namespace csi
{
    namespace idl
    {
        class Parameter;
    }
}
class HF_IdlTypeText;

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
    void                write_Param(
                            HF_IdlTypeText &    o_type,
                            Xml::Element &      o_names,
                            const ary::idl::Parameter &
                                                i_param ) const;
    void                enter_ContentCell() const;
    void                leave_ContentCell() const;
};



// IMPLEMENTATION



#endif


