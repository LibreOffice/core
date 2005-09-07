/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smplmailclient.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:09:05 $
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

#ifndef _SMPLMAILCLIENT_HXX_
#define _SMPLMAILCLIENT_HXX_

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_SYS_SHELL_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSimpleMailClient.hpp>
#endif

namespace css = ::com::sun::star;

class CSmplMailClient : public cppu::WeakImplHelper1<css::system::XSimpleMailClient>
{
public:
    virtual css::uno::Reference<css::system::XSimpleMailMessage> SAL_CALL createSimpleMailMessage()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL sendSimpleMailMessage(const css::uno::Reference<css::system::XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag)
        throw (css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException);

private:
    void validateParameter(const css::uno::Reference<css::system::XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag);
    void assembleCommandLine(const css::uno::Reference<css::system::XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag, std::vector<rtl::OUString>& rCommandArgs);
};

#endif
