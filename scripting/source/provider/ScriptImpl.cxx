/*************************************************************************
 *
 *  $RCSfile: ScriptImpl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dfoster $ $Date: 2003-07-23 13:58:46 $
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

#include <stdio.h>

#include "ScriptImpl.hxx"
#include <util/util.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace func_provider
{

//*************************************************************************
ScriptImpl::ScriptImpl(
    const Reference< beans::XPropertySet > & scriptingContext,
    const Reference< runtime::XScriptInvocation > & runtimeMgr,
    const ::rtl::OUString& scriptURI )
throw ( RuntimeException ) :
        m_XScriptingContext( scriptingContext ),
        m_RunTimeManager( runtimeMgr ),
        m_ScriptURI( scriptURI )
{
    OSL_TRACE( "<!constucting a ScriptImpl>\n" );
    validateXRef( m_XScriptingContext,
                  "ScriptImpl::ScriptImpl: No XScriptingContext\n" );
    validateXRef( m_RunTimeManager,
                  "ScriptImpl::ScriptImpl: No XScriptInvocation\n" );
}

//*************************************************************************
ScriptImpl::~ScriptImpl()
{
    OSL_TRACE( "<Destructing a ScriptImpl>\n" );
}

//*************************************************************************
Any SAL_CALL
ScriptImpl::invoke( const Sequence< Any >& aParams,
                      Sequence< sal_Int16 >& aOutParamIndex, Sequence< Any >& aOutParam )
throw ( lang::IllegalArgumentException, script::CannotConvertException,
        reflection::InvocationTargetException, RuntimeException )
{
    OSL_TRACE( "<ScriptImpl::invoke>" );
    Any result;
    Any anyScriptingContext;

    anyScriptingContext <<= m_XScriptingContext;
    try
    {
        result = m_RunTimeManager->invoke( m_ScriptURI, anyScriptingContext, aParams,
                                           aOutParamIndex, aOutParam );
    }
    catch ( lang::IllegalArgumentException & iae )
    {
        ::rtl::OUString temp = OUSTR( "ScriptImpl::invoke IllegalArgumentException : " );
        throw lang::IllegalArgumentException( temp.concat( iae.Message ),
                                              Reference< XInterface > (),
                                              iae.ArgumentPosition );
    }
    catch ( script::CannotConvertException & cce )
    {
        ::rtl::OUString temp = OUSTR( "ScriptImpl::invoke CannotConvertException : " );
        throw script::CannotConvertException( temp.concat( cce.Message ),
                                              Reference< XInterface > (),
                                              cce.DestinationTypeClass,
                                              cce.Reason,
                                              cce.ArgumentIndex );
    }
    catch ( reflection::InvocationTargetException & ite )
    {
        ::rtl::OUString temp = OUSTR( "ScriptImpl::invoke InvocationTargetException : " );
        throw reflection::InvocationTargetException( temp.concat( ite.Message ),
                Reference< XInterface > (),
                ite.TargetException );
    }
    catch ( RuntimeException & re )
    {
        ::rtl::OUString temp = OUSTR( "ScriptImpl::invoke RuntimeException : " );
        throw RuntimeException( temp.concat( re.Message ),
                                Reference< XInterface > () );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        throw RuntimeException(
            OUSTR( "ScriptImpl::invoke Unknown Exception caught - RuntimeException rethrown" ),
            Reference< XInterface > () );
    }
#endif
    return result;
}
} // namespace func_provider
