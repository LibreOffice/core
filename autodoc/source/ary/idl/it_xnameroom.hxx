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

#ifndef ARY_IDL_IT_XNAMEROOM_HXX
#define ARY_IDL_IT_XNAMEROOM_HXX

// BASE CLASSES
#include <ary/idl/i_type.hxx>
#include <nametreenode.hxx>




namespace ary
{
namespace idl
{


/** A namespace for ->Type s, as they are explicitely written in code.

    The search/identification string is usually the local name of
    the Type. But for templated structs, the search string has this
    pattern:
                <LocalName> '<' <StringOfTemplateTypeId>
*/
class ExplicitNameRoom : public Type
{
  public:
    enum E_ClassId { class_id = 2204 };

    // LIFECYCLE
                        ExplicitNameRoom();
                        ExplicitNameRoom(
                            const String &      i_sName,
                            const ExplicitNameRoom &
                                                i_rParent );
    virtual             ~ExplicitNameRoom();

    // OPERATIONS
    /** @param i_sSearchString
                A local type name  usually.
                For templated types see class docu.
        @see ExplicitNameRoom
    */
    void                Add_Name(
                            const String &      i_sSearchString,
                            Type_id             i_nId )
                            { aImpl.Add_Name(i_sSearchString,i_nId); }
    // INQUIRY
    const String &      Name() const            { return aImpl.Name(); }
    intt                Depth() const           { return aImpl.Depth(); }
    void                Get_FullName(
                            StringVector &      o_rText,
                            Ce_idList *         o_pRelatedCes,
                            const Gate &        i_rGate ) const;
    bool                IsAbsolute() const      { return Depth() > 0
                                                    ?   (*NameChain_Begin()).empty()
                                                    :   false; }
    /** @param i_sSearchString
                A local type name  usually.
                For templated types see class docu.
        @see ExplicitNameRoom
    */
    Type_id             Search_Name(
                            const String &      i_sSearchString ) const
                            { return aImpl.Search_Name(i_sSearchString); }

    StringVector::const_iterator
                        NameChain_Begin() const
                            { return aImpl.NameChain_Begin(); }
    StringVector::const_iterator
                        NameChain_End() const
                            { return aImpl.NameChain_End(); }
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
    // DATA
    NameTreeNode<Type_id>
                        aImpl;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
