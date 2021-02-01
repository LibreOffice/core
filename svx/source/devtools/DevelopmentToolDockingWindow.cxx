/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <svx/devtools/DevelopmentToolDockingWindow.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>

#include <comphelper/processfactory.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svx/svxids.hrc>

#include <sfx2/objsh.hxx>

#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

using namespace css;

namespace
{
typedef cppu::WeakComponentImplHelper<css::view::XSelectionChangeListener>
    SelectionChangeHandlerInterfaceBase;

class SelectionChangeHandler final : private ::cppu::BaseMutex,
                                     public SelectionChangeHandlerInterfaceBase
{
private:
    css::uno::Reference<css::frame::XController> mxController;
    VclPtr<DevelopmentToolDockingWindow> mpDockingWindow;

public:
    SelectionChangeHandler(const css::uno::Reference<css::frame::XController>& rxController,
                           DevelopmentToolDockingWindow* pDockingWindow)
        : SelectionChangeHandlerInterfaceBase(m_aMutex)
        , mxController(rxController)
        , mpDockingWindow(pDockingWindow)
    {
        connect();
    }

    ~SelectionChangeHandler() { mpDockingWindow.disposeAndClear(); }

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& /*rEvent*/) override
    {
        uno::Reference<view::XSelectionSupplier> xSupplier(mxController, uno::UNO_QUERY);
        if (xSupplier.is())
        {
            uno::Any aAny = xSupplier->getSelection();
            auto xInterface = aAny.get<uno::Reference<uno::XInterface>>();
            mpDockingWindow->selectionChanged(xInterface);
        }
    }

    void connect()
    {
        uno::Reference<view::XSelectionSupplier> xSupplier(mxController, uno::UNO_QUERY);
        xSupplier->addSelectionChangeListener(this);
    }

    void disconnect()
    {
        uno::Reference<view::XSelectionSupplier> xSupplier(mxController, uno::UNO_QUERY);
        xSupplier->removeSelectionChangeListener(this);
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject& /*rEvent*/) override {}
    virtual void SAL_CALL disposing() override {}

private:
    SelectionChangeHandler(const SelectionChangeHandler&) = delete;
    SelectionChangeHandler& operator=(const SelectionChangeHandler&) = delete;
};

} // end anonymous namespace

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "svx/ui/developmenttool.ui")
    , mpClassNameLabel(m_xBuilder->weld_label("class_name_value_id"))
    , mpClassListBox(m_xBuilder->weld_tree_view("class_listbox_id"))
    , mpLeftSideTreeView(m_xBuilder->weld_tree_view("leftside_treeview_id"))
    , mpSelectionToggle(m_xBuilder->weld_toggle_button("selection_toggle"))
    , maDocumentModelTreeHandler(
          mpLeftSideTreeView,
          pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel())
{
    mpLeftSideTreeView->connect_changed(LINK(this, DevelopmentToolDockingWindow, LeftSideSelected));
    mpSelectionToggle->connect_toggled(LINK(this, DevelopmentToolDockingWindow, SelectionToggled));

    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();

    mxRoot = pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel();

    introspect(mxRoot);
    maDocumentModelTreeHandler.inspectDocument();
    mxSelectionListener.set(new SelectionChangeHandler(xController, this));
}

IMPL_LINK(DevelopmentToolDockingWindow, LeftSideSelected, weld::TreeView&, rView, void)
{
    if (mpSelectionToggle->get_state() == TRISTATE_TRUE)
        return;

    OUString sID = rView.get_selected_id();
    auto xObject = DocumentModelTreeHandler::getObjectByID(sID);
    if (xObject.is())
        introspect(xObject);
}

IMPL_LINK_NOARG(DevelopmentToolDockingWindow, SelectionToggled, weld::ToggleButton&, void)
{
    updateSelection();
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::dispose()
{
    auto* pSelectionChangeHandler
        = dynamic_cast<SelectionChangeHandler*>(mxSelectionListener.get());
    if (pSelectionChangeHandler)
        pSelectionChangeHandler->disconnect();

    mxSelectionListener = uno::Reference<view::XSelectionChangeListener>();
    maDocumentModelTreeHandler.dispose();

    mpClassNameLabel.reset();
    mpClassListBox.reset();
    mpSelectionToggle.reset();
    mpLeftSideTreeView.reset();

    SfxDockingWindow::dispose();
}

void DevelopmentToolDockingWindow::updateSelection()
{
    TriState eTriState = mpSelectionToggle->get_state();
    if (eTriState == TRISTATE_TRUE)
    {
        introspect(mxCurrentSelection);
        mpLeftSideTreeView->set_sensitive(false);
    }
    else
    {
        mpLeftSideTreeView->set_sensitive(true);
        LeftSideSelected(*mpLeftSideTreeView);
    }
}

void DevelopmentToolDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size(300, 300));

    Invalidate();
}

void DevelopmentToolDockingWindow::selectionChanged(
    uno::Reference<uno::XInterface> const& xInterface)
{
    mxCurrentSelection = xInterface;
    updateSelection();
}

namespace
{
uno::Reference<reflection::XIdlClass>
TypeToIdlClass(const uno::Type& rType, const uno::Reference<uno::XComponentContext>& xContext)
{
    auto xReflection = reflection::theCoreReflection::get(xContext);

    uno::Reference<reflection::XIdlClass> xRetClass;
    typelib_TypeDescription* pTD = nullptr;
    rType.getDescription(&pTD);
    if (pTD)
    {
        OUString sOWName(pTD->pTypeName);
        xRetClass = xReflection->forName(sOWName);
    }
    return xRetClass;
}

OUString AnyToString(const uno::Any& aValue, const uno::Reference<uno::XComponentContext>& xContext)
{
    uno::Type aValType = aValue.getValueType();
    uno::TypeClass eType = aValType.getTypeClass();

    OUString aRetStr;
    switch (eType)
    {
        case uno::TypeClass_TYPE:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <TYPE>";
            break;
        }
        case uno::TypeClass_INTERFACE:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <INTERFACE>";
            break;
        }
        case uno::TypeClass_SERVICE:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <SERVICE>";
            break;
        }
        case uno::TypeClass_STRUCT:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <STRUCT>";
            break;
        }
        case uno::TypeClass_TYPEDEF:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <TYPEDEF>";
            break;
        }
        case uno::TypeClass_ENUM:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <ENUM>";
            break;
        }
        case uno::TypeClass_EXCEPTION:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <EXCEPTION>";
            break;
        }
        case uno::TypeClass_SEQUENCE:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <SEQUENCE>";
            break;
        }
        case uno::TypeClass_VOID:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <VOID>";
            break;
        }
        case uno::TypeClass_ANY:
        {
            auto xIdlClass = TypeToIdlClass(aValType, xContext);
            aRetStr = xIdlClass->getName() + " <ANY>";
            break;
        }
        case uno::TypeClass_UNKNOWN:
            aRetStr = "<Unknown>";
            break;
        case uno::TypeClass_BOOLEAN:
        {
            bool bBool = aValue.get<bool>();
            aRetStr = bBool ? u"True" : u"False";
            break;
        }
        case uno::TypeClass_CHAR:
        {
            sal_Unicode aChar = aValue.get<sal_Unicode>();
            aRetStr = OUString::number(aChar);
            break;
        }
        case uno::TypeClass_STRING:
        {
            aRetStr = "\"" + aValue.get<OUString>() + "\"";
            break;
        }
        case uno::TypeClass_FLOAT:
        {
            auto aNumber = aValue.get<float>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_DOUBLE:
        {
            auto aNumber = aValue.get<double>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_BYTE:
        {
            auto aNumber = aValue.get<sal_Int8>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_SHORT:
        {
            auto aNumber = aValue.get<sal_Int16>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_LONG:
        {
            auto aNumber = aValue.get<sal_Int32>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_HYPER:
        {
            auto aNumber = aValue.get<sal_Int64>();
            aRetStr = OUString::number(aNumber);
            break;
        }

        default:
            break;
    }
    return aRetStr;
}
}

void DevelopmentToolDockingWindow::introspect(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    if (!xContext.is())
        return;

    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    OUString aImplementationName = xServiceInfo->getImplementationName();

    mpClassNameLabel->set_label(aImplementationName);

    mpClassListBox->freeze();
    mpClassListBox->clear();

    std::unique_ptr<weld::TreeIter> pParent = mpClassListBox->make_iterator();
    OUString aServicesString("Services");
    mpClassListBox->insert(nullptr, -1, &aServicesString, nullptr, nullptr, nullptr, false,
                           pParent.get());
    mpClassListBox->set_text_emphasis(*pParent, true, 0);

    std::unique_ptr<weld::TreeIter> pResult = mpClassListBox->make_iterator();
    const uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
    for (auto const& aServiceName : aServiceNames)
    {
        mpClassListBox->insert(pParent.get(), -1, &aServiceName, nullptr, nullptr, nullptr, false,
                               pResult.get());
        mpClassListBox->set_text_emphasis(*pResult, false, 0);
    }

    uno::Reference<beans::XIntrospection> xIntrospection;
    xIntrospection = beans::theIntrospection::get(xContext);

    uno::Reference<beans::XIntrospectionAccess> xIntrospectionAccess;
    xIntrospectionAccess = xIntrospection->inspect(uno::makeAny(xInterface));
    {
        OUString aPropertiesString("Properties");

        mpClassListBox->insert(nullptr, -1, &aPropertiesString, nullptr, nullptr, nullptr, false,
                               pParent.get());
        mpClassListBox->set_text_emphasis(*pParent, true, 0);

        uno::Reference<beans::XPropertySet> xPropertySet(xInterface, uno::UNO_QUERY);

        const auto xProperties = xIntrospectionAccess->getProperties(
            beans::PropertyConcept::ALL - beans::PropertyConcept::DANGEROUS);
        for (auto const& xProperty : xProperties)
        {
            mpClassListBox->insert(pParent.get(), -1, &xProperty.Name, nullptr, nullptr, nullptr,
                                   false, pResult.get());

            if (xPropertySet.is())
            {
                OUString aValue;
                try
                {
                    uno::Any aAny = xPropertySet->getPropertyValue(xProperty.Name);
                    aValue = AnyToString(aAny, xContext);
                }
                catch (const beans::UnknownPropertyException&)
                {
                    aValue = "UnknownPropertyException";
                }

                mpClassListBox->set_text(*pResult, aValue, 1);
            }
        }
    }

    {
        uno::Reference<lang::XTypeProvider> xTypeProvider(xInterface, uno::UNO_QUERY);
        if (xTypeProvider.is())
        {
            OUString aTypesString("Interfaces");
            mpClassListBox->insert(nullptr, -1, &aTypesString, nullptr, nullptr, nullptr, false,
                                   pParent.get());
            mpClassListBox->set_text_emphasis(*pParent, true, 0);

            const auto xSequenceTypes = xTypeProvider->getTypes();
            for (auto const& xType : xSequenceTypes)
            {
                OUString aName = xType.getTypeName();
                mpClassListBox->insert(pParent.get(), -1, &aName, nullptr, nullptr, nullptr, false,
                                       pResult.get());
            }
        }
    }

    OUString aMethodsString("Methods");
    mpClassListBox->insert(nullptr, -1, &aMethodsString, nullptr, nullptr, nullptr, false,
                           pParent.get());
    mpClassListBox->set_text_emphasis(*pParent, true, 0);

    const auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
    for (auto const& xMethod : xMethods)
    {
        OUString aMethodName = xMethod->getName();
        mpClassListBox->insert(pParent.get(), -1, &aMethodName, nullptr, nullptr, nullptr, false,
                               pResult.get());
        mpClassListBox->set_text_emphasis(*pResult, false, 0);
    }

    mpClassListBox->thaw();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
