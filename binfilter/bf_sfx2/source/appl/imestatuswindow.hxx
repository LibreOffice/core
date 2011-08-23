/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#if !defined INCLUDED_SFX2_APPL_IMESTATUSWINDOW_HXX
#define INCLUDED_SFX2_APPL_IMESTATUSWINDOW_HXX

#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "cppuhelper/implbase1.hxx"

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace lang { class XMultiServiceFactory; }
} } }
namespace binfilter {

class SfxApplication;

namespace sfx2 { namespace appl {

// The MS compiler needs this typedef work-around to accept the using
// declarations within ImeStatusWindow:
typedef cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener >
ImeStatusWindow_Impl;

/** Control the behavior of any (platform-dependent) IME status windows.

    The decision of whether a status window shall be displayed or not can be
    stored permanently in the configuration (under key
    org.openoffice.office.Common/I18N/InputMethod/ShowStatusWindow; if that
    entry is nil, VCL is asked for a default).
 */
class ImeStatusWindow: private ImeStatusWindow_Impl
{
public:
    ImeStatusWindow(SfxApplication & rApplication,
                    ::com::sun::star::uno::Reference<
                            ::com::sun::star::lang::XMultiServiceFactory > const &
                        rServiceFactory);

    /** Set up VCL according to the configuration.

        Is it not strictly required that this method is called exactly once
        (though that will be the typical use).

        Must only be called with the Solar mutex locked.
     */
    void init();

    // At least the Solaris "CC: Forte Developer 7 C++ 5.4 2002/03/09" compiler
    // does not accept the following using-declarations for virtual functions,
    // so use (deprecated) access declarations instead:
    /*using*/ ImeStatusWindow_Impl::acquire;
    /*using*/ ImeStatusWindow_Impl::release;
    using ImeStatusWindow_Impl::operator new;
    using ImeStatusWindow_Impl::operator delete;

private:
    ImeStatusWindow(ImeStatusWindow &); // not implemented
    void operator =(ImeStatusWindow); // not implemented

    virtual ~ImeStatusWindow();

    virtual void SAL_CALL
    disposing(::com::sun::star::lang::EventObject const & rSource)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    propertyChange(::com::sun::star::beans::PropertyChangeEvent const & rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
    getConfig();

    SfxApplication & m_rApplication;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        m_xServiceFactory;

    osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        m_xConfig;
    bool m_bDisposed;
};

} }

}//end of namespace binfilter
#endif // INCLUDED_SFX2_APPL_IMESTATUSWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
