/*************************************************************************
 *
 *  $RCSfile: inpcontx.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:15 $
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

#ifndef ARY_CPP_INPCONTX_HXX
#define ARY_CPP_INPCONTX_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/ids.hxx>
#include <ary/cpp/c_etypes.hxx>



namespace ary
{
namespace cpp
{
    class Namespace;
    class Class;
    class Enum;

    class ProjectGroup;
    class FileGroup;

/** This class provides information about the context of an
    CodeEntity, which is going to be stored in the repository.
    The information is used mainly by class ary::cpp::Gate.
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
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId );
        /// Adds Enum data to current inner scope (Namespace or Class).
        void                Add_Enum(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId );
        /// Adds Typedef data to current inner scope (Namespace or Class).
        void                Add_Typedef(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId );
        /// Adds Operation data to current inner scope (Namespace or Class).
        void                Add_Operation(
                                const udmstri &     i_sLocalName,
                                OSid                i_nSignature,
                                Cid                 i_nId,
                                bool                i_bIsStaticMember );    /// True only for static class members.
        /// Adds Variable data to current inner scope (Namespace or Class).
        void                Add_Variable(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStaticMember );    /// True only for static class members.
        // INQUIRY
        Cid                 Id() const;

      private:
        virtual void        do_Add_Class(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId ) = 0;
        virtual void        do_Add_Enum(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId ) = 0;
        virtual void        do_Add_Typedef(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId ) = 0;
        virtual void        do_Add_Operation(
                                const udmstri &     i_sLocalName,
                                OSid                i_nSignature,
                                Cid                 i_nId,
                                bool                i_bIsStatic ) = 0;
        virtual void        do_Add_Variable(
                                const udmstri &     i_sLocalName,
                                Cid                 i_nId,
                                bool                i_bIsConst,
                                bool                i_bIsStatic ) = 0;
        virtual Cid         inq_Id() const = 0;
    };

    // LIFECYCLE
    virtual             ~InputContext() {}

    // OPERATIONS

    // INQUIRY
    ProjectGroup &      CurProjectGroup() const;
    FileGroup &         CurFileGroup() const;

    Namespace &         CurNamespace() const;
    Class *             CurClass() const;
    Enum *              CurEnum() const;

    Owner &             CurOwner() const;
    E_Protection        CurProtection() const;

  private:
    virtual ProjectGroup &
                        inq_CurProjectGroup() const = 0;
    virtual FileGroup & inq_CurFileGroup() const = 0;

    virtual Namespace & inq_CurNamespace() const = 0;
    virtual Class *     inq_CurClass() const = 0;
    virtual Enum *      inq_CurEnum() const = 0;

    virtual Owner &     inq_CurOwner() const = 0;
    virtual E_Protection
                        inq_CurProtection() const = 0;
};



// IMPLEMENTATION

inline ProjectGroup &
InputContext::CurProjectGroup() const
    { return inq_CurProjectGroup(); }
inline FileGroup &
InputContext::CurFileGroup() const
    { return inq_CurFileGroup(); }

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
InputContext::Owner::Add_Class( const udmstri &     i_sLocalName,
                                Cid                 i_nId )
    { do_Add_Class(i_sLocalName, i_nId); }
inline void
InputContext::Owner::Add_Enum( const udmstri &     i_sLocalName,
                               Cid                 i_nId )
    { do_Add_Enum(i_sLocalName, i_nId); }
inline void
InputContext::Owner::Add_Typedef( const udmstri &     i_sLocalName,
                                  Cid                 i_nId )
    { do_Add_Typedef(i_sLocalName, i_nId); }
inline void
InputContext::Owner::Add_Operation( const udmstri &     i_sLocalName,
                                    OSid                i_nSignature,
                                    Cid                 i_nId,
                                    bool                i_bIsStatic )
    { do_Add_Operation( i_sLocalName, i_nSignature, i_nId, i_bIsStatic ); }
inline void
InputContext::Owner::Add_Variable( const udmstri &     i_sLocalName,
                                   Cid                 i_nId,
                                   bool                i_bIsConst,
                                   bool                i_bIsStatic )
    { do_Add_Variable( i_sLocalName, i_nId, i_bIsConst, i_bIsStatic ); }
inline Cid
InputContext::Owner::Id() const
    { return inq_Id(); }




}   // namespace cpp
}   // namespace ary


#endif

