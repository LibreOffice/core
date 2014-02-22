/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/AccessibleOLEShape.hxx>

#include <svx/ShapeTypeHandler.hxx>
#include <svx/SvxShapeTypes.hxx>
#include <svx/svdoole2.hxx>

using namespace accessibility;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;



AccessibleOLEShape::AccessibleOLEShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleShape (rShapeInfo, rShapeTreeInfo)
{
}




AccessibleOLEShape::~AccessibleOLEShape (void)
{
}






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




OUString SAL_CALL AccessibleOLEShape::getAccessibleActionDescription (sal_Int32 /*nIndex*/)
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






OUString SAL_CALL
    AccessibleOLEShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("AccessibleOLEShape");
}




::com::sun::star::uno::Sequence< OUString> SAL_CALL
    AccessibleOLEShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed();
    
    uno::Sequence< OUString > aServiceNames =
        AccessibleShape::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName ("com.sun.star.drawing.AccessibleOLEShape");
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}






uno::Sequence<uno::Type> SAL_CALL
    AccessibleOLEShape::getTypes (void)
    throw (uno::RuntimeException)
{
    
    uno::Sequence<uno::Type> aTypeList (AccessibleShape::getTypes());
    
    long nTypeCount = aTypeList.getLength();
    aTypeList.realloc (nTypeCount + 1);
    const uno::Type aActionType =
        ::getCppuType((const uno::Reference<XAccessibleAction>*)0);
    aTypeList[nTypeCount] = aActionType;

    return aTypeList;
}


uno::Any SAL_CALL AccessibleOLEShape::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    uno::Any strRet;
    ::rtl::OUString style;
    if( m_pShape )
    {
        
        style = ::rtl::OUString::createFromAscii("style:");
        style += ((SdrOle2Obj*)m_pShape)->GetStyleString();
    }
    style += ::rtl::OUString::createFromAscii(";");
    strRet <<= style;
    return strRet;
}

/
OUString
    AccessibleOLEShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_APPLET:
            sName = "AppletOLEShape";
            break;
        case DRAWING_FRAME:
            sName = "FrameOLEShape";
            break;
        case DRAWING_OLE:
            sName = "OLEShape";
            break;
        case DRAWING_PLUGIN:
            sName = "PluginOLEShape";
            break;

        default:
            sName = "UnknownAccessibleOLEShape";
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ": " + xDescriptor->getShapeType();
    }

    return sName;
}



OUString
    AccessibleOLEShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return CreateAccessibleName ();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
