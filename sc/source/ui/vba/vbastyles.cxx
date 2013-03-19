/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "vbastyles.hxx"
#include "vbastyle.hxx"
#include <ooo/vba/excel/XRange.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static rtl::OUString SDEFAULTCELLSTYLENAME( RTL_CONSTASCII_USTRINGPARAM("Default") );
static css::uno::Any
lcl_createAPIStyleToVBAObject( const css::uno::Any& aObject, const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< beans::XPropertySet > xStyleProps( aObject, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XStyle > xStyle( new ScVbaStyle( xParent, xContext, xStyleProps, xModel ) );
    return uno::makeAny( xStyle );
}


ScVbaStyles::ScVbaStyles( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ) throw ( script::BasicErrorException ) : ScVbaStyles_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( ScVbaStyle::getStylesNameContainer( xModel ), uno::UNO_QUERY_THROW ) ), mxModel( xModel ), mxParent( xParent )
{
    try
    {
        mxMSF.set( mxModel, uno::UNO_QUERY_THROW );
        mxNameContainerCellStyles.set( m_xNameAccess, uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

uno::Sequence< rtl::OUString >
ScVbaStyles::getStyleNames() throw ( uno::RuntimeException )
{
    return mxNameContainerCellStyles->getElementNames();
}


uno::Any
ScVbaStyles::createCollectionObject(const uno::Any& aObject)
{
    return lcl_createAPIStyleToVBAObject( aObject, mxParent, mxContext, mxModel );
}

uno::Type SAL_CALL
ScVbaStyles::getElementType() throw (uno::RuntimeException)
{
    return excel::XStyle::static_type(0);
}

class EnumWrapper : public EnumerationHelper_BASE
{
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        uno::Reference<XHelperInterface > m_xParent;
        uno::Reference<uno::XComponentContext > m_xContext;
        uno::Reference<frame::XModel > m_xModel;

        sal_Int32 nIndex;
public:
        EnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess, const uno::Reference<XHelperInterface >& xParent, const uno::Reference<uno::XComponentContext >& xContext, const uno::Reference<frame::XModel >& xModel ) : m_xIndexAccess( xIndexAccess ), m_xParent( xParent ), m_xContext( xContext ), m_xModel( xModel ), nIndex( 0 ) {}
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return lcl_createAPIStyleToVBAObject( m_xIndexAccess->getByIndex( nIndex++ ), m_xParent, m_xContext, m_xModel );
                throw container::NoSuchElementException();
        }
};

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaStyles::createEnumeration() throw (uno::RuntimeException)
{
    return new EnumWrapper( m_xIndexAccess, mxParent, mxContext, mxModel );
}

uno::Reference< excel::XStyle > SAL_CALL
ScVbaStyles::Add( const ::rtl::OUString& _sName, const uno::Any& _aBasedOn ) throw (script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference< excel::XStyle > aRet;
    try
    {
        rtl::OUString sParentCellStyleName( RTL_CONSTASCII_USTRINGPARAM("Default"));
        if ( _aBasedOn.hasValue() )
        {
            uno::Reference< excel::XRange > oRange;
            if ( _aBasedOn >>= oRange)
            {
                uno::Reference< excel::XStyle > oStyle( oRange->getStyle(), uno::UNO_QUERY_THROW );
                if ( oStyle.is() )
                {
                    sParentCellStyleName = oStyle->getName();
                }
                else
                {
                    DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString() );
                }
            }
            else
            {
                DebugHelper::exception(SbERR_BAD_ARGUMENT, rtl::OUString());
            }
        }

        uno::Reference< style::XStyle > xStyle( mxMSF->createInstance( rtl::OUString("com.sun.star.style.CellStyle")), uno::UNO_QUERY_THROW );

        if (!mxNameContainerCellStyles->hasByName(_sName))
        {
            mxNameContainerCellStyles->insertByName(_sName, uno::makeAny( xStyle) );
        }
        if (!sParentCellStyleName.equals(SDEFAULTCELLSTYLENAME))
        {
            xStyle->setParentStyle( sParentCellStyleName );
        }
        aRet.set( Item( uno::makeAny( _sName ), uno::Any() ), uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
    return aRet;
}

void
ScVbaStyles::Delete(const rtl::OUString _sStyleName) throw ( script::BasicErrorException )
{
    try
    {
        if (mxNameContainerCellStyles->hasByName( _sStyleName ) )
            mxNameContainerCellStyles->removeByName( _sStyleName );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

rtl::OUString
ScVbaStyles::getServiceImplName()
{
    return rtl::OUString("ScVbaStyles");
}

uno::Sequence< rtl::OUString >
ScVbaStyles::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.XStyles" );
        }
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
