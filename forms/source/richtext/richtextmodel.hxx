/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richtextmodel.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:36:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        sal_Int16                   m_nLineEndFormat;
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

        ::cppu::OInterfaceContainerHelper
                                    m_aModifyListeners;

    public:
        static  RichTextEngine* getEditEngine( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel );

    protected:
        DECLARE_DEFAULT_LEAF_XTOR( ORichTextModel );

        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ONavigationBarModel, OControlModel );
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        DECLARE_SERVICE_REGISTRATION( ORichTextModel )

        // XPersistObject
        DECLARE_XPERSISTOBJECT()

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
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                            sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
        ) const;
        virtual void describeAggregateProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;

        // prevent method hiding
        using OControlModel::disposing;
        using OControlModel::getFastPropertyValue;

        // OComponentHelper
        virtual void SAL_CALL disposing();

        // IEngineTextChangeListener
        virtual void potentialTextChange( );

    private:
        void    implInit();
        void    implDoAggregation();
        void    implRegisterProperties();

        /** propagates a new text to the EditEngine

            This method needs to lock the global solar mutex, so our own mutex must not
            be locked when calling.

        @precond
            our mutex is not locked
        */
        void    impl_smlock_setEngineText( const ::rtl::OUString& _rText );

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

