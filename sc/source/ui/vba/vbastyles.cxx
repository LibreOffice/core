/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

        uno::Reference< style::XStyle > xStyle( mxMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.CellStyle"))), uno::UNO_QUERY_THROW );

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
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaStyles"));
}

uno::Sequence< rtl::OUString >
ScVbaStyles::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.XStyles" ) );
        }
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
