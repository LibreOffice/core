/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confproviderimpl2.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:12:25 $
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

#ifndef CONFIGMGR_API_PROVIDERIMPL2_HXX_
#define CONFIGMGR_API_PROVIDERIMPL2_HXX_

#ifndef CONFIGMGR_API_PROVIDERIMPL_HXX_
#include "providerimpl.hxx"
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class XInterface;
    template <class> class Sequence;
    template <class> class Reference;
}   }   }   }

namespace configmgr
{
    namespace css       = ::com::sun::star;
    namespace uno       = css::uno;
    namespace script    = css::script;

    class OConfigurationProvider;

    class OConfigurationProviderImpl : public OProviderImpl
    {
        friend class OConfigurationProvider;

    public:
        // factory methods
        uno::Reference<uno::XInterface>  createReadAccess( uno::Sequence<uno::Any> const& aArgs) CFG_UNO_THROW_ALL(  );
        uno::Reference<uno::XInterface>  createUpdateAccess( uno::Sequence<uno::Any> const& aArgs) CFG_UNO_THROW_ALL(  );
    private:
        OConfigurationProviderImpl(OConfigurationProvider* _pProvider,
                                   CreationContext const & xContext);
    };
} // namespace configmgr

#endif // CONFIGMGR_API_PROVIDERIMPL2_HXX_


