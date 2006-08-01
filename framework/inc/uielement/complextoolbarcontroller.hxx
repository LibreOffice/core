/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: complextoolbarcontroller.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:36:35 $
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

#ifndef __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX_

#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#include <svtools/toolboxcontroller.hxx>
#endif

#include <vcl/toolbox.hxx>

namespace framework
{

struct ExecuteInfo
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
    ::com::sun::star::util::URL                                                aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
};

struct NotifyInfo
{
    ::rtl::OUString                                                                           aEventName;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XControlNotificationListener > xNotifyListener;
    ::com::sun::star::util::URL                                                               aSourceURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >                    aInfoSeq;
};

class ToolBar;

class ComplexToolbarController : public svt::ToolboxController

{
    public:
        ComplexToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBar*     pToolBar,
                                  USHORT       nID,
                                  const rtl::OUString& aCommand );
        virtual ~ComplexToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        DECL_STATIC_LINK( ComplexToolbarController, ExecuteHdl_Impl, ExecuteInfo* );
        DECL_STATIC_LINK( ComplexToolbarController, Notify_Impl, NotifyInfo* );

    protected:
        static sal_Int32 getFontSizePixel( const Window* pWindow );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > getDispatchFromCommand( const rtl::OUString& aCommand ) const;
        void addNotifyInfo( const ::rtl::OUString&                                                        aEventName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& xDispatch,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rInfo );

        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand ) = 0;
        const ::com::sun::star::util::URL& getInitializedURL();
        void notifyFocusGet();
        void notifyFocusLost();
        void notifyTextChanged( const ::rtl::OUString& aText );

        ToolBar*                                                                    m_pToolbar;
        sal_uInt16                                                                  m_nID;
        sal_Bool                                                                    m_bMadeInvisible;
        mutable ::com::sun::star::util::URL                                         m_aURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;
};

}

#endif // __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX_
