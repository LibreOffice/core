/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeload.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 15:41:00 $
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
