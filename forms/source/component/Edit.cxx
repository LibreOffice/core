/*************************************************************************
 *
 *  $RCSfile: Edit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-19 11:52:16 $
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

//------------------------------------------------------------------
InterfaceRef SAL_CALL OEditControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OEditControl(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OEditControl::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        staruno::Sequence<staruno::Type> aBaseClassTypes = OBoundControl::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(3);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((staruno::Reference<starawt::XFocusListener>*)NULL);
        pOwnTypes[1] = getCppuType((staruno::Reference<starawt::XKeyListener>*)NULL);
        pOwnTypes[2] = getCppuType((staruno::Reference<starform::XChangeBroadcaster>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OEditControl::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starawt::XFocusListener*>(this)
            ,static_cast<starawt::XKeyListener*>(this)
            ,static_cast<starform::XChangeBroadcaster*>(this)
        );

    return aReturn;
}

DBG_NAME(OEditControl);
//------------------------------------------------------------------------------
OEditControl::OEditControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_EDIT)
               ,m_aChangeListeners(m_aMutex)
               ,m_nKeyEvent( 0 )
{
    DBG_CTOR(OEditControl,NULL);

    increment(m_refCount);
    {   // als FocusListener anmelden
        staruno::Reference<starawt::XWindow>  xComp;
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

// starform::XChangeBroadcaster
//------------------------------------------------------------------------------
void OEditControl::addChangeListener(const staruno::Reference<starform::XChangeListener>& l)
{
    m_aChangeListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void OEditControl::removeChangeListener(const staruno::Reference<starform::XChangeListener>& l)
{
    m_aChangeListeners.removeInterface( l );
}

// OComponentHelper
//------------------------------------------------------------------------------
void OEditControl::disposing()
{
    OBoundControl::disposing();

    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    m_aChangeListeners.disposeAndClear(aEvt);
}

// starlang::XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OEditControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_TEXTFIELD;
    return aSupported;
}

// starlang::XEventListener
//------------------------------------------------------------------------------
void OEditControl::disposing(const starlang::EventObject& Source) throw( staruno::RuntimeException )
{
    OBoundControl::disposing(Source);
}

// starawt::XFocusListener
//------------------------------------------------------------------------------
void OEditControl::focusGained( const starawt::FocusEvent& e )
{
    staruno::Reference<starbeans::XPropertySet>  xSet(getModel(), staruno::UNO_QUERY);
    if (xSet.is())
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= m_aHtmlChangeValue;
}

//------------------------------------------------------------------------------
void OEditControl::focusLost( const starawt::FocusEvent& e )
{
    staruno::Reference<starbeans::XPropertySet>  xSet(getModel(), staruno::UNO_QUERY);
    if (xSet.is())
    {
        ::rtl::OUString sNewHtmlChangeValue;
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= sNewHtmlChangeValue;
        if( sNewHtmlChangeValue != m_aHtmlChangeValue )
        {
            starlang::EventObject aEvt;
            aEvt.Source = *this;
            NOTIFY_LISTENERS(m_aChangeListeners, starform::XChangeListener, changed, aEvt);
        }
    }
}

// starawt::XKeyListener
//------------------------------------------------------------------------------
void OEditControl::keyPressed(const starawt::KeyEvent& e)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;

    // Steht das Control in einem Formular mit einer Submit-starutil::URL?
    staruno::Reference<starbeans::XPropertySet>  xSet(getModel(), staruno::UNO_QUERY);
    if( !xSet.is() )
        return;

    // nicht fuer multiline edits
    staruno::Any aTmp( xSet->getPropertyValue(PROPERTY_MULTILINE));
    if ((aTmp.getValueType().equals(::getBooleanCppuType())) && getBOOL(aTmp))
        return;

    staruno::Reference<starform::XFormComponent>  xFComp(xSet, staruno::UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;

    staruno::Reference<starbeans::XPropertySet>  xFormSet(xParent, staruno::UNO_QUERY);
    if( !xFormSet.is() )
        return;

    aTmp = xFormSet->getPropertyValue( PROPERTY_TARGET_URL );
    if (!aTmp.getValueType().equals(::getCppuType((const ::rtl::OUString*)NULL)) ||
        !getString(aTmp).getLength() )
        return;

    staruno::Reference<starcontainer::XIndexAccess>  xElements(xParent, staruno::UNO_QUERY);
    sal_Int32 nCount = xElements->getCount();
    if( nCount > 1 )
    {
        for( sal_Int32 nIndex=0; nIndex < nCount; nIndex++ )
        {
            //  staruno::Any aElement(xElements->getByIndex(nIndex));
            staruno::Reference<starbeans::XPropertySet>  xFCSet(*(InterfaceRef *)xElements->getByIndex(nIndex).getValue(), staruno::UNO_QUERY);

            if (hasProperty(PROPERTY_CLASSID, xFCSet) &&
                getINT16(xFCSet->getPropertyValue(PROPERTY_CLASSID)) == starform::FormComponentType::TEXTFIELD)
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
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OEditControl,
                                            OnKeyPressed) );
}

//------------------------------------------------------------------------------
void OEditControl::keyReleased(const starawt::KeyEvent& e)
{
}

//------------------------------------------------------------------------------
IMPL_LINK(OEditControl, OnKeyPressed, void*, EMPTYARG)
{
    m_nKeyEvent = 0;

    staruno::Reference<starform::XFormComponent>  xFComp(getModel(), staruno::UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    staruno::Reference<starform::XSubmit>  xSubmit(xParent, staruno::UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit( staruno::Reference<starawt::XControl>(), starawt::MouseEvent() );
    return 0L;
}

/*************************************************************************/
sal_Int32 OEditModel::nTextHandle = -1;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OEditModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OEditModel(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OEditModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}


DBG_NAME(OEditModel);
//------------------------------------------------------------------
OEditModel::OEditModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
             :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_EDIT, FRM_CONTROL_EDIT )
                                    // use the old control name for compytibility reasons
             ,m_nMaxLen(0)
             ,m_aNullDate(DBTypeConversion::STANDARD_DB_DATE)
             ,m_nKeyType(starutil::NumberFormat::UNDEFINED)
             ,m_nFormatKey(0)
             ,m_nFieldType(starsdbc::DataType::OTHER)
             ,m_bWritingFormattedFake(sal_False)
{
    DBG_CTOR(OEditModel,NULL);

    m_nClassId = starform::FormComponentType::TEXTFIELD;
    m_sDataFieldConnectivityProperty = PROPERTY_TEXT;
    if (OEditModel::nTextHandle == -1)
        OEditModel::nTextHandle = getOriginalHandle(PROPERTY_ID_TEXT);
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
void OEditModel::disposing()
{
    OEditBaseModel::disposing();
    m_xFormatter = NULL;
}

// stario::XPersistObject
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEditModel::getServiceName()
{
    return FRM_COMPONENT_EDIT;  // old (non-sun) name for compatibility !
}

// starlang::XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OEditModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseTextField");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_TEXTFIELD;
    return aSupported;
}

// starbeans::XPropertySet
//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OEditModel::getPropertySetInfo() throw(staruno::RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OEditModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(12)
        // Text auf transient setzen
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, starbeans::PropertyAttribute::TRANSIENT, 0);

        DECL_PROP1(NAME,                ::rtl::OUString,        BOUND);
        DECL_PROP2(CLASSID,             sal_Int16,              READONLY, TRANSIENT);
        DECL_PROP2(DEFAULT_TEXT,        ::rtl::OUString,        BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                          BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,        BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,              BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,        BOUND);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,        BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    starbeans::XPropertySet,READONLY, TRANSIENT);
        DECL_BOOL_PROP2(FILTERPROPOSAL,                         BOUND, MAYBEDEFAULT);
        DECL_IFACE_PROP2(CONTROLLABEL,  starbeans::XPropertySet,BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OEditModel::getInfoHelper()
{
    return *const_cast<OEditModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OEditModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    staruno::Any aCurrentText;
    // bin ich gerade loaded und habe dazu zeitweilig die MaxTextLen umgesetzt ?
    if (m_nMaxLen)
    {   // -> fuer die Dauer des Speicherns meinem aggregierten Model die alte TextLen einreden

        // before doing this we have to save the current text value of the aggregate, as this may be affected by resetting the text len
        // FS - 08.12.99 - 70606
        aCurrentText = m_xAggregateSet->getPropertyValue(PROPERTY_TEXT);

        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, staruno::makeAny((sal_Int16)0));
    }

    OEditBaseModel::write(_rxOutStream);

    if (m_nMaxLen)
    {   // wieder zuruecksetzen
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, staruno::makeAny((sal_Int16)m_nMaxLen));
        // and reset the text
        // First we set it to an empty string : Without this the second setPropertyValue would not do anything as it thinks
        // we aren't changing the prop (it didn't notify the - implicite - change of the text prop while setting the max text len)
        // This seems to be a bug with in toolkit's EditControl-implementation.
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, staruno::makeAny(::rtl::OUString()));
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, aCurrentText);
    }
}

//------------------------------------------------------------------------------
void OEditModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream)
{
    OEditBaseModel::read(_rxInStream);

    // Some versions (5.1 'til about 552) wrote a wrong DefaultControl-property value which is unknown
    // to older versions (5.0).
    // correct this ...
    if (m_xAggregateSet.is())
    {
        staruno::Any aDefaultControl = m_xAggregateSet->getPropertyValue(PROPERTY_DEFAULTCONTROL);
        if  (   (aDefaultControl.getValueType().getTypeClass() == staruno::TypeClass_STRING)
            &&  (getString(aDefaultControl).compareTo(FRM_CONTROL_TEXTFIELD) == COMPARE_EQUAL)
            )
        {
            m_xAggregateSet->setPropertyValue(PROPERTY_DEFAULTCONTROL, staruno::makeAny(::rtl::OUString(FRM_CONTROL_EDIT)));
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

// starform::XLoadListener
//------------------------------------------------------------------------------
void OEditModel::_loaded(const starlang::EventObject& rEvent)
{
    if (m_xField.is())
    {
        // jetzt den Key und typ ermitteln
        m_nFieldType  = getINT32(m_xField->getPropertyValue(PROPERTY_FIELDTYPE));
        m_nFormatKey = getINT32(m_xField->getPropertyValue(PROPERTY_FORMATKEY));

        // starutil::XNumberFormatter besorgen
        staruno::Reference<starsdbc::XRowSet>  xRowSet(rEvent.Source, staruno::UNO_QUERY);
        DBG_ASSERT(xRowSet.is(), "OEditModel::_loaded : source is not a row set ?");
        staruno::Reference<starutil::XNumberFormatsSupplier>  xSupplier = getNumberFormats(getConnection(xRowSet), sal_False, m_xServiceFactory);
        if (xSupplier.is())
        {
            m_xFormatter =  staruno::Reference<starutil::XNumberFormatter>(m_xServiceFactory
                                ->createInstance(FRM_NUMBER_FORMATTER), staruno::UNO_QUERY);
            if (m_xFormatter.is())
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);

            m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), m_nFormatKey);
            xSupplier->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate"))
                >>= m_aNullDate;
        }

        if (m_nKeyType != starutil::NumberFormat::SCIENTIFIC)
        {
            m_nMaxLen = getINT16(m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN));
            if (!m_nMaxLen)
            {
                sal_Int32 nFieldLen;
                m_xField->getPropertyValue(::rtl::OUString::createFromAscii("Precision")) >>= nFieldLen;

                if (nFieldLen && nFieldLen <= USHRT_MAX)
                {
                    staruno::Any aVal;
                    aVal <<= (sal_Int16)nFieldLen;
                    m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);

                    m_nMaxLen = nFieldLen;  // damit das Ganze beim _unloaded wieder zurueckgesetzt wird
                }
            }
            else
                m_nMaxLen = 0;
        }
    }
}

//------------------------------------------------------------------------------
void OEditModel::_unloaded()
{
    OEditBaseModel::_unloaded();
    if (m_xField.is())
    {
        if (m_nMaxLen)
        {
            staruno::Any aVal;
            aVal <<= (sal_Int16)0;  // nur wenn es 0 war, habe ich es in _loaded umgesetzt
            m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);
            m_nMaxLen = 0;
        }

        m_xFormatter = 0;
        m_nFieldType = starsdbc::DataType::OTHER;
        m_nFormatKey = 0;
        m_nKeyType   = starutil::NumberFormat::UNDEFINED;
        m_aNullDate  = DBTypeConversion::STANDARD_DB_DATE;
    }
}

//------------------------------------------------------------------------------
sal_Bool OEditModel::_commit()
{
    ::rtl::OUString aNewValue;
    m_xAggregateFastSet->getFastPropertyValue(OEditModel::nTextHandle) >>= aNewValue;
    if (aNewValue != m_aSaveValue)
    {
        if (!aNewValue.len() && !m_bRequired && m_bEmptyIsNull)
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                DBTypeConversion::setValue(m_xColumnUpdate, m_xFormatter, m_aNullDate, aNewValue,
                                           m_nFormatKey, m_nFieldType, m_nKeyType);
            }
            catch(...)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aNewValue;
    }
    return sal_True;
}

// starbeans::XPropertyChangeListener
//------------------------------------------------------------------------------
void OEditModel::_onValueChanged()
{

    // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
    // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
    // our own mutex locked
    // and in this special case do it before calling DBTypeConversion::getValue, as this uses the number formatter
    // which's implementation locks the SM, too :(
    // FS - 72451 - 31.01.00
    MutexRelease aRelease(m_aMutex);
    m_aSaveValue = DBTypeConversion::getValue(m_xColumn,
                                              m_xFormatter,
                                              m_aNullDate,
                                              m_nFormatKey,
                                              m_nKeyType);

    m_xAggregateFastSet->setFastPropertyValue(OEditModel::nTextHandle, staruno::makeAny(m_aSaveValue));
}

// starform::XReset
//------------------------------------------------------------------------------
void OEditModel::_reset()
{
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OEditModel::nTextHandle, staruno::makeAny(m_aDefaultText));
    }
}

//.........................................................................
}
//.........................................................................

