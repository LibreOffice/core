/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scopeguard.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:38:18 $
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

#if ! defined(INCLUDED_COMPHELPER_SCOPEGUARD_HXX)
#define INCLUDED_COMPHELPER_SCOPEGUARD_HXX

#if ! defined(INCLUDED_COMPHELPERDLLAPI_H)
#include "comphelper/comphelperdllapi.h"
#endif
#include "boost/function.hpp"
#include "boost/noncopyable.hpp"

namespace comphelper {

/** ScopeGuard to ease writing exception-safe code.
 */
class COMPHELPER_DLLPUBLIC ScopeGuard : private ::boost::noncopyable
                                        // noncopyable until we have
                                        // good reasons...
{
public:
    enum exc_handling { IGNORE_EXCEPTIONS, ALLOW_EXCEPTIONS };

    /** @param func function object to be executed in dtor
        @param excHandling switches whether thrown exceptions in dtor will be
                           silently ignored (but OSL_ asserted)
    */
    template <typename func_type>
    explicit ScopeGuard( func_type const & func,
                         exc_handling excHandling = IGNORE_EXCEPTIONS )
        : m_func( func ), m_excHandling( excHandling ) {}

    ~ScopeGuard();

    /** Dismisses the scope guard, i.e. the function won't
        be executed.
    */
    void dismiss();

private:
    ::boost::function0<void> m_func; // preferring portable syntax
    exc_handling const m_excHandling;
};

} // namespace comphelper

#endif // ! defined(INCLUDED_COMPHELPER_SCOPEGUARD_HXX)

