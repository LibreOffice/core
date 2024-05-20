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
#include <com/sun/star/sheet/XSheetConditionalEntry.hpp>
#include <basic/sberrors.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <utility>
#include <vector>
#include <unonames.hxx>
#include "vbaformatconditions.hxx"
#include "vbaformatcondition.hxx"
#include "vbastyles.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

void SAL_CALL
ScVbaFormatConditions::Delete(  )
{
    try
    {
        ScVbaStyles* pStyles = mxStyles.get();
        if ( !pStyles )
            DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
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
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

uno::Type SAL_CALL
ScVbaFormatConditions::getElementType()
{
    return cppu::UnoType<excel::XFormatCondition>::get();
}

static uno::Any xSheetConditionToFormatCondition( const uno::Reference< XHelperInterface >& xRangeParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< excel::XStyles >& xStyles, const uno::Reference< excel::XFormatConditions >& xFormatConditions, const uno::Reference< beans::XPropertySet >& xRangeProps,  const uno::Any& aObject )
{
    uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry;
    aObject >>= xSheetConditionalEntry;

    uno::Reference< excel::XStyle > xStyle( xStyles->Item( uno::Any( xSheetConditionalEntry->getStyleName() ), uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XFormatCondition > xCondition = new ScVbaFormatCondition( xRangeParent, xContext,  xSheetConditionalEntry, xStyle, xFormatConditions, xRangeProps );
    return uno::Any( xCondition );
}

uno::Any
ScVbaFormatConditions::createCollectionObject(const uno::Any& aObject )
{
    return xSheetConditionToFormatCondition( uno::Reference< XHelperInterface >( mxRangeParent, uno::UNO_QUERY_THROW ), mxContext, mxStyles, this, mxParentRangePropertySet, aObject );
}

namespace {

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
        EnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess, uno::Reference<excel::XRange > xRange, uno::Reference<uno::XComponentContext > xContext, uno::Reference<excel::XStyles > xStyles, uno::Reference< excel::XFormatConditions > xCollection, uno::Reference<beans::XPropertySet > xProps  ) : m_xIndexAccess(std::move( xIndexAccess )), m_xParentRange(std::move( xRange )), m_xContext(std::move( xContext )), m_xStyles(std::move( xStyles )), m_xParentCollection(std::move( xCollection )), m_xProps(std::move( xProps )), nIndex( 0 ) {}
        virtual sal_Bool SAL_CALL hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) override
        {
            try
            {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return xSheetConditionToFormatCondition( uno::Reference< XHelperInterface >( m_xParentRange, uno::UNO_QUERY_THROW ), m_xContext, m_xStyles, m_xParentCollection, m_xProps, m_xIndexAccess->getByIndex( nIndex++ ) );
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

uno::Reference< excel::XFormatCondition > SAL_CALL
ScVbaFormatConditions::Add( ::sal_Int32 _nType, const uno::Any& _aOperator, const uno::Any& _aFormula1, const uno::Any& _aFormula2 )
{
    return Add( _nType, _aOperator, _aFormula1, _aFormula2, uno::Reference< excel::XStyle >() );
}

uno::Reference< excel::XFormatCondition >
ScVbaFormatConditions::Add( ::sal_Int32 _nType, const uno::Any& _aOperator, const uno::Any& _aFormula1, const uno::Any& _aFormula2, const css::uno::Reference< excel::XStyle >& _xStyle  )
{
    // #TODO
    // #FIXME
    // This method will NOT handle r1c1 formulas [*]and only assumes that
    // the formulas are _xlA1 based ( need to hook into calc work this should
    // address this )
    // [*] reason: getA1Formula method below is just a hook and just
    // returns what it gets ( e.g. doesn't convert anything )
    uno::Reference< excel::XStyle > xStyle( _xStyle );
    uno::Reference< excel::XFormatCondition > xFormatCondition;
    try
    {
        OUString sStyleName;
        if ( !xStyle.is() )
        {
            sStyleName = getStyleName();
            xStyle = mxStyles->Add(sStyleName, uno::Any() );
        }
        else
        {
            sStyleName = xStyle->getName();
        }

        std::vector< beans::PropertyValue > aPropertyValueVector;
        sheet::ConditionOperator aType = ScVbaFormatCondition::retrieveAPIType(_nType, uno::Reference< sheet::XSheetCondition >() );
        uno::Any aValue;

        if ( aType == sheet::ConditionOperator_FORMULA)
            aValue <<= sheet::ConditionOperator_FORMULA;
        else
            aValue <<= ScVbaFormatCondition::retrieveAPIOperator(_aOperator);

        beans::PropertyValue aProperty( u"Operator"_ustr, 0, aValue, beans::PropertyState_DIRECT_VALUE );
        aPropertyValueVector.push_back( aProperty );

        if ( _aFormula1.hasValue() )
        {
            beans::PropertyValue aProp( u"Formula1"_ustr, 0, uno::Any( getA1Formula( _aFormula1 ) ), beans::PropertyState_DIRECT_VALUE );
            aPropertyValueVector.push_back( aProp );
        }
        if ( _aFormula2.hasValue() )
        {
            beans::PropertyValue aProp( u"Formula2"_ustr, 0, uno::Any( getA1Formula( _aFormula2 ) ), beans::PropertyState_DIRECT_VALUE );
            aPropertyValueVector.push_back( aProp );
        }
        aProperty.Name = "StyleName";
        aProperty.Value <<= sStyleName;

        mxSheetConditionalEntries->addNew(comphelper::containerToSequence(aPropertyValueVector));
        for (sal_Int32 i = mxSheetConditionalEntries->getCount()-1; i >= 0; i--)
        {
            uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry( mxSheetConditionalEntries->getByIndex(i), uno::UNO_QUERY_THROW );
            if (xSheetConditionalEntry->getStyleName() == sStyleName)
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
    DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    return xFormatCondition;
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaFormatConditions::createEnumeration()
{
    return new EnumWrapper( m_xIndexAccess, mxRangeParent, mxContext, mxStyles, this, mxParentRangePropertySet  );
}

void
ScVbaFormatConditions::notifyRange()
{
    try
    {
        mxParentRangePropertySet->setPropertyValue(SC_UNONAME_CONDFMT, uno::Any( mxSheetConditionalEntries ));
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

OUString
ScVbaFormatConditions::getA1Formula(const css::uno::Any& _aFormula)
{
    // #TODO, #FIXME hook-in proper formula conversion detection & logic
    OUString sFormula;
    if ( !( _aFormula >>= sFormula ) )
        DebugHelper::basicexception(ERRCODE_BASIC_BAD_PARAMETER, {} );
    return sFormula;
}

OUString
ScVbaFormatConditions::getStyleName()
{
    ScVbaStyles* pStyles = mxStyles.get();
    if ( !pStyles )
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {} );
    uno::Sequence< OUString > sCellStyleNames = pStyles->getStyleNames();
    return ContainerUtilities::getUniqueName(sCellStyleNames, u"Excel_CondFormat"_ustr, u"_");
}

void
ScVbaFormatConditions::removeFormatCondition( const OUString& _sStyleName, bool _bRemoveStyle)
{
    try
    {
        sal_Int32 nElems = mxSheetConditionalEntries->getCount();
        for (sal_Int32 i = 0; i < nElems; i++)
        {
            uno::Reference< sheet::XSheetConditionalEntry > xSheetConditionalEntry( mxSheetConditionalEntries->getByIndex(i), uno::UNO_QUERY_THROW );
            if (_sStyleName == xSheetConditionalEntry->getStyleName())
            {
                mxSheetConditionalEntries->removeByIndex(i);
                if (_bRemoveStyle)
                {
                    ScVbaStyles* pStyles = mxStyles.get();
                    if ( !pStyles )
                        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
                    pStyles->Delete( _sStyleName );
                }
                return;
            }
        }
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, {});
    }
}

OUString
ScVbaFormatConditions::getServiceImplName()
{
    return u"ScVbaFormatConditions"_ustr;
}

uno::Sequence< OUString >
ScVbaFormatConditions::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.FormatConditions"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
