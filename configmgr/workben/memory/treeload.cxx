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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <localsession.hxx>
#include <treecache.hxx>
#include <options.hxx>
#include <rtl/ustring.hxx>
#include "treeload.hxx"

// -----------------------------------------------------------------------------
namespace configmgr
{
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;

#define ASCII(x) ::rtl::OUString::createFromAscii(x)

// -----------------------------------------------------------------------------
// ------------------------- requestSubtree without API -------------------------
// -----------------------------------------------------------------------------

    OTreeLoad::OTreeLoad(uno::Reference<lang::XMultiServiceFactory> const& _xServiceProvider,
                         rtl::OUString const& _sSourceDirectory, rtl::OUString const& _sUpdateDirectory) throw (uno::Exception)
            :m_xServiceProvider(_xServiceProvider)
{
    // Create a TypeConverter
    uno::Reference<script::XTypeConverter>      xConverter;
    xConverter = xConverter.query(m_xServiceProvider->createInstance(ASCII( "com.sun.star.script.Converter" )) );

    m_xDefaultOptions = new OOptions(xConverter);
    m_xDefaultOptions->setNoCache(true);

    // create it .. and connect
    std::auto_ptr<LocalSession> pLocal( new LocalSession(m_xServiceProvider) );
    sal_Bool bOpen = pLocal->open(_sSourceDirectory, _sUpdateDirectory);

    IConfigSession* pConfigSession = pLocal.release();

    m_pTreeMgr = new TreeManager(pConfigSession, m_xDefaultOptions);
}
// -----------------------------------------------------------------------------
ISubtree* OTreeLoad::requestSubtree( OUString const& aSubtreePath) throw (uno::Exception)
{
    return m_pTreeMgr->requestSubtree(aSubtreePath, m_xDefaultOptions, /* MinLevel */ -1);
}
// -----------------------------------------------------------------------------
void OTreeLoad::releaseSubtree( OUString const& aSubtreePath) throw (uno::Exception)
{
    m_pTreeMgr->releaseSubtree(aSubtreePath, m_xDefaultOptions);
}

// -----------------------------------------------------------------------------
} // namespace
