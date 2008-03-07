/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_updatedialog.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:36:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>


#include "boost/optional.hpp"
#include "com/sun/star/awt/Rectangle.hpp"
#include "com/sun/star/awt/WindowAttribute.hpp"
#include "com/sun/star/awt/WindowClass.hpp"
#include "com/sun/star/awt/WindowDescriptor.hpp"
#include "com/sun/star/awt/XThrobber.hpp"
#include "com/sun/star/awt/XToolkit.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/system/XSystemShellExecute.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/task/XJob.hpp"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/URL.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "svtools/svlbitm.hxx"
#include "svtools/svlbox.hxx"
#include <svtools/controldims.hrc>
#include "svx/checklbx.hxx"
#include "tools/gen.hxx"
#include "tools/link.hxx"
#include "tools/resid.hxx"
#include "tools/resmgr.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
#include "vcl/button.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/image.hxx"
#include "vcl/msgbox.hxx"
#include "vcl/svapp.hxx"
#include "vos/mutex.hxx"

#include "comphelper/processfactory.hxx"

#include "dp_dependencies.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_identifier.hxx"
#include "dp_version.hxx"
#include "dp_misc.h"

#include "dp_gui.h"
#include "dp_gui.hrc"
#include "dp_gui_thread.hxx"
#include "dp_gui_updatedata.hxx"
#include "dp_gui_updatedialog.hxx"
#include "dp_gui_shared.hxx"
#include "dp_gui_system.hxx"

class KeyEvent;
class MouseEvent;
class Window;
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace css = ::com::sun::star;

using dp_gui::UpdateDialog;

namespace {

static sal_Unicode const LF = 0x000A;
static sal_Unicode const CR = 0x000D;

enum Kind { ENABLED_UPDATE, DISABLED_UPDATE, GENERAL_ERROR, SPECIFIC_ERROR };

rtl::OUString confineToParagraph(rtl::OUString const & text) {
    // Confine arbitrary text to a single paragraph in a dp_gui::AutoScrollEdit.
    // This assumes that U+000A and U+000D are the only paragraph separators in
    // a dp_gui::AutoScrollEdit, and that replacing them with a single space
    // each is acceptable:
    return text.replace(LF, ' ').replace(CR, ' ');
}
}

struct UpdateDialog::DisabledUpdate {
    rtl::OUString name;
    css::uno::Sequence< rtl::OUString > unsatisfiedDependencies;
    bool permission;
    // We also want to show release notes and publisher for disabled updates
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > aUpdateInfo;
};

struct UpdateDialog::SpecificError {
    rtl::OUString name;
    rtl::OUString message;
};

union UpdateDialog::IndexUnion{
    std::vector< dp_gui::UpdateData >::size_type enabledUpdate;
    std::vector< UpdateDialog::DisabledUpdate >::size_type disabledUpdate;
    std::vector< rtl::OUString >::size_type generalError;
    std::vector< UpdateDialog::SpecificError >::size_type specificError;
};

struct UpdateDialog::Index {
    static std::auto_ptr< UpdateDialog::Index const > newEnabledUpdate(
        std::vector< dp_gui::UpdateData >::size_type n);

    static std::auto_ptr< UpdateDialog::Index const > newDisabledUpdate(
        std::vector< UpdateDialog::DisabledUpdate >::size_type n);

    static std::auto_ptr< UpdateDialog::Index const > newGeneralError(
        std::vector< rtl::OUString >::size_type n);

    static std::auto_ptr< UpdateDialog::Index const > newSpecificError(
        std::vector< UpdateDialog::SpecificError >::size_type n);

    Kind kind;
    IndexUnion index;

private:
    explicit Index(Kind theKind);
};

std::auto_ptr< UpdateDialog::Index const >
UpdateDialog::Index::newEnabledUpdate(
    std::vector< dp_gui::UpdateData >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(ENABLED_UPDATE);
    p->index.enabledUpdate = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

std::auto_ptr< UpdateDialog::Index const >
UpdateDialog::Index::newDisabledUpdate(
    std::vector< UpdateDialog::DisabledUpdate >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(DISABLED_UPDATE);
    p->index.disabledUpdate = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

std::auto_ptr< UpdateDialog::Index const > UpdateDialog::Index::newGeneralError(
    std::vector< rtl::OUString >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(GENERAL_ERROR);
    p->index.generalError = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

std::auto_ptr< UpdateDialog::Index const >
UpdateDialog::Index::newSpecificError(
    std::vector< UpdateDialog::SpecificError >::size_type n)
{
    UpdateDialog::Index * p = new UpdateDialog::Index(SPECIFIC_ERROR);
    p->index.specificError = n;
    return std::auto_ptr< UpdateDialog::Index const >(p);
}

UpdateDialog::Index::Index(Kind theKind): kind(theKind) {}

class UpdateDialog::Thread: public dp_gui::Thread {
public:
    Thread(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        UpdateDialog & dialog,
        rtl::Reference< dp_gui::SelectedPackageIterator > const &
            selectedPackages,
        css::uno::Sequence< css::uno::Reference<
            css::deployment::XPackageManager > > const & packageManagers);

    void stop();

private:
    Thread(UpdateDialog::Thread &); // not defined
    void operator =(UpdateDialog::Thread &); // not defined

    struct Entry {
        explicit Entry(
            css::uno::Reference< css::deployment::XPackage > const & thePackage,
            css::uno::Reference< css::deployment::XPackageManager > const &
                thePackageManager,
            rtl::OUString const & theVersion);

        css::uno::Reference< css::deployment::XPackage > package;
        css::uno::Reference< css::deployment::XPackageManager > packageManager;
        rtl::OUString version;
        css::uno::Reference< css::xml::dom::XNode > info;
    };

    // A multimap in case an extension is installed in both "user" and "shared":
    typedef std::multimap< rtl::OUString, Entry > Map;

    virtual ~Thread();

    virtual void execute();

    void handleGeneralError(css::uno::Any const & exception) const;

    void handleSpecificError(
        css::uno::Reference< css::deployment::XPackage > const & package,
        css::uno::Any const & exception) const;

    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
    getUpdateInformation(
        css::uno::Reference< css::deployment::XPackage > const & package,
        css::uno::Sequence< rtl::OUString > const & urls,
        rtl::OUString const & identifier) const;

    void handle(
        css::uno::Reference< css::deployment::XPackage > const & package,
        css::uno::Reference< css::deployment::XPackageManager > const &
            packageManager,
        Map * map);

    bool update(
        css::uno::Reference< css::deployment::XPackage > const & package,
        css::uno::Reference< css::deployment::XPackageManager > const &
            packageManager,
        css::uno::Reference< css::xml::dom::XNode > const & updateInfo) const;

    css::uno::Reference< css::uno::XComponentContext > m_context;
    UpdateDialog & m_dialog;
    rtl::Reference< dp_gui::SelectedPackageIterator > m_selectedPackages;
    css::uno::Sequence< css::uno::Reference<
        css::deployment::XPackageManager > > m_packageManagers;
    css::uno::Reference< css::deployment::XUpdateInformationProvider >
        m_updateInformation;

    // guarded by Application::GetSolarMutex():
    css::uno::Reference< css::task::XAbortChannel > m_abort;
    bool m_stop;
};

UpdateDialog::Thread::Thread(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    UpdateDialog & dialog,
    rtl::Reference< dp_gui::SelectedPackageIterator > const & selectedPackages,
    css::uno::Sequence< css::uno::Reference<
        css::deployment::XPackageManager > > const & packageManagers):
    m_context(context),
    m_dialog(dialog),
    m_selectedPackages(selectedPackages),
    m_packageManagers(packageManagers),
    m_updateInformation(
        css::deployment::UpdateInformationProvider::create(context)),
    m_stop(false)
{}

void UpdateDialog::Thread::stop() {
    css::uno::Reference< css::task::XAbortChannel > abort;
    {
        vos::OGuard g(Application::GetSolarMutex());
        abort = m_abort;
        m_stop = true;
    }
    if (abort.is()) {
        abort->sendAbort();
    }
    m_updateInformation->cancel();
}

UpdateDialog::Thread::Entry::Entry(
    css::uno::Reference< css::deployment::XPackage > const & thePackage,
    css::uno::Reference< css::deployment::XPackageManager > const &
    thePackageManager,
    rtl::OUString const & theVersion):
    package(thePackage),
    packageManager(thePackageManager),
    version(theVersion)
{}

UpdateDialog::Thread::~Thread() {}

void UpdateDialog::Thread::execute() {
    OSL_ASSERT(m_selectedPackages.is() != (m_packageManagers.getLength() != 0));
    Map map;
    if (m_selectedPackages.is()) {
        for (;;) {
            css::uno::Reference< css::deployment::XPackage > p;
            css::uno::Reference< css::deployment::XPackageManager > m;
            {
                vos::OGuard g(Application::GetSolarMutex());
                if (m_stop) {
                    return;
                }
                m_selectedPackages->next(&p, &m);
            }
            if (!p.is()) {
                break;
            }
            handle(p, m, &map);
        }
    } else {
        for (sal_Int32 i = 0; i < m_packageManagers.getLength(); ++i) {
            css::uno::Reference< css::task::XAbortChannel > abort(
                m_packageManagers[i]->createAbortChannel());
            {
                vos::OGuard g(Application::GetSolarMutex());
                if (m_stop) {
                    return;
                }
                m_abort = abort;
            }
            css::uno::Sequence<
                css::uno::Reference< css::deployment::XPackage > > ps;
            try {
                ps = m_packageManagers[i]->getDeployedPackages(
                    abort,
                    css::uno::Reference< css::ucb::XCommandEnvironment >());
            } catch (css::deployment::DeploymentException & e) {
                handleGeneralError(e.Cause);
                continue;
            } catch (css::ucb::CommandFailedException & e) {
                handleGeneralError(e.Reason);
                continue;
            } catch (css::ucb::CommandAbortedException &) {
                return;
            } catch (css::lang::IllegalArgumentException & e) {
                throw css::uno::RuntimeException(e.Message, e.Context);
            }
            for (sal_Int32 j = 0; j < ps.getLength(); ++j) {
                {
                    vos::OGuard g(Application::GetSolarMutex());
                    if (m_stop) {
                        return;
                    }
                }
                handle(ps[j], m_packageManagers[i], &map);
            }
        }
    }
    if (!map.empty()) {
        const rtl::OUString sDefaultURL(dp_misc::getExtensionDefaultUpdateURL());
        if (sDefaultURL.getLength())
        {
            css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
                infos(
                    getUpdateInformation(
                        css::uno::Reference< css::deployment::XPackage >(),
                        css::uno::Sequence< rtl::OUString >(&sDefaultURL, 1), rtl::OUString()));
            for (sal_Int32 i = 0; i < infos.getLength(); ++i) {
                css::uno::Reference< css::xml::dom::XNode > node(
                    infos[i], css::uno::UNO_QUERY_THROW);
                dp_misc::DescriptionInfoset infoset(m_context, node);
                boost::optional< rtl::OUString > id(infoset.getIdentifier());
                if (!id) {
                    continue;
                }
                Map::iterator end(map.upper_bound(*id));
                for (Map::iterator j(map.lower_bound(*id)); j != end; ++j) {
                    rtl::OUString v(infoset.getVersion());
                    if (dp_misc::compareVersions(v, j->second.version) ==
                        dp_misc::GREATER)
                    {
                        j->second.version = v;
                        j->second.info = node;
                    }
                }
            }
            for (Map::const_iterator i(map.begin()); i != map.end(); ++i) {
                if (i->second.info.is() &&
                    !update(
                        i->second.package, i->second.packageManager,
                        i->second.info))
                {
                    break;
                }
            }
        }
    }
    vos::OGuard g(Application::GetSolarMutex());
    if (!m_stop) {
        m_dialog.checkingDone();
    }
}

void UpdateDialog::Thread::handleGeneralError(css::uno::Any const & exception)
    const
{
    rtl::OUString message;
    css::uno::Exception e;
    if (exception >>= e) {
        message = e.Message;
    }
    vos::OGuard g(Application::GetSolarMutex());
    if (!m_stop) {
        m_dialog.addGeneralError(message);
    }
}

//Parameter package can be null
void UpdateDialog::Thread::handleSpecificError(
    css::uno::Reference< css::deployment::XPackage > const & package,
    css::uno::Any const & exception) const
{
    UpdateDialog::SpecificError data;
    if (package.is())
        data.name = package->getDisplayName();
    css::uno::Exception e;
    if (exception >>= e) {
        data.message = e.Message;
    }
    vos::OGuard g(Application::GetSolarMutex());
    if (!m_stop) {
        m_dialog.addSpecificError(data);
    }
}

css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
UpdateDialog::Thread::getUpdateInformation(
    css::uno::Reference< css::deployment::XPackage > const & package,
    css::uno::Sequence< rtl::OUString > const & urls,
    rtl::OUString const & identifier) const
{
    try {
        return m_updateInformation->getUpdateInformation(urls, identifier);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::ucb::CommandFailedException & e) {
        handleSpecificError(package, e.Reason);
    } catch (css::ucb::CommandAbortedException &) {
    } catch (css::uno::Exception & e) {
        handleSpecificError(package, css::uno::makeAny(e));
    }
    return
        css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >();
}

void UpdateDialog::Thread::handle(
    css::uno::Reference< css::deployment::XPackage > const & package,
    css::uno::Reference< css::deployment::XPackageManager > const &
        packageManager,
    Map * map)
{
    rtl::OUString id(dp_misc::getIdentifier(package));
    css::uno::Sequence< rtl::OUString > urls(
        package->getUpdateInformationURLs());
    if (urls.getLength() == 0) {
        map->insert(
            Map::value_type(
                id, Entry(package, packageManager, package->getVersion())));
    } else {
        css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > >
            infos(getUpdateInformation(package, urls, id));
        rtl::OUString latestVersion(package->getVersion());
        sal_Int32 latestIndex = -1;
        for (sal_Int32 i = 0; i < infos.getLength(); ++i) {
            dp_misc::DescriptionInfoset infoset(
                m_context,
                css::uno::Reference< css::xml::dom::XNode >(
                    infos[i], css::uno::UNO_QUERY_THROW));
            boost::optional< rtl::OUString > id2(infoset.getIdentifier());
            if (!id2) {
                continue;
            }
            if (*id2 == id) {
                rtl::OUString v(infoset.getVersion());
                if (dp_misc::compareVersions(v, latestVersion) ==
                    dp_misc::GREATER)
                {
                    latestVersion = v;
                    latestIndex = i;
                }
            }
        }
        if (latestIndex != -1) {
            update(
                package, packageManager,
                css::uno::Reference< css::xml::dom::XNode >(
                    infos[latestIndex], css::uno::UNO_QUERY_THROW));
        }
    }
}

bool UpdateDialog::Thread::update(
    css::uno::Reference< css::deployment::XPackage > const & package,
    css::uno::Reference< css::deployment::XPackageManager > const &
        packageManager,
    css::uno::Reference< css::xml::dom::XNode > const & updateInfo) const
{
    dp_misc::DescriptionInfoset infoset(m_context, updateInfo);
    OSL_ASSERT(infoset.getVersion().getLength() != 0);
    css::uno::Sequence< css::uno::Reference< css::xml::dom::XElement > > ds(
        dp_misc::Dependencies::check(infoset));

    UpdateDialog::DisabledUpdate du;
    du.aUpdateInfo = updateInfo;
    du.unsatisfiedDependencies.realloc(ds.getLength());
    for (sal_Int32 i = 0; i < ds.getLength(); ++i) {
        du.unsatisfiedDependencies[i] = dp_misc::Dependencies::name(ds[i]);
    }
    du.permission = ! packageManager->isReadOnly();
    const ::boost::optional< ::rtl::OUString> updateWebsiteURL(infoset.getLocalizedUpdateWebsiteURL());
    rtl::OUStringBuffer b(package->getDisplayName());
    b.append(static_cast< sal_Unicode >(' '));
    b.append(m_dialog.m_version);
    b.append(static_cast< sal_Unicode >(' '));
    b.append(infoset.getVersion());
    if (updateWebsiteURL)
    {
        b.append(static_cast< sal_Unicode >(' '));
        b.append(m_dialog.m_browserbased);
    }
    du.name = b.makeStringAndClear();

    if (du.unsatisfiedDependencies.getLength() == 0 && du.permission)
    {
        dp_gui::UpdateData data;
        data.aInstalledPackage = package;
        data.aPackageManager = packageManager;
        data.aUpdateInfo = updateInfo;
        if (updateWebsiteURL)
            data.sWebsiteURL = *updateWebsiteURL;
        vos::OGuard g(Application::GetSolarMutex());
        if (!m_stop) {
            m_dialog.addEnabledUpdate(du.name, data);
        }
        return !m_stop;
    } else {
        vos::OGuard g(Application::GetSolarMutex());
        if (!m_stop) {
            m_dialog.addDisabledUpdate(du);
        }
        return !m_stop;
    }
}

// UpdateDialog ----------------------------------------------------------
UpdateDialog::UpdateDialog(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    Window * parent,
    rtl::Reference<dp_gui::DialogImpl> const & extensionManagerDialog ,
    rtl::Reference< dp_gui::SelectedPackageIterator > const & selectedPackages,
    css::uno::Sequence< css::uno::Reference<
        css::deployment::XPackageManager > > const & packageManagers,
    std::vector< dp_gui::UpdateData > * updateData):
    ModalDialog(parent,DpGuiResId(RID_DLG_UPDATE)),
    m_context(context),
    m_checking(this, DpGuiResId(RID_DLG_UPDATE_CHECKING)),
    m_update(this, DpGuiResId(RID_DLG_UPDATE_UPDATE)),
    m_updates(
        *this, DpGuiResId(RID_DLG_UPDATE_UPDATES),
        Image(DpGuiResId(RID_DLG_UPDATE_NORMALALERT)),
        Image(DpGuiResId(RID_DLG_UPDATE_HIGHCONTRASTALERT))),
    m_all(this, DpGuiResId(RID_DLG_UPDATE_ALL)),
    m_description(this, DpGuiResId(RID_DLG_UPDATE_DESCRIPTION)),
    m_PublisherLabel(this, DpGuiResId(RID_DLG_UPDATE_PUBLISHER_LABEL)),
    m_PublisherLink(this, DpGuiResId(RID_DLG_UPDATE_PUBLISHER_LINK)),
    m_ReleaseNotesLabel(this, DpGuiResId(RID_DLG_UPDATE_RELEASENOTES_LABEL)),
    m_ReleaseNotesLink(this, DpGuiResId(RID_DLG_UPDATE_RELEASENOTES_LINK)),
    m_descriptions(this, DpGuiResId(RID_DLG_UPDATE_DESCRIPTIONS)),
    m_line(this, DpGuiResId(RID_DLG_UPDATE_LINE)),
    m_help(this, DpGuiResId(RID_DLG_UPDATE_HELP)),
    m_ok(this, DpGuiResId(RID_DLG_UPDATE_OK)),
    m_cancel(this, DpGuiResId(RID_DLG_UPDATE_CANCEL)),
    m_error(String(DpGuiResId(RID_DLG_UPDATE_ERROR))),
    m_none(String(DpGuiResId(RID_DLG_UPDATE_NONE))),
    m_noInstallable(String(DpGuiResId(RID_DLG_UPDATE_NOINSTALLABLE))),
    m_failure(String(DpGuiResId(RID_DLG_UPDATE_FAILURE))),
    m_unknownError(String(DpGuiResId(RID_DLG_UPDATE_UNKNOWNERROR))),
    m_noDescription(String(DpGuiResId(RID_DLG_UPDATE_NODESCRIPTION))),
    m_noInstall(String(DpGuiResId(RID_DLG_UPDATE_NOINSTALL))),
    m_noDependency(String(DpGuiResId(RID_DLG_UPDATE_NODEPENDENCY))),
    m_noPermission(String(DpGuiResId(RID_DLG_UPDATE_NOPERMISSION))),
    m_noPermissionVista(String(DpGuiResId(RID_DLG_UPDATE_NOPERMISSION_VISTA))),
    m_browserbased(String(DpGuiResId(RID_DLG_UPDATE_BROWSERBASED))),
    m_version(String(DpGuiResId(RID_DLG_UPDATE_VERSION))),
    m_updateData(*updateData),
    m_thread(
        new UpdateDialog::Thread(
            context, *this, selectedPackages,
            packageManagers)),
    m_nFirstLineDelta(0),
    m_nOneLineMissing(0),
    m_extensionManagerDialog(extensionManagerDialog)

{
    OSL_ASSERT(updateData != NULL);
    css::uno::Reference< css::awt::XToolkit > toolkit;
    try {
        toolkit = css::uno::Reference< css::awt::XToolkit >(
            (css::uno::Reference< css::lang::XMultiComponentFactory >(
                m_context->getServiceManager(),
                css::uno::UNO_QUERY_THROW)->
             createInstanceWithContext(
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.Toolkit")),
                 m_context)),
            css::uno::UNO_QUERY_THROW);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception & e) {
        throw css::uno::RuntimeException(e.Message, e.Context);
    }
    Control c(this, DpGuiResId(RID_DLG_UPDATE_THROBBER));
    Point pos(c.GetPosPixel());
    Size size(c.GetSizePixel());
    try {
        m_throbber = css::uno::Reference< css::awt::XThrobber >(
            toolkit->createWindow(
                css::awt::WindowDescriptor(
                    css::awt::WindowClass_SIMPLE,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Throbber")),
                    GetComponentInterface(), 0,
                    css::awt::Rectangle(
                        pos.X(), pos.Y(), size.Width(), size.Height()),
                    css::awt::WindowAttribute::SHOW)),
            css::uno::UNO_QUERY_THROW);
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::uno::RuntimeException(e.Message, e.Context);
    }
    m_updates.SetSelectHdl(LINK(this, UpdateDialog, selectionHandler));
    m_all.SetToggleHdl(LINK(this, UpdateDialog, allHandler));
    m_ok.SetClickHdl(LINK(this, UpdateDialog, okHandler));
    m_cancel.SetClickHdl(LINK(this, UpdateDialog, cancelHandler));
    if ( ! dp_misc::office_is_running())
        m_help.Disable();
    FreeResource();
    String sTemp(m_noPermissionVista);
    sTemp.SearchAndReplaceAllAscii( "%PRODUCTNAME", BrandName::get() );
    m_noPermissionVista = sTemp;

    initDescription();
}

UpdateDialog::~UpdateDialog() {
    for (USHORT i = 0; i < m_updates.getItemCount(); ++i) {
        delete static_cast< UpdateDialog::Index const * >(
            m_updates.GetEntryData(i));
    }
}

BOOL UpdateDialog::Close() {
    m_thread->stop();
    return ModalDialog::Close();
}

short UpdateDialog::Execute() {
    m_throbber->start();
    m_thread->launch();
    return ModalDialog::Execute();
}

UpdateDialog::CheckListBox::CheckListBox(
    UpdateDialog & dialog, ResId const & resource,
    Image const & normalStaticImage, Image const & highContrastStaticImage):
    SvxCheckListBox(
        &dialog, resource, normalStaticImage, highContrastStaticImage),
    m_dialog(dialog)
{}

UpdateDialog::CheckListBox::~CheckListBox() {}

USHORT UpdateDialog::CheckListBox::getItemCount() const {
    ULONG i = GetEntryCount();
    OSL_ASSERT(i <= std::numeric_limits< USHORT >::max());
    return sal::static_int_cast< USHORT >(i);
}

void UpdateDialog::CheckListBox::MouseButtonDown(MouseEvent const & event) {
    // When clicking on a selected entry in an SvxCheckListBox, the entry's
    // checkbox is toggled on mouse button down:
    SvxCheckListBox::MouseButtonDown(event);
    m_dialog.enableOk();
}

void UpdateDialog::CheckListBox::MouseButtonUp(MouseEvent const & event) {
    // When clicking on an entry's checkbox in an SvxCheckListBox, the entry's
    // checkbox is toggled on mouse button up:
    SvxCheckListBox::MouseButtonUp(event);
    m_dialog.enableOk();
}

void UpdateDialog::CheckListBox::KeyInput(KeyEvent const & event) {
    SvxCheckListBox::KeyInput(event);
    m_dialog.enableOk();
}

void UpdateDialog::insertItem(
    rtl::OUString const & name, USHORT position,
    std::auto_ptr< UpdateDialog::Index const > index, SvLBoxButtonKind kind)
{
    m_updates.InsertEntry(
        name, position,
        const_cast< void * >(static_cast< void const * >(index.release())),
        kind);
        //TODO #i72487#: UpdateDialog::Index potentially leaks as the exception
        // behavior of SvxCheckListBox::InsertEntry is unspecified
}

void UpdateDialog::addAdditional(
    rtl::OUString const & name, USHORT position,
    std::auto_ptr< UpdateDialog::Index const > index, SvLBoxButtonKind kind)
{
    m_all.Enable();
    if (m_all.IsChecked()) {
        insertItem(name, position, index, kind);
        m_update.Enable();
        m_updates.Enable();
        m_description.Enable();
        m_descriptions.Enable();
    }
}

void UpdateDialog::addEnabledUpdate(
    rtl::OUString const & name, dp_gui::UpdateData const & data)
{
    std::vector< dp_gui::UpdateData >::size_type n = m_enabledUpdates.size();
    m_enabledUpdates.push_back(data);
    insertItem(
        name, sal::static_int_cast< USHORT >(n),
        UpdateDialog::Index::newEnabledUpdate(n),
        SvLBoxButtonKind_enabledCheckbox);
        // position overflow is rather harmless
    m_updates.CheckEntryPos(sal::static_int_cast< USHORT >(n));
        //TODO #i72487#: fragile computation; insertItem should instead return
        // pos
    m_update.Enable();
    m_updates.Enable();
    m_description.Enable();
    m_descriptions.Enable();
}

void UpdateDialog::addDisabledUpdate(UpdateDialog::DisabledUpdate const & data)
{
    std::vector< UpdateDialog::DisabledUpdate >::size_type n =
        m_disabledUpdates.size();
    m_disabledUpdates.push_back(data);
    addAdditional(
        data.name, sal::static_int_cast< USHORT >(m_enabledUpdates.size() + n),
        UpdateDialog::Index::newDisabledUpdate(n),
        SvLBoxButtonKind_disabledCheckbox);
        // position overflow is rather harmless
}

void UpdateDialog::addGeneralError(rtl::OUString const & message) {
    std::vector< rtl::OUString >::size_type n = m_generalErrors.size();
    m_generalErrors.push_back(message);
    addAdditional(
        m_error,
        sal::static_int_cast< USHORT >(
            m_enabledUpdates.size() + m_disabledUpdates.size() + n),
        UpdateDialog::Index::newGeneralError(n), SvLBoxButtonKind_staticImage);
        // position overflow is rather harmless
}

void UpdateDialog::addSpecificError(UpdateDialog::SpecificError const & data) {
    std::vector< UpdateDialog::SpecificError >::size_type n =
        m_specificErrors.size();
    m_specificErrors.push_back(data);
    addAdditional(
        data.name, LISTBOX_APPEND, UpdateDialog::Index::newSpecificError(n),
        SvLBoxButtonKind_staticImage);
}

void UpdateDialog::checkingDone() {
    m_checking.Hide();
    m_throbber->stop();
    css::uno::Reference< css::awt::XWindow >(
        m_throbber, css::uno::UNO_QUERY_THROW)->setVisible(false);
    if (m_updates.getItemCount() == 0)
    {
        clearDescription();
        m_description.Enable();
        m_descriptions.Enable();
        showDescription(
            ( m_disabledUpdates.empty() && m_generalErrors.empty() && m_specificErrors.empty() )
                ? m_none : m_noInstallable, false );
    }
    enableOk();
}

void UpdateDialog::enableOk() {
    if (!m_checking.IsVisible()) {
        m_ok.Enable(m_updates.GetCheckedEntryCount() != 0);
    }
}

// *********************************************************************************
void UpdateDialog::createNotifyJob( bool bPrepareOnly,
    css::uno::Sequence< css::uno::Sequence< rtl::OUString > > &rItemList )
{
    if ( !dp_misc::office_is_running() )
        return;

    // notify update check job
    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            xFactory->createInstance( OUSTR( "com.sun.star.configuration.ConfigurationProvider" )),
            css::uno::UNO_QUERY_THROW);

        css::beans::PropertyValue aProperty;
        aProperty.Name  = OUSTR( "nodepath" );
        aProperty.Value = css::uno::makeAny( OUSTR("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob") );

        css::uno::Sequence< css::uno::Any > aArgumentList( 1 );
        aArgumentList[0] = css::uno::makeAny( aProperty );

        css::uno::Reference< css::container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"), aArgumentList ),
            css::uno::UNO_QUERY_THROW );

        css::util::URL aURL;
        xNameAccess->getByName(OUSTR("URL")) >>= aURL.Complete;

        css::uno::Reference < css::util::XURLTransformer > xTransformer( xFactory->createInstance( OUSTR( "com.sun.star.util.URLTransformer" ) ),
            css::uno::UNO_QUERY_THROW );

        xTransformer->parseStrict(aURL);

        css::uno::Reference < css::frame::XDesktop > xDesktop( xFactory->createInstance( OUSTR( "com.sun.star.frame.Desktop" ) ),
            css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( xDesktop->getCurrentFrame(),
            css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, rtl::OUString(), 0);

        if( xDispatch.is() )
        {
            css::uno::Sequence< css::beans::PropertyValue > aPropList(2);
            aProperty.Name  = OUSTR( "updateList" );
            aProperty.Value = css::uno::makeAny( rItemList );
            aPropList[0] = aProperty;
            aProperty.Name  = OUSTR( "prepareOnly" );
            aProperty.Value = css::uno::makeAny( bPrepareOnly );
            aPropList[1] = aProperty;

            xDispatch->dispatch(aURL, aPropList );
        }
    }
    catch( const css::uno::Exception& e )
    {
         OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
}

// *********************************************************************************
void UpdateDialog::notifyMenubar( bool bPrepareOnly, bool bRecheckOnly )
{
    if ( !dp_misc::office_is_running() )
        return;

    css::uno::Sequence< css::uno::Sequence< rtl::OUString > > aItemList;
    sal_Int32 nCount = 0;

    if ( ! bRecheckOnly )
    {
        for ( sal_Int16 i = 0; i < m_updates.getItemCount(); ++i )
        {
            css::uno::Sequence< rtl::OUString > aItem(2);

            UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(m_updates.GetEntryData(i));

            if ( p->kind == ENABLED_UPDATE )
            {
                dp_gui::UpdateData aUpdData = m_enabledUpdates[ p->index.enabledUpdate ];
                aItem[0] = dp_misc::getIdentifier( aUpdData.aInstalledPackage );

                dp_misc::DescriptionInfoset aInfoset( m_context, aUpdData.aUpdateInfo );
                aItem[1] = aInfoset.getVersion();
            }
            else if ( p->kind == DISABLED_UPDATE )
                continue;
            else
                continue;

            aItemList.realloc( nCount + 1 );
            aItemList[ nCount ] = aItem;
            nCount += 1;
        }
    }
    createNotifyJob( bPrepareOnly, aItemList );
}

// *********************************************************************************

void UpdateDialog::initDescription()
{
    m_PublisherLabel.Hide();
    m_PublisherLink.Hide();
    m_ReleaseNotesLabel.Hide();
    m_ReleaseNotesLink.Hide();
    m_descriptions.Hide();

    Link aLink = LINK( this, UpdateDialog, hyperlink_clicked );
    m_PublisherLink.SetClickHdl( aLink );
    m_ReleaseNotesLink.SetClickHdl( aLink );

    long nTextWidth = m_PublisherLabel.GetCtrlTextWidth( m_PublisherLabel.GetText() );
    long nTemp = m_ReleaseNotesLabel.GetTextWidth( m_ReleaseNotesLabel.GetText() );
    if ( nTemp > nTextWidth )
        nTextWidth = nTemp;
    nTextWidth = nTextWidth * 110 / 100;

    Size aNewSize = m_PublisherLabel.GetSizePixel();
    if ( nTextWidth > aNewSize.Width() )
    {
        long nDelta = nTextWidth - aNewSize.Width();
        aNewSize.Width() = nTextWidth;
        m_PublisherLabel.SetSizePixel( aNewSize );
        m_ReleaseNotesLabel.SetSizePixel( aNewSize );

        aNewSize = m_PublisherLink.GetSizePixel();
        aNewSize.Width() = aNewSize.Width() - nDelta;
        Point aNewPos = m_PublisherLink.GetPosPixel();
        aNewPos.X() = aNewPos.X() + nDelta;
        m_PublisherLink.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = m_ReleaseNotesLink.GetPosPixel().Y();
        m_ReleaseNotesLink.SetPosSizePixel( aNewPos, aNewSize );
    }

    m_aFirstLinePos = m_descriptions.GetPosPixel();
    m_aFirstLineSize = m_descriptions.GetSizePixel();
    Size aMarginSize = LogicToPixel( Size( RSC_SP_CTRL_GROUP_X, RSC_SP_CTRL_GROUP_Y ), MAP_APPFONT );
    Point aThirdLinePos = m_ReleaseNotesLabel.GetPosPixel();
    aThirdLinePos.Y() = aThirdLinePos.Y() + m_ReleaseNotesLabel.GetSizePixel().Height() + aMarginSize.Height();
    m_nFirstLineDelta = aThirdLinePos.Y() - m_aFirstLinePos.Y();
    m_nOneLineMissing = m_ReleaseNotesLabel.GetPosPixel().Y() - m_PublisherLabel.GetPosPixel().Y();
}

void UpdateDialog::clearDescription()
{
    String sEmpty;
    m_PublisherLabel.Hide();
    m_PublisherLink.Hide();
    m_PublisherLink.SetDescription( sEmpty );
    m_PublisherLink.SetURL( sEmpty );
    m_ReleaseNotesLabel.Hide();
    m_ReleaseNotesLink.Hide();
    m_ReleaseNotesLink.SetURL( sEmpty );
    if ( m_PublisherLabel.GetPosPixel().Y() == m_ReleaseNotesLabel.GetPosPixel().Y() )
    {
        Point aNewPos = m_ReleaseNotesLabel.GetPosPixel();
        aNewPos.Y() += m_nOneLineMissing;
        m_ReleaseNotesLabel.SetPosPixel( aNewPos );
        aNewPos = m_ReleaseNotesLink.GetPosPixel();
        aNewPos.Y() += m_nOneLineMissing;
        m_ReleaseNotesLink.SetPosPixel( aNewPos );
    }
    m_descriptions.Hide();
    m_descriptions.Clear();
    m_descriptions.SetPosSizePixel( m_aFirstLinePos, m_aFirstLineSize );
}

bool UpdateDialog::showDescription(css::uno::Reference< css::xml::dom::XNode > const & aUpdateInfo)
{
    dp_misc::DescriptionInfoset infoset(m_context, aUpdateInfo);
    std::pair< rtl::OUString, rtl::OUString > pairPub = infoset.getLocalizedPublisherNameAndURL();
    rtl::OUString sPub = pairPub.first;
    rtl::OUString sURL = pairPub.second;
    rtl::OUString sRel = infoset.getLocalizedReleaseNotesURL();

    if ( sPub.getLength() == 0 && sURL.getLength() == 0 && sRel.getLength() == 0 )
        // nothing to show
        return false;

    bool bPublisher = false;
    if ( sPub.getLength() > 0 )
    {
        m_PublisherLabel.Show();
        m_PublisherLink.Show();
        m_PublisherLink.SetDescription( sPub );
        m_PublisherLink.SetURL( sURL );
        bPublisher = true;
    }

    if ( sRel.getLength() > 0 )
    {
        if ( !bPublisher )
        {
            m_ReleaseNotesLabel.SetPosPixel( m_PublisherLabel.GetPosPixel() );
            m_ReleaseNotesLink.SetPosPixel( m_PublisherLink.GetPosPixel() );
        }
        m_ReleaseNotesLabel.Show();
        m_ReleaseNotesLink.Show();
        m_ReleaseNotesLink.SetURL( sRel );
    }
    return true;
}

bool UpdateDialog::showDescription( const String& rDescription, bool bWithPublisher )
{
    if ( rDescription.Len() == 0 )
        // nothing to show
        return false;

    if ( bWithPublisher )
    {
        bool bOneLineMissing = !m_ReleaseNotesLabel.IsVisible() || !m_PublisherLabel.IsVisible();
        Point aNewPos = m_aFirstLinePos;
        aNewPos.Y() += m_nFirstLineDelta;
        if ( bOneLineMissing )
            aNewPos.Y() -= m_nOneLineMissing;
        Size aNewSize = m_aFirstLineSize;
        aNewSize.Height() -= m_nFirstLineDelta;
        if ( bOneLineMissing )
            aNewSize.Height() += m_nOneLineMissing;
        m_descriptions.SetPosSizePixel( aNewPos, aNewSize );
    }
    m_descriptions.Show();
    m_descriptions.SetDescription( rDescription );
    return true;
}

IMPL_LINK(UpdateDialog, selectionHandler, void *, EMPTYARG)
{
    rtl::OUStringBuffer b;
    bool bInserted = false;
    UpdateDialog::Index const * p = static_cast< UpdateDialog::Index const * >(
        m_updates.GetEntryData(m_updates.GetSelectEntryPos()));
    clearDescription();

    if (p != NULL)
    {
        //When the index is greater or equal than the amount of enabled updates then the "Show all"
        //button is probably checked. Then we show first all enabled and then the disabled
        //updates.
        USHORT pos = m_updates.GetSelectEntryPos();
        const std::vector< dp_gui::UpdateData >::size_type sizeEnabled =
            m_enabledUpdates.size();
        const std::vector< UpdateDialog::DisabledUpdate >::size_type sizeDisabled =
            m_disabledUpdates.size();
        if (pos < sizeEnabled)
            bInserted = showDescription(m_enabledUpdates[pos].aUpdateInfo);
        else if (pos >= sizeEnabled
            && pos < (sizeEnabled + sizeDisabled))
            bInserted = showDescription(m_disabledUpdates[pos - sizeEnabled].aUpdateInfo);

        switch (p->kind)
        {
            case ENABLED_UPDATE:
            {
                b.append(m_noDescription);
                break;
            }
            case DISABLED_UPDATE:
            {
                UpdateDialog::DisabledUpdate & data = m_disabledUpdates[
                    p->index.disabledUpdate];
                if (data.unsatisfiedDependencies.getLength() != 0) {
                    b.append(m_noInstall);
                    b.append(LF);
                    b.append(m_noDependency);
                    for (sal_Int32 i = 0;
                         i < data.unsatisfiedDependencies.getLength(); ++i)
                    {
                        b.append(LF);
                        b.appendAscii(RTL_CONSTASCII_STRINGPARAM("  "));
                            // U+2003 EM SPACE would be better than two spaces,
                            // but some fonts do not contain it
                        b.append(
                            confineToParagraph(
                                data.unsatisfiedDependencies[i]));
                    }
                }
                if (!data.permission) {
                    if (b.getLength() == 0) {
                        b.append(m_noInstall);
                    }
                    b.append(LF);
                    if (isVista())
                        b.append(m_noPermissionVista);
                    else
                        b.append(m_noPermission);
                }
                break;
            }
            case GENERAL_ERROR:
            {
                rtl::OUString & msg = m_generalErrors[p->index.generalError];
                b.append(m_failure);
                b.append(LF);
                b.append(msg.getLength() == 0 ? m_unknownError : msg);
                break;
            }
            case SPECIFIC_ERROR:
            {
                UpdateDialog::SpecificError & data = m_specificErrors[
                    p->index.specificError];
                b.append(m_failure);
                b.append(LF);
                b.append(
                    data.message.getLength() == 0
                    ? m_unknownError : data.message);
                break;
            }
            default:
                OSL_ASSERT(false);
                break;
        }
    }

    showDescription( b.makeStringAndClear(), bInserted );
    return 0;
}

IMPL_LINK(UpdateDialog, allHandler, void *, EMPTYARG) {
    if (m_all.IsChecked()) {
        m_update.Enable();
        m_updates.Enable();
        m_description.Enable();
        m_descriptions.Enable();
        std::vector< UpdateDialog::DisabledUpdate >::size_type n1 = 0;
        for (std::vector< UpdateDialog::DisabledUpdate >::iterator i(
                 m_disabledUpdates.begin());
             i != m_disabledUpdates.end(); ++i)
        {
            insertItem(
                i->name, LISTBOX_APPEND,
                UpdateDialog::Index::newDisabledUpdate(n1++),
                SvLBoxButtonKind_disabledCheckbox);
        }
        std::vector< rtl::OUString >::size_type n2 = 0;
        for (std::vector< rtl::OUString >::iterator i(m_generalErrors.begin());
             i != m_generalErrors.end(); ++i)
        {
            insertItem(
                m_error, LISTBOX_APPEND,
                UpdateDialog::Index::newGeneralError(n2++),
                SvLBoxButtonKind_staticImage);
        }
        std::vector< UpdateDialog::SpecificError >::size_type n3 = 0;
        for (std::vector< UpdateDialog::SpecificError >::iterator i(
                 m_specificErrors.begin());
             i != m_specificErrors.end(); ++i)
        {
            insertItem(
                i->name, LISTBOX_APPEND,
                UpdateDialog::Index::newSpecificError(n3++),
                SvLBoxButtonKind_staticImage);
        }
    } else {
        for (USHORT i = 0; i < m_updates.getItemCount();) {
            UpdateDialog::Index const * p =
                static_cast< UpdateDialog::Index const * >(
                    m_updates.GetEntryData(i));
            if (p->kind != ENABLED_UPDATE) {
                m_updates.RemoveEntry(i);
                    //TODO #i72487#: UpdateDialog::Index potentially leaks as
                    // SvxCheckListBox::RemoveEntry's exception behavior is
                    // unspecified
                delete p;
            } else {
                ++i;
            }
        }

        if (m_updates.getItemCount() == 0)
        {
            clearDescription();
            m_update.Disable();
            m_updates.Disable();
            if (m_checking.IsVisible())
                m_description.Disable();
            else
                showDescription(m_noInstallable,false);
        }
    }
    return 0;
}

IMPL_LINK(UpdateDialog, okHandler, void *, EMPTYARG)
{
    //If users are going to update a shared extension then we need
    //to warn them
    typedef ::std::vector<UpdateData>::const_iterator CIT;
    for (CIT i = m_enabledUpdates.begin(); i < m_enabledUpdates.end(); i++)
    {
        OSL_ASSERT(i->aPackageManager.is());
        //If the user has no write access to the shared folder then the update
        //for a shared extension is disable, that is it cannot be in m_enabledUpdates
        OSL_ASSERT(i->aPackageManager->isReadOnly() == sal_False);
        OSL_ASSERT(m_extensionManagerDialog.get());
        if (RET_CANCEL == m_extensionManagerDialog->continueUpdateForSharedExtension(
            this, i->aPackageManager))
        {
            EndDialog(RET_CANCEL);
        }
    }

    for (USHORT i = 0; i < m_updates.getItemCount(); ++i) {
        UpdateDialog::Index const * p =
            static_cast< UpdateDialog::Index const * >(
                m_updates.GetEntryData(i));
        if (p->kind == ENABLED_UPDATE && m_updates.IsChecked(i)) {
            m_updateData.push_back(m_enabledUpdates[p->index.enabledUpdate]);
        }
    }
    EndDialog(RET_OK);
    return 0;
}

IMPL_LINK(UpdateDialog, cancelHandler, void *, EMPTYARG) {
    m_thread->stop();
    EndDialog(RET_CANCEL);
    return 0;
}

IMPL_LINK( UpdateDialog, hyperlink_clicked, svt::FixedHyperlink*, pHyperlink )
{
    ::rtl::OUString sURL;
    if ( pHyperlink )
        sURL = ::rtl::OUString( pHyperlink->GetURL() );
    if ( sURL.getLength() == 0 )
        return 0;

    try
    {
        css::uno::Reference< css::system::XSystemShellExecute > xSystemShellExecute(
            m_context->getServiceManager()->createInstanceWithContext(
                ::rtl::OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" ),
                m_context), css::uno::UNO_QUERY_THROW);
        //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
        xSystemShellExecute->execute(
            sURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::DEFAULTS);
    }
    catch (css::uno::Exception& )
    {
    }

    return 1;
}

