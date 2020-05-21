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
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/weld.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/configurationlistener.hxx>

using namespace com::sun::star;

namespace sfx2
{

namespace {

class ClassificationCategoriesController;

}

using ClassificationPropertyListenerBase = comphelper::ConfigurationListenerProperty<OUString>;

namespace {

/// Listens to configuration changes, so no restart is needed after setting the classification path.
class ClassificationPropertyListener : public ClassificationPropertyListenerBase
{
    ClassificationCategoriesController& m_rController;

public:
    ClassificationPropertyListener(const rtl::Reference<comphelper::ConfigurationListener>& xListener, ClassificationCategoriesController& rController);
    void setProperty(const uno::Any& rProperty) override;
};

}

using ClassificationCategoriesControllerBase = cppu::ImplInheritanceHelper<svt::ToolboxController, lang::XServiceInfo>;

namespace {

class ClassificationControl;

/// Controller for .uno:ClassificationApply.
class ClassificationCategoriesController : public ClassificationCategoriesControllerBase
{
    VclPtr<ClassificationControl> m_pClassification;
    rtl::Reference<comphelper::ConfigurationListener> m_xListener;
    ClassificationPropertyListener m_aPropertyListener;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);

public:
    explicit ClassificationCategoriesController(const uno::Reference<uno::XComponentContext>& rContext);

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XComponent
    void SAL_CALL dispose() override;

    // XToolbarController
    uno::Reference<awt::XWindow> SAL_CALL createItemWindow(const uno::Reference<awt::XWindow>& rParent) override;

    // XStatusListener
    void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) override;

    void removeEntries();
};

/// Classification control is the parent of all widgets that belongs to ClassificationCategoriesController.
class SAL_WARN_UNUSED ClassificationControl final : public InterimItemWindow
{
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::ComboBox> m_xCategory;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    void SetOptimalSize();
    void DataChanged(const DataChangedEvent& rEvent) override;
    void GetFocus() override
    {
        if (m_xCategory)
            m_xCategory->grab_focus();
        InterimItemWindow::GetFocus();
    }

public:
    explicit ClassificationControl(vcl::Window* pParent);
    ~ClassificationControl() override;
    void dispose() override;
    weld::ComboBox& getCategory()
    {
        return *m_xCategory;
    }
    void set_sensitive(bool bSensitive)
    {
        Enable(bSensitive);
        m_xContainer->set_sensitive(bSensitive);
    }
    static sfx::ClassificationCreationOrigin getExistingClassificationOrigin();
    void toggleInteractivityOnOrigin();
    void setCategoryStateFromPolicy(const SfxClassificationHelper & rHelper);
};

OUString const & getCategoryType()
{
    return SfxClassificationHelper::policyTypeToString(SfxClassificationHelper::getPolicyType());
}

} // end anonymous namespace

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

OUString ClassificationCategoriesController::getImplementationName()
{
    return "com.sun.star.comp.sfx2.ClassificationCategoriesController";
}

sal_Bool ClassificationCategoriesController::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> ClassificationCategoriesController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

void ClassificationCategoriesController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    svt::ToolboxController::dispose();
    m_pClassification.disposeAndClear();
    m_aPropertyListener.dispose();
    m_xListener->dispose();
}

uno::Reference<awt::XWindow> ClassificationCategoriesController::createItemWindow(const uno::Reference<awt::XWindow>& rParent)
{
    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
    auto pToolbar = dynamic_cast<ToolBox*>(pParent.get());
    if (pToolbar)
    {
        m_pClassification = VclPtr<ClassificationControl>::Create(pToolbar);
        m_pClassification->getCategory().connect_changed(LINK(this, ClassificationCategoriesController, SelectHdl));
        m_pClassification->Show();
    }

    return VCLUnoHelper::GetInterface(m_pClassification);
}

IMPL_LINK(ClassificationCategoriesController, SelectHdl, weld::ComboBox&, rCategory, void)
{
    m_pClassification->toggleInteractivityOnOrigin();

    if (ClassificationControl::getExistingClassificationOrigin() == sfx::ClassificationCreationOrigin::MANUAL)
    {
        SfxObjectShell* pObjectShell = SfxObjectShell::Current();
        if (!pObjectShell)
            return;
        SfxClassificationHelper aHelper(pObjectShell->getDocProperties());
        m_pClassification->setCategoryStateFromPolicy(aHelper);
    }
    else
    {
        OUString aEntry = rCategory.get_active_text();

        const OUString& aType = getCategoryType();
        uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
            {"Name", uno::makeAny(aEntry)},
            {"Type", uno::makeAny(aType)},
        }));
        comphelper::dispatchCommand(".uno:ClassificationApply", aPropertyValues);
    }
}

void ClassificationCategoriesController::statusChanged(const frame::FeatureStateEvent& /*rEvent*/)
{
    if (!m_pClassification)
        return;

    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    if (!pObjectShell)
        return;

    SfxClassificationHelper aHelper(pObjectShell->getDocProperties());

    //toggle if the pop-up is enabled/disabled
    m_pClassification->toggleInteractivityOnOrigin();

    // check if classification was set via the advanced dialog
    if (ClassificationControl::getExistingClassificationOrigin() != sfx::ClassificationCreationOrigin::MANUAL)
    {
        weld::ComboBox& rCategories = m_pClassification->getCategory();
        if (rCategories.get_count() == 0)
        {
            std::vector<OUString> aNames = aHelper.GetBACNames();
            for (const OUString& rName : aNames)
                rCategories.append_text(rName);
        }
    }

    // Restore state based on the doc. model.
    m_pClassification->setCategoryStateFromPolicy(aHelper);

}

void ClassificationCategoriesController::removeEntries()
{
    m_pClassification->getCategory().clear();
}

ClassificationControl::ClassificationControl(vcl::Window* pParent)
    : InterimItemWindow(pParent, "sfx/ui/classificationbox.ui", "ClassificationBox")
    , m_xLabel(m_xBuilder->weld_label("label"))
    , m_xCategory(m_xBuilder->weld_combo_box("combobox"))
{
    m_xCategory->connect_key_press(LINK(this, ClassificationControl, KeyInputHdl));

    // WB_NOLABEL means here that the control won't be replaced with a label
    // when it wouldn't fit the available space.
    SetStyle(GetStyle() | WB_DIALOGCONTROL | WB_NOLABEL);

    OUString aText;
    switch (SfxClassificationHelper::getPolicyType())
    {
    case SfxClassificationPolicyType::IntellectualProperty:
        aText = SfxResId(STR_CLASSIFIED_INTELLECTUAL_PROPERTY);
        break;
    case SfxClassificationPolicyType::NationalSecurity:
        aText = SfxResId(STR_CLASSIFIED_NATIONAL_SECURITY);
        break;
    case SfxClassificationPolicyType::ExportControl:
        aText = SfxResId(STR_CLASSIFIED_EXPORT_CONTROL);
        break;
    }

    m_xLabel->set_label(aText);

    // Same as SvxColorDockingWindow.
    const Size aLogicalAttrSize(150, 0);
    Size aSize(LogicToPixel(aLogicalAttrSize, MapMode(MapUnit::MapAppFont)));
    m_xCategory->set_size_request(aSize.Width() - m_xLabel->get_preferred_size().Width(), -1);

    SetOptimalSize();
}

IMPL_LINK(ClassificationControl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

ClassificationControl::~ClassificationControl()
{
    disposeOnce();
}

void ClassificationControl::dispose()
{
    m_xLabel.reset();
    m_xCategory.reset();
    InterimItemWindow::dispose();
}

void ClassificationControl::SetOptimalSize()
{
    SetSizePixel(get_preferred_size());
}

void ClassificationControl::DataChanged(const DataChangedEvent& rEvent)
{
    if ((rEvent.GetType() == DataChangedEventType::SETTINGS) && (rEvent.GetFlags() & AllSettingsFlags::STYLE))
        SetOptimalSize();

    toggleInteractivityOnOrigin();

    InterimItemWindow::DataChanged(rEvent);
}

sfx::ClassificationCreationOrigin ClassificationControl::getExistingClassificationOrigin()
{
    SfxObjectShell* pObjectShell = SfxObjectShell::Current();
    if (!pObjectShell)
        return sfx::ClassificationCreationOrigin::NONE;

    uno::Reference<document::XDocumentProperties> xDocumentProperties = pObjectShell->getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();

    sfx::ClassificationKeyCreator aKeyCreator(SfxClassificationHelper::getPolicyType());
    return sfx::getCreationOriginProperty(xPropertyContainer, aKeyCreator);
}

void ClassificationControl::toggleInteractivityOnOrigin()
{
    if (getExistingClassificationOrigin() == sfx::ClassificationCreationOrigin::MANUAL)
    {
        set_sensitive(false);
    }
    else
    {
        set_sensitive(true);
    }
}

void ClassificationControl::setCategoryStateFromPolicy(const SfxClassificationHelper & rHelper)
{
    const OUString& rCategoryName = rHelper.GetBACName(SfxClassificationHelper::getPolicyType());
    if (!rCategoryName.isEmpty())
    {
        getCategory().set_active_text(rCategoryName);
    }
}

} // namespace sfx2

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* com_sun_star_sfx2_ClassificationCategoriesController_get_implementation(uno::XComponentContext* pContext, const uno::Sequence<uno::Any>&)
{
    return cppu::acquire(new sfx2::ClassificationCategoriesController(pContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
