/*************************************************************************
 *
 *  $RCSfile: Edit.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-20 11:55:53 $
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
#define _FORMS_EDIT_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

//.........................................................................
namespace frm
{

//==================================================================
//= OEditModel
//==================================================================
class OEditModel
                :public OEditBaseModel
                ,public ::comphelper::OAggregationArrayUsageHelper< OEditModel >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>         m_xFormatter;
    ::rtl::OUString                 m_aSaveValue;
    sal_Int32                   m_nFormatKey;
    ::com::sun::star::util::Date                m_aNullDate;
    sal_Int32                   m_nFieldType;
    sal_Int16                   m_nKeyType;
    sal_Int16                   m_nMaxLen;

    sal_Bool    m_bWritingFormattedFake : 1;
        // are we writing something which should be interpreted as formatted upon reading?
    sal_Bool    m_bNumericField : 1;
        // are we bound to some kind of numeric field?

    static sal_Int32            nTextHandle;

protected:
    virtual void _onValueChanged();
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    OEditModel(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OEditModel();

    void enableFormattedWriteFake() { m_bWritingFormattedFake = sal_True; }
    void disableFormattedWriteFake() { m_bWritingFormattedFake = sal_False; }
    sal_Bool lastReadWasFormattedFake() const { return (getLastReadVersion() & PF_FAKE_FORMATTED_FIELD) != 0; }

    friend InterfaceRef SAL_CALL OEditModel_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend class OFormattedFieldWrapper;
    friend class OFormattedModel;   // temporary

public:
    virtual void SAL_CALL disposing();

// ::com::sun::star::form::XBoundComponent
    virtual sal_Bool _commit();

// ::com::sun::star::io::XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream);
    virtual ::rtl::OUString SAL_CALL getServiceName();

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OEditModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::form::XLoadListener
    virtual void _loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void _unloaded();

// ::com::sun::star::form::XReset
    virtual void _reset();

// OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual sal_Int16 getPersistenceFlags() const;
};

//==================================================================
//= OEditControl
//==================================================================
typedef ::cppu::ImplHelper3<    ::com::sun::star::awt::XFocusListener,
                                ::com::sun::star::awt::XKeyListener,
                                ::com::sun::star::form::XChangeBroadcaster > OEditControl_BASE;

class OEditControl : public OBoundControl
                      ,public OEditControl_BASE
{
    ::cppu::OInterfaceContainerHelper
                        m_aChangeListeners;

    ::rtl::OUString         m_aHtmlChangeValue;
    sal_uInt32              m_nKeyEvent;

public:
    OEditControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OEditControl();

    DECLARE_UNO3_AGG_DEFAULTS(OEditControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OEditControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::form::XChangeBroadcaster
    virtual void SAL_CALL addChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener);
    virtual void SAL_CALL removeChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener);

// ::com::sun::star::awt::XFocusListener
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e );
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e );

// ::com::sun::star::awt::XKeyListener
    virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e);
    virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e);

private:
    DECL_LINK( OnKeyPressed, void* );
};

//.........................................................................
}
//.........................................................................

#endif // _FORMS_EDIT_HXX_

