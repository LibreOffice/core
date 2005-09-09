/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorcache.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:28:39 $
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

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_
#define __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_

//__________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//__________________________________________
// interface includes

#ifndef __COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif

//__________________________________________
// other includes

#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    @short  implements a cache for any accelerator configuration.

    @descr  Its implemented threadsafe, supports copy-on-write pattern
            and a flush mechansim to support concurrent access to the same
            configuration.

            copy-on-write ... How? Do the following:
 */
class AcceleratorCache : public ThreadHelpBase // attention! Must be the first base class to guarentee right initialize lock ...
{
    //______________________________________
    // const, types

    public:

        //---------------------------------------
        /** TODO document me
            commands -> keys
        */
        typedef ::comphelper::SequenceAsVector< css::awt::KeyEvent > TKeyList;
        typedef BaseHash< TKeyList > TCommand2Keys;

        //---------------------------------------
        /** TODO document me
            keys -> commands
        */
        typedef ::std::hash_map< css::awt::KeyEvent ,
                                 ::rtl::OUString    ,
                                 KeyEventHashCode   ,
                                 KeyEventEqualsFunc > TKey2Commands;

    //______________________________________
    // member

    private:

        //---------------------------------------
        /** map commands to keys in relation 1:n.
            First key is interpreted as preferred one! */
        TCommand2Keys m_lCommand2Keys;

        //---------------------------------------
        /** map keys to commands in relation 1:1. */
        TKey2Commands m_lKey2Commands;

    //______________________________________
    // interface

    public:

        //---------------------------------------
        /** @short  creates a new - but empty - cache instance. */
        AcceleratorCache();

        //---------------------------------------
        /** @short  make a copy of this cache.
            @descr  Used for the copy-on-write feature.
        */
        AcceleratorCache(const AcceleratorCache& rCopy);

        //---------------------------------------
        /** @short  does nothing real. */
        virtual ~AcceleratorCache();

        //---------------------------------------
        /** @short  write changes back to the original container.

            @param  rCopy
                    the (changed!) copy, which should be written
                    back to this original container.
          */
        virtual void takeOver(const AcceleratorCache& rCopy);

        //---------------------------------------
        /** TODO document me */
        virtual AcceleratorCache& operator=(const AcceleratorCache& rCopy);

        //---------------------------------------
        /** @short  checks if the specified key exists.

            @param  aKey
                    the key, which should be checked.

            @return [bool]
                    TRUE if the speicfied key exists inside this container.
         */
        virtual sal_Bool hasKey(const css::awt::KeyEvent& aKey) const;
        virtual sal_Bool hasCommand(const ::rtl::OUString& sCommand) const;

        //---------------------------------------
        /** TODO document me */
        virtual TKeyList getAllKeys() const;

        //---------------------------------------
        /** @short  add a new or change an existing key-command pair
                    of this container.

            @param  aKey
                    describe the key.

            @param  sCommand
                    describe the command.
          */
        virtual void setKeyCommandPair(const css::awt::KeyEvent& aKey    ,
                                       const ::rtl::OUString&    sCommand);

        //---------------------------------------
        /** @short  returns the list of keys, which are registered
                    for this command.

            @param  sCommand
                    describe the command.

            @return [TKeyList]
                    the list of registered keys. Can be empty!
          */
        virtual TKeyList getKeysByCommand(const ::rtl::OUString& sCommand) const;

        //---------------------------------------
        /** TODO */
        virtual ::rtl::OUString getCommandByKey(const css::awt::KeyEvent& aKey) const;

        //---------------------------------------
        /** TODO */
        virtual void removeKey(const css::awt::KeyEvent& aKey);
        virtual void removeCommand(const ::rtl::OUString& sCommand);
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_
