/*************************************************************************
 *
 *  $RCSfile: richtextmodel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 16:20:25 $
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
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTMODEL_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTMODEL_HXX

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif
#ifndef FRM_MODULE_HXX
#include "frm_module.hxx"
#endif
#ifndef FORMS_SOURCE_COMPONENT_FORMCONTROLFONT_HXX
#include "formcontrolfont.hxx"
#endif
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX
#include "richtextunowrapper.hxx"
#endif

#ifndef COMPHELPER_PROPERTYCONTAINERHELPER_HXX
#include <comphelper/propertycontainerhelper.hxx>
#endif
#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

class EditEngine;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= OModifyListenerContainer
    //====================================================================
    typedef ::comphelper::OListenerContainerBase    <   ::com::sun::star::util::XModifyListener
                                                    ,   ::com::sun::star::lang::EventObject
                                                    >   OModifyListenerContainer_Base;

    class OModifyListenerContainer : public OModifyListenerContainer_Base
    {
    public:
        OModifyListenerContainer( ::osl::Mutex& _rMutex );

    protected:
        virtual bool    implNotify(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener,
                            const ::com::sun::star::lang::EventObject& _rEvent
                        )   SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

    class RichTextEngine;
    //====================================================================
    //= ORichTextModel
    //====================================================================
    typedef ::cppu::ImplHelper3 <   ::com::sun::star::awt::XControlModel
                                ,   ::com::sun::star::lang::XUnoTunnel
                                ,   ::com::sun::star::util::XModifyBroadcaster
                                >   ORichTextModel_BASE;

    class ORichTextModel
            :public OControlModel
            ,public FontControlModel
            ,public IEngineTextChangeListener
            ,public ::comphelper::OPropertyContainerHelper
            ,public OAggregationArrayUsageHelper< ORichTextModel >
            ,public ORichTextModel_BASE
    {
    private:
        // <properties>
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >
                                    m_xReferenceDevice;
        ::com::sun::star::uno::Any  m_aTabStop;
        ::com::sun::star::uno::Any  m_aBackgroundColor;
        ::com::sun::star::uno::Any  m_aBorderColor;
        ::rtl::OUString             m_sDefaultControl;
        ::rtl::OUString             m_sHelpText;
        ::rtl::OUString             m_sHelpURL;
        ::rtl::OUString             m_sLastKnownEngineText;
        sal_Int16                   m_nBorder;
        sal_Bool                    m_bEnabled;
        sal_Bool                    m_bHardLineBreaks;
        sal_Bool                    m_bHScroll;
        sal_Bool                    m_bVScroll;
        sal_Bool                    m_bReadonly;
        sal_Bool                    m_bPrintable;
        sal_Bool                    m_bReallyActAsRichText; // despite the class name, the RichTextControl later on
                                                            // will create "ordinary" text peers depending on this property
        sal_Bool                    m_bHideInactiveSelection;
        // </properties>

        // <properties_for_awt_edit_compatibility>
        ::com::sun::star::uno::Any  m_aAlign;
        sal_Int16                   m_nEchoChar;
        sal_Int16                   m_nMaxTextLength;
        sal_Bool                    m_bMultiLine;
        // </properties_for_awt_edit_compatibility>

        RichTextEngine*             m_pEngine;
        bool                        m_bSettingEngineText;

        OModifyListenerContainer    m_aModifyListeners;

    public:
        static  RichTextEngine* getEditEngine( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel );

    protected:
        DECLARE_DEFAULT_LEAF_XTOR( ORichTextModel );

        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarModel, OControlModel );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        DECLARE_SERVICE_REGISTRATION( ORichTextModel );

        // XPersistObject
        DECLARE_XPERSISTOBJECT();

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XCloneable
        DECLARE_XCLONEABLE();

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertySet and friends
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                            sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

        // OAggregationArrayUsageHelper
        virtual void fillProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
            ) const;
        IMPLEMENT_INFO_SERVICE()

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // IEngineTextChangeListener
        virtual void potentialTextChange( );

    private:
        void    implInit();
        void    implDoAggregation();
        void    implRegisterProperties();
        void    implSetEngineText( const ::rtl::OUString& _rText );

        DECL_LINK( OnEngineContentModified, void* );

        static  ::com::sun::star::uno::Sequence< sal_Int8 > getEditEngineTunnelId();

    private:
        ORichTextModel();                                   // never implemented
        ORichTextModel( const ORichTextModel& );            // never implemented
        ORichTextModel& operator=( const ORichTextModel& ); // never implemented
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTMODEL_HXX

