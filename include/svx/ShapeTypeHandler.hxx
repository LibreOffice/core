/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SHAPETYPEHANDLER_HXX
#define INCLUDED_SVX_SHAPETYPEHANDLER_HXX

#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/AccessibleShapeInfo.hxx>
#include <svx/AccessibleShape.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <svx/svxdllapi.h>

#include <rtl/ustring.hxx>
#include <unordered_map>
#include <vector>

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
    OUString       msServiceName;
    tCreateFunction     maCreateFunction;
    ShapeTypeDescriptor (
        ShapeTypeId nId, const OUString& sName, tCreateFunction aFunction)
    :   mnShapeTypeId (nId),
        msServiceName (sName),
           maCreateFunction (aFunction)
    {}
    ShapeTypeDescriptor()
    :   mnShapeTypeId (-1),
        msServiceName (),
           maCreateFunction (nullptr)
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
            Returns a reference to a ShapeTypeHandler object.
     */
    static ShapeTypeHandler& Instance();

    /**  Determines the type id of a shape with the given service name.
         @param aServiceName
             Service name of the shape for which to return the type id.
         @return
             Returns the type id of the shape with the given service name or
             -1 when the service name is not known.
     */
    ShapeTypeId GetTypeId (const OUString& aServiceName) const;

    /**  Determines the type id of the specified shape.
         @param xShape
             Reference to the shape for which to return the type id.
         @return
             Returns the type id of the specified shape or
             -1 when the given reference is either not
             set or the referenced object does not support the
             XShapeDescriptor interface.
     */
    ShapeTypeId GetTypeId (const css::uno::Reference<
        css::drawing::XShape>& rxShape) const;

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
    rtl::Reference<AccessibleShape>
        CreateAccessibleObject (
            const AccessibleShapeInfo& rShapeInfo,
            const AccessibleShapeTreeInfo& rShapeTreeInfo) const;

    /**  Add new shape types to the internal tables.  Each new shape type is
         described by one shape type descriptor.  See
         ShapeTypeDescriptor for more details.

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
    static OUString CreateAccessibleBaseName (
        const css::uno::Reference< css::drawing::XShape >& rxShape)
            throw (css::uno::RuntimeException, std::exception);

protected:
    // Declare default constructor, copy constructor, destructor, and
    // assignment operation protected so that no one accidentally creates a
    // second instance of this singleton class or deletes it.
    ShapeTypeHandler();
    ShapeTypeHandler (const ShapeTypeHandler& aHandler);             // never implemented, this is a singleton class
    ShapeTypeHandler& operator= (const ShapeTypeHandler& aHandler); // never implemented, this is a singleton class

    /** This destructor is never called at the moment.  But because this
        class is a singleton this is not a problem.
    */
    virtual ~ShapeTypeHandler();

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
    typedef std::unordered_map<OUString,ShapeTypeId,
                               OUStringHash> tServiceNameToSlotId;
    mutable tServiceNameToSlotId maServiceNameToSlotId;

    /**  Determine the slot id of the specified shape type.  With this id
         internal methods can access the associated type descriptor.
         @param aServiceName
             Service name of the shape for which to return the slot id.
         @return
             Returns the slot id of the shape with the given service name or
             0 when the service name is not known.
     */
    SVX_DLLPRIVATE long GetSlotId (const OUString& aServiceName) const;

    /**  Determine the slot id of the specified shape type.  With this id
         internal methods can access the associated type descriptor.
         @param rxShape
             Shape for which to return the slot id.
         @return
             Returns the slot id of the shape with the given service name or
             0 when the service name is not known.
     */
    SVX_DLLPRIVATE long GetSlotId (const css::uno::Reference<
        css::drawing::XShape>& rxShape) const;
};

} // end of namespace accessible

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
