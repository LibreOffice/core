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

#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/Invocation.hpp>

#include <comphelper/processfactory.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svx/svxids.hrc>

#include <sfx2/objsh.hxx>

#include <sfx2/viewfrm.hxx>

#include <com/sun/star/frame/XController.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include "SelectionChangeHandler.hxx"

using namespace css;

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

// Object inspector nodes

class ObjectInspectorNode
{
public:
    css::uno::Reference<css::uno::XInterface> mxObject;

    ObjectInspectorNode() = default;

    ObjectInspectorNode(css::uno::Reference<css::uno::XInterface> const& xObject)
        : mxObject(xObject)
    {
    }

    virtual ~ObjectInspectorNode() {}

    virtual OUString getObjectName() = 0;

    virtual void fillChildren(std::unique_ptr<weld::TreeView>& rTree, weld::TreeIter const& rParent)
        = 0;
};

OUString lclAppendNode(std::unique_ptr<weld::TreeView>& pTree, ObjectInspectorNode* pEntry,
                       bool bChildrenOnDemand = false)
{
    OUString sName = pEntry->getObjectName();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    std::unique_ptr<weld::TreeIter> pCurrent = pTree->make_iterator();
    pTree->insert(nullptr, -1, &sName, &sId, nullptr, nullptr, bChildrenOnDemand, pCurrent.get());
    pTree->set_text_emphasis(*pCurrent, true, 0);
    return sId;
}

OUString lclAppendNodeToParent(std::unique_ptr<weld::TreeView>& pTree,
                               weld::TreeIter const& rParent, ObjectInspectorNode* pEntry,
                               bool bChildrenOnDemand = false)
{
    OUString sName = pEntry->getObjectName();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    std::unique_ptr<weld::TreeIter> pCurrent = pTree->make_iterator();
    pTree->insert(&rParent, -1, &sName, &sId, nullptr, nullptr, bChildrenOnDemand, nullptr);
    pTree->set_text_emphasis(*pCurrent, true, 0);
    return sId;
}

OUString lclAppendNodeWithIterToParent(std::unique_ptr<weld::TreeView>& pTree,
                                       weld::TreeIter const& rParent, weld::TreeIter& rCurrent,
                                       ObjectInspectorNode* pEntry, bool bChildrenOnDemand = false)
{
    OUString sName = pEntry->getObjectName();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    pTree->insert(&rParent, -1, &sName, &sId, nullptr, nullptr, bChildrenOnDemand, &rCurrent);
    pTree->set_text_emphasis(rCurrent, true, 0);
    return sId;
}

class ObjectInspectorNamedNode : public ObjectInspectorNode
{
public:
    OUString msName;

    ObjectInspectorNamedNode(OUString const& rName,
                             css::uno::Reference<css::uno::XInterface> const& xObject)
        : ObjectInspectorNode(xObject)
        , msName(rName)
    {
    }

    OUString getObjectName() override { return msName; }

    void fillChildren(std::unique_ptr<weld::TreeView>& /*rTree*/,
                      weld::TreeIter const& /*rParent*/) override
    {
    }
};

class ServicesNode : public ObjectInspectorNamedNode
{
public:
    ServicesNode(css::uno::Reference<css::uno::XInterface> const& xObject)
        : ObjectInspectorNamedNode("Services", xObject)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        auto xServiceInfo = uno::Reference<lang::XServiceInfo>(mxObject, uno::UNO_QUERY);
        const uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
        for (auto const& aServiceName : aServiceNames)
        {
            lclAppendNodeToParent(pTree, rParent,
                                  new ObjectInspectorNamedNode(aServiceName, mxObject));
        }
    }
};

class GenericPropertiesNode : public ObjectInspectorNamedNode
{
public:
    uno::Reference<uno::XComponentContext> mxContext;

    GenericPropertiesNode(OUString const& rName, uno::Reference<uno::XInterface> const& xObject,
                          uno::Reference<uno::XComponentContext> const& xContext)
        : ObjectInspectorNamedNode(rName, xObject)
        , mxContext(xContext)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        uno::Reference<beans::XIntrospection> xIntrospection
            = beans::theIntrospection::get(mxContext);
        auto xIntrospectionAccess = xIntrospection->inspect(uno::makeAny(mxObject));
        auto xInvocationFactory = css::script::Invocation::create(mxContext);
        uno::Sequence<uno::Any> aParameters = { uno::Any(mxObject) };
        auto xInvocationInterface = xInvocationFactory->createInstanceWithArguments(aParameters);
        uno::Reference<script::XInvocation> xInvocation(xInvocationInterface, uno::UNO_QUERY);

        const auto xProperties = xIntrospectionAccess->getProperties(
            beans::PropertyConcept::ALL - beans::PropertyConcept::DANGEROUS);

        for (auto const& xProperty : xProperties)
        {
            OUString aValue;
            uno::Any aAny;
            uno::Reference<uno::XInterface> xCurrent = mxObject;

            try
            {
                if (xInvocation->hasProperty(xProperty.Name))
                {
                    aAny = xInvocation->getValue(xProperty.Name);
                    aValue = AnyToString(aAny, mxContext);
                }
            }
            catch (...)
            {
                aValue = "<?>";
            }

            bool bComplex = false;
            if (aAny.hasValue())
            {
                auto xInterface = uno::Reference<uno::XInterface>(aAny, uno::UNO_QUERY);
                if (xInterface.is())
                {
                    xCurrent = xInterface;
                    bComplex = true;
                }
            }

            std::unique_ptr<weld::TreeIter> pCurrent = pTree->make_iterator();
            if (bComplex)
            {
                lclAppendNodeWithIterToParent(
                    pTree, rParent, *pCurrent,
                    new GenericPropertiesNode(xProperty.Name, xCurrent, mxContext), true);
            }
            else
            {
                lclAppendNodeWithIterToParent(
                    pTree, rParent, *pCurrent,
                    new ObjectInspectorNamedNode(xProperty.Name, xCurrent), false);
            }

            if (!aValue.isEmpty())
            {
                pTree->set_text(*pCurrent, aValue, 1);
            }
        }
    }
};

class PropertiesNode : public GenericPropertiesNode
{
public:
    PropertiesNode(uno::Reference<uno::XInterface> const& xObject,
                   uno::Reference<uno::XComponentContext> const& xContext)
        : GenericPropertiesNode("Properties", xObject, xContext)
    {
    }
};

class InterfacesNode : public ObjectInspectorNamedNode
{
public:
    InterfacesNode(css::uno::Reference<css::uno::XInterface> const& xObject)
        : ObjectInspectorNamedNode("Interfaces", xObject)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        uno::Reference<lang::XTypeProvider> xTypeProvider(mxObject, uno::UNO_QUERY);
        if (xTypeProvider.is())
        {
            const auto xSequenceTypes = xTypeProvider->getTypes();
            for (auto const& xType : xSequenceTypes)
            {
                OUString aName = xType.getTypeName();
                lclAppendNodeToParent(pTree, rParent,
                                      new ObjectInspectorNamedNode(aName, mxObject));
            }
        }
    }
};

class MethodsNode : public ObjectInspectorNamedNode
{
public:
    uno::Reference<uno::XComponentContext> mxContext;

    MethodsNode(css::uno::Reference<css::uno::XInterface> const& xObject,
                uno::Reference<uno::XComponentContext> const& xContext)
        : ObjectInspectorNamedNode("Methods", xObject)
        , mxContext(xContext)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        uno::Reference<beans::XIntrospection> xIntrospection
            = beans::theIntrospection::get(mxContext);
        auto xIntrospectionAccess = xIntrospection->inspect(uno::makeAny(mxObject));

        const auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
        for (auto const& xMethod : xMethods)
        {
            OUString aMethodName = xMethod->getName();

            lclAppendNodeToParent(pTree, rParent,
                                  new ObjectInspectorNamedNode(aMethodName, mxObject));
        }
    }
};

} // end anonymous namespace

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "svx/ui/developmenttool.ui")
    , mpClassNameLabel(m_xBuilder->weld_label("class_name_value_id"))
    , mpClassListBox(m_xBuilder->weld_tree_view("class_listbox_id"))
    , mpDocumentModelTreeView(m_xBuilder->weld_tree_view("leftside_treeview_id"))
    , mpSelectionToggle(m_xBuilder->weld_toggle_button("selection_toggle"))
    , maDocumentModelTreeHandler(
          mpDocumentModelTreeView,
          pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel())
{
    mpDocumentModelTreeView->connect_changed(
        LINK(this, DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler));
    mpSelectionToggle->connect_toggled(LINK(this, DevelopmentToolDockingWindow, SelectionToggled));
    mpClassListBox->connect_expanding(
        LINK(this, DevelopmentToolDockingWindow, ObjectInspectorExpandingHandler));

    auto* pViewFrame = pInputBindings->GetDispatcher()->GetFrame();

    uno::Reference<frame::XController> xController = pViewFrame->GetFrame().GetController();

    mxRoot = pInputBindings->GetDispatcher()->GetFrame()->GetObjectShell()->GetBaseModel();

    introspect(mxRoot);
    maDocumentModelTreeHandler.inspectDocument();
    mxSelectionListener.set(new SelectionChangeHandler(xController, this));
}

IMPL_LINK(DevelopmentToolDockingWindow, DocumentModelTreeViewSelectionHandler, weld::TreeView&,
          rView, void)
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

void DevelopmentToolDockingWindow::clearObjectInspectorChildren(weld::TreeIter const& rParent)
{
    bool bChild = false;
    do
    {
        bChild = mpClassListBox->iter_has_child(rParent);
        if (bChild)
        {
            std::unique_ptr<weld::TreeIter> pChild = mpClassListBox->make_iterator(&rParent);
            bChild = mpClassListBox->iter_children(*pChild);
            if (bChild)
            {
                clearObjectInspectorChildren(*pChild);
                OUString sID = mpClassListBox->get_id(*pChild);
                auto* pEntry = reinterpret_cast<ObjectInspectorNode*>(sID.toInt64());
                delete pEntry;
                mpClassListBox->remove(*pChild);
            }
        }
    } while (bChild);
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::dispose()
{
    auto* pSelectionChangeHandler
        = dynamic_cast<SelectionChangeHandler*>(mxSelectionListener.get());
    if (pSelectionChangeHandler)
        pSelectionChangeHandler->stopListening();

    mxSelectionListener = uno::Reference<view::XSelectionChangeListener>();
    maDocumentModelTreeHandler.dispose();

    // destroy all ObjectInspectorNodes from the tree
    mpClassListBox->all_foreach([this](weld::TreeIter& rEntry) {
        OUString sID = mpClassListBox->get_id(rEntry);
        auto* pEntry = reinterpret_cast<ObjectInspectorNode*>(sID.toInt64());
        delete pEntry;
        return false;
    });

    // dispose welded objects
    mpClassNameLabel.reset();
    mpClassListBox.reset();
    mpSelectionToggle.reset();
    mpDocumentModelTreeView.reset();

    SfxDockingWindow::dispose();
}

void DevelopmentToolDockingWindow::updateSelection()
{
    TriState eTriState = mpSelectionToggle->get_state();
    if (eTriState == TRISTATE_TRUE)
    {
        introspect(mxCurrentSelection);
        mpDocumentModelTreeView->set_sensitive(false);
    }
    else
    {
        mpDocumentModelTreeView->set_sensitive(true);
        DocumentModelTreeViewSelectionHandler(*mpDocumentModelTreeView);
    }
}

void DevelopmentToolDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size(300, 300));

    Invalidate();
}

IMPL_LINK(DevelopmentToolDockingWindow, ObjectInspectorExpandingHandler, weld::TreeIter const&,
          rParent, bool)
{
    OUString sID = mpClassListBox->get_id(rParent);
    if (sID.isEmpty())
        return true;

    clearObjectInspectorChildren(rParent);
    auto* pNode = reinterpret_cast<ObjectInspectorNode*>(sID.toInt64());
    pNode->fillChildren(mpClassListBox, rParent);

    return true;
}

void DevelopmentToolDockingWindow::selectionChanged(
    uno::Reference<uno::XInterface> const& xInterface)
{
    mxCurrentSelection = xInterface;
    updateSelection();
}

void DevelopmentToolDockingWindow::introspect(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
    if (!xContext.is())
        return;

    // Set implementation name
    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    OUString aImplementationName = xServiceInfo->getImplementationName();
    mpClassNameLabel->set_label(aImplementationName);

    // fill object inspector

    mpClassListBox->freeze();
    mpClassListBox->clear();

    lclAppendNode(mpClassListBox, new ServicesNode(xInterface), true);
    lclAppendNode(mpClassListBox, new InterfacesNode(xInterface), true);
    lclAppendNode(mpClassListBox, new PropertiesNode(xInterface, xContext), true);
    lclAppendNode(mpClassListBox, new MethodsNode(xInterface, xContext), true);

    mpClassListBox->thaw();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
