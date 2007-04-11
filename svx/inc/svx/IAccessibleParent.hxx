/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IAccessibleParent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:36:12 $
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

#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_PARENT_HXX
#define _SVX_ACCESSIBILITY_IACCESSIBLE_PARENT_HXX

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace accessibility {

class AccessibleShape;
class AccessibleShapeTreeInfo;

/** This interface contains methods missing from the
    <type>XAccessibleContext</type> interface that allow the modification of
    parent/child relationship.
*/
class IAccessibleParent
{
public:
    /** Allow for a virtual destructor.
    */
    virtual ~IAccessibleParent (void){};

    /** A call to this method requests the implementor to replace one child
        with another and send the appropriate notifications.  That are two
        child events: One notifying the removal of the current child and one
        about the existence of the new child.  The index of the new child is
        implementation dependent, i.e. it is not garanteed that the
        replacement has the same index as the current child has.

        <p>A default implementation can just use the ShapeTypeHandler::CreateAccessibleObject
        to let a factory create the new instance with the parameters given, and then
        place the new shape into the own structures.</p>

        @param pCurrentChild
            This child is about to be replaced.

        @param _rxShape
            The UNO shape which the old and new child represent

        @param _nIndex
            The IndexInParent of the old child. Note that the index in
            parent of the replacement is not necessarily the same as
            that of the current child.

        @param _rShapeTreeInfo
            The TreeInfo for the old child.

        @return
           If the replacement has taken place successfully <TRUE/> is
           returned.  If the replacement can not be carried out or an error
           occurs that does not result in an exception then <FALSE/> is
           returned.

        @raises RuntimeException
           in case something went heavily wrong
    */
    virtual sal_Bool ReplaceChild (
        AccessibleShape* pCurrentChild,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _rxShape,
        const long _nIndex,
        const AccessibleShapeTreeInfo& _rShapeTreeInfo
    )   throw (::com::sun::star::uno::RuntimeException) = 0;
};

} // end of namespace accessibility

#endif
