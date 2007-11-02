/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_ce.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:56:23 $
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

#ifndef ARY_IDL_IT_CE_HXX
#define ARY_IDL_IT_CE_HXX

// BASE CLASSES
#include <ary/idl/i_type.hxx>




namespace ary
{
namespace idl
{


/** A named ->Type related to its corresponding
     ->CodeEntity.
*/
class Ce_Type : public Type
{
  public:
    enum E_ClassId { class_id = 2201 };

    // LIFECYCLE
                        Ce_Type(
                            Ce_id               i_nRelatedCe,
                            Type_id             i_nTemplateType = Type_id(0) );
    virtual             ~Ce_Type();

    // INQUIRY
    Ce_id               RelatedCe() const       { return nRelatedCe; }
    Type_id             TemplateType() const    { return nTemplateType; }

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
                            int &               o_nSequemceCount,
                            const Gate &        i_rGate ) const;
    virtual Type_id     inq_TemplateParameterType() const;

    // DATA
    Ce_id               nRelatedCe;
    Type_id             nTemplateType;
};




}   // namespace idl
}   // namespace ary
#endif
