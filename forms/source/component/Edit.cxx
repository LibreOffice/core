/*************************************************************************
 *
 *  $RCSfile: Edit.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:56:58 $
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

#ifndef _FORMS_EDIT_HXX_
#include "Edit.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMIT_HPP_
#include <com/sun/star/form/XSubmit.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_VCLRSC_HXX
#include <tools/vclrsc.hxx>
#endif

#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

using namespace dbtools;

//.........................................................................
namespace frm
{
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
using namespace ::drafts::com::sun::star::form;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OEditControl_CreateInstance(const Reference< XMultiServiceFactory > & _rxFactory)
{
    return *(new OEditControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OEditControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        aTypes = concatSequences(OBoundControl::_getTypes(), OEditControl_BASE::getTypes());
    }
    return aTypes;
}

//------------------------------------------------------------------------------
Any SAL_CALL OEditControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OEditControl_BASE::queryInterface(_rType);

    return aReturn;
}

DBG_NAME(OEditControl);
//------------------------------------------------------------------------------
OEditControl::OEditControl(const Reference<XMultiServiceFactory>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_EDIT)
               ,m_aChangeListeners(m_aMutex)
               ,m_nKeyEvent( 0 )
{
    DBG_CTOR(OEditControl,NULL);

    increment(m_refCount);
    {   // als FocusListener anmelden
        Reference<XWindow>  xComp;
        if (query_aggregation(m_xAggregate, xComp))
        {
            xComp->addFocusListener(this);
            xComp->addKeyListener(this);
        }
    }
    // Refcount wieder bei 2 fuer beide Listener
    sal_Int32 n = decrement(m_refCount);
}

//------------------------------------------------------------------------------
OEditControl::~OEditControl()
{
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );

    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OEditControl,NULL);
}

// XChangeBroadcaster
//------------------------------------------------------------------------------
void OEditControl::addChangeListener(const Reference<XChangeListener>& l) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aChangeListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void OEditControl::removeChangeListener(const Reference<XChangeListener>& l) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aChangeListeners.removeInterface( l );
}

// OComponentHelper
//------------------------------------------------------------------------------
void OEditControl::disposing()
{
    OBoundControl::disposing();

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aChangeListeners.disposeAndClear(aEvt);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OEditControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_TEXTFIELD;
    return aSupported;
}

// XEventListener
//------------------------------------------------------------------------------
void OEditControl::disposing(const EventObject& Source) throw( RuntimeException )
{
    OBoundControl::disposing(Source);
}

// XFocusListener
//------------------------------------------------------------------------------
void OEditControl::focusGained( const FocusEvent& e ) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= m_aHtmlChangeValue;
}

//------------------------------------------------------------------------------
void OEditControl::focusLost( const FocusEvent& e ) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
    {
        ::rtl::OUString sNewHtmlChangeValue;
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= sNewHtmlChangeValue;
        if( sNewHtmlChangeValue != m_aHtmlChangeValue )
        {
            EventObject aEvt;
            aEvt.Source = *this;
            NOTIFY_LISTENERS(m_aChangeListeners, XChangeListener, changed, aEvt);
        }
    }
}

// XKeyListener
//------------------------------------------------------------------------------
void OEditControl::keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;

    // Steht das Control in einem Formular mit einer Submit-URL?
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if( !xSet.is() )
        return;

    // nicht fuer multiline edits
    Any aTmp( xSet->getPropertyValue(PROPERTY_MULTILINE));
    if ((aTmp.getValueType().equals(::getBooleanCppuType())) && getBOOL(aTmp))
        return;

    Reference<XFormComponent>  xFComp(xSet, UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;

    Reference<XPropertySet>  xFormSet(xParent, UNO_QUERY);
    if( !xFormSet.is() )
        return;

    aTmp = xFormSet->getPropertyValue( PROPERTY_TARGET_URL );
    if (!aTmp.getValueType().equals(::getCppuType((const ::rtl::OUString*)NULL)) ||
        !getString(aTmp).getLength() )
        return;

    Reference<XIndexAccess>  xElements(xParent, UNO_QUERY);
    sal_Int32 nCount = xElements->getCount();
    if( nCount > 1 )
    {
        Reference<XPropertySet>  xFCSet;
        for( sal_Int32 nIndex=0; nIndex < nCount; nIndex++ )
        {
            //  Any aElement(xElements->getByIndex(nIndex));
            xElements->getByIndex(nIndex) >>= xFCSet;
            OSL_ENSURE(xFCSet.is(),"OEditControl::keyPressed: No XPropertySet!");

            if (hasProperty(PROPERTY_CLASSID, xFCSet) &&
                getINT16(xFCSet->getPropertyValue(PROPERTY_CLASSID)) == FormComponentType::TEXTFIELD)
            {
                // Noch ein weiteres Edit gefunden ==> dann nicht submitten
                if (xFCSet != xSet)
                    return;
            }
        }
    }

    // Da wir noch im Haender stehen, submit asynchron ausloesen
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OEditControl,OnKeyPressed) );
}

//------------------------------------------------------------------------------
void OEditControl::keyReleased(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
IMPL_LINK(OEditControl, OnKeyPressed, void*, EMPTYARG)
{
    m_nKeyEvent = 0;

    Reference<XFormComponent>  xFComp(getModel(), UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit( Reference<XControl>(), MouseEvent() );
    return 0L;
}

/*************************************************************************/
//------------------------------------------------------------------
InterfaceRef SAL_CALL OEditModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OEditModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OEditModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}


DBG_NAME(OEditModel);
//------------------------------------------------------------------
OEditModel::OEditModel(const Reference<XMultiServiceFactory>& _rxFactory)
             :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_EDIT, FRM_CONTROL_EDIT, sal_True )
                                    // use the old control name for compytibility reasons
    ,m_bMaxTextLenModified(sal_False)
    ,m_nKeyType(NumberFormat::UNDEFINED)
    ,m_aNullDate(DBTypeConversion::getStandardDate())
    ,m_nFormatKey(0)
    ,m_nFieldType(DataType::OTHER)
    ,m_bWritingFormattedFake(sal_False)
    ,m_bNumericField(sal_False)
{
    DBG_CTOR(OEditModel,NULL);

    m_nClassId = FormComponentType::TEXTFIELD;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}

//------------------------------------------------------------------
OEditModel::OEditModel( const OEditModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
        :OEditBaseModel( _pOriginal, _rxFactory )
    ,m_bMaxTextLenModified(sal_False)
    ,m_nKeyType(NumberFormat::UNDEFINED)
    ,m_aNullDate(DBTypeConversion::getStandardDate())
    ,m_nFormatKey(0)
    ,m_nFieldType(DataType::OTHER)
    ,m_bWritingFormattedFake(sal_False)
    ,m_bNumericField(sal_False)
{
    DBG_CTOR( OEditModel, NULL );

    // Note that most of the properties are not clone from the original object:
    // Things as the format key, it's type, and such, depend on the field being part of a loaded form
    // (they're initialized in onConnectedDbColumn). Even if the original object _is_ part of such a form, we ourself
    // certainly aren't, so these members are defaulted. If we're inserted into a form which is already loaded,
    // they will be set to new values, anyway ....
}

//------------------------------------------------------------------
OEditModel::~OEditModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OEditModel,NULL);
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OEditModel )

//------------------------------------------------------------------------------
void OEditModel::disposing()
{
    OEditBaseModel::disposing();
    m_xFormatter = NULL;
}

// XPersistObject
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEditModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_EDIT;  // old (non-sun) name for compatibility !
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OEditModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 3);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_DATABASE_TEXTFIELD;
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_TEXTFIELD;
    pArray[aSupported.getLength()-3] = FRM_SUN_COMPONENT_BINDDB_TEXTFIELD;
    return aSupported;
}

// XPropertySet
//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OEditModel::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
// -----------------------------------------------------------------------------
void SAL_CALL OEditModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle ) const
{
    if ( PROPERTY_ID_PERSISTENCE_MAXTEXTLENGTH == nHandle )
    {
        if ( m_bMaxTextLenModified )
            rValue <<= sal_Int16(0);
        else if ( m_xAggregateSet.is() )
            rValue = m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN);
    }
    else
    {
        OEditBaseModel::getFastPropertyValue(rValue, nHandle );
    }
}

//------------------------------------------------------------------------------
void OEditModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(12)
        // Text auf transient setzen
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP1(NAME,                ::rtl::OUString,        BOUND);
        DECL_PROP2(CLASSID,             sal_Int16,              READONLY, TRANSIENT);
        DECL_PROP2(PERSISTENCE_MAXTEXTLENGTH,sal_Int16,         READONLY, TRANSIENT);
        DECL_PROP2(DEFAULT_TEXT,        ::rtl::OUString,        BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                          BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,        BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,              BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,        BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,    XPropertySet,           BOUND,READONLY, TRANSIENT);
        DECL_BOOL_PROP2(FILTERPROPOSAL,                         BOUND, MAYBEDEFAULT);
        DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,           BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,      READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OEditModel::getInfoHelper()
{
    return *const_cast<OEditModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OEditModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    Any aCurrentText;
    sal_Int16 nOldTextLen = 0;
    // bin ich gerade loaded und habe dazu zeitweilig die MaxTextLen umgesetzt ?
    if ( m_bMaxTextLenModified )
    {   // -> fuer die Dauer des Speicherns meinem aggregierten Model die alte TextLen einreden

        // before doing this we have to save the current text value of the aggregate, as this may be affected by resetting the text len
        // FS - 08.12.99 - 70606
        aCurrentText = m_xAggregateSet->getPropertyValue(PROPERTY_TEXT);

        m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN) >>= nOldTextLen;
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, makeAny((sal_Int16)0));
    }

    OEditBaseModel::write(_rxOutStream);

    if ( m_bMaxTextLenModified )
    {   // wieder zuruecksetzen
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, makeAny(nOldTextLen));
        // and reset the text
        // First we set it to an empty string : Without this the second setPropertyValue would not do anything as it thinks
        // we aren't changing the prop (it didn't notify the - implicite - change of the text prop while setting the max text len)
        // This seems to be a bug with in toolkit's EditControl-implementation.
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, makeAny(::rtl::OUString()));
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, aCurrentText);
    }
}

//------------------------------------------------------------------------------
void OEditModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OEditBaseModel::read(_rxInStream);

    // Some versions (5.1 'til about 552) wrote a wrong DefaultControl-property value which is unknown
    // to older versions (5.0).
    // correct this ...
    if (m_xAggregateSet.is())
    {
        Any aDefaultControl = m_xAggregateSet->getPropertyValue(PROPERTY_DEFAULTCONTROL);
        if  (   (aDefaultControl.getValueType().getTypeClass() == TypeClass_STRING)
            &&  (getString(aDefaultControl).compareTo(FRM_CONTROL_TEXTFIELD) == COMPARE_EQUAL)
            )
        {
            m_xAggregateSet->setPropertyValue(PROPERTY_DEFAULTCONTROL, makeAny(::rtl::OUString(FRM_CONTROL_EDIT)));
            // Older as well as current versions should understand this : the former knew only the FRM_CONTROL_EDIT,
            // the latter are registered for both FRM_CONTROL_EDIT and FRM_CONTROL_TEXTFIELD.
        }
    }
}

//------------------------------------------------------------------------------
sal_Int16 OEditModel::getPersistenceFlags() const
{
    sal_Int16 nFlags = OEditBaseModel::getPersistenceFlags();

    if (m_bWritingFormattedFake)
        nFlags |= PF_FAKE_FORMATTED_FIELD;

    return nFlags;
}

//------------------------------------------------------------------------------
void OEditModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    m_bNumericField = sal_False;
    Reference<XPropertySet> xField = getField();
    if (xField.is())
    {
        // jetzt den Key und typ ermitteln
        m_nFieldType  = getINT32(xField->getPropertyValue(PROPERTY_FIELDTYPE));
        m_nFormatKey = getINT32(xField->getPropertyValue(PROPERTY_FORMATKEY));

        switch (m_nFieldType)
        {
            case ::com::sun::star::sdbc::DataType::DATE:
            case ::com::sun::star::sdbc::DataType::TIME:
            case ::com::sun::star::sdbc::DataType::TIMESTAMP:
            case ::com::sun::star::sdbc::DataType::BIT:
            case ::com::sun::star::sdbc::DataType::TINYINT:
            case ::com::sun::star::sdbc::DataType::SMALLINT:
            case ::com::sun::star::sdbc::DataType::INTEGER:
            case ::com::sun::star::sdbc::DataType::REAL:
            case ::com::sun::star::sdbc::DataType::BIGINT:
            case ::com::sun::star::sdbc::DataType::DOUBLE:
            case ::com::sun::star::sdbc::DataType::NUMERIC:
            case ::com::sun::star::sdbc::DataType::DECIMAL:
                m_bNumericField = sal_True;
                break;
            default:
                m_bNumericField = sal_False;
                break;
        }

        // XNumberFormatter besorgen
        Reference< XRowSet >  xRowSet( _rxForm, UNO_QUERY );
        DBG_ASSERT(xRowSet.is(), "OEditModel::onConnectedDbColumn : source is not a row set ?");
        Reference<XNumberFormatsSupplier>  xSupplier = getNumberFormats(getConnection(xRowSet), sal_False, m_xServiceFactory);
        if (xSupplier.is())
        {
            m_xFormatter =  Reference<XNumberFormatter>(m_xServiceFactory
                                ->createInstance(FRM_NUMBER_FORMATTER), UNO_QUERY);
            if (m_xFormatter.is())
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);

            m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), m_nFormatKey);
            xSupplier->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate"))
                >>= m_aNullDate;
        }

        if (m_nKeyType != NumberFormat::SCIENTIFIC)
        {
            m_bMaxTextLenModified = getINT16(m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN)) != 0;
            if ( !m_bMaxTextLenModified )
            {
                sal_Int32 nFieldLen;
                xField->getPropertyValue(::rtl::OUString::createFromAscii("Precision")) >>= nFieldLen;

                if (nFieldLen && nFieldLen <= USHRT_MAX)
                {
                    Any aVal;
                    aVal <<= (sal_Int16)nFieldLen;
                    m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);

                    m_bMaxTextLenModified = sal_True;
                }
            }
            else
                m_bMaxTextLenModified = sal_False; // to get sure that the text len won't be set in unloaded
        }
    }
}

//------------------------------------------------------------------------------
void OEditModel::onDisconnectedDbColumn()
{
    OEditBaseModel::onDisconnectedDbColumn();
    if (getField().is())
    {
        if ( m_bMaxTextLenModified )
        {
            Any aVal;
            aVal <<= (sal_Int16)0;  // nur wenn es 0 war, habe ich es in onConnectedDbColumn umgesetzt
            m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);
            m_bMaxTextLenModified = sal_False;
        }

        m_xFormatter = 0;
        m_nFieldType = DataType::OTHER;
        m_nFormatKey = 0;
        m_nKeyType   = NumberFormat::UNDEFINED;
        m_aNullDate  = DBTypeConversion::getStandardDate();
    }
}

//------------------------------------------------------------------------------
sal_Bool OEditModel::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
{
    OSL_PRECOND( _rxBinding.is(), "OEditModel::approveValueBinding: invalid binding!" );

    // only strings are accepted for simplicity
    return  _rxBinding.is()
        &&  _rxBinding->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) );
}

//------------------------------------------------------------------------------
sal_Bool OEditModel::commitControlValueToDbColumn( bool _bPostReset )
{
    ::rtl::OUString sNewValue;
    m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) >>= sNewValue;
    if (sNewValue != m_aSaveValue)
    {
        if (!sNewValue.getLength() && !m_bRequired && m_bEmptyIsNull)
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                if ( m_bNumericField )
                    DBTypeConversion::setValue(m_xColumnUpdate, m_xFormatter, m_aNullDate, sNewValue, m_nFormatKey, m_nFieldType, m_nKeyType);
                else
                    m_xColumnUpdate->updateString(sNewValue);
            }
            catch(Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = sNewValue;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
Any OEditModel::translateDbColumnToControlValue()
{
    m_aSaveValue = DBTypeConversion::getValue(
        m_xColumn, m_xFormatter, m_aNullDate, m_nFormatKey, m_nKeyType
    );

    // #i2817# OJ
    sal_uInt16 nMaxTextLen = getINT16( m_xAggregateSet->getPropertyValue( PROPERTY_MAXTEXTLEN ) );
    if ( nMaxTextLen && m_aSaveValue.getLength() > nMaxTextLen )
    {
        sal_Int32 nDiff = m_aSaveValue.getLength() - nMaxTextLen;
        m_aSaveValue = m_aSaveValue.replaceAt( nMaxTextLen, nDiff, ::rtl::OUString() );
    }

    return makeAny( m_aSaveValue );
}

//------------------------------------------------------------------------------
Any OEditModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

//.........................................................................
}
//.........................................................................

