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
#include <vcl/fixed.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxresid.hxx>
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

class ClassificationControl;

/// Controller for .uno:ClassificationApply.
class ClassificationCategoriesController : public ClassificationCategoriesControllerBase
{
    VclPtr<ClassificationControl> m_pClassification;
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

/// Classification control is the parent of all widgets that belongs to ClassificationCategoriesController.
class SAL_WARN_UNUSED ClassificationControl : public vcl::Window
{
    VclPtr<FixedText> m_pLabel;
    VclPtr<ListBox> m_pCategories;
    void SetOptimalSize();
    virtual void DataChanged(const DataChangedEvent& rEvent) override;

public:
    ClassificationControl(vcl::Window* pParent);
    virtual ~ClassificationControl();
    virtual void dispose() override;
    virtual void Resize() override;
    VclPtr<ListBox> getCategories();
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
    , m_pClassification(nullptr)
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
    m_pClassification.disposeAndClear();
    m_aPropertyListener.dispose();
    m_xListener->dispose();
}

uno::Reference<awt::XWindow> ClassificationCategoriesController::createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception)
{
    vcl::Window* pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent);
    if (pToolbar)
    {
        m_pClassification = VclPtr<ClassificationControl>::Create(pToolbar);
        m_pClassification->getCategories()->SetSelectHdl(LINK(this, ClassificationCategoriesController, SelectHdl));
    }

    return uno::Reference<awt::XWindow>(VCLUnoHelper::GetInterface(m_pClassification));
}

IMPL_LINK_NOARG_TYPED(ClassificationCategoriesController, SelectHdl, ListBox&, void)
{
    OUString aEntry = m_pClassification->getCategories()->GetSelectEntry();
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Name", uno::makeAny(aEntry)},
    }));
    comphelper::dispatchCommand(".uno:ClassificationApply", aPropertyValues);
}

void ClassificationCategoriesController::statusChanged(const frame::FeatureStateEvent& /*rEvent*/) throw (uno::RuntimeException, std::exception)
{
    if (!m_pClassification)
        return;

    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    if (!pObjectShell)
        return;

    SfxClassificationHelper aHelper(pObjectShell->getDocProperties());
    if (m_pClassification->getCategories()->GetEntryCount() == 0)
    {
        std::vector<OUString> aNames = aHelper.GetBACNames();
        for (const OUString& rName : aNames)
            m_pClassification->getCategories()->InsertEntry(rName);
        // Normally VclBuilder::makeObject() does this.
        m_pClassification->getCategories()->EnableAutoSize(true);
    }

    // Restore state based on the doc. model.
    const OUString& rCategoryName = aHelper.GetBACName();
    if (!rCategoryName.isEmpty())
        m_pClassification->getCategories()->SelectEntry(rCategoryName);
}

void ClassificationCategoriesController::removeEntries()
{
    if (m_pClassification)
        m_pClassification->getCategories()->Clear();
}

ClassificationControl::ClassificationControl(vcl::Window* pParent)
    : Window(pParent, WB_DIALOGCONTROL)
    , m_pLabel(nullptr)
    , m_pCategories(nullptr)
{
    OUString aText = SfxResId(STR_CLASSIFIED_INTELLECTUAL_PROPERTY);
    m_pLabel = VclPtr<FixedText>::Create(this);
    Size aTextSize(m_pLabel->GetTextWidth(aText), m_pLabel->GetTextHeight());
    // Padding.
    aTextSize.Width() += 6;
    m_pLabel->SetText(aText);
    m_pLabel->SetSizePixel(aTextSize);
    m_pLabel->Show();

    m_pCategories = VclPtr<ListBox>::Create(this, WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_DROPDOWN|WB_SIMPLEMODE);
    m_pCategories->Show();

    SetOptimalSize();
}

ClassificationControl::~ClassificationControl()
{
    disposeOnce();
}

void ClassificationControl::dispose()
{
    m_pLabel.disposeAndClear();
    m_pCategories.disposeAndClear();
    vcl::Window::dispose();
}

void ClassificationControl::Resize()
{
    // Give the label what it wants, and the remaining size to the listbox.
    Size aSize(GetOutputSizePixel());
    long nWLabel = m_pLabel->GetOutputSizePixel().Width();
    long nW = aSize.Width();
    long nH = aSize.Height();

    long nPrefHeight = m_pLabel->get_preferred_size().Height();
    long nOffset = (nH - nPrefHeight) / 2;
    m_pLabel->SetPosSizePixel(Point(0, nOffset), Size(nWLabel, nPrefHeight));

    nPrefHeight = m_pCategories->get_preferred_size().Height();
    nOffset = (nH - nPrefHeight) / 2;
    m_pCategories->SetPosSizePixel(Point(nWLabel, nOffset), Size(nW - nWLabel, nPrefHeight));
}

VclPtr<ListBox> ClassificationControl::getCategories()
{
    return m_pCategories;
}

void ClassificationControl::SetOptimalSize()
{
    // Same as SvxColorDockingWindow.
    const Size aLogicalAttrSize(150, 0);
    Size aSize(LogicToPixel(aLogicalAttrSize,MAP_APPFONT));

    Point aPosition = m_pCategories->GetPosPixel();

    aSize.Height() = std::max(aSize.Height(), m_pLabel->get_preferred_size().Height());
    aSize.Height() = std::max(aSize.Height(), m_pCategories->get_preferred_size().Height());

    aSize.Width() = aPosition.X() + aSize.Width();

    SetSizePixel(aSize);
}

void ClassificationControl::DataChanged(const DataChangedEvent& rEvent)
{
    if ((rEvent.GetType() == DataChangedEventType::SETTINGS) && (rEvent.GetFlags() & AllSettingsFlags::STYLE))
        SetOptimalSize();

    Window::DataChanged(rEvent);
}

} // namespace sfx2

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* SAL_CALL com_sun_star_sfx2_ClassificationCategoriesController_get_implementation(uno::XComponentContext* pContext, const uno::Sequence<uno::Any>&)
{
    return cppu::acquire(new sfx2::ClassificationCategoriesController(pContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
