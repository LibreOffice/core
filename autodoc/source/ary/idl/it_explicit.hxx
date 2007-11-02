/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_explicit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:57:01 $
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

#ifndef ARY_IDL_IT_EXPLICIT_HXX
#define ARY_IDL_IT_EXPLICIT_HXX

// BASE CLASSES
#include "it_named.hxx"




namespace ary
{
namespace idl
{


/** A named @->Type, not yet related to its corresponding
    @->CodeEntity.
*/
class ExplicitType : public Named_Type
{
  public:
    enum E_ClassId { class_id = 2203 };

    // LIFECYCLE
                        ExplicitType(
                            const String &      i_sName,
                            Type_id             i_nXNameRoom,
                            Ce_id               i_nModuleOfOccurrence,
                            Type_id             i_nTemplateType );
    virtual             ~ExplicitType();

    // INQUIRY
    Ce_id               ModuleOfOccurrence() const
                                                { return nModuleOfOccurrence; }
    Type_id             NameRoom() const        { return nXNameRoom; }
    Type_id             TemplateType() const    { return nTemplateType; }

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface CppEntity:
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
    Type_id             nXNameRoom;             // As written in code.
    Ce_id               nModuleOfOccurrence;
    Type_id             nTemplateType;
};




}   // namespace idl
}   // namespace ary
#endif
