/*************************************************************************
 *
 *  $RCSfile: Grid.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-02 10:28:06 $
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

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_XLOADLISTENER_HPP_
#include <com/sun/star/form/XLoadListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _FRM_INTERFACE_CONTAINER_HXX_
#include "InterfaceContainer.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OGridControlModel
//==================================================================
typedef ::cppu::ImplHelper5<    ::com::sun::star::awt::XControlModel,
                                ::com::sun::star::form::XGridColumnFactory,
                                ::com::sun::star::form::XLoadListener,
                                ::com::sun::star::form::XReset,
                                ::com::sun::star::view::XSelectionSupplier> OGridControlModel_BASE;

class OGridColumn;
class OGridControlModel :public OControlModel
                        ,public OInterfaceContainer
                        ,public OAggregationArrayUsageHelper<OGridControlModel>
                            // though we don't use aggregation, we're derived from an OPropertySetAggregationHelper,
                            // which expects that we use an OPropertyArrayAggregationHelper, which we ensure
                            // with deriving from OAggregationArrayUsageHelper
                        ,public OGridControlModel_BASE
{
    ::cppu::OInterfaceContainerHelper       m_aSelectListeners,
                                            m_aResetListeners;

// [properties]
    ::com::sun::star::uno::Any              m_aRowHeight;           // Zeilenhoehe
    ::com::sun::star::uno::Any              m_aTabStop;
    ::com::sun::star::uno::Any              m_aTextColor;
    ::com::sun::star::uno::Any              m_aBackgroundColor;
    ::com::sun::star::uno::Any              m_aCursorColor;             // transient
    ::com::sun::star::awt::FontDescriptor   m_aFont;
    ::com::sun::star::awt::FontDescriptor   m_aOldFont;
    ::rtl::OUString                         m_aDefaultControl;
    ::rtl::OUString                         m_sHelpText;
// [properties]

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xSelection;
    sal_uInt32                  m_nFontEvent;

// [properties]
    ::rtl::OUString             m_sHelpURL;                 // URL
    sal_Int16                   m_nBorder;
    sal_Bool                    m_bEnable           : 1;
    sal_Bool                    m_bNavigation       : 1;
    sal_Bool                    m_bRecordMarker     : 1;
    sal_Bool                    m_bPrintable        : 1;
    sal_Bool                    m_bAlwaysShowCursor : 1;    // transient
    sal_Bool                    m_bDisplaySynchron  : 1;        // transient
// [properties]

protected:
    void _reset();

public:
    OGridControlModel(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    virtual ~OGridControlModel();

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

// starcontainer::XChild
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OGridControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XReset
    virtual void SAL_CALL reset();
    virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener);
    virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener);

// ::com::sun::star::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select(const ::com::sun::star::uno::Any& aElement) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XGridColumnFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> SAL_CALL createColumn(const ::rtl::OUString& ColumnType);
    virtual StringSequence SAL_CALL getColumnTypes();

// stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName();
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference<stario::XObjectInputStream>& _rxInStream);

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue);

// ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle );

// OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual void insert(sal_Int32 _nIndex, const InterfaceRef& _Object, sal_Bool bEvents)
                    throw(::com::sun::star::lang::IllegalArgumentException);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>  createColumn(sal_Int32 nTypeId) const;

    OGridColumn* getColumnImplementation(const InterfaceRef& _rxIFace) const;

protected:
    virtual void implRemoved(const InterfaceRef& _rxObject);
    virtual void implInserted(const InterfaceRef& _rxObject);
    virtual void implReplaced(const InterfaceRef& _rxReplacedObject, const InterfaceRef& _rxNewObject);

private:
    DECL_LINK( OnFontChanged, void* );
};

//.........................................................................
}   // namespace frm
//.........................................................................


