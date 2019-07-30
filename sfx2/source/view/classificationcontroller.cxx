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

#include <vcl/vclptr.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/event.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sfx2/strings.hrc>
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
    void setProperty(const uno::Any& rProperty) override;
};

using ClassificationCategoriesControllerBase = cppu::ImplInheritanceHelper<svt::ToolboxController, lang::XServiceInfo>;

class ClassificationControl;

/// Controller for .uno:ClassificationApply.
class ClassificationCategoriesController : public ClassificationCategoriesControllerBase
{
    VclPtr<ClassificationControl> m_pClassification;
    rtl::Reference<comphelper::ConfigurationListener> m_xListener;
    ClassificationPropertyListener m_aPropertyListener;

    DECL_LINK(SelectHdl, ListBox&, void);

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
class SAL_WARN_UNUSED ClassificationControl : public vcl::Window
{
    VclPtr<FixedText> m_pLabel;
    VclPtr<ListBox> m_pCategory;
    void SetOptimalSize();
    void DataChanged(const DataChangedEvent& rEvent) override;

public:
    explicit ClassificationControl(vcl::Window* pParent);
    ~ClassificationControl() override;
    void dispose() override;
    void Resize() override;
    const VclPtr<ListBox>& getCategory()
    {
        return m_pCategory;
    }
    static sfx::ClassificationCreationOrigin getExistingClassificationOrigin();
    void toggleInteractivityOnOrigin();
    void setCategoryStateFromPolicy(SfxClassificationHelper & rHelper);
};

namespace
{

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
        m_pClassification->getCategory()->SetSelectHdl(LINK(this, ClassificationCategoriesController, SelectHdl));
    }

    return VCLUnoHelper::GetInterface(m_pClassification);
}

IMPL_LINK(ClassificationCategoriesController, SelectHdl, ListBox&, rCategory, void)
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
        OUString aEntry = rCategory.GetSelectedEntry();

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
        VclPtr<ListBox> pCategories = m_pClassification->getCategory();
        if (pCategories->GetEntryCount() == 0)
        {
            std::vector<OUString> aNames = aHelper.GetBACNames();
            for (const OUString& rName : aNames)
                pCategories->InsertEntry(rName);
            // Normally VclBuilder::makeObject() does this.
            pCategories->EnableAutoSize(true);
        }
    }

    // Restore state based on the doc. model.
    m_pClassification->setCategoryStateFromPolicy(aHelper);

}

void ClassificationCategoriesController::removeEntries()
{
    m_pClassification->getCategory()->Clear();
}

// WB_NOLABEL means here that the control won't be replaced with a label
// when it wouldn't fit the available space.
ClassificationControl::ClassificationControl(vcl::Window* pParent)
    : Window(pParent, WB_DIALOGCONTROL | WB_NOLABEL)
{
    m_pLabel = VclPtr<FixedText>::Create(this, WB_CENTER);
    m_pCategory = VclPtr<ListBox>::Create(this, WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_DROPDOWN|WB_SIMPLEMODE);

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
    Size aTextSize(m_pLabel->GetTextWidth(aText), m_pLabel->GetTextHeight());

    // Padding.
    aTextSize.AdjustWidth(12 );
    m_pLabel->SetText(aText);
    m_pLabel->SetSizePixel(aTextSize);
    m_pLabel->Show();

    m_pCategory->Show();

    SetOptimalSize();
}

ClassificationControl::~ClassificationControl()
{
    disposeOnce();
}

void ClassificationControl::dispose()
{
    m_pLabel.disposeAndClear();
    m_pCategory.disposeAndClear();
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

    nPrefHeight = m_pCategory->get_preferred_size().Height();
    nOffset = (nH - nPrefHeight) / 2;
    m_pCategory->SetPosSizePixel(Point(0 + nWLabel, nOffset), Size(nW - nWLabel, nPrefHeight));
}

void ClassificationControl::SetOptimalSize()
{
    // Same as SvxColorDockingWindow.
    const Size aLogicalAttrSize(150, 0);
    Size aSize(LogicToPixel(aLogicalAttrSize, MapMode(MapUnit::MapAppFont)));

    Point aPosition = m_pCategory->GetPosPixel();

    aSize.setHeight( std::max(aSize.Height(), m_pLabel->get_preferred_size().Height()) );
    aSize.setHeight( std::max(aSize.Height(), m_pCategory->get_preferred_size().Height()) );

    aSize.setWidth( aPosition.X() + aSize.Width() );

    SetSizePixel(aSize);
}

void ClassificationControl::DataChanged(const DataChangedEvent& rEvent)
{
    if ((rEvent.GetType() == DataChangedEventType::SETTINGS) && (rEvent.GetFlags() & AllSettingsFlags::STYLE))
        SetOptimalSize();

    toggleInteractivityOnOrigin();

    Window::DataChanged(rEvent);
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
        Disable();
    }
    else
    {
        Enable();
    }
}

void ClassificationControl::setCategoryStateFromPolicy(SfxClassificationHelper & rHelper)
{
    const OUString& rCategoryName = rHelper.GetBACName(SfxClassificationHelper::getPolicyType());
    if (!rCategoryName.isEmpty())
    {
        getCategory()->SelectEntry(rCategoryName);
    }
}

} // namespace sfx2

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* com_sun_star_sfx2_ClassificationCategoriesController_get_implementation(uno::XComponentContext* pContext, const uno::Sequence<uno::Any>&)
{
    return cppu::acquire(new sfx2::ClassificationCategoriesController(pContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
