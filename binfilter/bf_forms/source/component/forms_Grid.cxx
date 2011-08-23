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

#include "Grid.hxx"
#include "Columns.hxx"

#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>

#include "services.hxx"
#include "property.hrc"

#include <comphelper/container.hxx>

#include <vcl/svapp.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/property.hxx>

#include <com/sun/star/io/XMarkableStream.hpp>

namespace binfilter {

using namespace ::com::sun::star::uno;

// TODO : find a place for this !
namespace internal
{
//------------------------------------------------------------------
// Vergleichen von Strings
static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
    NameCompare(const void* pFirst, const void* pSecond)
{
    return ((::rtl::OUString*)pFirst)->compareTo(*(::rtl::OUString*)pSecond);
}

//------------------------------------------------------------------
sal_Int32 findPos(const ::rtl::OUString& aStr, const StringSequence& rList)
{
    const ::rtl::OUString* pStrList = rList.getConstArray();
    ::rtl::OUString* pResult = (::rtl::OUString*) bsearch(&aStr, (void*)pStrList, rList.getLength(), sizeof(::rtl::OUString),
        &NameCompare);

    if (pResult)
        return (pResult - pStrList);
    else
        return -1;
}

} // namespace internal

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::view;

const sal_uInt16 ROWHEIGHT			=	0x0001;
const sal_uInt16 FONTTYPE			=	0x0002;
const sal_uInt16 FONTSIZE			=	0x0004;
const sal_uInt16 FONTATTRIBS		=	0x0008;
const sal_uInt16 TABSTOP			=	0x0010;
const sal_uInt16 TEXTCOLOR			=	0x0020;
const sal_uInt16 FONTDESCRIPTOR 	=	0x0040;
const sal_uInt16 RECORDMARKER		=	0x0080;
const sal_uInt16 BACKGROUNDCOLOR	=	0x0100;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGridControlModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OGridControlModel(_rxFactory));
}

DBG_NAME(OGridControlModel)
//------------------------------------------------------------------
OGridControlModel::OGridControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OControlModel(_rxFactory, ::rtl::OUString())
                    ,OInterfaceContainer(_rxFactory, m_aMutex, ::getCppuType(static_cast<Reference<XPropertySet>*>(NULL)))
                    ,OErrorBroadcaster( OComponentHelper::rBHelper )
                    ,m_aSelectListeners(m_aMutex)
                    ,m_aResetListeners(m_aMutex)
                    ,m_aDefaultControl(FRM_CONTROL_GRID)		// use the old control name for compytibility reasons
                    ,m_nFontEvent(0)
                    ,m_bEnable(sal_True)
                    ,m_bNavigation(sal_True)
                    ,m_nBorder(1)
                    ,m_bRecordMarker(sal_True)
                    ,m_bPrintable(sal_True)
                    ,m_bAlwaysShowCursor(sal_False)
                    ,m_bDisplaySynchron(sal_True)
                    ,m_nFontRelief(awt::FontRelief::NONE)
                    ,m_nFontEmphasis(awt::FontEmphasisMark::NONE)
{
    DBG_CTOR(OGridControlModel,NULL);

    m_nClassId = FormComponentType::GRIDCONTROL;
}

//------------------------------------------------------------------
OGridControlModel::OGridControlModel( const OGridControlModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
    ,OInterfaceContainer( _rxFactory, m_aMutex, ::getCppuType( static_cast<Reference<XPropertySet>*>( NULL ) ) )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_aSelectListeners( m_aMutex )
    ,m_aResetListeners( m_aMutex )
{
    DBG_CTOR(OGridControlModel,NULL);

    m_aDefaultControl = _pOriginal->m_aDefaultControl;
    m_nFontEvent = _pOriginal->m_nFontEvent;
    m_bEnable = _pOriginal->m_bEnable;
    m_bNavigation = _pOriginal->m_bNavigation;
    m_nBorder = _pOriginal->m_nBorder;
    m_bRecordMarker = _pOriginal->m_bRecordMarker;
    m_bPrintable = _pOriginal->m_bPrintable;
    m_bAlwaysShowCursor = _pOriginal->m_bAlwaysShowCursor;
    m_bDisplaySynchron = _pOriginal->m_bDisplaySynchron;
    m_nFontRelief = _pOriginal->m_nFontRelief;
    m_nFontEmphasis = _pOriginal->m_nFontEmphasis;

    // clone the columns
    cloneColumns( _pOriginal );

    // TODO: clone the events?
}

//------------------------------------------------------------------
OGridControlModel::~OGridControlModel()
{
    if (m_nFontEvent)
        Application::RemoveUserEvent(m_nFontEvent);

    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OGridControlModel,NULL);
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OGridControlModel )

//------------------------------------------------------------------------------
void OGridControlModel::cloneColumns( const OGridControlModel* _pOriginalContainer )
{
    try
    {
        Reference< XCloneable > xColCloneable;

        const OInterfaceArray::const_iterator pColumnStart = m_aItems.begin();
        const OInterfaceArray::const_iterator pColumnEnd = m_aItems.end();
        for ( OInterfaceArray::const_iterator pColumn = pColumnStart; pColumn != pColumnEnd; ++pColumn )
        {
            // ask the col for a factory for the clone
            xColCloneable = xColCloneable.query( *pColumn );
            DBG_ASSERT( xColCloneable.is(), "OGridControlModel::cloneColumns: column is not cloneable!" );

            if ( xColCloneable.is() )
            {
                // create a clone of the column
                Reference< XCloneable > xColClone( xColCloneable->createClone() );
                DBG_ASSERT( xColClone.is(), "OGridControlModel::cloneColumns: invalid column clone!" );

                if ( xColClone.is() )
                {
                    // insert this clone into our own container
                    insertByIndex( pColumn - pColumnStart, xColClone->queryInterface( m_aElementType ) );
                }

            }
        }
    }
    catch( const Exception& )
    {
        DBG_ERROR( "OGridControlModel::cloneColumns: caught an exception while cloning the columns!" );
    }
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence OGridControlModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlModel"));
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_GRIDCONTROL;
    return aSupported;
}

//------------------------------------------------------------------------------
Any SAL_CALL OGridControlModel::queryAggregation( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = OGridControlModel_BASE::queryInterface(_rType);

    if ( !aReturn.hasValue() )
    {
        aReturn = OControlModel::queryAggregation( _rType );
        if ( !aReturn.hasValue() )
        {
            aReturn = OInterfaceContainer::queryInterface( _rType );
            if ( !aReturn.hasValue() )
                aReturn = OErrorBroadcaster::queryInterface( _rType );
        }
    }
    return aReturn;
}

// XSQLErrorListener
//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::errorOccured( const SQLErrorEvent& _rEvent ) throw (RuntimeException)
{
    // forward the errors which happened to my columns to my own listeners
    onError( _rEvent );
}

// XChild
//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::setParent(const InterfaceRef& Parent) throw(NoSupportException, RuntimeException)
{
    if (m_xParentFormLoadable.is())
        m_xParentFormLoadable->removeLoadListener(this);

    OControlModel::setParent(Parent);

    Reference< XForm > xForm(m_xParent, UNO_QUERY);
    m_xParentFormLoadable = Reference< XLoadable >	(xForm, UNO_QUERY);
    if (m_xParentFormLoadable.is())
        m_xParentFormLoadable->addLoadListener(this);
}
//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL OGridControlModel::getTypes(	) throw(RuntimeException)
{
    return concatSequences(
        concatSequences(
            OControlModel::getTypes(),
            OInterfaceContainer::getTypes(),
            OErrorBroadcaster::getTypes()
        ),
        OGridControlModel_BASE::getTypes()
    );
}

// OComponentHelper
//------------------------------------------------------------------------------
void OGridControlModel::disposing()
{
    OControlModel::disposing();
    OErrorBroadcaster::disposing();
    OInterfaceContainer::disposing();

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aSelectListeners.disposeAndClear(aEvt);
    m_aResetListeners.disposeAndClear(aEvt);
}

// XEventListener
//------------------------------------------------------------------------------
void OGridControlModel::disposing(const EventObject& _rEvent) throw( RuntimeException )
{
    OControlModel::disposing( _rEvent );
    OInterfaceContainer::disposing( _rEvent );
}

// XSelectionSupplier
//-----------------------------------------------------------------------------
sal_Bool SAL_CALL OGridControlModel::select(const Any& rElement) throw(IllegalArgumentException, RuntimeException)
{
    Reference<XPropertySet> xSel;
    if (rElement.hasValue() && !::cppu::extractInterface(xSel, rElement))
    {
        throw IllegalArgumentException();
    }
    InterfaceRef xMe = static_cast<XWeak*>(this);

    if (xSel.is())
    {
        Reference<XChild> xAsChild(xSel, UNO_QUERY);
        if (!xAsChild.is() || (xAsChild->getParent() != xMe))
        {
            throw IllegalArgumentException();
        }
    }

    if (xSel != m_xSelection)
    {
        m_xSelection = xSel;
        EventObject aEvt(xMe);
        m_aSelectListeners.notifyEach(
            &XSelectionChangeListener::selectionChanged, aEvt);
        return sal_True;
    }
    return sal_False;
}

//-----------------------------------------------------------------------------
Any SAL_CALL OGridControlModel::getSelection() throw(RuntimeException)
{
    return makeAny(m_xSelection);
}

//-----------------------------------------------------------------------------
void OGridControlModel::addSelectionChangeListener(const Reference< XSelectionChangeListener >& _rxListener) throw( RuntimeException )
{
    m_aSelectListeners.addInterface(_rxListener);
}

//-----------------------------------------------------------------------------
void OGridControlModel::removeSelectionChangeListener(const Reference< XSelectionChangeListener >& _rxListener) throw( RuntimeException )
{
    m_aSelectListeners.removeInterface(_rxListener);
}

// XGridColumnFactory
//------------------------------------------------------------------------------
Reference<XPropertySet> SAL_CALL OGridControlModel::createColumn(const ::rtl::OUString& ColumnType) throw ( :: com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    const Sequence< ::rtl::OUString >& rColumnTypes = frm::getColumnTypes();
    return createColumn(::binfilter::internal::findPos(ColumnType, rColumnTypes));//STRIP008 	return createColumn(::internal::findPos(ColumnType, rColumnTypes));
}

//------------------------------------------------------------------------------
Reference<XPropertySet>  OGridControlModel::createColumn(sal_Int32 nTypeId) const
{
    Reference<XPropertySet>  xReturn;
    switch (nTypeId)
    {
        case TYPE_CHECKBOX: 		xReturn = new CheckBoxColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_COMBOBOX: 		xReturn = new ComboBoxColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_CURRENCYFIELD:	xReturn = new CurrencyFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_DATEFIELD:		xReturn = new DateFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_LISTBOX:			xReturn = new ListBoxColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_NUMERICFIELD: 	xReturn = new NumericFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_PATTERNFIELD: 	xReturn = new PatternFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_TEXTFIELD:		xReturn = new TextFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_TIMEFIELD:		xReturn = new TimeFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_FORMATTEDFIELD:	xReturn = new FormattedFieldColumn(OControlModel::m_xServiceFactory); break;
        default:
            DBG_ERROR("OGridControlModel::createColumn: Unknown Column");
            break;
    }
    return xReturn;
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OGridControlModel::getColumnTypes() throw ( ::com::sun::star::uno::RuntimeException)
{
    return frm::getColumnTypes();
}

// XReset
//-----------------------------------------------------------------------------
void SAL_CALL OGridControlModel::reset() throw ( ::com::sun::star::uno::RuntimeException)
{
    ::cppu::OInterfaceIteratorHelper aIter(m_aResetListeners);
    EventObject aEvt(static_cast<XWeak*>(this));
    sal_Bool bContinue = sal_True;
    while (aIter.hasMoreElements() && bContinue)
        bContinue =((XResetListener*)aIter.next())->approveReset(aEvt);

    if (bContinue)
    {
        _reset();
        m_aResetListeners.notifyEach(&XResetListener::resetted, aEvt);
    }
}

//-----------------------------------------------------------------------------
void SAL_CALL OGridControlModel::addResetListener(const Reference<XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aResetListeners.addInterface(_rxListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL OGridControlModel::removeResetListener(const Reference<XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aResetListeners.removeInterface(_rxListener);
}

//-----------------------------------------------------------------------------
void OGridControlModel::_reset()
{
    Reference<XReset> xReset;
    sal_Int32 nCount = getCount();
    for (sal_Int32 nIndex=0; nIndex < nCount; nIndex++)
    {
        getByIndex( nIndex ) >>= xReset;
        if (xReset.is())
            xReset->reset();
    }
}

// XPropertySet
//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OGridControlModel::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OGridControlModel::getInfoHelper()
{
    return *const_cast<OGridControlModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OGridControlModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    BEGIN_AGGREGATION_PROPERTY_HELPER(33, m_xAggregateSet)
        DECL_PROP1(NAME,				::rtl::OUString,	BOUND);
        DECL_PROP2(CLASSID, 			sal_Int16,			READONLY, TRANSIENT);
        DECL_PROP1(TAG, 				::rtl::OUString,	BOUND);
        DECL_PROP1(TABINDEX,			sal_Int16,			BOUND);
        DECL_PROP3(TABSTOP, 			sal_Bool,			BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP2(HASNAVIGATION,		sal_Bool,			BOUND, MAYBEDEFAULT);
        DECL_PROP1(ENABLED, 			sal_Bool,			BOUND);
        DECL_PROP1(BORDER,				sal_Int16,			BOUND);
        DECL_PROP1(DEFAULTCONTROL,		::rtl::OUString,	BOUND);
        DECL_PROP3(TEXTCOLOR,			sal_Int32,			BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP3(BACKGROUNDCOLOR, 	sal_Int32,			BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP2(FONT,				FontDescriptor, 	BOUND, MAYBEDEFAULT);
        DECL_PROP3(ROWHEIGHT,			sal_Int32,			BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(HELPTEXT,			::rtl::OUString,	BOUND);
        DECL_PROP1(FONT_NAME,			::rtl::OUString,	MAYBEDEFAULT);
        DECL_PROP1(FONT_STYLENAME,		::rtl::OUString,	MAYBEDEFAULT);
        DECL_PROP1(FONT_FAMILY, 		sal_Int16,			MAYBEDEFAULT);
        DECL_PROP1(FONT_CHARSET,		sal_Int16,			MAYBEDEFAULT);
        DECL_PROP1(FONT_HEIGHT, 		float,				MAYBEDEFAULT);
        DECL_PROP1(FONT_WEIGHT, 		float,				MAYBEDEFAULT);
        DECL_PROP1(FONT_SLANT,			sal_Int16,			MAYBEDEFAULT);
        DECL_PROP1(FONT_UNDERLINE,		sal_Int16,			MAYBEDEFAULT);
        DECL_BOOL_PROP1(FONT_WORDLINEMODE,					MAYBEDEFAULT);
        DECL_PROP3(TEXTLINECOLOR,		sal_Int32,			BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP2(FONTEMPHASISMARK,	sal_Int16,			BOUND, MAYBEDEFAULT);
        DECL_PROP2(FONTRELIEF,			sal_Int16,			BOUND, MAYBEDEFAULT);
        DECL_PROP1(FONT_STRIKEOUT,		sal_Int16,			MAYBEDEFAULT);
        DECL_PROP2(RECORDMARKER,		sal_Bool,			BOUND, MAYBEDEFAULT );
        DECL_PROP2(PRINTABLE,			sal_Bool,			BOUND, MAYBEDEFAULT );
        DECL_PROP4(CURSORCOLOR, 		sal_Int32,			BOUND, MAYBEDEFAULT, MAYBEVOID , TRANSIENT);
        DECL_PROP3(ALWAYSSHOWCURSOR,	sal_Bool,			BOUND, MAYBEDEFAULT, TRANSIENT);
        DECL_PROP3(DISPLAYSYNCHRON, 	sal_Bool,			BOUND, MAYBEDEFAULT, TRANSIENT);
        DECL_PROP2(HELPURL, 			::rtl::OUString,	BOUND, MAYBEDEFAULT);
    END_AGGREGATION_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
void OGridControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_TEXTLINECOLOR:
            rValue = m_aTextLineColor;
            break;
        case PROPERTY_ID_FONTEMPHASISMARK:
            rValue <<= m_nFontEmphasis;
            break;
        case PROPERTY_ID_FONTRELIEF:
            rValue <<= m_nFontRelief;
            break;
        case PROPERTY_ID_HELPTEXT:
            rValue <<= m_sHelpText;
            break;
        case PROPERTY_ID_HELPURL:
            rValue <<= m_sHelpURL;
            break;
        case PROPERTY_ID_DISPLAYSYNCHRON:
            setBOOL(rValue, m_bDisplaySynchron);
            break;
        case PROPERTY_ID_ALWAYSSHOWCURSOR:
            setBOOL(rValue, m_bAlwaysShowCursor);
            break;
        case PROPERTY_ID_CURSORCOLOR:
            rValue = m_aCursorColor;
            break;
        case PROPERTY_ID_PRINTABLE:
            setBOOL(rValue, m_bPrintable);
            break;
        case PROPERTY_ID_TABSTOP:
            rValue = m_aTabStop;
            break;
        case PROPERTY_ID_HASNAVIGATION:
            setBOOL(rValue, m_bNavigation);
            break;
        case PROPERTY_ID_RECORDMARKER:
            setBOOL(rValue, m_bRecordMarker);
            break;
        case PROPERTY_ID_ENABLED:
            setBOOL(rValue, m_bEnable);
            break;
        case PROPERTY_ID_BORDER:
            rValue <<= (sal_Int16)m_nBorder;
            break;
        case PROPERTY_ID_DEFAULTCONTROL:
            rValue <<= m_aDefaultControl;
            break;
        case PROPERTY_ID_TEXTCOLOR:
            rValue = m_aTextColor;
            break;
        case PROPERTY_ID_BACKGROUNDCOLOR:
            rValue = m_aBackgroundColor;
            break;
        case PROPERTY_ID_FONT:
            rValue = makeAny(m_aFont);
            break;
        case PROPERTY_ID_ROWHEIGHT:
            rValue = m_aRowHeight;
            break;
        case PROPERTY_ID_FONT_NAME:
            rValue <<= m_aFont.Name;
            break;
        case PROPERTY_ID_FONT_STYLENAME:
            rValue <<= m_aFont.StyleName;
            break;
        case PROPERTY_ID_FONT_FAMILY:
            rValue <<= (sal_Int16)m_aFont.Family;
            break;
        case PROPERTY_ID_FONT_CHARSET:
            rValue <<= (sal_Int16)m_aFont.CharSet;
            break;
        case PROPERTY_ID_FONT_HEIGHT:
            rValue <<= (float)(m_aFont.Height);
            break;
        case PROPERTY_ID_FONT_WEIGHT:
            rValue <<= (float)m_aFont.Weight;
            break;
        case PROPERTY_ID_FONT_SLANT:
            rValue = makeAny(m_aFont.Slant);
            break;
        case PROPERTY_ID_FONT_UNDERLINE:
            rValue <<= (sal_Int16)m_aFont.Underline;
            break;
        case PROPERTY_ID_FONT_STRIKEOUT:
            rValue <<= (sal_Int16)m_aFont.Strikeout;
            break;
        case PROPERTY_ID_FONT_WORDLINEMODE:
            rValue = ::cppu::bool2any(m_aFont.WordLineMode);
            break;
        default:
            OControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OGridControlModel::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                                    sal_Int32 nHandle, const Any& rValue )throw( IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_TEXTLINECOLOR:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTextLineColor, ::getCppuType((const sal_Int32*)NULL));
            break;
        case PROPERTY_ID_FONTEMPHASISMARK:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nFontEmphasis);
            break;
        case PROPERTY_ID_FONTRELIEF:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nFontRelief);
            break;
        case PROPERTY_ID_HELPTEXT:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sHelpText);
            break;
        case PROPERTY_ID_HELPURL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sHelpURL);
            break;
        case PROPERTY_ID_DISPLAYSYNCHRON:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bDisplaySynchron);
            break;
        case PROPERTY_ID_ALWAYSSHOWCURSOR:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAlwaysShowCursor);
            break;
        case PROPERTY_ID_CURSORCOLOR:
            if (!rValue.hasValue() || !m_aCursorColor.hasValue())
            {
                if (rValue.hasValue() && (TypeClass_LONG != rValue.getValueType().getTypeClass()))
                {
                    throw IllegalArgumentException();
                }
                rOldValue = m_aCursorColor;
                rConvertedValue = rValue;
                bModified = (rOldValue.getValue() != rConvertedValue.getValue());
            }
            else
                bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, getINT32(m_aCursorColor));
            break;
        case PROPERTY_ID_PRINTABLE:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bPrintable);
            break;
        case PROPERTY_ID_TABSTOP:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTabStop, ::getBooleanCppuType());
            break;
        case PROPERTY_ID_HASNAVIGATION:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bNavigation);
            break;
        case PROPERTY_ID_RECORDMARKER:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bRecordMarker);
            break;
        case PROPERTY_ID_ENABLED:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bEnable);
            break;
        case PROPERTY_ID_BORDER:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nBorder);
            break;
        case PROPERTY_ID_DEFAULTCONTROL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDefaultControl);
            break;
        case PROPERTY_ID_TEXTCOLOR:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTextColor, ::getCppuType((const sal_Int32*)NULL));
            break;
        case PROPERTY_ID_BACKGROUNDCOLOR:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aBackgroundColor, ::getCppuType((const sal_Int32*)NULL));
            break;
        case PROPERTY_ID_FONT:
            {
                com::sun::star::uno::Any aAny(makeAny(m_aFont));
                bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, aAny, ::getCppuType((const FontDescriptor*)NULL));
            }
            break;
        case PROPERTY_ID_ROWHEIGHT:
            {
                bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aRowHeight, ::getCppuType((const sal_Int32*)NULL));

                sal_Int32 nNewVal( 0 );
                if ( ( rConvertedValue >>= nNewVal ) && ( nNewVal <= 0 ) )
                {
                    rConvertedValue.clear();
                    bModified = m_aRowHeight.hasValue();
                }
            }
            break;
        case PROPERTY_ID_FONT_NAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFont.Name);
            break;
        case PROPERTY_ID_FONT_STYLENAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFont.StyleName);
            break;
        case PROPERTY_ID_FONT_FAMILY:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, (sal_Int16)m_aFont.Family);
            break;
        case PROPERTY_ID_FONT_CHARSET:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, (sal_Int16)m_aFont.CharSet);
            break;
        case PROPERTY_ID_FONT_HEIGHT:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, float(m_aFont.Height));
            break;
        case PROPERTY_ID_FONT_WEIGHT:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFont.Weight);
            break;
        case PROPERTY_ID_FONT_SLANT:
            bModified = tryPropertyValueEnum(rConvertedValue, rOldValue, rValue, m_aFont.Slant);
            break;
        case PROPERTY_ID_FONT_UNDERLINE:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, (sal_Int16)m_aFont.Underline);
            break;
        case PROPERTY_ID_FONT_STRIKEOUT:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, (sal_Int16)m_aFont.Strikeout);
            break;
        case PROPERTY_ID_FONT_WORDLINEMODE:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, (sal_Bool)m_aFont.WordLineMode);
            break;
        default:
            bModified = OControlModel::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue);
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OGridControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw ( ::com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_TEXTLINECOLOR:
            m_aTextLineColor = rValue;
            break;
        case PROPERTY_ID_FONTEMPHASISMARK:
            rValue >>= m_nFontEmphasis;
            break;
        case PROPERTY_ID_FONTRELIEF:
            rValue >>= m_nFontRelief;
            break;
        case PROPERTY_ID_HELPTEXT:
            rValue >>= m_sHelpText;
            break;
        case PROPERTY_ID_HELPURL:
            rValue >>= m_sHelpURL;
            break;
        case PROPERTY_ID_DISPLAYSYNCHRON:
            m_bDisplaySynchron = getBOOL(rValue);
            break;
        case PROPERTY_ID_ALWAYSSHOWCURSOR:
            m_bAlwaysShowCursor = getBOOL(rValue);
            break;
        case PROPERTY_ID_CURSORCOLOR:
            m_aCursorColor = rValue;
            break;
        case PROPERTY_ID_PRINTABLE:
            m_bPrintable = getBOOL(rValue);
            break;
        case PROPERTY_ID_TABSTOP:
            m_aTabStop = rValue;
            break;
        case PROPERTY_ID_HASNAVIGATION:
            m_bNavigation = getBOOL(rValue);
            break;
        case PROPERTY_ID_ENABLED:
            m_bEnable = getBOOL(rValue);
            break;
        case PROPERTY_ID_RECORDMARKER:
            m_bRecordMarker = getBOOL(rValue);
            break;
        case PROPERTY_ID_BORDER:
            rValue >>= m_nBorder;
            break;
        case PROPERTY_ID_DEFAULTCONTROL:
            rValue >>= m_aDefaultControl;
            break;
        case PROPERTY_ID_TEXTCOLOR:
            m_aTextColor = rValue;
            break;
        case PROPERTY_ID_BACKGROUNDCOLOR:
            m_aBackgroundColor = rValue;
            break;
        case PROPERTY_ID_FONT:
            rValue >>= m_aFont;
            break;
        case PROPERTY_ID_FONT_NAME:
            rValue >>= m_aFont.Name;
            break;
        case PROPERTY_ID_FONT_STYLENAME:
            rValue >>= m_aFont.StyleName;
            break;
        case PROPERTY_ID_FONT_FAMILY:
            m_aFont.Family = getINT16(rValue);
            break;
        case PROPERTY_ID_FONT_CHARSET:
            m_aFont.CharSet = getINT16(rValue);
            break;
        case PROPERTY_ID_FONT_HEIGHT:
            m_aFont.Height = sal_Int16(getFloat(rValue));
            break;
        case PROPERTY_ID_FONT_WEIGHT:
            rValue >>= m_aFont.Weight;
            break;
        case PROPERTY_ID_FONT_SLANT:
            rValue >>= m_aFont.Slant;
            break;
        case PROPERTY_ID_FONT_UNDERLINE:
            m_aFont.Underline = getINT16(rValue);
            break;
        case PROPERTY_ID_FONT_STRIKEOUT:
            m_aFont.Strikeout = getINT16(rValue);
            break;
        case PROPERTY_ID_FONT_WORDLINEMODE:
            m_aFont.WordLineMode = getBOOL(rValue);
            break;
        case PROPERTY_ID_ROWHEIGHT:
            m_aRowHeight = rValue;
            break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }

    switch (nHandle)
    {
        case PROPERTY_ID_FONT:
            if (m_nFontEvent)
            {
                Application::RemoveUserEvent(m_nFontEvent);
                m_nFontEvent = 0;
            }
            break;
        case PROPERTY_ID_FONT_NAME:
        case PROPERTY_ID_FONT_STYLENAME:
        case PROPERTY_ID_FONT_FAMILY:
        case PROPERTY_ID_FONT_CHARSET:
        case PROPERTY_ID_FONT_HEIGHT:
        case PROPERTY_ID_FONT_WEIGHT:
        case PROPERTY_ID_FONT_SLANT:
        case PROPERTY_ID_FONT_UNDERLINE:
        case PROPERTY_ID_FONT_STRIKEOUT:
        case PROPERTY_ID_FONT_WORDLINEMODE:
            if (m_nFontEvent)
                Application::RemoveUserEvent(m_nFontEvent);
            m_nFontEvent = Application::PostUserEvent( LINK(this, OGridControlModel, OnFontChanged) );
            break;
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( OGridControlModel, OnFontChanged, void*, EMPTYARG )
{
    Any aOldVal; aOldVal <<= m_aOldFont;
    Any aNewVal; aNewVal <<= m_aFont;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_nFontEvent = 0;
        m_aOldFont = m_aFont;
    }
    firePropertyChange(PROPERTY_ID_FONT, aNewVal, aOldVal);
    return 1;
}

//XPropertyState

//------------------------------------------------------------------------------
Any OGridControlModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    Any aReturn;
    switch (nHandle)
    {
        case PROPERTY_ID_FONT:
            aReturn <<= ::comphelper::getDefaultFont();
            break;

        case PROPERTY_ID_DEFAULTCONTROL:
            aReturn <<= ::rtl::OUString( FRM_CONTROL_GRID  );
            break;

        case PROPERTY_ID_FONTEMPHASISMARK:
            aReturn <<= awt::FontEmphasisMark::NONE;
            break;

        case PROPERTY_ID_FONTRELIEF:
            aReturn <<= awt::FontRelief::NONE;
            break;

        case PROPERTY_ID_PRINTABLE:
        case PROPERTY_ID_HASNAVIGATION:
        case PROPERTY_ID_RECORDMARKER:
        case PROPERTY_ID_DISPLAYSYNCHRON:
        case PROPERTY_ID_ENABLED:
            aReturn = makeBoolAny(sal_True);
            break;

        case PROPERTY_ID_ALWAYSSHOWCURSOR:
        case PROPERTY_ID_FONT_WORDLINEMODE:
            aReturn = makeBoolAny(sal_False);
            break;

        case PROPERTY_ID_HELPURL:
        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_FONT_NAME:
        case PROPERTY_ID_FONT_STYLENAME:
            aReturn <<= ::rtl::OUString();
            break;

        case PROPERTY_ID_BORDER:
        case PROPERTY_ID_FONT_FAMILY:
        case PROPERTY_ID_FONT_CHARSET:
        case PROPERTY_ID_FONT_SLANT:
        case PROPERTY_ID_FONT_UNDERLINE:
        case PROPERTY_ID_FONT_STRIKEOUT:
            aReturn <<= (sal_Int16)1;
            break;

        case PROPERTY_ID_FONT_HEIGHT:
        case PROPERTY_ID_FONT_WEIGHT:
            aReturn <<= (float)0;
            break;

        case PROPERTY_ID_TABSTOP:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_BACKGROUNDCOLOR:
        case PROPERTY_ID_ROWHEIGHT:
        case PROPERTY_ID_CURSORCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
            // void
            break;

        default:
            aReturn = OControlModel::getPropertyDefaultByHandle(nHandle);
    }
    return aReturn;
}

// XLoadListener
//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::loaded(const EventObject& rEvent) throw(RuntimeException)
{
    Reference<XLoadListener>  xListener;
    sal_Int32 nCount = getCount();
    for (sal_Int32 nIndex=0; nIndex < nCount; ++nIndex)
    {
        getByIndex(nIndex) >>= xListener;
        if (xListener.is())
            xListener->loaded(rEvent);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::unloaded(const EventObject& rEvent) throw(RuntimeException)
{
    Reference<XLoadListener>  xListener;
    sal_Int32 nCount = getCount();
    for (sal_Int32 nIndex=0; nIndex < nCount; nIndex++)
    {
        getByIndex(nIndex) >>= xListener;
        if (xListener.is())
            xListener->unloaded(rEvent);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::reloading(const EventObject& rEvent) throw(RuntimeException)
{
    Reference<XLoadListener>  xListener;
    sal_Int32 nCount = getCount();
    for (sal_Int32 nIndex=0; nIndex < nCount; nIndex++)
    {
        getByIndex(nIndex) >>= xListener;
        if (xListener.is())
            xListener->reloading(rEvent);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::unloading(const EventObject& rEvent) throw(RuntimeException)
{
    Reference<XLoadListener>  xListener;
    sal_Int32 nCount = getCount();
    for (sal_Int32 nIndex=0; nIndex < nCount; nIndex++)
    {
        getByIndex(nIndex) >>= xListener;
        if (xListener.is())
            xListener->unloading(rEvent);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OGridControlModel::reloaded(const EventObject& rEvent) throw(RuntimeException)
{
    Reference<XLoadListener>  xListener;
    sal_Int32 nCount = getCount();
    for (sal_Int32 nIndex=0; nIndex < nCount; nIndex++)
    {
        getByIndex(nIndex) >>= xListener;
        if (xListener.is())
            xListener->reloaded(rEvent);
    }
}

//------------------------------------------------------------------------------
OGridColumn* OGridControlModel::getColumnImplementation(const InterfaceRef& _rxIFace) const
{
    OGridColumn* pImplementation = NULL;
    Reference< XUnoTunnel > xUnoTunnel( _rxIFace, UNO_QUERY );
    if ( xUnoTunnel.is() )
        pImplementation = reinterpret_cast<OGridColumn*>(xUnoTunnel->getSomething(OGridColumn::getUnoTunnelImplementationId()));

    return pImplementation;
}

//------------------------------------------------------------------------------
void OGridControlModel::gotColumn(const Reference< XInterface >& _rxColumn)
{
    // if our form is already loaded, tell the column
    // 18.05.2001 - 86558 - frank.schoenheit@germany.sun.com
    if (m_xParentFormLoadable.is() && m_xParentFormLoadable->isLoaded())
    {
        Reference< XLoadListener > xColumnLoadListener(_rxColumn, UNO_QUERY);
        if (xColumnLoadListener.is())
        {	// it's kind of a fake ...
            EventObject aFakedLoadEvent;
            aFakedLoadEvent.Source = m_xParentFormLoadable;
            xColumnLoadListener->loaded(aFakedLoadEvent);
        }
    }
}

//------------------------------------------------------------------------------
void OGridControlModel::lostColumn(const Reference< XInterface >& _rxColumn)
{
    if (Reference<XInterface>(m_xSelection, UNO_QUERY).get() == Reference<XInterface>(_rxColumn, UNO_QUERY).get())
    {	// the currently selected element was replaced
        m_xSelection.clear();
        EventObject aEvt(static_cast<XWeak*>(this));
        m_aSelectListeners.notifyEach(
            &XSelectionChangeListener::selectionChanged, aEvt);
    }
}

//------------------------------------------------------------------------------
void OGridControlModel::implRemoved(const InterfaceRef& _rxObject)
{
    OInterfaceContainer::implRemoved(_rxObject);

    Reference< XSQLErrorBroadcaster > xBroadcaster( _rxObject, UNO_QUERY );
    if ( xBroadcaster.is() )
        xBroadcaster->removeSQLErrorListener( this );

    lostColumn(_rxObject);
}

//------------------------------------------------------------------------------
void OGridControlModel::implInserted( const ElementDescription* _pElement )
{
    OInterfaceContainer::implInserted( _pElement );

    Reference< XSQLErrorBroadcaster > xBroadcaster( _pElement->xInterface, UNO_QUERY );
    if ( xBroadcaster.is() )
        xBroadcaster->addSQLErrorListener( this );

    gotColumn( _pElement->xInterface );
}

//------------------------------------------------------------------------------
void OGridControlModel::implReplaced( const InterfaceRef& _rxReplacedObject, const ElementDescription* _pElement )
{
    OInterfaceContainer::implReplaced( _rxReplacedObject, _pElement );
    lostColumn( _rxReplacedObject );
    gotColumn( _pElement->xInterface );
}

//------------------------------------------------------------------------------
ElementDescription* OGridControlModel::createElementMetaData( )
{
    return new ColumnDescription;
}

//------------------------------------------------------------------------------
void OGridControlModel::approveNewElement( const Reference< XPropertySet >& _rxObject, ElementDescription* _pElement )
{
    OGridColumn* pCol = getColumnImplementation( _rxObject );
    if ( !pCol )
        throw IllegalArgumentException();

    OInterfaceContainer::approveNewElement( _rxObject, _pElement );

    // if we're here, the object passed all tests
    if ( _pElement )
        static_cast< ColumnDescription* >( _pElement )->pColumn = pCol;
}

// XPersistObject
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGridControlModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_GRID;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OGridControlModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::write(_rxOutStream);

    Reference<XMarkableStream>	xMark(_rxOutStream, UNO_QUERY);

    // 1. Version
    _rxOutStream->writeShort(0x0008);

    // 2. Columns
    sal_Int32 nLen = getCount();
    _rxOutStream->writeLong(nLen);
    OGridColumn* pCol;
    for (sal_Int32 i = 0; i < nLen; i++)
    {
        // zuerst den Servicenamen fuer das darunterliegende Model
        pCol = getColumnImplementation(m_aItems[i]);
        DBG_ASSERT(pCol != NULL, "OGridControlModel::write : such items should never reach it into my container !");

        _rxOutStream << pCol->getModelName();

        // dann das Object selbst
        sal_Int32 nMark = xMark->createMark();
        sal_Int32 nObjLen = 0;
        _rxOutStream->writeLong(nObjLen);

        // schreiben der Col
        pCol->write(_rxOutStream);

        // feststellen der Laenge
        nObjLen = xMark->offsetToMark(nMark) - 4;
        xMark->jumpToMark(nMark);
        _rxOutStream->writeLong(nObjLen);
        xMark->jumpToFurthest();
        xMark->deleteMark(nMark);
    }

    // 3. Events
    writeEvents(_rxOutStream);

    // 4. Attribute
    // Maskierung fuer alle any Typen
    sal_uInt16 nAnyMask = 0;
    if (m_aRowHeight.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= ROWHEIGHT;
    if (!(m_aFont == getDefaultFont())) 	// have no !=, only a ==
        nAnyMask |= FONTATTRIBS | FONTSIZE | FONTTYPE | FONTDESCRIPTOR;
    if (m_aTabStop.getValueType().getTypeClass() == TypeClass_BOOLEAN)
        nAnyMask |= TABSTOP;
    if (m_aTextColor.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= TEXTCOLOR;
    if (m_aBackgroundColor.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= BACKGROUNDCOLOR;
    if (!m_bRecordMarker)
        nAnyMask |= RECORDMARKER;

    _rxOutStream->writeShort(nAnyMask);

    if (nAnyMask & ROWHEIGHT)
        _rxOutStream->writeLong(getINT32(m_aRowHeight));

    // alte Structuren
    if (nAnyMask & FONTDESCRIPTOR)
    {
        // Attrib
        _rxOutStream->writeShort( VCLUnoHelper::ConvertFontWeight( m_aFont.Weight ) );
        _rxOutStream->writeShort( m_aFont.Slant );
        _rxOutStream->writeShort( m_aFont.Underline );
        _rxOutStream->writeShort( m_aFont.Strikeout );
        _rxOutStream->writeShort( sal_Int16(m_aFont.Orientation * 10) );
        _rxOutStream->writeBoolean( m_aFont.Kerning );
        _rxOutStream->writeBoolean( m_aFont.WordLineMode );

        // Size
        _rxOutStream->writeLong( m_aFont.Width );
        _rxOutStream->writeLong( m_aFont.Height );
        _rxOutStream->writeShort( VCLUnoHelper::ConvertFontWidth( m_aFont.CharacterWidth ) );

        // Type
        _rxOutStream->writeUTF( m_aFont.Name );
        _rxOutStream->writeUTF( m_aFont.StyleName );
        _rxOutStream->writeShort( m_aFont.Family );
        _rxOutStream->writeShort( m_aFont.CharSet );
        _rxOutStream->writeShort( m_aFont.Pitch );
    }

    _rxOutStream << m_aDefaultControl;
    _rxOutStream->writeShort(m_nBorder);
    _rxOutStream->writeBoolean(m_bEnable);

    if (nAnyMask & TABSTOP)
        _rxOutStream->writeBoolean(getBOOL(m_aTabStop));

    _rxOutStream->writeBoolean(m_bNavigation);

    if (nAnyMask & TEXTCOLOR)
        _rxOutStream->writeLong(getINT32(m_aTextColor));

    // neu ab Version 6
    _rxOutStream << m_sHelpText;

    if (nAnyMask & FONTDESCRIPTOR)
        _rxOutStream << m_aFont;

    if (nAnyMask & RECORDMARKER)
        _rxOutStream->writeBoolean(m_bRecordMarker);

    // neu ab Version 7
    _rxOutStream->writeBoolean(m_bPrintable);

    // new since 8
    if (nAnyMask & BACKGROUNDCOLOR)
        _rxOutStream->writeLong(getINT32(m_aBackgroundColor));
}

//------------------------------------------------------------------------------
void OGridControlModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::read(_rxInStream);

    Reference<XMarkableStream>	xMark(_rxInStream, UNO_QUERY);

    // 1. Version
    sal_Int16 nVersion = _rxInStream->readShort();

    // 2. Einlesen der Spalten
    sal_Int32 nLen = _rxInStream->readLong();
    if (nLen)
    {
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            // Lesen des Modelnamen

            ::rtl::OUString sModelName;
            _rxInStream >> sModelName;

            Reference<XPropertySet>  xCol(createColumn(getColumnTypeByModelName(sModelName)));
            DBG_ASSERT(xCol.is(), "OGridControlModel::read : unknown column type !");
            sal_Int32 nObjLen = _rxInStream->readLong();
            if (nObjLen)
            {
                sal_Int32 nMark = xMark->createMark();
                if (xCol.is())
                {
                    OGridColumn* pCol = getColumnImplementation(xCol);
                    pCol->read(_rxInStream);
                }
                xMark->jumpToMark(nMark);
                _rxInStream->skipBytes(nObjLen);
                xMark->deleteMark(nMark);
            }

            if ( xCol.is() )
                implInsert( i, xCol, sal_False, NULL, sal_False );
        }
    }

    // In der Basisimplementierung werden die Events nur gelesen, Elemente im Container existieren
    // da aber vor TF_ONE fuer das GridControl immer Events geschrieben wurden, muessen sie auch immer
    // mit gelesen werden
    sal_Int32 nObjLen = _rxInStream->readLong();
    if (nObjLen)
    {
        sal_Int32 nMark = xMark->createMark();
        Reference<XPersistObject>  xObj(m_xEventAttacher, UNO_QUERY);
        if (xObj.is())
            xObj->read(_rxInStream);
        xMark->jumpToMark(nMark);
        _rxInStream->skipBytes(nObjLen);
        xMark->deleteMark(nMark);
    }

    // Attachement lesen
    for (sal_Int32 i = 0; i < nLen; i++)
    {
        InterfaceRef  xIfc(m_aItems[i], UNO_QUERY);
        Reference<XPropertySet>  xSet(xIfc, UNO_QUERY);
        Any aHelper;
        aHelper <<= xSet;
        m_xEventAttacher->attach( i, xIfc, aHelper );
    }

    // 4. Einlesen der Attribute
    if (nVersion == 1)
        return;

    // Maskierung fuer any
    sal_uInt16 nAnyMask = _rxInStream->readShort();

    if (nAnyMask & ROWHEIGHT)
    {
        sal_Int32 nValue = _rxInStream->readLong();
        m_aRowHeight <<= (sal_Int32)nValue;
    }

    if (nAnyMask & FONTATTRIBS)
    {
        m_aFont.Weight = (float)VCLUnoHelper::ConvertFontWeight( _rxInStream->readShort() );

        m_aFont.Slant = (FontSlant)_rxInStream->readShort();
        m_aFont.Underline = _rxInStream->readShort();
        m_aFont.Strikeout = _rxInStream->readShort();
        m_aFont.Orientation = ( (float)_rxInStream->readShort() ) / 10;
        m_aFont.Kerning = _rxInStream->readBoolean();
        m_aFont.WordLineMode = _rxInStream->readBoolean();
    }
    if (nAnyMask & FONTSIZE)
    {
        m_aFont.Width = (sal_Int16)_rxInStream->readLong();
        m_aFont.Height = (sal_Int16)_rxInStream->readLong();
        m_aFont.CharacterWidth = (float)VCLUnoHelper::ConvertFontWidth( _rxInStream->readShort() );
    }
    if (nAnyMask & FONTTYPE)
    {
        m_aFont.Name = _rxInStream->readUTF();
        m_aFont.StyleName = _rxInStream->readUTF();
        m_aFont.Family = _rxInStream->readShort();
        m_aFont.CharSet = _rxInStream->readShort();
        m_aFont.Pitch = _rxInStream->readShort();
    }

    // Name
    _rxInStream >> m_aDefaultControl;
    m_nBorder = _rxInStream->readShort();
    m_bEnable = _rxInStream->readBoolean();

    if (nAnyMask & TABSTOP)
    {
        m_aTabStop = makeBoolAny(_rxInStream->readBoolean());
    }

    if (nVersion > 3)
        m_bNavigation = _rxInStream->readBoolean();

    if (nAnyMask & TEXTCOLOR)
    {
        sal_Int32 nValue = _rxInStream->readLong();
        m_aTextColor <<= (sal_Int32)nValue;
    }

    // neu ab Version 6
    if (nVersion > 5)
        _rxInStream >> m_sHelpText;

    if (nAnyMask & FONTDESCRIPTOR)
    {
        _rxInStream >> m_aFont;
    }

    m_aOldFont = m_aFont;

    if (nAnyMask & RECORDMARKER)
        m_bRecordMarker = _rxInStream->readBoolean();

    // neu ab Version 7
    if (nVersion > 6)
        m_bPrintable = _rxInStream->readBoolean();

    if (nAnyMask & BACKGROUNDCOLOR)
    {
        sal_Int32 nValue = _rxInStream->readLong();
        m_aBackgroundColor <<= (sal_Int32)nValue;
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
