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


#include <svx/AccessibleGraphicShape.hxx>

#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>

using namespace ::accessibility;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

AccessibleGraphicShape::AccessibleGraphicShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessibleGraphicShape::~AccessibleGraphicShape (void)
{
}




//=====  XAccessibleImage  ====================================================

::rtl::OUString SAL_CALL AccessibleGraphicShape::getAccessibleImageDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
//IAccessibility2 Implementation 2009-----
    if(m_pShape)
            return m_pShape->GetTitle();
//-----IAccessibility2 Implementation 2009
    return AccessibleShape::getAccessibleDescription ();
}




sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageHeight (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getSize().Height;
}




sal_Int32 SAL_CALL AccessibleGraphicShape::getAccessibleImageWidth (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getSize().Width;
}




//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    AccessibleGraphicShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleShape::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleImage*>(this));
    return aReturn;
}



void SAL_CALL
    AccessibleGraphicShape::acquire (void)
    throw ()
{
    AccessibleShape::acquire ();
}



void SAL_CALL
    AccessibleGraphicShape::release (void)
    throw ()
{
    AccessibleShape::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleGraphicShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("AccessibleGraphicShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleGraphicShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames =
        AccessibleShape::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName (RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.drawing.AccessibleGraphicShape"));
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}




//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleGraphicShape::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (AccessibleShape::getTypes());
    // ...and add the additional type for the component.
    long nTypeCount = aTypeList.getLength();
    aTypeList.realloc (nTypeCount + 1);
    const uno::Type aImageType =
        ::getCppuType((const uno::Reference<XAccessibleImage>*)0);
    aTypeList[nTypeCount] = aImageType;

    return aTypeList;
}




/// Create the base name of this object, i.e. the name without appended number.
::rtl::OUString
    AccessibleGraphicShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_GRAPHIC_OBJECT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("GraphicObjectShape"));
            break;

        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleGraphicShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}



::rtl::OUString
    AccessibleGraphicShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
//IAccessibility2 Implementation 2009-----
    //Solution: Don't use the same information for accessible name and accessible description.
        //return CreateAccessibleName ();
     ::rtl::OUString sDesc;
        if(m_pShape)
        sDesc =  m_pShape->GetTitle();
        if(sDesc.getLength() > 0)
            return sDesc;
        return CreateAccessibleBaseName();
//-----IAccessibility2 Implementation 2009
}
//IAccessibility2 Implementation 2009-----
//  Return this object's role.
sal_Int16 SAL_CALL AccessibleGraphicShape::getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nAccessibleRole =  AccessibleRole::SHAPE;
    if( m_pShape->getSdrModelFromSdrObject().GetImageMapForObject(m_pShape) != NULL )
        return AccessibleRole::IMAGE_MAP;
    else
        //return AccessibleRole::SHAPE;
        return AccessibleShape::getAccessibleRole();
    return nAccessibleRole;
}
//-----IAccessibility2 Implementation 2009


