/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: factreg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:46:41 $
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
#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif

namespace animcore {

extern rtl_StandardModuleCount g_moduleCount;

#define DECL_NODE_FACTORY(N)\
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance_##N( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rSMgr ) throw (::com::sun::star::uno::Exception);\
extern ::rtl::OUString getImplementationName_##N();\
extern ::com::sun::star::uno::Sequence< ::rtl::OUString> getSupportedServiceNames_##N(void)

DECL_NODE_FACTORY( PAR );
DECL_NODE_FACTORY( SEQ );
DECL_NODE_FACTORY( ITERATE );
DECL_NODE_FACTORY( ANIMATE );
DECL_NODE_FACTORY( SET );
DECL_NODE_FACTORY( ANIMATECOLOR );
DECL_NODE_FACTORY( ANIMATEMOTION );
DECL_NODE_FACTORY( ANIMATETRANSFORM );
DECL_NODE_FACTORY( TRANSITIONFILTER );
DECL_NODE_FACTORY( AUDIO );
DECL_NODE_FACTORY( COMMAND );
DECL_NODE_FACTORY( TargetPropertiesCreator );

}
