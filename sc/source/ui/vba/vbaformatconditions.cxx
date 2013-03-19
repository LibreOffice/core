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

#include <ooo/vba/excel/XRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <vector>
#include "vbaformatconditions.hxx"
#include "vbaformatcondition.hxx"
#include "vbaworkbook.hxx"
#include "vbastyles.hxx"
#include "vbaglobals.hxx"
using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef std::vector< beans::PropertyValue > VecPropValues;

static rtl::OUString OPERATOR( RTL_CONSTASCII_USTRINGPARAM("Operator") );
static rtl::OUString FORMULA1( RTL_CONSTASCII_USTRINGPARAM("Formula1") );
static rtl::OUString FORMULA2( RTL_CONSTASCII_USTRINGPARAM("Formula2") );
static rtl::OUString STYLENAME( RTL_CONSTASCII_USTRINGPARAM("StyleName") );
static rtl::OUString sStyleNamePrefix( RTL_CONSTASCII_USTRINGPARAM("Excel_CondFormat") );

void SAL_CALL
ScVbaFormatConditions::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        ScVbaStyles* pStyles = static_cast< ScVbaStyles* >( mxStyles.get() );
        if ( !pStyles )
            DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
        sal_Int32 nCount = mxSheetConditionalEntries->getCount();
        for (sal_Int32 i = nCount - 1; i >= 0; i--)
        {
            uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry( mxSheetConditionalEntries->getByIndex(i), uno::UNO_QUERY_THROW );
            pStyles->Delete(xSheetConditionalEntry->getStyleName());
            mxSheetConditionalEntries->removeByIndex(i);
        }
        notifyRange();
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

uno::Type SAL_CALL
ScVbaFormatConditions::getElementType() throw (css::uno::RuntimeException)
{
    return excel::XFormatCondition::static_type(0);
}


uno::Any xSheetConditionToFormatCondition( const uno::Reference< XHelperInterface >& xRangeParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< excel::XStyles >& xStyles, const uno::Reference< excel::XFormatConditions >& xFormatConditions, const uno::Reference< beans::XPropertySet >& xRangeProps,  const uno::Any& aObject )
{
    uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry;
    aObject >>= xSheetConditionalEntry;

    uno::Reference< excel::XStyle > xStyle( xStyles->Item( uno::makeAny( xSheetConditionalEntry->getStyleName() ), uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XFormatCondition > xCondition = new ScVbaFormatCondition( xRangeParent, xContext,  xSheetConditionalEntry, xStyle, xFormatConditions, xRangeProps );
    return uno::makeAny( xCondition );
}

uno::Any
ScVbaFormatConditions::createCollectionObject(const uno::Any& aObject )
{
    return xSheetConditionToFormatCondition( uno::Reference< XHelperInterface >( mxRangeParent, uno::UNO_QUERY_THROW ), mxContext, mxStyles, this, mxParentRangePropertySet, aObject );
}

class EnumWrapper : public EnumerationHelper_BASE
{
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        uno::Reference<excel::XRange > m_xParentRange;
        uno::Reference<uno::XComponentContext > m_xContext;
        uno::Reference<excel::XStyles > m_xStyles;
        uno::Reference<excel::XFormatConditions > m_xParentCollection;
        uno::Reference<beans::XPropertySet > m_xProps;

        sal_Int32 nIndex;
public:
        EnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess, const uno::Reference<excel::XRange >& xRange, const uno::Reference<uno::XComponentContext >& xContext, const uno::Reference<excel::XStyles >& xStyles, const uno::Reference< excel::XFormatConditions >& xCollection, const uno::Reference<beans::XPropertySet >& xProps  ) : m_xIndexAccess( xIndexAccess ), m_xParentRange( xRange ), m_xContext( xContext ), m_xStyles( xStyles ), m_xParentCollection( xCollection ), m_xProps( xProps ), nIndex( 0 ) {}
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return xSheetConditionToFormatCondition( uno::Reference< XHelperInterface >( m_xParentRange, uno::UNO_QUERY_THROW ), m_xContext, m_xStyles, m_xParentCollection, m_xProps, m_xIndexAccess->getByIndex( nIndex++ ) );
                throw container::NoSuchElementException();
        }
};

uno::Reference< excel::XFormatCondition > SAL_CALL
ScVbaFormatConditions::Add( ::sal_Int32 _nType, const uno::Any& _aOperator, const uno::Any& _aFormula1, const uno::Any& _aFormula2 ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return Add( _nType, _aOperator, _aFormula1, _aFormula2, uno::Reference< excel::XStyle >() );
}

uno::Reference< excel::XFormatCondition >
ScVbaFormatConditions::Add( ::sal_Int32 _nType, const uno::Any& _aOperator, const uno::Any& _aFormula1, const uno::Any& _aFormula2, const css::uno::Reference< excel::XStyle >& _xStyle  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    // #TODO
    // #FIXME
    // This method will NOT handle r1c1 formulas [*]and only assumes that
    // the formulas are _xlA1 based ( need to hook into calc work ths should
    // address this )
    // [*] reason: getA1Formula method below is just a hook and just
    // returns whats it gets ( e.g. doesn't convert anything )
    uno::Reference< excel::XStyle > xStyle( _xStyle );
    uno::Reference< excel::XFormatCondition > xFormatCondition;
    try
    {
        rtl::OUString sStyleName;
        if ( !xStyle.is() )
        {
            sStyleName = getStyleName();
            xStyle = mxStyles->Add(sStyleName, uno::Any() );
        }
        else
        {
            sStyleName = xStyle->getName();
        }

        VecPropValues aPropertyValueVector;
        sheet::ConditionOperator aType = ScVbaFormatCondition::retrieveAPIType(_nType, uno::Reference< sheet::XSheetCondition >() );
        uno::Any aValue;

        if ( aType == sheet::ConditionOperator_FORMULA)
            aValue = uno::makeAny( sheet::ConditionOperator_FORMULA );
        else
            aValue = uno::makeAny( ScVbaFormatCondition::retrieveAPIOperator(_aOperator) );

        beans::PropertyValue aProperty( OPERATOR, 0, aValue, beans::PropertyState_DIRECT_VALUE );
        aPropertyValueVector.push_back( aProperty );

        if ( _aFormula1.hasValue() )
        {
            beans::PropertyValue aProp( FORMULA1, 0, uno::makeAny( getA1Formula( _aFormula1 ) ), beans::PropertyState_DIRECT_VALUE );
            aPropertyValueVector.push_back( aProp );
        }
        if ( _aFormula2.hasValue() )
        {
            beans::PropertyValue aProp( FORMULA2, 0, uno::makeAny( getA1Formula( _aFormula2 ) ), beans::PropertyState_DIRECT_VALUE );
            aPropertyValueVector.push_back( aProp );
        }
        aProperty.Name = STYLENAME;
        aProperty.Value = uno::makeAny( sStyleName );

        // convert vector to sequence
        uno::Sequence< beans::PropertyValue > aPropertyValueList(aPropertyValueVector.size());
        VecPropValues::iterator it = aPropertyValueVector.begin();
        VecPropValues::iterator it_end = aPropertyValueVector.end();
        for ( sal_Int32 index=0; it != it_end; ++it )
            aPropertyValueList[ index++ ] = *it;

        mxSheetConditionalEntries->addNew(aPropertyValueList);
        for (sal_Int32 i = mxSheetConditionalEntries->getCount()-1; i >= 0; i--)
        {
            uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry( mxSheetConditionalEntries->getByIndex(i), uno::UNO_QUERY_THROW );
            if (xSheetConditionalEntry->getStyleName().equals(sStyleName))
            {
                xFormatCondition =  new ScVbaFormatCondition(uno::Reference< XHelperInterface >( mxRangeParent, uno::UNO_QUERY_THROW ), mxContext, xSheetConditionalEntry, xStyle, this, mxParentRangePropertySet);
                notifyRange();
                return xFormatCondition;
            }
        }
    }
    catch (uno::Exception& )
    {
    }
    DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    return xFormatCondition;
}


uno::Reference< container::XEnumeration > SAL_CALL
ScVbaFormatConditions::createEnumeration() throw (uno::RuntimeException)
{
    return new EnumWrapper( m_xIndexAccess, mxRangeParent, mxContext, mxStyles, this, mxParentRangePropertySet  );
}


void
ScVbaFormatConditions::notifyRange() throw ( script::BasicErrorException )
{
    try
    {
        mxParentRangePropertySet->setPropertyValue( rtl::OUString("ConditionalFormat"), uno::makeAny( mxSheetConditionalEntries ));
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

rtl::OUString
ScVbaFormatConditions::getA1Formula(const css::uno::Any& _aFormula) throw ( script::BasicErrorException )
{
    // #TODO, #FIXME hook-in proper formula conversion detection & logic
    rtl::OUString sFormula;
    if ( !( _aFormula >>= sFormula ) )
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    return sFormula;
}

rtl::OUString
ScVbaFormatConditions::getStyleName()
{
    ScVbaStyles* pStyles = static_cast< ScVbaStyles* >( mxStyles.get() );
    if ( !pStyles )
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    uno::Sequence< rtl::OUString > sCellStyleNames = pStyles->getStyleNames();
    return ContainerUtilities::getUniqueName(sCellStyleNames, sStyleNamePrefix, rtl::OUString("_"));
}

void
ScVbaFormatConditions::removeFormatCondition( const rtl::OUString& _sStyleName, sal_Bool _bRemoveStyle) throw ( script::BasicErrorException )
{
    try
    {
        sal_Int32 nElems = mxSheetConditionalEntries->getCount();
        for (sal_Int32 i = 0; i < nElems; i++)
        {
            uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry( mxSheetConditionalEntries->getByIndex(i), uno::UNO_QUERY_THROW );
            if (_sStyleName.equals(xSheetConditionalEntry->getStyleName()))
            {
                mxSheetConditionalEntries->removeByIndex(i);
                if (_bRemoveStyle)
                {
                    ScVbaStyles* pStyles = static_cast< ScVbaStyles* >( mxStyles.get() );
                    if ( !pStyles )
                        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
                    pStyles->Delete( _sStyleName );
                }
                return;
            }
        }
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString());
    }
}

rtl::OUString
ScVbaFormatConditions::getServiceImplName()
{
    return rtl::OUString("ScVbaFormatConditions");
}

uno::Sequence< rtl::OUString >
ScVbaFormatConditions::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.FormatConditions" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
