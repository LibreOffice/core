/*************************************************************************
 *
 *  $RCSfile: Grid.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _UNOTOOLS_PROPERTY_ARRAY_HELPER_HXX_
#include <unotools/proparrhlp.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    namespace starview  = ::com::sun::star::view;

//==================================================================
// OGridControlModel
//==================================================================
class OGridColumn;
class OGridControlModel :public OControlModel
                        ,public OInterfaceContainer
                        ,public OAggregationArrayUsageHelper<OGridControlModel>
                            // though we don't use aggregation, we're derived from an OPropertySetAggregationHelper,
                            // which expects that we use an OPropertyArrayAggregationHelper, which we ensure
                            // with deriving from OAggregationArrayUsageHelper
                        ,public starawt::XControlModel
                        ,public starform::XGridColumnFactory
                        ,public starform::XLoadListener
                        ,public starform::XReset
                        ,public starview::XSelectionSupplier
{
    ::cppu::OInterfaceContainerHelper
                        m_aSelectListeners,
                        m_aResetListeners;

    staruno::Reference<starbeans::XPropertySet>         m_xSelection;

    sal_uInt32              m_nFontEvent;

// [properties]
    ::rtl::OUString             m_aDefaultControl;
    staruno::Any                m_aRowHeight;           // Zeilenhoehe
    staruno::Any                m_aTabStop;
    staruno::Any                m_aTextColor;
    staruno::Any                m_aBackgroundColor;
    starawt::FontDescriptor     m_aFont;
    starawt::FontDescriptor     m_aOldFont;
    sal_Int16                   m_nBorder;
    sal_Bool                    m_bEnable : 1;
    sal_Bool                    m_bNavigation : 1;
    sal_Bool                    m_bRecordMarker : 1;
    sal_Bool                    m_bPrintable : 1;
    sal_Bool                    m_bAlwaysShowCursor : 1;    // transient
    sal_Bool                    m_bDisplaySynchron : 1;     // transient
    staruno::Any                m_aCursorColor;             // transient
    ::rtl::OUString             m_sHelpURL;                 // URL
// [properties]

protected:
    virtual staruno::Sequence<staruno::Type> _getTypes();
    void _reset();

public:
    OGridControlModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory);

    virtual ~OGridControlModel();

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OControlModel);
    virtual staruno::Any SAL_CALL queryAggregation( const staruno::Type& _rType ) throw (staruno::RuntimeException);

// starcontainer::XChild
    virtual void SAL_CALL setParent(const InterfaceRef& Parent) throw(starlang::NoSupportException, staruno::RuntimeException);

// starlang::XServiceInfo
    IMPLEMENTATION_NAME(OGridControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(staruno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// starlang::XEventListener
    virtual void SAL_CALL disposing(const starlang::EventObject& _rSource) throw(staruno::RuntimeException);

// starform::XLoadListener
    virtual void SAL_CALL loaded(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL unloaded(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL unloading(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL reloading(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);
    virtual void SAL_CALL reloaded(const starlang::EventObject& aEvent) throw(staruno::RuntimeException);

// starform::XReset
    virtual void SAL_CALL reset();
    virtual void SAL_CALL addResetListener(const staruno::Reference<starform::XResetListener>& _rxListener);
    virtual void SAL_CALL removeResetListener(const staruno::Reference<starform::XResetListener>& _rxListener);

// ::com::sun::star::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select(const staruno::Any& aElement) throw(starlang::IllegalArgumentException, staruno::RuntimeException);
    virtual staruno::Any SAL_CALL getSelection() throw(staruno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener(const staruno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(staruno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener(const staruno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener) throw(staruno::RuntimeException);

// starform::XGridColumnFactory
    virtual staruno::Reference<starbeans::XPropertySet> SAL_CALL createColumn(const ::rtl::OUString& ColumnType);
    virtual StringSequence SAL_CALL getColumnTypes();

// stario::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName();
    virtual void SAL_CALL write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream);
    virtual void SAL_CALL read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream);

// starbeans::XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual void SAL_CALL getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue,
                                          sal_Int32 nHandle, const staruno::Any& rValue )
                                        throw(starlang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue);

// starbeans::XPropertyState
    virtual starbeans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual staruno::Any getPropertyDefaultByHandle( sal_Int32 nHandle );

// OAggregationArrayUsageHelper
    virtual void fillProperties(
        staruno::Sequence< starbeans::Property >& /* [out] */ _rProps,
        staruno::Sequence< starbeans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual void insert(sal_Int32 _nIndex, const InterfaceRef& _Object, sal_Bool bEvents)
                    throw(starlang::IllegalArgumentException);

    staruno::Reference<starbeans::XPropertySet>  createColumn(sal_Int32 nTypeId) const;

    OGridColumn* getColumnImplementation(const InterfaceRef& _rxIFace) const;

private:
    DECL_LINK( OnFontChanged, void* );
};

//.........................................................................
}   // namespace frm
//.........................................................................


