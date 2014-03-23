/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_GRAPHIC_SHAPE_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_GRAPHIC_SHAPE_HXX

#include <svx/AccessibleGraphicShape.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
namespace accessibility {

/** This class makes Impress shapes accessible.
*/
class AccessiblePresentationGraphicShape
    :   public AccessibleGraphicShape
{
public:
    //=====  internal  ========================================================
    AccessiblePresentationGraphicShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessiblePresentationGraphicShape (void);

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
    /// Return this object's role.
    virtual sal_Int16 SAL_CALL getAccessibleRole () throw (::com::sun::star::uno::RuntimeException);
};

} // end of namespace accessibility

#endif
