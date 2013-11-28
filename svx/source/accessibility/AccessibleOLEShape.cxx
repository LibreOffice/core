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


#include <svx/AccessibleOLEShape.hxx>

#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <svx/svdoole2.hxx>
using namespace accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

AccessibleOLEShape::AccessibleOLEShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessibleOLEShape::~AccessibleOLEShape (void)
{
}




//=====  XAccessibleAction  ===================================================

sal_Int32 SAL_CALL AccessibleOLEShape::getAccessibleActionCount (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}




sal_Bool SAL_CALL AccessibleOLEShape::doAccessibleAction (sal_Int32 /*nIndex*/)
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
}




::rtl::OUString SAL_CALL AccessibleOLEShape::getAccessibleActionDescription (sal_Int32 /*nIndex*/)
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
}




Reference<XAccessibleKeyBinding> SAL_CALL AccessibleOLEShape::getAccessibleActionKeyBinding (sal_Int32 /*nIndex*/)
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
}




//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    AccessibleOLEShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleShape::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleAction*>(this));
    return aReturn;
}



void SAL_CALL
    AccessibleOLEShape::acquire (void)
    throw ()
{
    AccessibleShape::acquire ();
}



void SAL_CALL
    AccessibleOLEShape::release (void)
    throw ()
{
    AccessibleShape::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleOLEShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleOLEShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleOLEShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed();
    // Get list of supported service names from base class...
    uno::Sequence< ::rtl::OUString > aServiceNames =
        AccessibleShape::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    static const ::rtl::OUString sAdditionalServiceName (RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.drawing.AccessibleOLEShape"));
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}




//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleOLEShape::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (AccessibleShape::getTypes());
    // ...and add the additional type for the component.
    long nTypeCount = aTypeList.getLength();
    aTypeList.realloc (nTypeCount + 1);
    const uno::Type aActionType =
        ::getCppuType((const uno::Reference<XAccessibleAction>*)0);
    aTypeList[nTypeCount] = aActionType;

    return aTypeList;
}
//=====  XAccessibleExtendedAttributes  ========================================================
uno::Any SAL_CALL AccessibleOLEShape::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Any strRet;
    ::rtl::OUString style;
    if( m_pShape )
    {
        //style = ::rtl::OUString::createFromAscii("style=");
        style = ::rtl::OUString::createFromAscii("style:");
        style += ((SdrOle2Obj*)m_pShape)->GetStyleString();
    }
    style += ::rtl::OUString::createFromAscii(";");
    strRet <<= style;
    return strRet;
}


/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessibleOLEShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_APPLET:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("AppletOLEShape"));
            break;
        case DRAWING_FRAME:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("FrameOLEShape"));
            break;
        case DRAWING_OLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("OLEShape"));
            break;
        case DRAWING_PLUGIN:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PluginOLEShape"));
            break;

        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleOLEShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}



::rtl::OUString
    AccessibleOLEShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return CreateAccessibleName ();
}

