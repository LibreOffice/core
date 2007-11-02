/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_property.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:09:42 $
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

#ifndef ARY_IDL_I_PROPERTY_HXX
#define ARY_IDL_I_PROPERTY_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_property
{
    struct attr;
}


/** Represents an IDL property.
*/
class Property : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2004 };

    class Stereotypes
    {
      public:
        enum E_Flags
        {
            readonly = 1,
            bound = 2,
            constrained = 4,
            maybeambiguous = 8,
            maybedefault = 16,
            maybevoid = 32,
            removable = 64,
            transient = 128,
            s_MAX
        };
                            Stereotypes()       : nFlags(0) {}

        bool                HasAny() const      { return nFlags != 0; }
        bool                IsReadOnly() const  { return (nFlags & UINT32(readonly)) != 0; }
        bool                IsBound() const     { return (nFlags & UINT32(bound)) != 0; }
        bool                IsConstrained() const
                                                { return (nFlags & UINT32(constrained)) != 0; }
        bool                IsMayBeAmbiguous() const
                                                { return (nFlags & UINT32(maybeambiguous)) != 0; }
        bool                IsMayBeDefault() const
                                                { return (nFlags & UINT32(maybedefault)) != 0; }
        bool                IsMayBeVoid() const { return (nFlags & UINT32(maybevoid)) != 0; }
        bool                IsRemovable() const { return (nFlags & UINT32(removable)) != 0; }
        bool                IsTransient() const { return (nFlags & UINT32(transient)) != 0; }

        void                Set_Flag(
                                E_Flags         i_flag )
                                                { nFlags |= UINT32(i_flag); }
      private:
        // DATA
        UINT32              nFlags;
    };


    // LIFECYCLE
                        Property(
                            const String &      i_sName,
                            Ce_id               i_nService,
                            Ce_id               i_nModule,
                            Type_id             i_nType,
                            Stereotypes         i_stereotypes );
                        ~Property();
    // INQUIRY
    Type_id             Type() const;

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

    friend struct ifc_property::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Ce_id               nNameRoom;

    Type_id             nType;
    Stereotypes         aStereotypes;
};




// IMPLEMENTATION
inline Type_id
Property::Type() const
{
    return nType;
}




}   // namespace idl
}   // namespace ary
#endif
