/*************************************************************************
 *
 *  $RCSfile: FormattedField.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-21 11:10:53 $
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

#ifndef _FORMS_FORMATTEDFIELD_HXX_
#include "FormattedField.hxx"
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

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_VCLRSC_HXX
#include <tools/vclrsc.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMIT_HPP_
#include <com/sun/star/form/XSubmit.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XKEYLISTENER_HPP_
#include <com/sun/star/awt/XKeyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
    // needed as long as we use the SolarMutex
using namespace dbtools;
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


//namespace stario       = ::com::sun::star::io;

/** implements handling for compatibly reading/writing data from/into an input/output stream.
    data written in a block secured by this class should be readable by older versions which
    use the same mechanism.
*/
/*************************************************************************/

class OMarkableStreamBlock
{
        Reference< XMarkableStream >           m_xMarkStream;
        Reference< XDataInputStream >          m_xInStream;
        Reference< XDataOutputStream >         m_xOutStream;

    sal_Int32   m_nBlockStart;
    sal_Int32   m_nBlockLen;

public:
    /** starts reading of a "skippable" block of data within the given input stream<BR>
        The object given by _rxInput must support the XMarkableStream interface.
    */
        OMarkableStreamBlock(const Reference< XDataInputStream >& _rxInput);
    /** starts writing of a "skippable" block of data into the given output stream
        The object given by _rxOutput must support the XMarkableStream interface.
    */
        OMarkableStreamBlock(const Reference< XDataOutputStream >& _rxOutput);

    ~OMarkableStreamBlock();
};

//-------------------------------------------------------------------------
OMarkableStreamBlock::OMarkableStreamBlock(const Reference< XDataInputStream >& _rxInput)
    :m_xInStream(_rxInput)
    ,m_xMarkStream(_rxInput, UNO_QUERY)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    DBG_ASSERT(m_xInStream.is() && m_xMarkStream.is(), "OMarkableStreamBlock::OMarkableStreamBlock : invaid argument !");
    if (m_xInStream.is() && m_xMarkStream.is())
    {
        m_nBlockLen = _rxInput->readLong();
        m_nBlockStart = m_xMarkStream->createMark();
    }
}

//-------------------------------------------------------------------------
OMarkableStreamBlock::OMarkableStreamBlock(const Reference< XDataOutputStream >& _rxOutput)
    :m_xOutStream(_rxOutput)
    ,m_xMarkStream(_rxOutput, UNO_QUERY)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    DBG_ASSERT(m_xOutStream.is() && m_xMarkStream.is(), "OMarkableStreamBlock::OMarkableStreamBlock : invaid argument !");
    if (m_xOutStream.is() && m_xMarkStream.is())
    {
        m_nBlockStart = m_xMarkStream->createMark();
        // a placeholder where we will write the overall length (within the destructor)
        m_nBlockLen = 0;
        m_xOutStream->writeLong(m_nBlockLen);
    }
}

//-------------------------------------------------------------------------
OMarkableStreamBlock::~OMarkableStreamBlock()
{
    if (m_xInStream.is() &&  m_xMarkStream.is())
    {   // we're working on an input stream
        m_xMarkStream->jumpToMark(m_nBlockStart);
        m_xInStream->skipBytes(m_nBlockLen);
        m_xMarkStream->deleteMark(m_nBlockStart);
    }
    else if (m_xOutStream.is() && m_xMarkStream.is())
    {
        m_nBlockLen = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
        m_xMarkStream->jumpToMark(m_nBlockStart);
        m_xOutStream->writeLong(m_nBlockLen);
        m_xMarkStream->jumpToFurthest();
        m_xMarkStream->deleteMark(m_nBlockStart);
    }
}

//.........................................................................
namespace frm
{

/*************************************************************************/

class StandardFormatsSupplier : protected SvNumberFormatsSupplierObj
{
protected:
    SvNumberFormatter*  m_pMyPrivateFormatter;

public:
    StandardFormatsSupplier(const Reference<XMultiServiceFactory>& _rxFactory);

    operator Reference<XNumberFormatsSupplier> ();
    SvNumberFormatsSupplierObj::operator new;
    SvNumberFormatsSupplierObj::operator delete;

protected:
    ~StandardFormatsSupplier() { if (m_pMyPrivateFormatter) delete m_pMyPrivateFormatter; }
};


//------------------------------------------------------------------
StandardFormatsSupplier::StandardFormatsSupplier(const Reference< XMultiServiceFactory > & _rxFactory)
    :SvNumberFormatsSupplierObj()
    ,m_pMyPrivateFormatter(new SvNumberFormatter(_rxFactory, Application::GetAppInternational().GetLanguage()))
{
    SetNumberFormatter(m_pMyPrivateFormatter);
}

//------------------------------------------------------------------
StandardFormatsSupplier::operator Reference<XNumberFormatsSupplier> ()
{
    return static_cast<XNumberFormatsSupplier*>(static_cast<SvNumberFormatsSupplierObj*>(this));
}

//------------------------------------------------------------------
Reference<XNumberFormatsSupplier>  OFormattedModel::s_xDefaultFormatter;
sal_Int32 OFormattedModel::nValueHandle = -1;

/*************************************************************************/
//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormattedControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedControl(_rxFactory));
}

//------------------------------------------------------------------
Sequence<Type> OFormattedControl::_getTypes()
{
        static Sequence<Type> aTypes;
    if (!aTypes.getLength())
        aTypes = concatSequences(OFormattedControl_BASE::getTypes(), OBoundControl::_getTypes());
    return aTypes;
}

//------------------------------------------------------------------
Any SAL_CALL OFormattedControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OFormattedControl_BASE::queryInterface(_rType);
    return aReturn;
}


DBG_NAME(OFormattedControl);
//------------------------------------------------------------------------------
OFormattedControl::OFormattedControl(const Reference<XMultiServiceFactory>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_FORMATTEDFIELD)
               ,m_nKeyEvent(0)
{
    DBG_CTOR(OFormattedControl,NULL);

    increment(m_refCount);
    {   // als FocusListener anmelden
                Reference<XWindow>  xComp;
        if (query_aggregation(m_xAggregate, xComp))
        {
            xComp->addKeyListener(this);
        }
    }
    // Refcount wieder bei 1 fuer den Listener
    sal_Int32 n = decrement(m_refCount);
}

//------------------------------------------------------------------------------
OFormattedControl::~OFormattedControl()
{
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );

    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OFormattedControl,NULL);
}

// XKeyListener
//------------------------------------------------------------------------------
void OFormattedControl::disposing(const EventObject& _rSource) throw(RuntimeException)
{
    OBoundControl::disposing(_rSource);
}

//------------------------------------------------------------------------------
void OFormattedControl::keyPressed(const KeyEvent& e)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;

    // Steht das Control in einem Formular mit einer Submit-URL?
    Reference<com::sun::star::beans::XPropertySet>  xSet(getModel(), UNO_QUERY);
    if( !xSet.is() )
        return;

    Reference<XFormComponent>  xFComp(xSet, UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;

    Reference<com::sun::star::beans::XPropertySet>  xFormSet(xParent, UNO_QUERY);
    if( !xFormSet.is() )
        return;

    Any aTmp(xFormSet->getPropertyValue( PROPERTY_TARGET_URL ));
    if (!isA(aTmp, static_cast< ::rtl::OUString* >(NULL)) ||
        !getString(aTmp).getLength() )
        return;

    Reference<XIndexAccess>  xElements(xParent, UNO_QUERY);
    sal_Int32 nCount = xElements->getCount();
    if( nCount > 1 )
    {

        Reference<com::sun::star::beans::XPropertySet>  xFCSet;
        for( sal_Int32 nIndex=0; nIndex < nCount; nIndex++ )
        {
            //  Any aElement(xElements->getByIndex(nIndex));
            xElements->getByIndex(nIndex) >>= xFCSet;

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
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OFormattedControl,
                                            OnKeyPressed) );
}

//------------------------------------------------------------------------------
void OFormattedControl::keyReleased(const KeyEvent& e)
{
}

//------------------------------------------------------------------------------
IMPL_LINK(OFormattedControl, OnKeyPressed, void*, EMPTYARG)
{
    m_nKeyEvent = 0;

        Reference<XFormComponent>  xFComp(getModel(), UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
        Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
    if (xSubmit.is())
                xSubmit->submit( Reference<XControl> (), MouseEvent() );
    return 0L;
}

//------------------------------------------------------------------------------
StringSequence  OFormattedControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_FORMATTEDFIELD;
    return aSupported;
}

//------------------------------------------------------------------------------
void OFormattedControl::setDesignMode(sal_Bool bOn)
{
    OBoundControl::setDesignMode(bOn);
}

/*************************************************************************/
//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormattedModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedModel(_rxFactory));
}

//------------------------------------------------------------------
OFormattedModel::OFormattedModel(const Reference<XMultiServiceFactory>& _rxFactory)
            :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_FORMATTEDFIELD, FRM_CONTROL_FORMATTEDFIELD )
                                    // use the old control name for compytibility reasons
            ,OPropertyChangeListener(m_aMutex)
            ,m_bOriginalNumeric(sal_False)
            ,m_bNumeric(sal_False)
            ,m_xOriginalFormatter(NULL)
            ,m_nKeyType(NumberFormat::UNDEFINED)
            ,m_aNullDate(DBTypeConversion::getStandardDate())
{
    m_nClassId = FormComponentType::TEXTFIELD;

    increment(m_refCount);
    setPropertyToDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    decrement(m_refCount);

    m_sDataFieldConnectivityProperty = PROPERTY_EFFECTIVE_VALUE;
    if (OFormattedModel::nValueHandle == -1)
        OFormattedModel::nValueHandle = getOriginalHandle(PROPERTY_ID_EFFECTIVE_VALUE);

    increment(m_refCount);
    if (m_xAggregateSet.is())
    {
        OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet);
        pMultiplexer->addProperty(PROPERTY_FORMATKEY);
    }
    decrement(m_refCount);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence OFormattedModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OEditBaseModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseFormattedField");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_FORMATTEDFIELD;
    return aSupported;
}

// XPersistObject
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormattedModel::getServiceName()
{
    return ::rtl::OUString(FRM_COMPONENT_EDIT);
//  return ::rtl::OUString(FRM_COMPONENT_FORMATTEDFIELD);   // old (non-sun) name for compatibility !
}

// XPropertySet
//------------------------------------------------------------------------------
Reference<com::sun::star::beans::XPropertySetInfo> SAL_CALL OFormattedModel::getPropertySetInfo() throw(RuntimeException)
{
        Reference<com::sun::star::beans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OFormattedModel::getInfoHelper()
{
    return *const_cast<OFormattedModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OFormattedModel::fillProperties(
                Sequence< com::sun::star::beans::Property >& _rProps,
                Sequence< com::sun::star::beans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(11)
        DECL_PROP1(NAME,                ::rtl::OUString,        BOUND);
        DECL_PROP2(CLASSID,             sal_Int16,              READONLY, TRANSIENT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                          BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,        BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,              BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,        BOUND);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,        BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    com::sun::star::beans::XPropertySet,READONLY, TRANSIENT);
        DECL_BOOL_PROP2(FILTERPROPOSAL,                         BOUND, MAYBEDEFAULT);
        DECL_IFACE_PROP2(CONTROLLABEL,  com::sun::star::beans::XPropertySet,BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);

        // der Supplier ist fuer uns nur read-only
        ModifyPropertyAttributes(_rAggregateProps, PROPERTY_FORMATSSUPPLIER, com::sun::star::beans::PropertyAttribute::READONLY, 0);
        // TreatAsNumeric nicht transient : wir wollen es an der UI anbinden (ist noetig, um dem EffectiveDefault
        // - der kann Text oder Zahl sein - einen Sinn zu geben)
        ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TREATASNUMERIC, 0, com::sun::star::beans::PropertyAttribute::TRANSIENT);
        RemoveProperty(_rAggregateProps, PROPERTY_STRICTFORMAT);
            // no strict format property for formatted fields: it does not make sense, 'cause
            // there is no general way to decide which characters/sub strings are allowed during the input of an
            // arbitraryly formatted control
            // 81441 - 12/07/00 - FS
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
void OFormattedModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    OEditBaseModel::getFastPropertyValue(rValue, nHandle);
}

//------------------------------------------------------------------------------
void OFormattedModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
{
    OEditBaseModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
}

//------------------------------------------------------------------------------
sal_Bool OFormattedModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
                                                        throw( IllegalArgumentException )
{
    return OEditBaseModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
}

//------------------------------------------------------------------------------
void OFormattedModel::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
    {   // das aggregierte Model koennte auf die Idee kommen
                Reference<XNumberFormatsSupplier>  xSupplier = calcDefaultFormatsSupplier();
        DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::setPropertyToDefaultByHandle(FORMATSSUPPLIER) : have no aggregate !");
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
    }
    else
        OEditBaseModel::setPropertyToDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
void OFormattedModel::setPropertyToDefault(const ::rtl::OUString& aPropertyName) throw( com::sun::star::beans::UnknownPropertyException, RuntimeException )
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );

    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
        setPropertyToDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    else
        OEditBaseModel::setPropertyToDefault(aPropertyName);
}

//------------------------------------------------------------------------------
Any OFormattedModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
    {
        Reference<XNumberFormatsSupplier>  xSupplier = calcFormFormatsSupplier();
        return makeAny(xSupplier);
    }
    else
        return OEditBaseModel::getPropertyDefaultByHandle(nHandle);
}

//------------------------------------------------------------------------------
Any SAL_CALL OFormattedModel::getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw( com::sun::star::beans::UnknownPropertyException, RuntimeException )
{
    OPropertyArrayAggregationHelper& rPH = (OPropertyArrayAggregationHelper&)getInfoHelper();
    sal_Int32 nHandle = rPH.getHandleByName( aPropertyName );

    if (nHandle == PROPERTY_ID_FORMATSSUPPLIER)
        return getPropertyDefaultByHandle(PROPERTY_ID_FORMATSSUPPLIER);
    else
        return OEditBaseModel::getPropertyDefault(aPropertyName);
}

//------------------------------------------------------------------------------
void OFormattedModel::_propertyChanged( const com::sun::star::beans::PropertyChangeEvent& evt ) throw(RuntimeException)
{
    if (evt.Source == m_xAggregate)
    {
        if (evt.PropertyName.equals(PROPERTY_FORMATKEY))
        {
            if (evt.NewValue.getValueType().getTypeClass() == TypeClass_LONG)
            {
                try
                {
                    Reference<XNumberFormatsSupplier> xSupplier(calcFormatsSupplier());
                    m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), getINT32(evt.NewValue));
                    // as m_aSaveValue (which is used by _commit) is format dependent we have
                    // to recalc it, which is done by _onValueChanged
                    if (m_xColumn.is() && m_xAggregateFastSet.is())
                    {
                        ::osl::MutexGuard aGuard(m_aMutex);     // _onValueChanged expects that ...
                        _onValueChanged();
                    }
                }
                catch(Exception&)
                {
                }
            }
        }
        // our base class does not listen at the aggregate at the moment ... and it does not expect
        // to get events from objects other than the field we're bound to. So do not call the
        // base class method here.
        return;
    }
}

//------------------------------------------------------------------------------
Reference<XNumberFormatsSupplier>  OFormattedModel::calcFormatsSupplier() const
{
    Reference<XNumberFormatsSupplier>  xSupplier;

    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::calcFormatsSupplier : have no aggregate !");
    // hat mein aggregiertes Model einen FormatSupplier ?
    if( m_xAggregateSet.is() )

        m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER) >>= xSupplier;
    if (!xSupplier.is())
        // testen, ob meine Parent-starform einen Formatter hat
        xSupplier = calcFormFormatsSupplier();

    if (!xSupplier.is())
        xSupplier = calcDefaultFormatsSupplier();

    DBG_ASSERT(xSupplier.is(), "OFormattedModel::calcFormatsSupplier : no supplier !");
        // jetzt sollte aber einer da sein
    return xSupplier;
}

//------------------------------------------------------------------------------
Reference<XNumberFormatsSupplier>  OFormattedModel::calcFormFormatsSupplier() const
{
    Reference<XChild>  xMe;
    query_interface(static_cast<XWeak*>(const_cast<OFormattedModel*>(this)), xMe);
    // damit stellen wir sicher, dass wir auch fuer den Fall der Aggregation das richtige
    // Objekt bekommen
    DBG_ASSERT(xMe.is(), "OFormattedModel::calcFormFormatsSupplier : I should have a content interface !");

    // jetzt durchhangeln nach oben, bis wir auf eine starform treffen (angefangen mit meinem eigenen Parent)
    Reference<XChild>  xParent(xMe->getParent(), UNO_QUERY);
    Reference<XForm>  xNextParentForm(xParent, UNO_QUERY);
    while (!xNextParentForm.is() && xParent.is())
    {
        xParent         = Reference<XChild> (xParent->getParent(), UNO_QUERY);
        xNextParentForm = Reference<XForm> (xParent, UNO_QUERY);
    }

    if (!xNextParentForm.is())
    {
        DBG_ERROR("OFormattedModel::calcFormFormatsSupplier : have no ancestor which is a form !");
        return NULL;
    }

    // den FormatSupplier von meinem Vorfahren (falls der einen hat)
        Reference<XRowSet>  xRowSet(xNextParentForm, UNO_QUERY);
        Reference<XNumberFormatsSupplier>  xSupplier;
    if (xRowSet.is())
        xSupplier = getNumberFormats(getConnection(xRowSet), sal_True, m_xServiceFactory);
    return xSupplier;
}

//------------------------------------------------------------------------------
Reference<XNumberFormatsSupplier>  OFormattedModel::calcDefaultFormatsSupplier() const
{
    if (!s_xDefaultFormatter.is())
        s_xDefaultFormatter = *new StandardFormatsSupplier(m_xServiceFactory);
    return s_xDefaultFormatter;
}

//------------------------------------------------------------------------------
sal_Int32 OFormattedModel::calcFormatKey() const
{
    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::calcFormatKey : have no aggregate !");
    // hat mein aggregiertes Model einen FormatSupplier ?
    Any aFormatKey = m_xAggregateSet.is() ? m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY): Any();
    if (aFormatKey.hasValue())
        return getINT32(aFormatKey);

    if (m_xField.is())
        return getINT32(m_xField->getPropertyValue(PROPERTY_FORMATKEY));

    return 0;
}

//------------------------------------------------------------------------------
void OFormattedModel::getFormatDescription(::rtl::OUString& sFormat, LanguageType& eLanguage)
{
}

// XBoundComponent
//------------------------------------------------------------------------------
void OFormattedModel::loaded(const EventObject& rEvent)
{
    // HACK : our _loaded accesses our NumberFormatter which locks the solar mutex (as it doesn't have
    // an own one). To prevent deadlocks with other threads which may request a property from us in an
    // UI-triggered action (e.g. an tooltip) we lock the solar mutex _here_ before our base class locks
    // it's own muext (which is used for property requests)
    // alternative a): we use two mutexes, one which is passed to the OPropertysetHelper and used for
    // property requests and one for our own code. This would need a lot of code rewriting
    // alternative b): The NumberFormatter has to be really threadsafe (with an own mutex), which is
    // the only "clean" solution for me.
    // FS - 69603 - 02.11.99

    ::vos::OGuard aGuard(Application::GetSolarMutex());
    OEditBaseModel::loaded(rEvent);
}

//------------------------------------------------------------------------------
void OFormattedModel::_loaded(const EventObject& rEvent)
{
    static const ::rtl::OUString s_aNullDataProp = ::rtl::OUString::createFromAscii("NullDate");

    m_xOriginalFormatter = NULL;

    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::_loaded : have no aggregate !");
    if (m_xAggregateSet.is())
    {   // all the following doesn't make any sense if we have no aggregate ...
        Any aSupplier = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
        DBG_ASSERT(((Reference<XNumberFormatsSupplier> *)aSupplier.getValue())->is(), "OFormattedModel::_loaded : invalid property value !");
        // das sollte im Constructor oder im read auf was richtiges gesetzt worden sein

        Any aFmtKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        if (!aFmtKey.hasValue())
        {   // unser aggregiertes Model hat noch keine Format-Informationen, also geben wir die von dem Feld, an das
            // wir gebunden sind, weiter
            sal_Int32 nType = DataType::VARCHAR;
            if (m_xField.is())
            {
                aFmtKey = m_xField->getPropertyValue(PROPERTY_FORMATKEY);
                m_xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= nType ;
            }

            Reference<XNumberFormatsSupplier>  xSupplier = calcFormFormatsSupplier();
            DBG_ASSERT(xSupplier.is(), "OFormattedModel::_loaded : bound to a field but no parent with a formatter ? how this ?");
            if (xSupplier.is())
            {
                m_bOriginalNumeric = getBOOL(getPropertyValue(PROPERTY_TREATASNUMERIC));

                if (!aFmtKey.hasValue())
                {   // we aren't bound to a field (or this field's format is invalid)
                    // -> determine the standard text (or numeric) format of the supplier
                    Reference<XNumberFormatTypes>  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);
                    if (xTypes.is())
                    {
                        UniString sLanguage, sCountry;
                        ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
                        Locale aApplicationLocale(sLanguage, sCountry, ::rtl::OUString());

                        sal_Int32 nNewKey = 0;
                        if (m_bOriginalNumeric)
                            aFmtKey <<= (sal_Int32)xTypes->getStandardFormat(NumberFormat::NUMBER, aApplicationLocale);
                        else
                            aFmtKey <<= (sal_Int32)xTypes->getStandardFormat(NumberFormat::TEXT, aApplicationLocale);
                    }
                }

                aSupplier >>= m_xOriginalFormatter;
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, aFmtKey);

                // das Numeric-Flag an mein gebundenes Feld anpassen
                if (m_xField.is())
                {
                    m_bNumeric = sal_False;
                    switch (nType)
                    {
                        case DataType::BIT:
                        case DataType::TINYINT:
                        case DataType::SMALLINT:
                        case DataType::INTEGER:
                        case DataType::BIGINT:
                        case DataType::FLOAT:
                        case DataType::REAL:
                        case DataType::DOUBLE:
                        case DataType::NUMERIC:
                        case DataType::DECIMAL:
                        case DataType::DATE:
                        case DataType::TIME:
                        case DataType::TIMESTAMP:
                            m_bNumeric = sal_True;
                            break;
                    }
                }
                else
                    m_bNumeric = m_bOriginalNumeric;

                setPropertyValue(PROPERTY_TREATASNUMERIC, makeAny((sal_Bool)m_bNumeric));

                m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), getINT32(aFmtKey));
                xSupplier->getNumberFormatSettings()->getPropertyValue(s_aNullDataProp) >>= m_aNullDate;
            }
        }
        else
        {
            Reference<XNumberFormatsSupplier>  xSupplier = calcFormatsSupplier();

            m_bNumeric = getBOOL(getPropertyValue(PROPERTY_TREATASNUMERIC));
            m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), getINT32(aFmtKey));
            xSupplier->getNumberFormatSettings()->getPropertyValue(s_aNullDataProp) >>= m_aNullDate;
        }
    }
    else
    {   // try to get some defaults ...
        Reference<XNumberFormatsSupplier>  xSupplier = calcFormatsSupplier();

        m_bNumeric = getBOOL(getPropertyValue(PROPERTY_TREATASNUMERIC));
        m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), 0);
        xSupplier->getNumberFormatSettings()->getPropertyValue(s_aNullDataProp) >>= m_aNullDate;
    }

    OEditBaseModel::_loaded(rEvent);
}

//------------------------------------------------------------------------------
void OFormattedModel::_unloaded()
{
    OEditBaseModel::_unloaded();
    if (m_xOriginalFormatter.is())
    {   // unser aggregiertes Model hatte keinerlei Format-Informationen
        m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(m_xOriginalFormatter));
        m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, Any());
        setPropertyValue(PROPERTY_TREATASNUMERIC, makeAny((sal_Bool)m_bOriginalNumeric));
        m_xOriginalFormatter = NULL;
    }

    m_nKeyType   = NumberFormat::UNDEFINED;
    m_aNullDate  = DBTypeConversion::getStandardDate();
}

//------------------------------------------------------------------------------
void OFormattedModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    OEditBaseModel::write(_rxOutStream);
    _rxOutStream->writeShort(0x0003);

    DBG_ASSERT(m_xAggregateSet.is(), "OFormattedModel::write : have no aggregate !");

    // mein Format (evtl. void) in ein persistentes Format bringen (der Supplier zusammen mit dem Key ist es zwar auch,
    // aber deswegen muessen wir ja nicht gleich den ganzen Supplier speichern, das waere ein klein wenig Overhead ;)

        Reference<XNumberFormatsSupplier>  xSupplier;
        Any aFmtKey;
    sal_Bool bVoidKey = sal_True;
    if (m_xAggregateSet.is())
    {
                Any aSupplier = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATSSUPPLIER);
                if (aSupplier.getValueType().getTypeClass() != TypeClass_VOID)
        {
            DBG_ASSERT(isAReference(aSupplier, static_cast<XNumberFormatsSupplier*>(NULL)),
                "OFormattedModel::write : invalid formats supplier !");
                        xSupplier = *(Reference<XNumberFormatsSupplier> *)aSupplier.getValue();
        }

        aFmtKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        bVoidKey = (!xSupplier.is() || !aFmtKey.hasValue()) || (m_bLoaded && m_xOriginalFormatter.is());
            // (kein Fomatter und/oder Key) oder (loaded und faked Formatter)
    }

    _rxOutStream->writeBoolean(!bVoidKey);
    if (!bVoidKey)
    {
        // aus dem FormatKey und dem Formatter persistente Angaben basteln

                Any aKey = m_xAggregateSet->getPropertyValue(PROPERTY_FORMATKEY);
        sal_Int32 nKey = aKey.hasValue() ? getINT32(aKey) : 0;

                Reference<XNumberFormats>  xFormats = xSupplier->getNumberFormats();

        ::rtl::OUString         sFormatDescription;
        LanguageType    eFormatLanguage = LANGUAGE_DONTKNOW;

        static const ::rtl::OUString s_aLocaleProp = ::rtl::OUString::createFromAscii("Locale");
                Reference<com::sun::star::beans::XPropertySet>  xFormat = xFormats->getByKey(nKey);
        if (hasProperty(s_aLocaleProp, xFormat))
        {
                        Any aLocale = xFormat->getPropertyValue(s_aLocaleProp);
                        DBG_ASSERT(isA(aLocale, static_cast<Locale*>(NULL)), "OFormattedModel::write : invalid language property !");
                        if (isA(aLocale, static_cast<Locale*>(NULL)))
            {
                                Locale* pLocale = (Locale*)aLocale.getValue();
                eFormatLanguage = ConvertIsoNamesToLanguage(
                    ::rtl::OUStringToOString(pLocale->Language, RTL_TEXTENCODING_ASCII_US).getStr(),
                    ::rtl::OUStringToOString(pLocale->Country, RTL_TEXTENCODING_ASCII_US).getStr()
                );
            }
        }

        static const ::rtl::OUString s_aFormatStringProp = ::rtl::OUString::createFromAscii("FormatString");
        if (hasProperty(s_aFormatStringProp, xFormat))
            xFormat->getPropertyValue(s_aFormatStringProp) >>= sFormatDescription;

        _rxOutStream->writeUTF(sFormatDescription);
        _rxOutStream->writeLong((sal_Int32)eFormatLanguage);
    }

    // version 2 : write the properties common to all OEditBaseModels
    writeCommonEditProperties(_rxOutStream);

    // version 3 : write the effective value property of the aggregate
    // Due to a bug within the UnoControlFormattedFieldModel implementation (our default aggregate) this props value isn't correctly read
    // and this can't be corrected without being incompatible.
    // so we have our own handling.

    // and to be a little bit more compatible we make the following section skippable
    {
                Reference< XDataOutputStream > xOut(_rxOutStream, UNO_QUERY);
        OMarkableStreamBlock aDownCompat(xOut);

        // a sub version within the skippable block
        _rxOutStream->writeShort(0x0000);

        // version 0: the effective value of the aggregate
                Any aEffectiveValue;
        if (m_xAggregateSet.is())
        {
            try { aEffectiveValue = m_xAggregateSet->getPropertyValue(PROPERTY_EFFECTIVE_VALUE); } catch(Exception&) { }
        }

        {
            OMarkableStreamBlock aDownCompat2(xOut);
            switch (aEffectiveValue.getValueType().getTypeClass())
            {
                case TypeClass_STRING:
                    _rxOutStream->writeShort(0x0000);
                    _rxOutStream->writeUTF(::comphelper::getString(aEffectiveValue));
                    break;
                case TypeClass_DOUBLE:
                    _rxOutStream->writeShort(0x0001);
                    _rxOutStream->writeDouble(::comphelper::getDouble(aEffectiveValue));
                    break;
                default:    // void and all unknown states
                    DBG_ASSERT(!aEffectiveValue.hasValue(), "FmXFormattedModel::write : unknown property value type !");
                    _rxOutStream->writeShort(0x0002);
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void OFormattedModel::read(const Reference<XObjectInputStream>& _rxInStream)
{
    OEditBaseModel::read(_rxInStream);
    sal_uInt16 nVersion = _rxInStream->readShort();

        Reference<XNumberFormatsSupplier>  xSupplier;
    sal_Int32 nKey = -1;
    switch (nVersion)
    {
        case 0x0001 :
        case 0x0002 :
        case 0x0003 :
        {
            sal_Bool bNonVoidKey = _rxInStream->readBoolean();
            if (bNonVoidKey)
            {
                // den String und die Language lesen ....
                ::rtl::OUString sFormatDescription = _rxInStream->readUTF();
                LanguageType eDescriptionLanguage = (LanguageType)_rxInStream->readLong();

                // und daraus von einem Formatter zu einem Key zusammenwuerfeln lassen ...
                xSupplier = calcFormatsSupplier();
                    // calcFormatsSupplier nimmt erst den vom Model, dann einen von der starform, dann einen ganz neuen ....
                                Reference<XNumberFormats>  xFormats = xSupplier->getNumberFormats();

                if (xFormats.is())
                {
                    UniString sLanguage, sCountry;
                    ConvertLanguageToIsoNames(eDescriptionLanguage, sLanguage, sCountry);
                                        Locale aDescriptionLanguage(
                        sLanguage,
                        sCountry,
                        ::rtl::OUString());
                    nKey = xFormats->queryKey(sFormatDescription, aDescriptionLanguage, sal_False);
                    if (nKey == (sal_Int32)-1)
                    {   // noch nicht vorhanden in meinem Formatter ...
                        nKey = xFormats->addNew(sFormatDescription, aDescriptionLanguage);
                    }
                }
            }
            if ((nVersion == 0x0002) || (nVersion == 0x0003))
                readCommonEditProperties(_rxInStream);

            if (nVersion == 0x0003)
            {   // since version 3 there is a "skippable" block at this position
                                Reference< XDataInputStream > xIn(_rxInStream, UNO_QUERY);
                OMarkableStreamBlock aDownCompat(xIn);

                sal_Int16 nSubVersion = _rxInStream->readShort();

                // version 0 and higher : the "effective value" property
                                Any aEffectiveValue;
                {
                    OMarkableStreamBlock aDownCompat2(xIn);
                    switch (_rxInStream->readShort())
                    {
                        case 0: // String
                            aEffectiveValue <<= _rxInStream->readUTF();
                            break;
                        case 1: // double
                            aEffectiveValue <<= (double)_rxInStream->readDouble();
                            break;
                        case 2:
                            break;
                        case 3:
                            DBG_ERROR("FmXFormattedModel::read : unknown effective value type !");
                    }
                }

                // this property is only to be set if we have no control source : in all other cases the base class did a
                // reset after it's read and this set the effective value to a default value
                if (m_xAggregateSet.is() && (m_aControlSource.getLength() == 0))
                {
                    try
                    {
                        m_xAggregateSet->setPropertyValue(PROPERTY_EFFECTIVE_VALUE, aEffectiveValue);
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
        }
        break;
        default :
            DBG_ERROR("OFormattedModel::read : unknown version !");
            // dann bleibt das Format des aggregierten Sets, wie es bei der Erzeugung ist : void
            defaultCommonEditProperties();
            break;
    }

    if ((nKey != -1) && m_xAggregateSet.is())
    {
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATSSUPPLIER, makeAny(xSupplier));
                m_xAggregateSet->setPropertyValue(PROPERTY_FORMATKEY, makeAny((sal_Int32)nKey));
    }
    else
    {
        setPropertyToDefault(PROPERTY_FORMATSSUPPLIER);
        setPropertyToDefault(PROPERTY_FORMATKEY);
    }
}

//------------------------------------------------------------------------------
sal_Int16 OFormattedModel::getPersistenceFlags() const
{
    return (OEditBaseModel::getPersistenceFlags() & ~PF_HANDLE_COMMON_PROPS);
    // a) we do our own call to writeCommonEditProperties
}
//------------------------------------------------------------------------------
sal_Bool OFormattedModel::_commit()
{
        Any aNewValue = m_xAggregateFastSet->getFastPropertyValue( OFormattedModel::nValueHandle );
    if (!compare(aNewValue, m_aSaveValue))
    {
        // Leerstring + EmptyIsNull = void
        if  (   !aNewValue.hasValue()
            ||  (   (aNewValue.getValueType().getTypeClass() == TypeClass_STRING)
                &&  (getString(aNewValue).getLength() == 0)
                &&  m_bEmptyIsNull
                )
            )
            m_xColumnUpdate->updateNull();
        else
        {
            // als Value koennen nur double, string oder void auftreten
            try
            {
                if (aNewValue.getValueType().getTypeClass() == TypeClass_DOUBLE)
                {
                    DBTypeConversion::setValue(m_xColumnUpdate, m_aNullDate, getDouble(aNewValue), m_nKeyType);
                }
                else
                {
                    DBG_ASSERT(aNewValue.getValueType().getTypeClass() == TypeClass_STRING, "OFormattedModel::_commit : invalud value type !");
                    m_xColumnUpdate->updateString(getString(aNewValue));
                }
            }
            catch(Exception&)
            {
                return sal_False;
            }
            m_aSaveValue = aNewValue;
        }
    }
    return sal_True;
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void OFormattedModel::_onValueChanged()
{
    if (m_bNumeric)
        m_aSaveValue <<= m_xColumn->getDouble();
    else
        m_aSaveValue <<= m_xColumn->getString();

    if (m_xColumn->wasNull())
        m_aSaveValue.clear();

    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OFormattedModel::nValueHandle, m_aSaveValue);
    }
}

// XReset
//------------------------------------------------------------------------------
void OFormattedModel::_reset( void )
{
    if (!m_xAggregateSet.is())
    {
        DBG_ERROR("OFormattedModel::_reset : no aggregate !");
        return;
    }
        Any aValue = m_xAggregateSet->getPropertyValue(PROPERTY_EFFECTIVE_DEFAULT);
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OFormattedModel::nValueHandle, aValue);
    }
}

//.........................................................................
}
//.........................................................................





