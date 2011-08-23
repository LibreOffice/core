/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ARY_IDL_I_FUNCTION_HXX
#define ARY_IDL_I_FUNCTION_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>

// USED SERVICES
#include <ary/idl/i_param.hxx>
#include <ary/idl/ik_function.hxx>
#include <ary/stdconstiter.hxx>




namespace ary
{
namespace idl
{


/** Represents an IDL function.

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
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
