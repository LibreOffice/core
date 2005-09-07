/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_comrela.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:06:49 $
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

#ifndef ARY_IDL_I_COMRELA_HXX
#define ARY_IDL_I_COMRELA_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_language.hxx>


namespace ary
{
    namespace info
    {
        class CodeInformation;
    }

namespace idl
{


/** Contains data for an IDL code entity related to another one like a base of
    an interface or of a service or the supported interface of a service.
*/
class CommentedRelation
{
  public:
    // LIFECYCLE

                        CommentedRelation(
                            Type_id             i_nType,
                            info::CodeInformation *
                                                i_pInfo )
                                                :   nType(i_nType),
                                                    pInfo(i_pInfo)
                                                {}
    // INQUIRY
    Type_id             Type() const            { return nType; }
    info::CodeInformation *
                        Info() const            { return pInfo; }

  private:
    // DATA
    Type_id             nType;
    info::CodeInformation *
                        pInfo;
};


}   // namespace idl
}   // namespace ary


#endif
