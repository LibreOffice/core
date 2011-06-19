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

#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#define _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX

#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <svx/AccessibleShape.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include "svx/svxdllapi.h"

#include <rtl/ustring.hxx>
#include <vector>
#include <boost/unordered_map.hpp>

namespace accessibility {

/** Use an integer to represent shape type ids.  A ShapeTypeId is unique
    inside one project but is not over the project boundaries.
*/
typedef int ShapeTypeId;

/** Define the function type for creating accessible objects for given
    service names.
*/
typedef AccessibleShape* (*tCreateFunction)
    (const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    ShapeTypeId nId);

/** Each shape type is described by listing its id, its service name and a
    function which creates a new accessible object that can represent that
    service.  The id has to be unique with respect to the create function.
*/
struct ShapeTypeDescriptor
{
    ShapeTypeId         mnShapeTypeId;
    rtl::OUString       msServiceName;
    tCreateFunction     maCreateFunction;
    ShapeTypeDescriptor (
        ShapeTypeId nId, const rtl::OUString& sName, tCreateFunction aFunction)
    :   mnShapeTypeId (nId),
        msServiceName (sName),
           maCreateFunction (aFunction)
    {}
    ShapeTypeDescriptor (void)
    :   mnShapeTypeId (-1),
        msServiceName (),
           maCreateFunction (NULL)
    {}
};

/** @descr
        This class is a singleton that has the purpose to transform between
        service names of shapes and associated enum values and to create new
        accessible objects for given shapes.
*/
class SVX_DLLPUBLIC ShapeTypeHandler
{
public:
    enum { UNKNOWN_SHAPE_TYPE = 0 };

    /** This function returns a reference to the only instance of this class.
        Use this instance to retrieve a shape's type and service name.
        @return
            Returns a reference to a <type>ShapeTypeHandler</type> object.
     */
    static ShapeTypeHandler& Instance (void);

    /**  Determines the type id of a shape with the given service name.
         @param aServiceName
             Service name of the shape for which to return the type id.
         @return
             Returns the type id of the shape with the given service name or
             -1 when the service name is not known.
     */
    ShapeTypeId GetTypeId (const ::rtl::OUString& aServiceName) const;

    /**  Determines the type id of the specified shape.
         @param xShape
             Reference to the shape for which to return the type id.
         @return
             Returns the type id of the specified shape or
             -1 when the given reference is either not
             set or the referenced object does not support the
             <type>XShapeDescriptor</type> interface.
     */
    ShapeTypeId GetTypeId (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape) const;

    /**  Return the service name of a shape with the specified type.
         @param aTypeId
             Id of the shape type for which to retrieve the service name.
             An unknown shape id results in an empty string.
         @return
             The returned string contains the service name, with which a
             shape of the given type can be created.
     */
    const ::rtl::OUString& GetServiceName (ShapeTypeId aTypeId) const;

    /**  Create a new accessible object for the given shape.
         @param rShapeInfo
             Bundle of information passed to the new accessible shape.
         @param rShapeTreeInfo
             Bundle of information passed down the shape tree.
         @return
             Pointer to the implementation object that implements the
             <code>XAccessible</code> interface.  This pointer may be NULL
             if the specified shape is of unknown type.
     */
    AccessibleShape*
        CreateAccessibleObject (
            const AccessibleShapeInfo& rShapeInfo,
            const AccessibleShapeTreeInfo& rShapeTreeInfo) const;

    /** Compatibility function.
    */
    AccessibleShape*
        CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& rxShape,
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            const AccessibleShapeTreeInfo& rShapeTreeInfo) const
    {
        AccessibleShapeInfo aShapeInfo(rxShape, rxParent);
        return CreateAccessibleObject (aShapeInfo, rShapeTreeInfo);
    }

    /**  Add new shape types to the internal tables.  Each new shape type is
         described by one shape type descriptor.  See
         <type>ShapeTypeDescriptor</type> for more details.

         @param nDescriptorCount
             Number of new shape types.
         @param aDescriptorList
             Array of new shape type descriptors.
         @return
             The returned flag indicates whether the specified shape
             descriptors have been successfully added.
     */
    bool AddShapeTypeList (int nDescriptorCount,
        ShapeTypeDescriptor aDescriptorList[]);

    /// get the accessible base name for an object
    static ::rtl::OUString CreateAccessibleBaseName (
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape)
            throw (::com::sun::star::uno::RuntimeException);

protected:
    // Declare default constructor, copy constructor, destructor, and
    // assignment operation protected so that no one accidentally creates a
    // second instance of this singleton class or deletes it.
    ShapeTypeHandler (void);
    ShapeTypeHandler (const ShapeTypeHandler& aHandler);             // never implemented, this is a singleton class
    ShapeTypeHandler& operator= (const ShapeTypeHandler& aHandler); // never implemented, this is a singleton class

    /** This destructor is never called at the moment.  But because this
        class is a singleton this is not a problem.
    */
    virtual ~ShapeTypeHandler (void);

private:
    /// Pointer to the only instance of this class.
    static ShapeTypeHandler* instance;

    /** List of shape type descriptors.  This list is normally build up in
        several steps when libraries that implement shapes are loaded and
        call the addShapeTypeList method.  After that no modifications of
        the list take place.
    */
    ::std::vector<ShapeTypeDescriptor> maShapeTypeDescriptorList;

    /** This hash map allows the fast look up of a type descriptor for a
        given service name.
    */
    typedef ::boost::unordered_map<
        ::rtl::OUString,ShapeTypeId,
        ::rtl::OUStringHash,
        //        ::comphelper::UStringHash,
        ::comphelper::UStringEqual> tServiceNameToSlotId;
    mutable tServiceNameToSlotId maServiceNameToSlotId;

    /**  Determine the slot id of the specified shape type.  With this id
         internal methods can access the associated type descriptor.
         @param aServiceName
             Service name of the shape for which to return the slot id.
         @return
             Returns the slot id of the shape with the given service name or
             0 when the service name is not known.
     */
    SVX_DLLPRIVATE long GetSlotId (const ::rtl::OUString& aServiceName) const;

    /**  Determine the slot id of the specified shape type.  With this id
         internal methods can access the associated type descriptor.
         @param rxShape
             Shape for which to return the slot id.
         @return
             Returns the slot id of the shape with the given service name or
             0 when the service name is not known.
     */
    SVX_DLLPRIVATE long GetSlotId (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape) const;
};

} // end of namespace accessible

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
