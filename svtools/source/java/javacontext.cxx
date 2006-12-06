/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javacontext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2006-12-06 14:40:23 $
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
#include "precompiled_svtools.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include "com/sun/star/uno/Any.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include "com/sun/star/uno/Type.hxx"
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include "svtdata.hxx"
#endif
#ifndef _SVTOOLS_JAVACONTEXT_HXX_
#include "javacontext.hxx"
#endif
#ifndef _SVTOOLS_JAVAINTERACTION_HXX_
#include "javainteractionhandler.hxx"
#endif



using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::task;
namespace svt
{

JavaContext::JavaContext( const Reference< XCurrentContext > & ctx )
    :
    m_aRefCount(0),
    m_xNextContext( ctx ),
    m_bShowErrorsOnce(false)
{
}

JavaContext::JavaContext( const Reference< XCurrentContext > & ctx,
                          bool bShowErrorsOnce)
    : m_aRefCount(0),
      m_xNextContext( ctx ),
      m_bShowErrorsOnce(bShowErrorsOnce)
{
}

JavaContext::~JavaContext()
{
}

Any SAL_CALL JavaContext::queryInterface(const Type& aType )
    throw (RuntimeException)
{
    if (aType == getCppuType(reinterpret_cast<Reference<XInterface>*>(0)))
        return Any(Reference<XInterface>(static_cast<XInterface*>(this)));
    else if (aType == getCppuType(reinterpret_cast<Reference<XCurrentContext>*>(0)))
        return Any(Reference<XCurrentContext>( static_cast<XCurrentContext*>(this)));
    return Any();
}

void SAL_CALL JavaContext::acquire(  ) throw ()
{
    osl_incrementInterlockedCount( &m_aRefCount );
}

void SAL_CALL JavaContext::release(  ) throw ()
{
    if (! osl_decrementInterlockedCount( &m_aRefCount ))
        delete this;
}

Any SAL_CALL JavaContext::getValueByName( const OUString& Name) throw (RuntimeException)
{
    Any retVal;

    if ( 0 == Name.compareToAscii( JAVA_INTERACTION_HANDLER_NAME ))
    {
        {
            osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
            if (!m_xHandler.is())
                m_xHandler = Reference< XInteractionHandler >(
                    new JavaInteractionHandler(m_bShowErrorsOnce));
        }
        retVal = makeAny(m_xHandler);

    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}


}
