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
#include <basic/sberrors.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static css::uno::Any
lcl_createAPIStyleToVBAObject( const css::uno::Any& aObject, const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< beans::XPropertySet > xStyleProps( aObject, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XStyle > xStyle( new ScVbaStyle( xParent, xContext, xStyleProps, xModel ) );
    return uno::Any( xStyle );
}

ScVbaStyles::ScVbaStyles( const uno::Reference< XHelperInterface >& xParent,
                          const uno::Reference< css::uno::XComponentContext > & xContext,
                          const uno::Reference< frame::XModel >& xModel )
: ScVbaStyles_BASE( xParent,
                    xContext,
                    uno::Reference< container::XIndexAccess >( ScVbaStyle::getStylesNameContainer( xModel ), uno::UNO_QUERY_THROW ) ),
  mxModel( xModel )
{
    try
    {
        mxMSF.set( mxModel, uno::UNO_QUERY_THROW );
        mxNameContainerCellStyles.set( m_xNameAccess, uno::UNO_QUERY_THROW );
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

uno::Sequence< OUString >
ScVbaStyles::getStyleNames()
{
    return mxNameContainerCellStyles->getElementNames();
}

uno::Any
ScVbaStyles::createCollectionObject(const uno::Any& aObject)
{
    return lcl_createAPIStyleToVBAObject( aObject, mxParent, mxContext, mxModel );
}

uno::Type SAL_CALL
ScVbaStyles::getElementType()
{
    return cppu::UnoType<excel::XStyle>::get();
}

namespace {

class EnumWrapper : public EnumerationHelper_BASE
{
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        uno::Reference<XHelperInterface > m_xParent;
        uno::Reference<uno::XComponentContext > m_xContext;
        uno::Reference<frame::XModel > m_xModel;

        sal_Int32 nIndex;
public:
        EnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess, uno::Reference<XHelperInterface > xParent, uno::Reference<uno::XComponentContext > xContext, uno::Reference<frame::XModel > xModel ) : m_xIndexAccess(std::move( xIndexAccess )), m_xParent(std::move( xParent )), m_xContext(std::move( xContext )), m_xModel(std::move( xModel )), nIndex( 0 ) {}
        virtual sal_Bool SAL_CALL hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }
        virtual uno::Any SAL_CALL nextElement(  ) override
        {
            try
            {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return lcl_createAPIStyleToVBAObject( m_xIndexAccess->getByIndex( nIndex++ ), m_xParent, m_xContext, m_xModel );
            }
            catch (const container::NoSuchElementException&)
            {
                throw;
            }
            catch (const lang::WrappedTargetException&)
            {
                throw;
            }
            catch (const uno::RuntimeException&)
            {
                throw;
            }
            catch (const uno::Exception& e)
            {
                css::uno::Any a(cppu::getCaughtException());
                throw css::lang::WrappedTargetException(
                    "wrapped Exception " + e.Message,
                    css::uno::Reference<css::uno::XInterface>(), a);
            }
            throw container::NoSuchElementException();
        }
};

}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaStyles::createEnumeration()
{
    return new EnumWrapper( m_xIndexAccess, mxParent, mxContext, mxModel );
}

uno::Reference< excel::XStyle > SAL_CALL
ScVbaStyles::Add( const OUString& _sName, const uno::Any& _aBasedOn )
{
    uno::Reference< excel::XStyle > aRet;
    try
    {
        OUString sParentCellStyleName(u"Default"_ustr);
        if ( _aBasedOn.hasValue() )
        {
            uno::Reference< excel::XRange > oRange;
            if ( _aBasedOn >>= oRange)
            {
                uno::Reference< excel::XStyle > oStyle( oRange->getStyle(), uno::UNO_QUERY_THROW );
                sParentCellStyleName = oStyle->getName();
            }
            else
            {
                DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, {});
            }
        }

        uno::Reference< style::XStyle > xStyle( mxMSF->createInstance(u"com.sun.star.style.CellStyle"_ustr), uno::UNO_QUERY_THROW );

        if (!mxNameContainerCellStyles->hasByName(_sName))
        {
            mxNameContainerCellStyles->insertByName(_sName, uno::Any( xStyle) );
        }
        if (sParentCellStyleName != "Default")
        {
            xStyle->setParentStyle( sParentCellStyleName );
        }
        aRet.set( Item( uno::Any( _sName ), uno::Any() ), uno::UNO_QUERY_THROW );
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
    return aRet;
}

void
ScVbaStyles::Delete(const OUString& _sStyleName)
{
    try
    {
        if (mxNameContainerCellStyles->hasByName( _sStyleName ) )
            mxNameContainerCellStyles->removeByName( _sStyleName );
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

OUString
ScVbaStyles::getServiceImplName()
{
    return u"ScVbaStyles"_ustr;
}

uno::Sequence< OUString >
ScVbaStyles::getServiceNames()
{
        static uno::Sequence< OUString > const aServiceNames
        {
            u"ooo.vba.excel.XStyles"_ustr
        };
        return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
