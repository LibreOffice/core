/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: langselectionstatusbarcontroller.cxx,v $
 * $Revision: 1.6.40.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <helper/mischelper.hxx>


using namespace ::com::sun::star;

namespace framework
{

uno::Reference< linguistic2::XLanguageGuessing > LanguageGuessingHelper::GetGuesser() const
{
    if (!m_xLanguageGuesser.is())
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xMgr ( comphelper::getProcessServiceFactory() );
            if (xMgr.is())
            {
                m_xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                        xMgr->createInstance(
                            rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                            uno::UNO_QUERY );
            }
        }
        catch (uno::Exception &r)
        {
            (void) r;
            DBG_ASSERT( 0, "failed to get language guessing component" );
        }    
    }
    return m_xLanguageGuesser;
}
    
} // namespace framework

