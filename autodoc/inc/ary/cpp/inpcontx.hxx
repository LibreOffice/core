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

#ifndef ARY_CPP_INPCONTX_HXX
#define ARY_CPP_INPCONTX_HXX



// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>



namespace ary
{
namespace loc
{
    class File;
}
namespace cpp
{
    class Gate;
    class Namespace;
    class Class;
    class Enum;

    class OperationSignature;
}
}



namespace ary
{
namespace cpp
{


/** The context of a CodeEntity, which is going to be stored in the
    repository. The information is used mainly by ->ary::cpp::CeAdmin.
*/
class InputContext
{
  public:
    class Owner
    {
      public:
        // LIFECYCLE
        virtual             ~Owner() {}

        // OPERATIONS
        /// Adds Class data to current inner scope (Namespace or Class).
        void                Add_Class(
                                const String  &     i_sLocalName,
                                Cid                 i_nId );
        /// Adds Enum data to current inner scope (Namespace or Class).
        void                Add_Enum(
                                const String  &     i_sLocalName,
                                Cid                 i_nId );
        /// Adds Typedef data to current inner scope (Namespace or Class).
        void                Add_Typedef(
                                const String  &     i_sLocalName,
                                Cid                 i_nId );
        /// Adds Operation data to current inner scope (Namespace or Class).
        void                Add_Operation(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsStaticMember );    /// True only for static class members.
        /// Adds Variable data to current inner scope (Namespace or Class).
        void                Add_Variable(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStaticMember );    /// True only for static class members.
        // INQUIRY
        Ce_id               CeId() const;

        /** @attention Must only be used by ary::cpp::GatePilot!
            Will work nowhere else!
        */
        virtual bool        HasClass(
                                const String  &     i_sLocalName ) = 0;
      private:
        virtual void        do_Add_Class(
                                const String  &     i_sLocalName,
                                Cid                 i_nId ) = 0;
        virtual void        do_Add_Enum(
                                const String  &     i_sLocalName,
                                Cid                 i_nId ) = 0;
        virtual void        do_Add_Typedef(
                                const String  &     i_sLocalName,
                                Cid                 i_nId ) = 0;
        virtual void        do_Add_Operation(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsStatic ) = 0;
        virtual void        do_Add_Variable(
                                const String  &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStatic ) = 0;
        virtual Ce_id       inq_CeId() const = 0;
    };

    // LIFECYCLE
    virtual             ~InputContext() {}

    // OPERATIONS

    // INQUIRY
    loc::File &         CurFile() const;

    Namespace &         CurNamespace() const;
    Class *             CurClass() const;
    Enum *              CurEnum() const;

    Owner &             CurOwner() const;
    E_Protection        CurProtection() const;

  private:
    virtual loc::File & inq_CurFile() const = 0;

    virtual Namespace & inq_CurNamespace() const = 0;
    virtual Class *     inq_CurClass() const = 0;
    virtual Enum *      inq_CurEnum() const = 0;

    virtual Owner &     inq_CurOwner() const = 0;
    virtual E_Protection
                        inq_CurProtection() const = 0;
};




// IMPLEMENTATION
inline loc::File &
InputContext::CurFile() const
    { return inq_CurFile(); }

inline Namespace &
InputContext::CurNamespace() const
    { return inq_CurNamespace(); }
inline Class *
InputContext::CurClass() const
    { return inq_CurClass(); }
inline Enum *
InputContext::CurEnum() const
    { return inq_CurEnum(); }
inline InputContext::Owner &
InputContext::CurOwner() const
    { return inq_CurOwner(); }
inline E_Protection
InputContext::CurProtection() const
    { return inq_CurProtection(); }


inline void
InputContext::Owner::Add_Class( const String  &     i_sLocalName,
                                Cid                 i_nId )
    { do_Add_Class(i_sLocalName, i_nId); }
inline void
InputContext::Owner::Add_Enum( const String  &     i_sLocalName,
                               Cid                 i_nId )
    { do_Add_Enum(i_sLocalName, i_nId); }
inline void
InputContext::Owner::Add_Typedef( const String  &     i_sLocalName,
                                  Cid                 i_nId )
    { do_Add_Typedef(i_sLocalName, i_nId); }
inline void
InputContext::Owner::Add_Operation( const String  &             i_sLocalName,
                                    Cid                         i_nId,
                                    bool                        i_bIsStatic )
    { do_Add_Operation( i_sLocalName, i_nId, i_bIsStatic ); }
inline void
InputContext::Owner::Add_Variable( const String  &     i_sLocalName,
                                   Cid                 i_nId,
                                   bool                i_bIsConst,
                                   bool                i_bIsStatic )
    { do_Add_Variable( i_sLocalName, i_nId, i_bIsConst, i_bIsStatic ); }
inline Ce_id
InputContext::Owner::CeId() const
    { return inq_CeId(); }





}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
