/*************************************************************************
 *
 *  $RCSfile: ListBox.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:06 $
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

#ifndef _FORMS_LISTBOX_HXX_
#define _FORMS_LISTBOX_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>

//.........................................................................
namespace frm
{

const ::rtl::OUString LISTBOX_EMPTY_VALUE = ::rtl::OUString::createFromAscii("$$$empty$$$");

//==================================================================
//= OListBoxModel
//==================================================================
class OListBoxModel
            :public OBoundControlModel
            ,public starsdb::XSQLErrorBroadcaster
            ,public starutil::XRefreshable
            ,public ::utl::OAggregationArrayUsageHelper< OListBoxModel >
{
    staruno::Any                        m_aSaveValue;

    // <properties>
    starform::ListSourceType        m_eListSourceType;      // type der list source
    staruno::Any                    m_aBoundColumn;
    StringSequence                  m_aListSourceSeq;       //
    StringSequence                  m_aValueSeq;            // alle Werte, readonly
    staruno::Sequence<sal_Int16>    m_aDefaultSelectSeq;    // DefaultSelected
    // </properties>

    ::cppu::OInterfaceContainerHelper   m_aRefreshListeners;
    ::cppu::OInterfaceContainerHelper   m_aErrorListeners;

    static sal_Int32        nSelectHandle;
    // [properties]

    sal_Int16               m_nNULLPos; // Position an der der NULLwert abgelegt wird
    sal_Bool                m_bBoundComponent : 1;

    // Helper functions
    StringSequence GetCurValueSeq() const;

    virtual staruno::Sequence<staruno::Type>    _getTypes();

protected:
    // UNO Anbindung
    virtual void _onValueChanged();

public:
    OListBoxModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OListBoxModel();

// XServiceInfo
    IMPLEMENTATION_NAME(OListBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(staruno::RuntimeException);

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxModel, OBoundControlModel);
    virtual staruno::Any SAL_CALL queryAggregation( const staruno::Type& _rType ) throw (staruno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue )
                throw (staruno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue )
                throw (starlang::IllegalArgumentException);

// XLoadListener
    virtual void         _loaded(const starlang::EventObject& rEvent);
    virtual void         _unloaded();

// XBoundComponent
    virtual sal_Bool _commit();

// XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// XPersistObject
    virtual ::rtl::OUString SAL_CALL    getServiceName() throw(staruno::RuntimeException);
    virtual void SAL_CALL
        write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw(stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL
        read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException);

// XReset
    virtual void _reset();

// XRefreshable
    virtual void SAL_CALL refresh() throw(staruno::RuntimeException);
    virtual void SAL_CALL addRefreshListener(const staruno::Reference<starutil::XRefreshListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeRefreshListener(const staruno::Reference<starutil::XRefreshListener>& _rxListener) throw(staruno::RuntimeException);

// XSQLErrorBroadcaster
    virtual void SAL_CALL addSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    void loadData();

    void onError(starsdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription);
};

//==================================================================
//= OListBoxControl
//==================================================================

class OListBoxControl   :public OBoundControl
                        ,public starawt::XFocusListener
                        ,public starawt::XItemListener
                        ,public starform::XChangeBroadcaster
{
    ::cppu::OInterfaceContainerHelper       m_aChangeListeners;

    staruno::Any                            m_aCurrentSelection;
    Timer                                   m_aChangeTimer;

protected:
    // UNO Anbindung
    virtual staruno::Sequence<staruno::Type>    _getTypes();

public:
    OListBoxControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);
    virtual ~OListBoxControl();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OListBoxControl, OBoundControl);
    virtual staruno::Any SAL_CALL queryAggregation( const staruno::Type& _rType ) throw (staruno::RuntimeException);

// XServiceInfo
    IMPLEMENTATION_NAME(OListBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(staruno::RuntimeException);

// XChangeBroadcaster
    virtual void SAL_CALL addChangeListener(const staruno::Reference<starform::XChangeListener>& _rxListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeChangeListener(const staruno::Reference<starform::XChangeListener>& _rxListener) throw(staruno::RuntimeException);

// XFocusListener
    virtual void SAL_CALL focusGained(const starawt::FocusEvent& _rEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL focusLost(const starawt::FocusEvent& _rEvent) throw(staruno::RuntimeException);

// XItemListener
    virtual void SAL_CALL itemStateChanged(const starawt::ItemEvent& _rEvent) throw(staruno::RuntimeException);

// XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& Source) throw (staruno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

private:
    DECL_LINK( OnTimeout, void* );
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_LISTBOX_HXX_

