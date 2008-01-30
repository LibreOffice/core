/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbregisterednamesconfig.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:23:40 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include "dbregisterednamesconfig.hxx"


#include <svtools/itemset.hxx>
#include <tools/diagnose_ex.h>

#include <svx/svxids.hrc>

#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef SVX_DBREGISTERSETTING_HXX
#include "dbregistersettings.hxx"
#endif

#ifndef _OFFAPP_CONNPOOLOPTIONS_HXX_
#include "connpooloptions.hxx"
#endif

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDbRegisteredNamesNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("org.openoffice.Office.DataAccess/RegisteredNames");
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDbNameNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Name");
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getDbLocationNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Location");
        return s_sNodeName;
    }

    //====================================================================
    //= DbRegisteredNamesConfig
    //====================================================================
    //--------------------------------------------------------------------
    void DbRegisteredNamesConfig::GetOptions(SfxItemSet& _rFillItems)
    {
        // the config node where all pooling relevant info are stored under
        OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_READONLY);

        TNameLocationMap aSettings;

        // then look for which of them settings are stored in the configuration
        Sequence< ::rtl::OUString > aDriverKeys = aDbRegisteredNamesRoot.getNodeNames();
        const ::rtl::OUString* pDriverKeys = aDriverKeys.getConstArray();
        const ::rtl::OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
        for (;pDriverKeys != pDriverKeysEnd; ++pDriverKeys)
        {
            // the name of the driver in this round
            OConfigurationNode aThisDriverSettings = aDbRegisteredNamesRoot.openNode(*pDriverKeys);
            ::rtl::OUString sName, sLocation;
            aThisDriverSettings.getNodeValue(getDbNameNodeName()) >>= sName;
            aThisDriverSettings.getNodeValue(getDbLocationNodeName()) >>= sLocation;
            sLocation = SvtPathOptions().SubstituteVariable(sLocation);

            aSettings.insert(TNameLocationMap::value_type(sName,sLocation));
        }

        _rFillItems.Put(DatabaseMapItem(SID_SB_DB_REGISTER, aSettings));
    }

    //--------------------------------------------------------------------
    void DbRegisteredNamesConfig::SetOptions(const SfxItemSet& _rSourceItems)
    {
        // the config node where all pooling relevant info are stored under
        OConfigurationTreeRoot aDbRegisteredNamesRoot = OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(), getDbRegisteredNamesNodeName(), -1, OConfigurationTreeRoot::CM_UPDATABLE);

        if (!aDbRegisteredNamesRoot.isValid())
            // already asserted by the OConfigurationTreeRoot
            return;

        sal_Bool bNeedCommit = sal_False;


        // the settings for the single drivers
        SFX_ITEMSET_GET( _rSourceItems, pDriverSettings, DatabaseMapItem, SID_SB_DB_REGISTER, sal_True );
        if (pDriverSettings)
        {
            Reference< XNameAccess > xDatabaseContext = Reference< XNameAccess >(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatabaseContext"))), UNO_QUERY);
            Reference< XNamingService> xNamingService(xDatabaseContext,UNO_QUERY);
            ::rtl::OUString sName, sLocation;
            OConfigurationNode aThisDriverSettings;

            const TNameLocationMap& rNewSettings = pDriverSettings->getSettings();
            TNameLocationMap::const_iterator aEnd = rNewSettings.end();
            for (   TNameLocationMap::const_iterator aLoop = rNewSettings.begin();
                    aLoop != aEnd;
                    ++aLoop
                )
            {
                // need the name as ::rtl::OUString
                sName = aLoop->first;

                // the sub-node for this driver
                if (aDbRegisteredNamesRoot.hasByName(sName))
                {
                    aThisDriverSettings = aDbRegisteredNamesRoot.openNode(sName);
                    // set the values
                    aThisDriverSettings.setNodeValue(getDbNameNodeName(), makeAny(sName));
                    aThisDriverSettings.setNodeValue(getDbLocationNodeName(), makeAny(aLoop->second));
                    bNeedCommit = sal_True;
                }
                else
                {
                    try
                    {
                        xNamingService->registerObject(sName,Reference< ::com::sun::star::uno::XInterface >(xDatabaseContext->getByName(aLoop->second),UNO_QUERY));
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
            if (bNeedCommit)
                aDbRegisteredNamesRoot.commit();

            // delete unused entry
            Sequence< ::rtl::OUString > aDriverKeys = xDatabaseContext->getElementNames();
            const ::rtl::OUString* pDriverKeys = aDriverKeys.getConstArray();
            const ::rtl::OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
            for (;pDriverKeys != pDriverKeysEnd; ++pDriverKeys)
            {
                if ( rNewSettings.find(*pDriverKeys) == rNewSettings.end() )
                {
                    try
                    {
                        xNamingService->revokeObject(*pDriverKeys);
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
    }

//........................................................................
}   // namespace svx
//........................................................................
