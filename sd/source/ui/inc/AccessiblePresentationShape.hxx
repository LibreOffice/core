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



#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_PRESENTATION_SHAPE_HXX

#include <svx/AccessibleShape.hxx>

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
//IAccessibility2 Implementation 2009-----
    ::rtl::OUString GetStyle();
//-----IAccessibility2 Implementation 2009

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
