/*************************************************************************
 *
 *  $RCSfile: Grid.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:28:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "Grid.hxx"
#ifndef _FRM_COLUMNS_HXX
#include "Columns.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

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
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::view;

const sal_uInt16 ROWHEIGHT          =   0x0001;
const sal_uInt16 FONTTYPE           =   0x0002;
const sal_uInt16 FONTSIZE           =   0x0004;
const sal_uInt16 FONTATTRIBS        =   0x0008;
const sal_uInt16 TABSTOP            =   0x0010;
const sal_uInt16 TEXTCOLOR          =   0x0020;
const sal_uInt16 FONTDESCRIPTOR     =   0x0040;
const sal_uInt16 RECORDMARKER       =   0x0080;
const sal_uInt16 BACKGROUNDCOLOR    =   0x0100;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGridControlModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OGridControlModel(_rxFactory));
}

DBG_NAME(OGridControlModel);
//------------------------------------------------------------------
OGridControlModel::OGridControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OControlModel(_rxFactory, ::rtl::OUString())
                    ,FontControlModel( false )
                    ,OInterfaceContainer(_rxFactory, m_aMutex, ::getCppuType(static_cast<Reference<XPropertySet>*>(NULL)))
                    ,OErrorBroadcaster( OComponentHelper::rBHelper )
                    ,m_aSelectListeners(m_aMutex)
                    ,m_aResetListeners(m_aMutex)
                    ,m_aDefaultControl( FRM_SUN_CONTROL_GRIDCONTROL )
                    ,m_bEnable(sal_True)
                    ,m_bNavigation(sal_True)
                    ,m_nBorder(1)
                    ,m_bRecordMarker(sal_True)
                    ,m_bPrintable(sal_True)
                    ,m_bAlwaysShowCursor(sal_False)
                    ,m_bDisplaySynchron(sal_True)
{
    DBG_CTOR(OGridControlModel,NULL);

    m_nClassId = FormComponentType::GRIDCONTROL;
}

//------------------------------------------------------------------
OGridControlModel::OGridControlModel( const OGridControlModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
    ,OInterfaceContainer( _rxFactory, m_aMutex, ::getCppuType( static_cast<Reference<XPropertySet>*>( NULL ) ) )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,FontControlModel( _pOriginal )
    ,m_aSelectListeners( m_aMutex )
    ,m_aResetListeners( m_aMutex )
{
    DBG_CTOR(OGridControlModel,NULL);

    m_aDefaultControl = _pOriginal->m_aDefaultControl;
    m_bEnable = _pOriginal->m_bEnable;
    m_bNavigation = _pOriginal->m_bNavigation;
    m_nBorder = _pOriginal->m_nBorder;
    m_bRecordMarker = _pOriginal->m_bRecordMarker;
    m_bPrintable = _pOriginal->m_bPrintable;
    m_bAlwaysShowCursor = _pOriginal->m_bAlwaysShowCursor;
    m_bDisplaySynchron = _pOriginal->m_bDisplaySynchron;

    // clone the columns
    cloneColumns( _pOriginal );

    // TODO: clone the events?
}

//------------------------------------------------------------------
OGridControlModel::~OGridControlModel()
{
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
    m_xParentFormLoadable = Reference< XLoadable >  (xForm, UNO_QUERY);
    if (m_xParentFormLoadable.is())
        m_xParentFormLoadable->addLoadListener(this);
}
//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL OGridControlModel::getTypes(  ) throw(RuntimeException)
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

    setParent(NULL);

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
        NOTIFY_LISTENERS(m_aSelectListeners, XSelectionChangeListener, selectionChanged, aEvt);
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
    return createColumn(::internal::findPos(ColumnType, rColumnTypes));
}

//------------------------------------------------------------------------------
Reference<XPropertySet>  OGridControlModel::createColumn(sal_Int32 nTypeId) const
{
    Reference<XPropertySet>  xReturn;
    switch (nTypeId)
    {
        case TYPE_CHECKBOX:         xReturn = new CheckBoxColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_COMBOBOX:         xReturn = new ComboBoxColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_CURRENCYFIELD:    xReturn = new CurrencyFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_DATEFIELD:        xReturn = new DateFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_LISTBOX:          xReturn = new ListBoxColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_NUMERICFIELD:     xReturn = new NumericFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_PATTERNFIELD:     xReturn = new PatternFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_TEXTFIELD:        xReturn = new TextFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_TIMEFIELD:        xReturn = new TimeFieldColumn(OControlModel::m_xServiceFactory); break;
        case TYPE_FORMATTEDFIELD:   xReturn = new FormattedFieldColumn(OControlModel::m_xServiceFactory); break;
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
        NOTIFY_LISTENERS(m_aResetListeners, XResetListener, resetted, aEvt);
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
    BEGIN_DESCRIBE_AGGREGATION_PROPERTIES(33, m_xAggregateSet)
        DECL_PROP1(NAME,                ::rtl::OUString,    BOUND);
        DECL_PROP2(CLASSID,             sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP1(TAG,                 ::rtl::OUString,    BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,          BOUND);
        DECL_PROP3(TABSTOP,             sal_Bool,           BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP2(HASNAVIGATION,       sal_Bool,           BOUND, MAYBEDEFAULT);
        DECL_PROP1(ENABLED,             sal_Bool,           BOUND);
        DECL_PROP1(BORDER,              sal_Int16,          BOUND);
        DECL_PROP1(DEFAULTCONTROL,      ::rtl::OUString,    BOUND);
        DECL_PROP3(TEXTCOLOR,           sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP3(BACKGROUNDCOLOR,     sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP2(FONT,                FontDescriptor,     BOUND, MAYBEDEFAULT);
        DECL_PROP3(ROWHEIGHT,           sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,    BOUND);
        DECL_PROP1(FONT_NAME,           ::rtl::OUString,    MAYBEDEFAULT);
        DECL_PROP1(FONT_STYLENAME,      ::rtl::OUString,    MAYBEDEFAULT);
        DECL_PROP1(FONT_FAMILY,         sal_Int16,          MAYBEDEFAULT);
        DECL_PROP1(FONT_CHARSET,        sal_Int16,          MAYBEDEFAULT);
        DECL_PROP1(FONT_HEIGHT,         float,              MAYBEDEFAULT);
        DECL_PROP1(FONT_WEIGHT,         float,              MAYBEDEFAULT);
        DECL_PROP1(FONT_SLANT,          sal_Int16,          MAYBEDEFAULT);
        DECL_PROP1(FONT_UNDERLINE,      sal_Int16,          MAYBEDEFAULT);
        DECL_BOOL_PROP1(FONT_WORDLINEMODE,                  MAYBEDEFAULT);
        DECL_PROP3(TEXTLINECOLOR,       sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP2(FONTEMPHASISMARK,    sal_Int16,          BOUND, MAYBEDEFAULT);
        DECL_PROP2(FONTRELIEF,          sal_Int16,          BOUND, MAYBEDEFAULT);
        DECL_PROP1(FONT_STRIKEOUT,      sal_Int16,          MAYBEDEFAULT);
        DECL_PROP2(RECORDMARKER,        sal_Bool,           BOUND, MAYBEDEFAULT );
        DECL_PROP2(PRINTABLE,           sal_Bool,           BOUND, MAYBEDEFAULT );
        DECL_PROP4(CURSORCOLOR,         sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID , TRANSIENT);
        DECL_PROP3(ALWAYSSHOWCURSOR,    sal_Bool,           BOUND, MAYBEDEFAULT, TRANSIENT);
        DECL_PROP3(DISPLAYSYNCHRON,     sal_Bool,           BOUND, MAYBEDEFAULT, TRANSIENT);
        DECL_PROP2(HELPURL,             ::rtl::OUString,    BOUND, MAYBEDEFAULT);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
void OGridControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
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
        case PROPERTY_ID_BACKGROUNDCOLOR:
            rValue = m_aBackgroundColor;
            break;
        case PROPERTY_ID_ROWHEIGHT:
            rValue = m_aRowHeight;
            break;

        default:
            if ( isFontRelatedProperty( nHandle ) )
                FontControlModel::getFastPropertyValue( rValue, nHandle );
            else
                OControlModel::getFastPropertyValue( rValue, nHandle );
    }
}

//------------------------------------------------------------------------------
sal_Bool OGridControlModel::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                                    sal_Int32 nHandle, const Any& rValue )throw( IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
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
        case PROPERTY_ID_BACKGROUNDCOLOR:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aBackgroundColor, ::getCppuType((const sal_Int32*)NULL));
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
        default:
            if ( isFontRelatedProperty( nHandle ) )
                bModified = FontControlModel::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
            else
                bModified = OControlModel::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue);
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OGridControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw ( ::com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
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
        case PROPERTY_ID_BACKGROUNDCOLOR:
            m_aBackgroundColor = rValue;
            break;
        case PROPERTY_ID_ROWHEIGHT:
            m_aRowHeight = rValue;
            break;

        default:
            if ( isFontRelatedProperty( nHandle ) )
            {
                FontDescriptor aOldFont( getFont() );

                FontControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );

                if ( isFontAggregateProperty( nHandle ) )
                    firePropertyChange( PROPERTY_ID_FONT, makeAny( getFont() ), makeAny( aOldFont ) );
            }
            else
                OControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }

}

//XPropertyState

//------------------------------------------------------------------------------
Any OGridControlModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    Any aReturn;
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULTCONTROL:
            aReturn <<= ::rtl::OUString( STARDIV_ONE_FORM_CONTROL_GRID  );
            break;

        case PROPERTY_ID_PRINTABLE:
        case PROPERTY_ID_HASNAVIGATION:
        case PROPERTY_ID_RECORDMARKER:
        case PROPERTY_ID_DISPLAYSYNCHRON:
        case PROPERTY_ID_ENABLED:
            aReturn = makeBoolAny(sal_True);
            break;

        case PROPERTY_ID_ALWAYSSHOWCURSOR:
            aReturn = makeBoolAny(sal_False);
            break;

        case PROPERTY_ID_HELPURL:
        case PROPERTY_ID_HELPTEXT:
            aReturn <<= ::rtl::OUString();
            break;

        case PROPERTY_ID_BORDER:
            aReturn <<= (sal_Int16)1;
            break;

        case PROPERTY_ID_TABSTOP:
        case PROPERTY_ID_BACKGROUNDCOLOR:
        case PROPERTY_ID_ROWHEIGHT:
        case PROPERTY_ID_CURSORCOLOR:
            // void
            break;

        default:
            if ( isFontRelatedProperty( nHandle ) )
                aReturn = FontControlModel::getPropertyDefaultByHandle( nHandle );
            else
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
        {   // it's kind of a fake ...
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
    {   // the currently selected element was replaced
        m_xSelection.clear();
        EventObject aEvt(static_cast<XWeak*>(this));
        NOTIFY_LISTENERS(m_aSelectListeners, XSelectionChangeListener, selectionChanged, aEvt);
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
    return FRM_COMPONENT_GRID;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OGridControlModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::write(_rxOutStream);

    Reference<XMarkableStream>  xMark(_rxOutStream, UNO_QUERY);

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
    if ( getFont() != getDefaultFont() )
        nAnyMask |= FONTATTRIBS | FONTSIZE | FONTTYPE | FONTDESCRIPTOR;
    if (m_aTabStop.getValueType().getTypeClass() == TypeClass_BOOLEAN)
        nAnyMask |= TABSTOP;
    if ( hasTextColor() )
        nAnyMask |= TEXTCOLOR;
    if (m_aBackgroundColor.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= BACKGROUNDCOLOR;
    if (!m_bRecordMarker)
        nAnyMask |= RECORDMARKER;

    _rxOutStream->writeShort(nAnyMask);

    if (nAnyMask & ROWHEIGHT)
        _rxOutStream->writeLong(getINT32(m_aRowHeight));

    // old structures
    const FontDescriptor& aFont = getFont();
    if ( nAnyMask & FONTDESCRIPTOR )
    {
        // Attrib
        _rxOutStream->writeShort( VCLUnoHelper::ConvertFontWeight( aFont.Weight ) );
        _rxOutStream->writeShort( aFont.Slant );
        _rxOutStream->writeShort( aFont.Underline );
        _rxOutStream->writeShort( aFont.Strikeout );
        _rxOutStream->writeShort( sal_Int16(aFont.Orientation * 10) );
        _rxOutStream->writeBoolean( aFont.Kerning );
        _rxOutStream->writeBoolean( aFont.WordLineMode );

        // Size
        _rxOutStream->writeLong( aFont.Width );
        _rxOutStream->writeLong( aFont.Height );
        _rxOutStream->writeShort( VCLUnoHelper::ConvertFontWidth( aFont.CharacterWidth ) );

        // Type
        _rxOutStream->writeUTF( aFont.Name );
        _rxOutStream->writeUTF( aFont.StyleName );
        _rxOutStream->writeShort( aFont.Family );
        _rxOutStream->writeShort( aFont.CharSet );
        _rxOutStream->writeShort( aFont.Pitch );
    }

    if ( m_aDefaultControl == FRM_SUN_CONTROL_GRIDCONTROL )
        // for compatibility, write a sevice name which older versions understand (up to 5.1)
        _rxOutStream << STARDIV_ONE_FORM_CONTROL_GRID;
    else
        _rxOutStream << m_aDefaultControl;

    _rxOutStream->writeShort(m_nBorder);
    _rxOutStream->writeBoolean(m_bEnable);

    if (nAnyMask & TABSTOP)
        _rxOutStream->writeBoolean(getBOOL(m_aTabStop));

    _rxOutStream->writeBoolean(m_bNavigation);

    if (nAnyMask & TEXTCOLOR)
        _rxOutStream->writeLong( getTextColor() );

    // neu ab Version 6
    _rxOutStream << m_sHelpText;

    if (nAnyMask & FONTDESCRIPTOR)
        _rxOutStream << getFont();

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

    Reference<XMarkableStream>  xMark(_rxInStream, UNO_QUERY);

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

    FontDescriptor aFont( getFont() );
    if ( nAnyMask & FONTATTRIBS )
    {
        aFont.Weight = (float)VCLUnoHelper::ConvertFontWeight( _rxInStream->readShort() );

        aFont.Slant = (FontSlant)_rxInStream->readShort();
        aFont.Underline = _rxInStream->readShort();
        aFont.Strikeout = _rxInStream->readShort();
        aFont.Orientation = ( (float)_rxInStream->readShort() ) / 10;
        aFont.Kerning = _rxInStream->readBoolean();
        aFont.WordLineMode = _rxInStream->readBoolean();
    }
    if ( nAnyMask & FONTSIZE )
    {
        aFont.Width = (sal_Int16)_rxInStream->readLong();
        aFont.Height = (sal_Int16)_rxInStream->readLong();
        aFont.CharacterWidth = (float)VCLUnoHelper::ConvertFontWidth( _rxInStream->readShort() );
    }
    if ( nAnyMask & FONTTYPE )
    {
        aFont.Name = _rxInStream->readUTF();
        aFont.StyleName = _rxInStream->readUTF();
        aFont.Family = _rxInStream->readShort();
        aFont.CharSet = _rxInStream->readShort();
        aFont.Pitch = _rxInStream->readShort();
    }

    if ( nAnyMask & ( FONTATTRIBS | FONTSIZE | FONTTYPE ) )
        setFont( aFont );

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
        setTextColor( (sal_Int32)nValue );
    }

    // neu ab Version 6
    if (nVersion > 5)
        _rxInStream >> m_sHelpText;

    if (nAnyMask & FONTDESCRIPTOR)
    {
        FontDescriptor aFont;
        _rxInStream >> aFont;
        setFont( aFont );
    }

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
}   // namespace frm
//.........................................................................

