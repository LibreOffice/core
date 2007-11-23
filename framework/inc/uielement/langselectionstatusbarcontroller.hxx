/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: langselectionstatusbarcontroller.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:46:21 $
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

#ifndef __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif
#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif
#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef _SVTOOLS_STATUSBARCONTROLLER_HXX
#include <svtools/statusbarcontroller.hxx>
#endif

// component helper namespace
namespace framework {

class LangSelectionStatusbarController : public svt::StatusbarController
{
    public:
        explicit LangSelectionStatusbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

        // XServiceInfo
        DECLARE_XSERVICEINFO

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusbarController
        virtual ::sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       ::sal_Bool bMouseEvent,
                                       const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nItemId, ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);

    private:
        virtual ~LangSelectionStatusbarController() {}
        LangSelectionStatusbarController(LangSelectionStatusbarController &); // not defined
        void operator =(LangSelectionStatusbarController &); // not defined

        sal_Int16       m_nScriptType;
        ::rtl::OUString m_aCurrentLanguage;
        ::rtl::OUString m_aCurLang;
        ::rtl::OUString m_aKeyboardLang;
        ::rtl::OUString m_aGuessedText;

        void LangMenu() throw (::com::sun::star::uno::RuntimeException);
};

} // framework namespace

#endif // __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_
