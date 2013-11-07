/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "precompiled_desktop.hxx"

#include "dp_gui_handleversionexception.hxx"
#include "dp_gui_dialog2.hxx"
#include "dp_version.hxx"
#include "dp_gui_shared.hxx"
#include "dp_gui.hrc"
#include <vcl/msgbox.hxx>

using namespace dp_gui;
using namespace dp_misc;
using ::rtl::OUString;


namespace {

OUString getVersion( OUString const & sVersion )
{
    return ( sVersion.getLength() == 0 ) ? OUString( RTL_CONSTASCII_USTRINGPARAM( "0" ) ) : sVersion;
}

OUString getVersion( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &rPackage )
{
    return getVersion( rPackage->getVersion());
}
}



extern "C" {


bool handleVersionException(
    com::sun::star::deployment::VersionException verExc,
    DialogHelper* pDialogHelper,
    const bool bChooseNewestVersion )
{
    bool bApprove = false;

    sal_uInt32 id;
    switch (dp_misc::compareVersions(
        verExc.NewVersion, verExc.Deployed->getVersion() ))
    {
    case dp_misc::LESS:
        id = RID_WARNINGBOX_VERSION_LESS;
        break;
    case dp_misc::EQUAL:
        id = RID_WARNINGBOX_VERSION_EQUAL;
        break;
    default: // dp_misc::GREATER
        id = RID_WARNINGBOX_VERSION_GREATER;
        break;
    }
    OSL_ASSERT( verExc.Deployed.is() );

    if ( bChooseNewestVersion )
    {
        bApprove = id == RID_WARNINGBOX_VERSION_GREATER;
    }
    else
    {
        const bool bEqualNames = verExc.NewDisplayName.equals(
            verExc.Deployed->getDisplayName());
        {
            vos::OGuard guard(Application::GetSolarMutex());
            WarningBox box( pDialogHelper ? pDialogHelper->getWindow() : NULL, ResId(id, *DeploymentGuiResMgr::get()));
            String s;
            if (bEqualNames)
            {
                s = box.GetMessText();
            }
            else if (id == RID_WARNINGBOX_VERSION_EQUAL)
            {
                //hypothetical: requires two instances of an extension with the same
                //version to have different display names. Probably the developer forgot
                //to change the version.
                s = String(ResId(RID_STR_WARNINGBOX_VERSION_EQUAL_DIFFERENT_NAMES, *DeploymentGuiResMgr::get()));
            }
            else if (id == RID_WARNINGBOX_VERSION_LESS)
            {
                s = String(ResId(RID_STR_WARNINGBOX_VERSION_LESS_DIFFERENT_NAMES, *DeploymentGuiResMgr::get()));
            }
            else if (id == RID_WARNINGBOX_VERSION_GREATER)
            {
                s = String(ResId(RID_STR_WARNINGBOX_VERSION_GREATER_DIFFERENT_NAMES, *DeploymentGuiResMgr::get()));
            }
            s.SearchAndReplaceAllAscii( "$NAME", verExc.NewDisplayName);
            s.SearchAndReplaceAllAscii( "$OLDNAME", verExc.Deployed->getDisplayName());
            s.SearchAndReplaceAllAscii( "$NEW", getVersion(verExc.NewVersion) );
            s.SearchAndReplaceAllAscii( "$DEPLOYED", getVersion(verExc.Deployed) );
            box.SetMessText(s);
            bApprove = box.Execute() == RET_OK;
        }
    }

    return bApprove;
}

} // end of extern "C"
