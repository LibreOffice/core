/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_param.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:09:21 $
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

#ifndef ARY_IDL_I_PARAM_HXX
#define ARY_IDL_I_PARAM_HXX

// BASE CLASSES
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
namespace idl
{


/** Represents a parameter in an IDL function.
*/
class Parameter
{
  public:
    // LIFECYCLE
                        Parameter();
                        Parameter(
                            const String &      i_sName,
                            Type_id             i_nType,
                            E_ParameterDirection
                                                i_eDirection );
                        ~Parameter();

    // INQUIRY
    const String &      Name() const            { return sName; }
    Type_id             Type() const            { return nType; }
    E_ParameterDirection
                        Direction() const       { return eDirection; }

  private:
    // DATA
    String              sName;
    Type_id             nType;
    E_ParameterDirection
                        eDirection;
};




}   // namespace idl
}   // namespace ary
#endif
