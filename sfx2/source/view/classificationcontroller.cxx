/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <vcl/vclptr.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/classificationhelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/configurationlistener.hxx>

using namespace com::sun::star;

namespace sfx2
{

class ClassificationCategoriesController;
using ClassificationPropertyListenerBase = comphelper::ConfigurationListenerProperty<OUString>;

/// Listens to configuration changes, so no restart is needed after setting the classification path.
class ClassificationPropertyListener : public ClassificationPropertyListenerBase
{
    ClassificationCategoriesController& m_rController;

public:
    ClassificationPropertyListener(const rtl::Reference<comphelper::ConfigurationListener>& xListener, ClassificationCategoriesController& rController);
    virtual void setProperty(const uno::Any& rProperty) override;
};

using ClassificationCategoriesControllerBase = cppu::ImplInheritanceHelper<svt::ToolboxController, lang::XServiceInfo>;

/// Controller for .uno:ClassificationApply.
class ClassificationCategoriesController : public ClassificationCategoriesControllerBase
{
    VclPtr<ListBox> m_pCategories;
    rtl::Reference<comphelper::ConfigurationListener> m_xListener;
    ClassificationPropertyListener m_aPropertyListener;

    DECL_LINK_TYPED(SelectHdl, ListBox&, void);

public:
    explicit ClassificationCategoriesController(const uno::Reference<uno::XComponentContext>& rContext);
    virtual ~ClassificationCategoriesController();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception) override;
    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw (uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose() throw (uno::RuntimeException, std::exception) override;

    // XToolbarController
    virtual uno::Reference<awt::XWindow> SAL_CALL createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) throw (uno::RuntimeException, std::exception) override;

    void removeEntries();
};

ClassificationPropertyListener::ClassificationPropertyListener(const rtl::Reference<comphelper::ConfigurationListener>& xListener, ClassificationCategoriesController& rController)
    : ClassificationPropertyListenerBase(xListener, "WritePath")
    , m_rController(rController)
{
}

void ClassificationPropertyListener::setProperty(const uno::Any& /*rProperty*/)
{
    // So that its gets re-filled with entries from the new policy.
    m_rController.removeEntries();
}

ClassificationCategoriesController::ClassificationCategoriesController(const uno::Reference<uno::XComponentContext>& rContext)
    : ClassificationCategoriesControllerBase(rContext, uno::Reference<frame::XFrame>(), OUString(".uno:ClassificationApply"))
    , m_pCategories(nullptr)
    , m_xListener(new comphelper::ConfigurationListener("/org.openoffice.Office.Paths/Paths/Classification"))
    , m_aPropertyListener(m_xListener, *this)
{

}

ClassificationCategoriesController::~ClassificationCategoriesController()
{
}

OUString ClassificationCategoriesController::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.sfx2.ClassificationCategoriesController");
}

sal_Bool ClassificationCategoriesController::supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> ClassificationCategoriesController::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aServices
    {
        "com.sun.star.frame.ToolbarController"
    };
    return aServices;
}

void ClassificationCategoriesController::dispose() throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
    m_pCategories.disposeAndClear();
    m_aPropertyListener.dispose();
    m_xListener->dispose();
}

uno::Reference<awt::XWindow> ClassificationCategoriesController::createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception)
{
    vcl::Window* pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent);
    if (pToolbar)
    {
        m_pCategories = VclPtr<ListBox>::Create(pToolbar, WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_DROPDOWN|WB_SIMPLEMODE);
        m_pCategories->SetSelectHdl(LINK(this, ClassificationCategoriesController, SelectHdl));
        // Same as SvxFontNameBox_Impl.
        m_pCategories->SetSizePixel(m_pCategories->LogicToPixel(Size(60, 160), MAP_APPFONT));
    }

    return uno::Reference<awt::XWindow>(VCLUnoHelper::GetInterface(m_pCategories));
}

IMPL_LINK_NOARG_TYPED(ClassificationCategoriesController, SelectHdl, ListBox&, void)
{
    OUString aEntry = m_pCategories->GetSelectEntry();
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Name", uno::makeAny(aEntry)},
    }));
    comphelper::dispatchCommand(".uno:ClassificationApply", aPropertyValues);
}

void ClassificationCategoriesController::statusChanged(const frame::FeatureStateEvent& /*rEvent*/) throw (uno::RuntimeException, std::exception)
{
    if (!m_pCategories)
        return;

    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    if (!pObjectShell)
        return;

    SfxClassificationHelper aHelper(pObjectShell->getDocProperties());
    if (m_pCategories->GetEntryCount() == 0)
    {
        std::vector<OUString> aNames = aHelper.GetBACNames();
        for (const OUString& rName : aNames)
            m_pCategories->InsertEntry(rName);
        // Normally VclBuilder::makeObject() does this.
        m_pCategories->EnableAutoSize(true);
        m_pCategories->SetSizePixel(m_pCategories->GetOptimalSize());
    }

    // Restore state based on the doc. model.
    const OUString& rCategoryName = aHelper.GetBACName();
    if (!rCategoryName.isEmpty())
        m_pCategories->SelectEntry(rCategoryName);
}

void ClassificationCategoriesController::removeEntries()
{
    if (m_pCategories)
        m_pCategories->Clear();
}

} // namespace sfx2

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* SAL_CALL com_sun_star_sfx2_ClassificationCategoriesController_get_implementation(uno::XComponentContext* pContext, const uno::Sequence<uno::Any>&)
{
    return cppu::acquire(new sfx2::ClassificationCategoriesController(pContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
