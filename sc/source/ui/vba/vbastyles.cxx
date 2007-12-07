/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbastyles.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:03:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "vbastyles.hxx"
#include "vbastyle.hxx"

using namespace ::org::openoffice;
using namespace ::com::sun::star;

static rtl::OUString SDEFAULTCELLSTYLENAME( RTL_CONSTASCII_USTRINGPARAM("Default") );
css::uno::Any
lcl_createAPIStyleToVBAObject( const css::uno::Any& aObject, const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< beans::XPropertySet > xStyleProps( aObject, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XStyle > xStyle( new ScVbaStyle( xParent, xContext, xStyleProps, xModel ) );
    return uno::makeAny( xStyle );
}


ScVbaStyles::ScVbaStyles( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xModel ) throw ( script::BasicErrorException ) : ScVbaStyles_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( ScVbaStyle::getStylesNameContainer( xModel ), uno::UNO_QUERY_THROW ) ), mxModel( xModel ), mxParent( xParent )
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
        uno::Reference<vba::XHelperInterface > m_xParent;
        uno::Reference<uno::XComponentContext > m_xContext;
        uno::Reference<frame::XModel > m_xModel;

        sal_Int32 nIndex;
public:
        EnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess, const uno::Reference<vba::XHelperInterface >& xParent, const uno::Reference<uno::XComponentContext >& xContext, const uno::Reference<frame::XModel >& xModel ) : m_xIndexAccess( xIndexAccess ), m_xParent( xParent ), m_xContext( xContext ), m_xModel( xModel ), nIndex( 0 ) {}
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

rtl::OUString&
ScVbaStyles::getServiceImplName()
{
        static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaStyles") );
        return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaStyles::getServiceNames()
{
        static uno::Sequence< rtl::OUString > aServiceNames;
        if ( aServiceNames.getLength() == 0 )
        {
                aServiceNames.realloc( 1 );
                aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.XStyles" ) );
        }
        return aServiceNames;
}
