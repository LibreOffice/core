/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configinteractionhandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:45:54 $
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

#ifndef CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX
#define CONFIGMGR_CONFIGINTERACTIONHANDLER_HXX

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

namespace configmgr {
namespace apihelper {
    namespace uno  = com::sun::star::uno;
    namespace task = com::sun::star::task;

/**
    represents the InteractionHandler for configuration errors from the current context.

    <p>Should only be kept in scope while the error is being handled.</p>
  */
    class ConfigurationInteractionHandler : Noncopyable
    {
        class Context;
        Context * m_pContext;
    public:
        ConfigurationInteractionHandler();
        ~ConfigurationInteractionHandler();

        bool is() const { return m_pContext != 0; }
        void handle( const uno::Reference< task::XInteractionRequest > & xRequest );
    };


} // namespace apihelper
} // namespace configmgr

#endif
