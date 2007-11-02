/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_tplparam.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:58:08 $
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

#ifndef ARY_IDL_IT_TPLPARAM_HXX
#define ARY_IDL_IT_TPLPARAM_HXX

// BASE CLASSES
#include "it_named.hxx"




namespace ary
{
namespace idl
{


/** @resp Represents a template type when it is used within the
    declaring struct.
*/
class TemplateParamType : public Named_Type
{
  public:
    enum E_ClassId { class_id = 2205 };

    // LIFECYCLE
                        TemplateParamType(
                            const char *        i_sName );
    virtual             ~TemplateParamType();

    Ce_id               StructId() const;       /// The struct which declares this type.
    void                Set_StructId(
                            Ce_id               i_nStruct );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface Object:
    virtual ClassId     get_AryClass() const;

    // Interface Type:
    virtual void        inq_Get_Text(
                            StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequenceCount,
                            const Gate &        i_rGate ) const;
    // DATA
    Ce_id               nStruct;                /// The struct which declares this type.
};




// IMPLEMENTATION
inline Ce_id
TemplateParamType::StructId() const
{
    return nStruct;
}

inline void
TemplateParamType::Set_StructId( Ce_id i_nStruct )
{
    nStruct = i_nStruct;
}




}   // namespace idl
}   // namespace ary
#endif
