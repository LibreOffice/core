/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_tplparam.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:56:03 $
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



// USED SERVICES
    // BASE CLASSES
#include "it_named.hxx"
    // COMPONENTS
    // PARAMETERS


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

    // INQUIRY
    Ce_id               StructId(               /// The struct which declares this type.
                            Ce_id               i_nStruct ) const;
    // ACCESS
    void                Set_StructId(
                            Ce_id               i_nStruct );

  private:
    // Interface RepositoryEntity:
    virtual void        do_Visit( Host & io_rHost ) const;
    virtual RCid        inq_ClassId() const;

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

inline Ce_id
TemplateParamType::StructId( Ce_id i_nStruct ) const
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
