/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorcache.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 13:44:52 $
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
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_
#include <accelerators/acceleratorcache.hxx>
#endif

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
#include <xml/acceleratorconfigurationreader.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

//_______________________________________________
// interface includes

#ifndef __COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif

#ifndef __COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

namespace framework
{

//-----------------------------------------------
AcceleratorCache::AcceleratorCache()
    : ThreadHelpBase(&Application::GetSolarMutex())
{
}

//-----------------------------------------------
AcceleratorCache::AcceleratorCache(const AcceleratorCache& rCopy)
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    m_lCommand2Keys = rCopy.m_lCommand2Keys;
    m_lKey2Commands = rCopy.m_lKey2Commands;
}

//-----------------------------------------------
AcceleratorCache::~AcceleratorCache()
{
    // Dont save anything automaticly here.
    // The user has to do that explicitly!
}

//-----------------------------------------------
void AcceleratorCache::takeOver(const AcceleratorCache& rCopy)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    m_lCommand2Keys = rCopy.m_lCommand2Keys;
    m_lKey2Commands = rCopy.m_lKey2Commands;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache& AcceleratorCache::operator=(const AcceleratorCache& rCopy)
{
    takeOver(rCopy);
    return *this;
}

//-----------------------------------------------
sal_Bool AcceleratorCache::hasKey(const css::awt::KeyEvent& aKey) const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    return (m_lKey2Commands.find(aKey) != m_lKey2Commands.end());
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
sal_Bool AcceleratorCache::hasCommand(const ::rtl::OUString& sCommand) const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    return (m_lCommand2Keys.find(sCommand) != m_lCommand2Keys.end());
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache::TKeyList AcceleratorCache::getAllKeys() const
{
    TKeyList lKeys;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    TKey2Commands::const_iterator pIt;
    for (  pIt  = m_lKey2Commands.begin();
           pIt != m_lKey2Commands.end()  ;
         ++pIt                           )
    {
        lKeys.push_back(pIt->first);
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lKeys;
}

//-----------------------------------------------
void AcceleratorCache::setKeyCommandPair(const css::awt::KeyEvent& aKey    ,
                                         const ::rtl::OUString&    sCommand)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // register command for the specified key
    m_lKey2Commands[aKey] = sCommand;

    // update optimized structure to bind multiple keys to one command
    TKeyList& rKeyList = m_lCommand2Keys[sCommand];
    rKeyList.push_back(aKey);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache::TKeyList AcceleratorCache::getKeysByCommand(const ::rtl::OUString& sCommand) const
{
    TKeyList lKeys;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    TCommand2Keys::const_iterator pCommand = m_lCommand2Keys.find(sCommand);
    if (pCommand == m_lCommand2Keys.end())
        throw css::container::NoSuchElementException(
                ::rtl::OUString(), css::uno::Reference< css::uno::XInterface >());
    lKeys = pCommand->second;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lKeys;
}

//-----------------------------------------------
::rtl::OUString AcceleratorCache::getCommandByKey(const css::awt::KeyEvent& aKey) const
{
    ::rtl::OUString sCommand;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        throw css::container::NoSuchElementException(
                ::rtl::OUString(), css::uno::Reference< css::uno::XInterface >());
    sCommand = pKey->second;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return sCommand;
}

//-----------------------------------------------
void AcceleratorCache::removeKey(const css::awt::KeyEvent& aKey)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // check if key exists
    TKey2Commands::const_iterator pKey = m_lKey2Commands.find(aKey);
    if (pKey == m_lKey2Commands.end())
        return;

    // get its registered command
    // Because we must know its place inside the optimized
    // structure, which bind keys to commands, too!
    ::rtl::OUString sCommand = pKey->second;
    pKey = m_lKey2Commands.end(); // nobody should use an undefined value .-)

    // remove key from primary list
    m_lKey2Commands.erase(aKey);

    // remove key from optimized command list
    m_lCommand2Keys.erase(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AcceleratorCache::removeCommand(const ::rtl::OUString& sCommand)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    const TKeyList&                            lKeys = getKeysByCommand(sCommand);
    AcceleratorCache::TKeyList::const_iterator pKey ;
    for (  pKey  = lKeys.begin();
           pKey != lKeys.end()  ;
         ++pKey                 )
    {
        const css::awt::KeyEvent& rKey = *pKey;
        removeKey(rKey);
    }
    m_lCommand2Keys.erase(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

} // namespace framework
