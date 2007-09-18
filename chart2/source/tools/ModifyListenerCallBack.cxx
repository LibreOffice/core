/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModifyListenerCallBack.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:09:12 $
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
#include "precompiled_chart2.hxx"

#include "ModifyListenerCallBack.hxx"
#include "MutexContainer.hxx"

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart {

typedef ::cppu::WeakComponentImplHelper1<
        ::com::sun::star::util::XModifyListener >
    ModifyListenerCallBack_Base;

class ModifyListenerCallBack_impl
    : public ::chart::MutexContainer
    , public ModifyListenerCallBack_Base
{
public:
    explicit ModifyListenerCallBack_impl( const Link& rCallBack );
    virtual ~ModifyListenerCallBack_impl();

    void startListening( const Reference< util::XModifyBroadcaster >& xBroadcaster );
    void stopListening();

    //XModifyListener
    virtual void SAL_CALL modified( const lang::EventObject& aEvent ) throw (uno::RuntimeException);

    //XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) throw (uno::RuntimeException);

    using ::cppu::WeakComponentImplHelperBase::disposing;

private:
    Link m_aLink;//will be callef on modify
    Reference< util::XModifyBroadcaster > m_xBroadcaster;//broadcaster to listen at
};


ModifyListenerCallBack_impl::ModifyListenerCallBack_impl( const Link& rCallBack )
                        : ModifyListenerCallBack_Base( m_aMutex )
                        , m_aLink( rCallBack )
                        , m_xBroadcaster(0)
{
}

ModifyListenerCallBack_impl::~ModifyListenerCallBack_impl()
{
}

//XModifyListener
void SAL_CALL ModifyListenerCallBack_impl::modified( const lang::EventObject& /*aEvent*/ ) throw (uno::RuntimeException)
{
    m_aLink.Call(0);
}

//XEventListener
void SAL_CALL ModifyListenerCallBack_impl::disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
{
    m_xBroadcaster.clear();
}

void ModifyListenerCallBack_impl::startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster )
{
    if( m_xBroadcaster == xBroadcaster )
        return;

    stopListening();
    m_xBroadcaster = xBroadcaster;
    if( m_xBroadcaster.is() )
        m_xBroadcaster->addModifyListener( this );
}
void ModifyListenerCallBack_impl::stopListening()
{
    if( m_xBroadcaster.is() )
    {
        m_xBroadcaster->removeModifyListener( this );
        m_xBroadcaster.clear();
    }
}

//-------------------------------------------

ModifyListenerCallBack::ModifyListenerCallBack( const Link& rCallBack )
                        : pModifyListener_impl( new ModifyListenerCallBack_impl(rCallBack) )
                        , m_xModifyListener( pModifyListener_impl )
{
}

ModifyListenerCallBack::~ModifyListenerCallBack()
{
    stopListening();
}

void ModifyListenerCallBack::startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster >& xBroadcaster )
{
    pModifyListener_impl->startListening( xBroadcaster );
}
void ModifyListenerCallBack::stopListening()
{
    pModifyListener_impl->stopListening();
}

} // namespace chart

