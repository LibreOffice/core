/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShapeTypeHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:37:00 $
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

#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#define _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_TREE_INFO_HXX
#include <svx/AccessibleShapeTreeInfo.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include <svx/AccessibleShapeInfo.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <rtl/ustring.hxx>
#include <vector>
#include <hash_map>

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
    typedef ::std::hash_map<
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
