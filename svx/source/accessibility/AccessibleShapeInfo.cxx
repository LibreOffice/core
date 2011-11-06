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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"


#include <svx/AccessibleShapeInfo.hxx>


namespace accessibility {

AccessibleShapeInfo::AccessibleShapeInfo (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& rxShape,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        IAccessibleParent* pChildrenManager,
        sal_Int32 nIndex)
    : mxShape (rxShape),
      mxParent (rxParent),
      mpChildrenManager (pChildrenManager),
      mnIndex (nIndex)
{
    // empty.
}




AccessibleShapeInfo::AccessibleShapeInfo (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape>& rxShape,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        sal_Int32 nIndex)
    : mxShape (rxShape),
      mxParent (rxParent),
      mpChildrenManager (NULL),
      mnIndex (nIndex)
{
    // empty.
}

AccessibleShapeInfo::AccessibleShapeInfo (const AccessibleShapeInfo &rOther)
    : mxShape (rOther.mxShape),
      mxParent (rOther.mxParent),
      mpChildrenManager (rOther.mpChildrenManager),
      mnIndex (rOther.mnIndex)
{
    // empty.
}


AccessibleShapeInfo::~AccessibleShapeInfo (void)
{
    // empty.
}

} // end of namespace accessibility.
