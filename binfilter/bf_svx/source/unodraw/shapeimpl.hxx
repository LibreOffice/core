/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_SHAPEIMPL_HXX
#define _SVX_SHAPEIMPL_HXX

#include "unoshape.hxx"
class SvGlobalName;
namespace binfilter {


/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxShapeCaption : public SvxShapeText
{
public:
    SvxShapeCaption( SdrObject* pObj ) throw();
    virtual ~SvxShapeCaption() throw();
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxOle2Shape : public SvxShape
{
public:
    SvxOle2Shape( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ();
    virtual ~SvxOle2Shape() throw();

    sal_Bool createObject( const SvGlobalName &aClassName );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual void SAL_CALL 	setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
};

/***********************************************************************
*                                                                      *
***********************************************************************/
}//end of namespace binfilter
#ifndef SVX_LIGHT

#ifndef _PLUGIN_HXX 
#include <bf_so3/plugin.hxx>
#endif
namespace binfilter {
class SvxPluginShape : public SvxOle2Shape
{
private:
    SvPlugInObjectRef mxPlugin;

public:
    SvxPluginShape( SdrObject* pObj ) throw();
    virtual ~SvxPluginShape() throw();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();

    //XPropertySet
    virtual void SAL_CALL 	setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
};
}//end of namespace binfilter
#endif

/***********************************************************************
*                                                                      *
***********************************************************************/

#ifndef SVX_LIGHT

#ifndef _APPLET_HXX 
#include <bf_so3/applet.hxx>
#endif
namespace binfilter {
class SvxAppletShape : public SvxOle2Shape
{
private:
    SvAppletObjectRef mxApplet;

public:
    SvxAppletShape( SdrObject* pObj ) throw();
    virtual ~SvxAppletShape() throw();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();

    //XPropertySet
    virtual void SAL_CALL 	setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
};
}//end of namespace binfilter
#endif

/***********************************************************************
*                                                                      *
***********************************************************************/

#ifndef SVX_LIGHT

namespace binfilter {
class SvxFrameShape : public SvxOle2Shape
{
public:
    SvxFrameShape( SdrObject* pObj ) throw();
    virtual ~SvxFrameShape() throw();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();

    //XPropertySet
    virtual void SAL_CALL 	setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
};
}//end of namespace binfilter
#endif

#endif


