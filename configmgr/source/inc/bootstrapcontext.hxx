/*************************************************************************
 *
 *  $RCSfile: bootstrapcontext.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-06-12 16:37:43 $
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

#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#define CONFIGMGR_BOOTSTRAPCONTEXT_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _RTL_BOOTSTRAP_H_
#include <rtl/bootstrap.h>
#endif

#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include <com/sun/star/uno/XCurrentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;
    using ::rtl::OUString;
// -----------------------------------------------------------------------------
    /** Base class for a customized ComponentContext
    */
    class BootstrapContext
    : public ::cppu::WeakImplHelper1< uno::XCurrentContext >
    {
    public:
        typedef uno::Reference< uno::XComponentContext > ComponentContext;
        typedef uno::Sequence < beans::NamedValue > Arguments;
    // creation and destruction
    public:
        /** Constructs a BoottrapContext based on the given arguments and context.

            @param _aArguments
                The arguments used to create this component.
                These values will override values from the context or bootstrap data.

            @param _xContext
                The context of this component.
                Values from here will override values from bootstrap data.
        */
        explicit
        BootstrapContext(Arguments const & _aArguments, ComponentContext const & _xContext = ComponentContext());

        /// Destroys this BootstrapContext
        ~BootstrapContext();

        /// sets the INI file to use for bootstrap data
        void setBootstrapURL( const OUString& _aURL );

    // interface implementations
    public:
    // XCurrentContext
        /** Retrieves a value from this context.

            @param Name
                The name of the value to retrieve.
                A prefix of "com.sun.star.configuration.bootstrap." is stripped/ignored

            @returns
                The requested value, or <VOID/> if the value is not found.
        */
        virtual uno::Any SAL_CALL
            getValueByName( const OUString& Name )
                throw (uno::RuntimeException);

    private:
        bool lookupInArguments( uno::Any & _rValue, const OUString& _aName );
        bool lookupInContext  ( uno::Any & _rValue, const OUString& _aName );
        bool lookupInBootstrap( uno::Any & _rValue, const OUString& _aName );

        static OUString makeLongName (OUString const & _aName);
        static OUString makeShortName(OUString const & _aName);

        /// The context that some requests are delegated to
        Arguments           m_aArguments;
        ComponentContext    m_xContext;
        rtlBootstrapHandle  m_hBootstrapData;
    };
// -----------------------------------------------------------------------------

} // namespace configmgr

#endif


