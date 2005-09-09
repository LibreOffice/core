/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessiblePresentationShape.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:59:42 $
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

#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif

namespace accessibility {

/** This class makes Impress shapes accessible.
*/
class AccessiblePresentationShape
    :   public AccessibleShape
{
public:
    //=====  internal  ========================================================
    AccessiblePresentationShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessiblePresentationShape (void);

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  internal  ========================================================

    /// Create a name string that contains the accessible name.
    virtual ::rtl::OUString
        CreateAccessibleBaseName ()
        throw (::com::sun::star::uno::RuntimeException);

    /// Create a description string that contains the accessible description.
    virtual ::rtl::OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException);

private:
    /** Don't use the default constructor.  Use the public constructor that
        takes the original shape and the parent as arguments instead.
    */
    AccessiblePresentationShape (void);

    /// Don't use the constructor.  Not yet implemented.
    AccessiblePresentationShape (const AccessiblePresentationShape&);

    /// Don't use the assignment operator.  Not yet implemented.
    AccessiblePresentationShape& operator= (const AccessiblePresentationShape&);
};

} // end of namespace accessibility

#endif
