/*************************************************************************
 *
 *  $RCSfile: i_function.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:06:16 $
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

#ifndef ARY_IDL_I_FUNCTION_HXX
#define ARY_IDL_I_FUNCTION_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
#include <ary/idl/i_param.hxx>
    // PARAMETERS
#include <ary/idl/ik_function.hxx>
#include <ary/stdconstiter.hxx>



namespace ary
{
namespace idl
{


/*  OPEN?
*/

/** @resp
    Represents an IDL function.

    @descr
    Special case constructor:
    Constructors have return type "0".
*/
class Function : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2002 };

    typedef std::vector< Parameter >    ParamList;
    typedef std::vector< Type_id >      ExceptionList;

    // LIFECYCLE
    /// Normal function
                        Function(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Ce_id               i_nNameRoom,
                            Type_id             i_nReturnType,
                            bool                i_bOneWay );
    /// Constructor
                        Function(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Ce_id               i_nNameRoom );
                        ~Function();

    // OPERATIONS
    void                Add_Parameter(
                            const String &      i_sName,
                            Type_id             i_nType,
                            E_ParameterDirection
                                                i_eDirection );
    /// The function's parameter list ends with the ellipse "..." .
    void                Set_Ellipse();
    void                Add_Exception(
                            Type_id             i_nException );

    // INQUIRY
    Type_id             ReturnType() const;
    const ParamList &   Parameters() const      { return aParameters; }
    const ExceptionList &
                        Exceptions() const      { return aExceptions; }
    bool                IsOneway() const;
    bool                HasEllipse() const      { return bEllipse; }

  private:
    // Interface ary::RepositoryEntity
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Locals
    friend struct ifc_function::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Ce_id               nNameRoom;

    Type_id             nReturnType;
    ParamList           aParameters;
    ExceptionList       aExceptions;
    bool                bOneWay;
    bool                bEllipse;
};



// IMPLEMENTATION

inline void
Function::Add_Parameter( const String &         i_sName,
                         Type_id                i_nType,
                         E_ParameterDirection   i_eDirection )
{
    aParameters.push_back( Parameter(i_sName,i_nType,i_eDirection) );
}

inline void
Function::Set_Ellipse()
{
    bEllipse = true;
}

inline void
Function::Add_Exception( Type_id i_nException )
{
    aExceptions.push_back(i_nException);
}

inline Type_id
Function::ReturnType() const
    { return nReturnType; }

inline bool
Function::IsOneway() const
    { return bOneWay; }



}   // namespace idl
}   // namespace ary



#endif

