/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confproviderimpl2.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_API_PROVIDERIMPL2_HXX_
#define CONFIGMGR_API_PROVIDERIMPL2_HXX_

#include "providerimpl.hxx"

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


