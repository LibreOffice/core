/*************************************************************************
 *
 *  $RCSfile: wrapexception.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:29:00 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_WRAPEXCEPTION_HXX
#define CONFIGMGR_WRAPEXCEPTION_HXX

#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/AuthenticationFailedException.hpp>
#include <com/sun/star/configuration/backend/InvalidAuthenticationMechanismException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>

#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/configuration/backend/ConnectionLostException.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

#define WRAP_EXCEPTION( ETyp, Raise ) \
    catch (ETyp & _e_) { Raise( uno::makeAny(_e_) ); }

#define WRAP_EXCEPTION1( ETyp, Raise, Arg ) \
    catch (ETyp & _e_) { Raise( uno::makeAny(_e_), (Arg) ); }

#define PASS_EXCEPTION( ETyp ) \
    catch (ETyp & ) { throw; }

#define WRAP_CONFIGBACKEND_CREATION_EXCEPTIONS( Raise )      \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::InsufficientAccessRightsException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::AuthenticationFailedException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::InvalidAuthenticationMechanismException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::CannotConnectException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::BackendSetupException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::BackendAccessException, Raise)     \
    WRAP_EXCEPTION(::com::sun::star::configuration::MissingBootstrapFileException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::InvalidBootstrapFileException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::InstallationIncompleteException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::CannotLoadConfigurationException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::lang::WrappedTargetException, Raise)                   \
    WRAP_EXCEPTION(::com::sun::star::lang::WrappedTargetRuntimeException, Raise)            \
    WRAP_EXCEPTION(::com::sun::star::lang::DisposedException, Raise)                        \
    WRAP_EXCEPTION(::com::sun::star::uno::RuntimeException, Raise)      \
    WRAP_EXCEPTION(::com::sun::star::uno::Exception, Raise)

#define WRAP_CONFIGBACKEND_EXCEPTIONS( Raise )      \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::InsufficientAccessRightsException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::ConnectionLostException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::BackendAccessException, Raise)     \

#define WRAP_CONFIGDATA_EXCEPTIONS( Raise )      \
    WRAP_EXCEPTION(::com::sun::star::configuration::backend::MalformedDataException, Raise)     \
    WRAP_CONFIGBACKEND_EXCEPTIONS( Raise )      \
    WRAP_EXCEPTION(::com::sun::star::lang::WrappedTargetException, Raise)                   \

#define WRAP_SAX_EXCEPTIONS( Raise )      \
    WRAP_EXCEPTION(::com::sun::star::xml::sax::SAXParseException, Raise)    \
    WRAP_EXCEPTION(::com::sun::star::xml::sax::SAXException, Raise)         \

#define WRAP_OTHER_EXCEPTIONS( Raise ) \
    WRAP_EXCEPTION(::com::sun::star::uno::Exception, Raise)

#define WRAP_CONFIGBACKEND_CREATION_EXCEPTIONS1( Raise, Arg )      \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::InsufficientAccessRightsException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::AuthenticationFailedException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::InvalidAuthenticationMechanismException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::CannotConnectException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::BackendSetupException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::BackendAccessException, Raise, Arg)     \
    WRAP_EXCEPTION1(::com::sun::star::configuration::MissingBootstrapFileException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::InvalidBootstrapFileException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::InstallationIncompleteException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::CannotLoadConfigurationException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::lang::WrappedTargetException, Raise, Arg)                   \
    WRAP_EXCEPTION1(::com::sun::star::lang::WrappedTargetRuntimeException, Raise, Arg)            \
    WRAP_EXCEPTION1(::com::sun::star::lang::DisposedException, Raise, Arg)                        \
    WRAP_EXCEPTION1(::com::sun::star::uno::RuntimeException, Raise, Arg)      \
    WRAP_EXCEPTION1(::com::sun::star::uno::Exception, Raise, Arg)

#define WRAP_CONFIGBACKEND_EXCEPTIONS1( Raise, Arg )      \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::InsufficientAccessRightsException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::ConnectionLostException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::BackendAccessException, Raise, Arg)     \

#define WRAP_CONFIGDATA_EXCEPTIONS1( Raise, Arg )      \
    WRAP_EXCEPTION1(::com::sun::star::configuration::backend::MalformedDataException, Raise, Arg)     \
    WRAP_CONFIGBACKEND_EXCEPTIONS1( Raise, Arg )      \
    WRAP_EXCEPTION1(::com::sun::star::lang::WrappedTargetException, Raise, Arg)                   \

#define WRAP_SAX_EXCEPTIONS1( Raise, Arg )      \
    WRAP_EXCEPTION1(::com::sun::star::xml::sax::SAXParseException, Raise, Arg)    \
    WRAP_EXCEPTION1(::com::sun::star::xml::sax::SAXException, Raise, Arg)         \

#define WRAP_OTHER_EXCEPTIONS1( Raise, Arg ) \
    WRAP_EXCEPTION1(::com::sun::star::uno::Exception, Raise, Arg)

#endif // CONFIGMGR_WRAPEXCEPTION_HXX
