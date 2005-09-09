/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: odma_resultset.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:54:13 $
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

// @@@ Adjust multi-include-protection-ifdef.
#ifndef ODMA_RESULTSET_HXX
#define ODMA_RESULTSET_HXX

#ifndef _UCBHELPER_RESULTSETHELPER_HXX
#include <ucbhelper/resultsethelper.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

// @@@ Adjust multi-include-protection-ifdef and header file name.
#ifndef ODMA_CONTENT_HXX
#include "odma_content.hxx"
#endif

// @@@ Adjust namespace name.
namespace odma {

class DynamicResultSet : public ::ucb::ResultSetImplHelper
{
      vos::ORef< Content > m_xContent;
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XCommandEnvironment > m_xEnv;

private:
    virtual void initStatic();
    virtual void initDynamic();

public:
    DynamicResultSet(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
              const vos::ORef< Content >& rxContent,
            const com::sun::star::ucb::OpenCommandArgument2& rCommand,
              const com::sun::star::uno::Reference<
                com::sun::star::ucb::XCommandEnvironment >& rxEnv );
};

}

#endif
