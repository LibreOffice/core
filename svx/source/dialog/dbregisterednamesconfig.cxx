/*************************************************************************
 *
 *  $RCSfile: dbregisterednamesconfig.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:37:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "dbregisterednamesconfig.hxx"


#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#include "svxids.hrc"

#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
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
                    aThisDriverSettings = aDbRegisteredNamesRoot.openNode(sName);
                else
                    aThisDriverSettings = aDbRegisteredNamesRoot.createNode(sName);

                // set the values
                aThisDriverSettings.setNodeValue(getDbNameNodeName(), makeAny(sName));
                aThisDriverSettings.setNodeValue(getDbLocationNodeName(), makeAny(aLoop->second));
            }
            bNeedCommit = sal_True;

            // delete unused entry
            Sequence< ::rtl::OUString > aDriverKeys = aDbRegisteredNamesRoot.getNodeNames();
            const ::rtl::OUString* pDriverKeys = aDriverKeys.getConstArray();
            const ::rtl::OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
            for (;pDriverKeys != pDriverKeysEnd; ++pDriverKeys)
            {
                if ( rNewSettings.find(*pDriverKeys) == rNewSettings.end() )
                    aDbRegisteredNamesRoot.removeNode(*pDriverKeys);
            }
        }
        if (bNeedCommit)
            aDbRegisteredNamesRoot.commit();
    }

//........................................................................
}   // namespace svx
//........................................................................


