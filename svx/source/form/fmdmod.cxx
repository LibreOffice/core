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
#include <svx/fmdmod.hxx>
#include "fmservs.hxx"
#include <fmobj.hxx>
#include <svx/unoshape.hxx>
#include <comphelper/processfactory.hxx>
#include <svx/fmglob.hxx>

using namespace ::svxform;

//-----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SvxFmMSFactory::createInstance(const ::rtl::OUString& ServiceSpecifier) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xRet;
    if ( ServiceSpecifier.indexOf( ::rtl::OUString::createFromAscii("com.sun.star.form.component.") ) == 0 )
    {
        xRet = ::comphelper::getProcessServiceFactory()->createInstance(ServiceSpecifier);
    }
    else if ( ServiceSpecifier == ::rtl::OUString( ::rtl::OUString::createFromAscii("com.sun.star.drawing.ControlShape") ) )
    {
        SdrModel* pTargetModel = getSdrModel();
        OSL_ENSURE(pTargetModel, "Got no SdrModel for SdrShape construction (!)");
        SdrObject* pObj = new FmFormObj(*pTargetModel, ::rtl::OUString(), OBJ_FM_CONTROL);
        xRet = *new SvxShapeControl(pObj);
    }
    if (!xRet.is())
        xRet = SvxUnoDrawMSFactory::createInstance(ServiceSpecifier);
    return xRet;
}

//-----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL  SvxFmMSFactory::createInstanceWithArguments(const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments) throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException )
{
    return SvxUnoDrawMSFactory::createInstanceWithArguments(ServiceSpecifier, Arguments );
}

//-----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SvxFmMSFactory::getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException )
{
    static const ::rtl::OUString aSvxComponentServiceNameList[] =
    {
        FM_SUN_COMPONENT_TEXTFIELD,
        FM_SUN_COMPONENT_FORM,
        FM_SUN_COMPONENT_LISTBOX,
        FM_SUN_COMPONENT_COMBOBOX,
        FM_SUN_COMPONENT_RADIOBUTTON,
        FM_SUN_COMPONENT_GROUPBOX,
        FM_SUN_COMPONENT_FIXEDTEXT,
        FM_SUN_COMPONENT_COMMANDBUTTON,
        FM_SUN_COMPONENT_CHECKBOX,
        FM_SUN_COMPONENT_GRIDCONTROL,
        FM_SUN_COMPONENT_IMAGEBUTTON,
        FM_SUN_COMPONENT_FILECONTROL,
        FM_SUN_COMPONENT_TIMEFIELD,
        FM_SUN_COMPONENT_DATEFIELD,
        FM_SUN_COMPONENT_NUMERICFIELD,
        FM_SUN_COMPONENT_CURRENCYFIELD,
        FM_SUN_COMPONENT_PATTERNFIELD,
        FM_SUN_COMPONENT_HIDDENCONTROL,
        FM_SUN_COMPONENT_IMAGECONTROL
    };

    static const sal_uInt16 nSvxComponentServiceNameListCount = sizeof(aSvxComponentServiceNameList) / sizeof ( aSvxComponentServiceNameList[0] );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSeq( nSvxComponentServiceNameListCount );
    ::rtl::OUString* pStrings = aSeq.getArray();
    for( sal_uInt16 nIdx = 0; nIdx < nSvxComponentServiceNameListCount; nIdx++ )
        pStrings[nIdx] = aSvxComponentServiceNameList[nIdx];

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aParentSeq( SvxUnoDrawMSFactory::getAvailableServiceNames() );
    return concatServiceNames( aParentSeq, aSeq );
}

/*
// XServiceManager
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SvxFmDrawModel::createInstance(const ::rtl::OUString& ServiceName)
            const throw( ::com::sun::star::lang::ServiceNotRegisteredException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xRet;
    sal_uInt16 nTokenCount = ServiceName.getTokenCount('.');
    if (nTokenCount == 5 &&
        ServiceName.getToken( 0, '.' ) == ::rtl::OUString::createFromAscii("stardiv") &&
        ServiceName.getToken( 1, '.' ) == ::rtl::OUString::createFromAscii("one") &&
        ServiceName.getToken( 2, '.' ) == ::rtl::OUString::createFromAscii("form") &&
        ServiceName.getToken( 3, '.' ) == ::rtl::OUString::createFromAscii("component"))
    {
        xRet = ::comphelper::getProcessServiceFactory()->createInstance(ServiceName);
    }
    else
    if (nTokenCount == 4 &&
        ServiceName.getToken( 0, '.' ) == ::rtl::OUString::createFromAscii("stardiv") &&
        ServiceName.getToken( 1, '.' ) == ::rtl::OUString::createFromAscii("one") &&
        ServiceName.getToken( 2, '.' ) == ::rtl::OUString::createFromAscii("drawing") &&
        ServiceName.getToken( 3, '.' ) == ::rtl::OUString::createFromAscii("ControlShape"))
    {
        SdrObject* pObj = new FmFormObj();
        xRet = *new SvxShapeControl(pObj);
    }
    if (!xRet.is())
        xRet = SvxUnoDrawModel::createInstance(ServiceName);
    return xRet;
}
*/


