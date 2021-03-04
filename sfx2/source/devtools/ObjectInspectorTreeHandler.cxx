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

#include <sfx2/devtools/ObjectInspectorTreeHandler.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

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
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/Invocation.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>

using namespace css;

namespace
{
constexpr OUStringLiteral constTypeDescriptionManagerSingletonName
    = u"/singletons/com.sun.star.reflection.theTypeDescriptionManager";

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
    OUString aRetStr;

    // return early if we don't have any value
    if (!aValue.hasValue())
        return aRetStr;

    uno::Type aValType = aValue.getValueType();
    uno::TypeClass eType = aValType.getTypeClass();

    switch (eType)
    {
        case uno::TypeClass_INTERFACE:
        {
            aRetStr = "<Object>";
            break;
        }
        case uno::TypeClass_STRUCT:
        {
            aRetStr = "<Struct>";
            break;
        }
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
        case uno::TypeClass_UNSIGNED_SHORT:
        {
            auto aNumber = aValue.get<sal_uInt16>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_UNSIGNED_LONG:
        {
            auto aNumber = aValue.get<sal_uInt32>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_UNSIGNED_HYPER:
        {
            auto aNumber = aValue.get<sal_uInt64>();
            aRetStr = OUString::number(aNumber);
            break;
        }
        case uno::TypeClass_TYPE:
        {
            auto aType = aValue.get<uno::Type>();
            aRetStr = aType.getTypeName();
            break;
        }
        case uno::TypeClass_ENUM:
        {
            sal_Int32 nIntValue = 0;
            if (cppu::enum2int(nIntValue, aValue))
            {
                uno::Reference<container::XHierarchicalNameAccess> xManager;
                xManager.set(xContext->getValueByName(constTypeDescriptionManagerSingletonName),
                             uno::UNO_QUERY);

                uno::Reference<reflection::XEnumTypeDescription> xTypeDescription;
                xTypeDescription.set(xManager->getByHierarchicalName(aValType.getTypeName()),
                                     uno::UNO_QUERY);

                uno::Sequence<sal_Int32> aValues = xTypeDescription->getEnumValues();
                sal_Int32 nValuesIndex
                    = std::find(aValues.begin(), aValues.end(), nIntValue) - aValues.begin();
                uno::Sequence<OUString> aNames = xTypeDescription->getEnumNames();
                aRetStr = aNames[nValuesIndex];
            }
            break;
        }

        default:
            break;
    }
    return aRetStr;
}

OUString getAnyType(const uno::Any& aValue, const uno::Reference<uno::XComponentContext>& xContext)
{
    uno::Type aValType = aValue.getValueType();
    auto xIdlClass = TypeToIdlClass(aValType, xContext);
    return xIdlClass->getName();
}

// Object inspector nodes

class ObjectInspectorNodeInterface
{
public:
    ObjectInspectorNodeInterface() = default;

    virtual ~ObjectInspectorNodeInterface() {}

    virtual OUString getObjectName() = 0;

    virtual bool shouldShowExpander() { return false; }

    virtual void fillChildren(std::unique_ptr<weld::TreeView>& rTree, const weld::TreeIter* pParent)
        = 0;

    virtual std::vector<std::pair<sal_Int32, OUString>> getColumnValues()
    {
        return std::vector<std::pair<sal_Int32, OUString>>();
    }
};

OUString lclAppendNode(std::unique_ptr<weld::TreeView>& pTree, ObjectInspectorNodeInterface* pEntry)
{
    OUString sName = pEntry->getObjectName();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    std::unique_ptr<weld::TreeIter> pCurrent = pTree->make_iterator();
    pTree->insert(nullptr, -1, &sName, &sId, nullptr, nullptr, pEntry->shouldShowExpander(),
                  pCurrent.get());
    pTree->set_text_emphasis(*pCurrent, true, 0);

    for (auto const& rPair : pEntry->getColumnValues())
    {
        pTree->set_text(*pCurrent, rPair.second, rPair.first);
    }

    return sId;
}

OUString lclAppendNodeToParent(std::unique_ptr<weld::TreeView>& pTree,
                               const weld::TreeIter* pParent, ObjectInspectorNodeInterface* pEntry)
{
    OUString sName = pEntry->getObjectName();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    std::unique_ptr<weld::TreeIter> pCurrent = pTree->make_iterator();
    pTree->insert(pParent, -1, &sName, &sId, nullptr, nullptr, pEntry->shouldShowExpander(),
                  pCurrent.get());
    pTree->set_text_emphasis(*pCurrent, true, 0);

    for (auto const& rPair : pEntry->getColumnValues())
    {
        pTree->set_text(*pCurrent, rPair.second, rPair.first);
    }

    return sId;
}

class SimpleStringNode : public ObjectInspectorNodeInterface
{
protected:
    OUString msName;

public:
    SimpleStringNode(OUString const& rName)
        : msName(rName)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& /*rTree*/,
                      const weld::TreeIter* /*pParent*/) override
    {
    }

    OUString getObjectName() override { return msName; }
};

class MethodNode : public ObjectInspectorNodeInterface
{
private:
    uno::Reference<reflection::XIdlMethod> mxMethod;

public:
    MethodNode(uno::Reference<reflection::XIdlMethod> const& xMethod)
        : mxMethod(xMethod)
    {
    }

    OUString getObjectName() override { return mxMethod->getName(); }

    static OUString simpleTypeName(uno::Reference<reflection::XIdlClass> const& xClass)
    {
        switch (xClass->getTypeClass())
        {
            case uno::TypeClass_INTERFACE:
                return "object";
            case uno::TypeClass_STRUCT:
                return "struct";
            case uno::TypeClass_ENUM:
                return "enum";
            case uno::TypeClass_SEQUENCE:
                return "sequence";
            default:
                break;
        }
        return xClass->getName();
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        OUString aOutString;
        auto xClass = mxMethod->getReturnType();
        aOutString = simpleTypeName(xClass);

        OUString aInString;
        const auto aParameters = mxMethod->getParameterInfos();
        bool bFirst = true;
        for (auto const& rParameterInfo : aParameters)
        {
            if (!bFirst)
                aInString += ", ";
            else
                bFirst = false;

            switch (rParameterInfo.aMode)
            {
                case reflection::ParamMode_IN:
                    aInString += "[in] ";
                    break;
                case reflection::ParamMode_OUT:
                    aInString += "[out] ";
                    break;
                case reflection::ParamMode_INOUT:
                    aInString += "[in&out] ";
                    break;
                default:
                    break;
            }

            aInString += rParameterInfo.aName + " : " + simpleTypeName(rParameterInfo.aType);
        }

        OUString aImplementationClass = mxMethod->getDeclaringClass()->getName();

        return {
            { 1, aOutString },
            { 2, aInString },
            { 3, aImplementationClass },
        };
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& /*rTree*/,
                      const weld::TreeIter* /*pParent*/) override
    {
    }
};

class BasicValueNode : public SimpleStringNode
{
protected:
    uno::Any maAny;
    uno::Reference<uno::XComponentContext> mxContext;

    ObjectInspectorNodeInterface* createNodeObjectForAny(OUString const& rName, uno::Any& rAny);

public:
    BasicValueNode(OUString const& rName, uno::Any const& rAny,
                   uno::Reference<uno::XComponentContext> const& xContext)
        : SimpleStringNode(rName)
        , maAny(rAny)
        , mxContext(xContext)
    {
    }

    uno::Any getAny() { return maAny; }

    bool shouldShowExpander() override
    {
        if (maAny.hasValue())
        {
            switch (maAny.getValueType().getTypeClass())
            {
                case uno::TypeClass_INTERFACE:
                case uno::TypeClass_SEQUENCE:
                    return true;
                default:
                    break;
            }
        }
        return false;
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        if (maAny.hasValue())
        {
            OUString aValue = AnyToString(maAny, mxContext);
            OUString aType = getAnyType(maAny, mxContext);

            return {
                { 1, aValue },
                { 2, aType },
            };
        }

        return ObjectInspectorNodeInterface::getColumnValues();
    }
};

class GenericPropertiesNode : public BasicValueNode
{
public:
    GenericPropertiesNode(OUString const& rName, uno::Any const& rAny,
                          uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode(rName, rAny, xContext)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      const weld::TreeIter* pParent) override;
};

class StructNode : public BasicValueNode
{
public:
    StructNode(OUString const& rName, uno::Any const& rAny,
               uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode(rName, rAny, xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      const weld::TreeIter* pParent) override;
};

class SequenceNode : public BasicValueNode
{
public:
    SequenceNode(OUString const& rName, uno::Any const& rAny,
                 uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode(rName, rAny, xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      const weld::TreeIter* pParent) override
    {
        auto xReflection = reflection::theCoreReflection::get(mxContext);
        uno::Reference<reflection::XIdlClass> xClass
            = xReflection->forName(maAny.getValueType().getTypeName());
        uno::Reference<reflection::XIdlArray> xIdlArray = xClass->getArray();

        int nLength = xIdlArray->getLen(maAny);

        for (int i = 0; i < nLength; i++)
        {
            uno::Any aArrayValue = xIdlArray->get(maAny, i);
            uno::Reference<uno::XInterface> xCurrent;

            auto* pObjectInspectorNode = createNodeObjectForAny(OUString::number(i), aArrayValue);
            if (pObjectInspectorNode)
                lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        auto xReflection = reflection::theCoreReflection::get(mxContext);
        uno::Reference<reflection::XIdlClass> xClass
            = xReflection->forName(maAny.getValueType().getTypeName());
        uno::Reference<reflection::XIdlArray> xIdlArray = xClass->getArray();

        int nLength = xIdlArray->getLen(maAny);

        OUString aValue = "0 to " + OUString::number(nLength - 1);
        OUString aType = getAnyType(maAny, mxContext);

        return {
            { 1, aValue },
            { 2, aType },
        };
    }
};

void GenericPropertiesNode::fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                                         const weld::TreeIter* pParent)
{
    if (!maAny.hasValue())
        return;

    const auto xNameAccess = uno::Reference<container::XNameAccess>(maAny, uno::UNO_QUERY);
    if (xNameAccess.is())
    {
        const uno::Sequence<OUString> aNames = xNameAccess->getElementNames();
        for (OUString const& rName : aNames)
        {
            uno::Any aAny = xNameAccess->getByName(rName);
            auto* pObjectInspectorNode = createNodeObjectForAny("@" + rName, aAny);
            lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
    }

    const auto xIndexAccess = uno::Reference<container::XIndexAccess>(maAny, uno::UNO_QUERY);
    if (xIndexAccess.is())
    {
        for (sal_Int32 nIndex = 0; nIndex < xIndexAccess->getCount(); ++nIndex)
        {
            uno::Any aAny = xIndexAccess->getByIndex(nIndex);
            auto* pObjectInspectorNode
                = createNodeObjectForAny("@" + OUString::number(nIndex), aAny);
            lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
    }

    const auto xEnumAccess = uno::Reference<container::XEnumerationAccess>(maAny, uno::UNO_QUERY);
    if (xEnumAccess.is())
    {
        uno::Reference<container::XEnumeration> xEnumeration = xEnumAccess->createEnumeration();
        if (xEnumeration.is())
        {
            for (sal_Int32 nIndex = 0; xEnumeration->hasMoreElements(); nIndex++)
            {
                uno::Any aAny = xEnumeration->nextElement();
                auto* pObjectInspectorNode
                    = createNodeObjectForAny("@{" + OUString::number(nIndex) + "}", aAny);
                lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
            }
        }
    }

    uno::Reference<beans::XIntrospection> xIntrospection = beans::theIntrospection::get(mxContext);
    auto xIntrospectionAccess = xIntrospection->inspect(maAny);
    auto xInvocationFactory = css::script::Invocation::create(mxContext);
    uno::Sequence<uno::Any> aParameters = { maAny };
    auto xInvocationInterface = xInvocationFactory->createInstanceWithArguments(aParameters);
    uno::Reference<script::XInvocation> xInvocation(xInvocationInterface, uno::UNO_QUERY);

    const auto xProperties = xIntrospectionAccess->getProperties(
        beans::PropertyConcept::ALL - beans::PropertyConcept::DANGEROUS);

    for (auto const& xProperty : xProperties)
    {
        uno::Any aCurrentAny;

        try
        {
            if (xInvocation->hasProperty(xProperty.Name))
            {
                aCurrentAny = xInvocation->getValue(xProperty.Name);
            }
        }
        catch (...)
        {
        }

        auto* pObjectInspectorNode = createNodeObjectForAny(xProperty.Name, aCurrentAny);
        if (pObjectInspectorNode)
            lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
    }
}

void StructNode::fillChildren(std::unique_ptr<weld::TreeView>& pTree, const weld::TreeIter* pParent)
{
    auto xReflection = reflection::theCoreReflection::get(mxContext);
    uno::Reference<reflection::XIdlClass> xClass
        = xReflection->forName(maAny.getValueType().getTypeName());

    const auto xFields = xClass->getFields();

    for (auto const& xField : xFields)
    {
        OUString aFieldName = xField->getName();
        uno::Any aFieldValue = xField->get(maAny);

        auto* pObjectInspectorNode = createNodeObjectForAny(aFieldName, aFieldValue);
        if (pObjectInspectorNode)
        {
            lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
    }
}

ObjectInspectorNodeInterface* BasicValueNode::createNodeObjectForAny(OUString const& rName,
                                                                     uno::Any& rAny)
{
    if (!rAny.hasValue())
        return nullptr;

    switch (rAny.getValueType().getTypeClass())
    {
        case uno::TypeClass_INTERFACE:
            return new GenericPropertiesNode(rName, rAny, mxContext);

        case uno::TypeClass_SEQUENCE:
            return new SequenceNode(rName, rAny, mxContext);

        case uno::TypeClass_STRUCT:
            return new StructNode(rName, rAny, mxContext);

        default:
            break;
    }

    return new BasicValueNode(rName, rAny, mxContext);
}

ObjectInspectorNodeInterface* getSelectedNode(weld::TreeView const& rTreeView)
{
    OUString sID = rTreeView.get_selected_id();
    if (sID.isEmpty())
        return nullptr;

    if (auto* pNode = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64()))
        return pNode;

    return nullptr;
}

uno::Reference<uno::XInterface> getSelectedXInterface(weld::TreeView const& rTreeView)
{
    uno::Reference<uno::XInterface> xInterface;

    if (auto* pNode = getSelectedNode(rTreeView))
    {
        if (auto* pBasicValueNode = dynamic_cast<BasicValueNode*>(pNode))
        {
            uno::Any aAny = pBasicValueNode->getAny();
            xInterface.set(aAny, uno::UNO_QUERY);
        }
    }

    return xInterface;
}

} // end anonymous namespace

ObjectInspectorTreeHandler::ObjectInspectorTreeHandler(
    std::unique_ptr<weld::TreeView>& pInterfacesTreeView,
    std::unique_ptr<weld::TreeView>& pServicesTreeView,
    std::unique_ptr<weld::TreeView>& pPropertiesTreeView,
    std::unique_ptr<weld::TreeView>& pMethodsTreeView,
    std::unique_ptr<weld::Label>& pClassNameLabel,
    std::unique_ptr<weld::Toolbar>& pObjectInspectorToolbar,
    std::unique_ptr<weld::Notebook>& pObjectInspectorNotebook)
    : mpInterfacesTreeView(pInterfacesTreeView)
    , mpServicesTreeView(pServicesTreeView)
    , mpPropertiesTreeView(pPropertiesTreeView)
    , mpMethodsTreeView(pMethodsTreeView)
    , mpClassNameLabel(pClassNameLabel)
    , mpObjectInspectorToolbar(pObjectInspectorToolbar)
    , mpObjectInspectorNotebook(pObjectInspectorNotebook)
{
    mpInterfacesTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerInterfaces));
    mpServicesTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerServices));
    mpPropertiesTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerProperties));
    mpMethodsTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerMethods));

    mpPropertiesTreeView->connect_popup_menu(
        LINK(this, ObjectInspectorTreeHandler, PopupMenuHandler));

    mpInterfacesTreeView->connect_changed(LINK(this, ObjectInspectorTreeHandler, SelectionChanged));
    mpServicesTreeView->connect_changed(LINK(this, ObjectInspectorTreeHandler, SelectionChanged));
    mpPropertiesTreeView->connect_changed(LINK(this, ObjectInspectorTreeHandler, SelectionChanged));
    mpMethodsTreeView->connect_changed(LINK(this, ObjectInspectorTreeHandler, SelectionChanged));

    mpObjectInspectorToolbar->connect_clicked(
        LINK(this, ObjectInspectorTreeHandler, ToolbarButtonClicked));
    mpObjectInspectorToolbar->set_item_sensitive("inspect", false);
    mpObjectInspectorToolbar->set_item_sensitive("back", false);

    mpObjectInspectorNotebook->connect_leave_page(
        LINK(this, ObjectInspectorTreeHandler, NotebookLeavePage));
    mpObjectInspectorNotebook->connect_enter_page(
        LINK(this, ObjectInspectorTreeHandler, NotebookEnterPage));
}

void ObjectInspectorTreeHandler::handleExpanding(std::unique_ptr<weld::TreeView>& pTreeView,
                                                 weld::TreeIter const& rParent)
{
    OUString sID = pTreeView->get_id(rParent);
    if (sID.isEmpty())
        return;

    clearObjectInspectorChildren(pTreeView, rParent);
    auto* pNode = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64());
    pNode->fillChildren(pTreeView, &rParent);
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerInterfaces, weld::TreeIter const&, rParent,
          bool)
{
    handleExpanding(mpInterfacesTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerServices, weld::TreeIter const&, rParent,
          bool)
{
    handleExpanding(mpServicesTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerProperties, weld::TreeIter const&, rParent,
          bool)
{
    handleExpanding(mpPropertiesTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerMethods, weld::TreeIter const&, rParent, bool)
{
    handleExpanding(mpMethodsTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, SelectionChanged, weld::TreeView&, rTreeView, void)
{
    bool bHaveNodeWithObject = false;

    if (mpPropertiesTreeView.get() == &rTreeView)
    {
        auto* pNode = getSelectedNode(rTreeView);
        if (auto* pBasicValueNode = dynamic_cast<BasicValueNode*>(pNode))
        {
            uno::Any aAny = pBasicValueNode->getAny();
            uno::Reference<uno::XInterface> xInterface(aAny, uno::UNO_QUERY);
            bHaveNodeWithObject = xInterface.is();
        }
    }

    mpObjectInspectorToolbar->set_item_sensitive("inspect", bHaveNodeWithObject);
}

IMPL_LINK(ObjectInspectorTreeHandler, PopupMenuHandler, const CommandEvent&, rCommandEvent, bool)
{
    if (rCommandEvent.GetCommand() != CommandEventId::ContextMenu)
        return false;

    auto xInterface = getSelectedXInterface(*mpPropertiesTreeView);
    if (xInterface.is())
    {
        std::unique_ptr<weld::Builder> xBuilder(
            Application::CreateBuilder(mpPropertiesTreeView.get(), "sfx/ui/devtoolsmenu.ui"));
        std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu("inspect_menu"));

        OString sCommand(
            xMenu->popup_at_rect(mpPropertiesTreeView.get(),
                                 tools::Rectangle(rCommandEvent.GetMousePosPixel(), Size(1, 1))));

        if (sCommand == "inspect")
        {
            addToStack(uno::Any(xInterface));
            inspectObject(xInterface);
        }
    }
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ToolbarButtonClicked, const OString&, rSelectionId, void)
{
    if (rSelectionId == "inspect")
    {
        auto xInterface = getSelectedXInterface(*mpPropertiesTreeView);
        if (xInterface.is())
        {
            addToStack(uno::Any(xInterface));
            inspectObject(xInterface);
        }
    }
    else if (rSelectionId == "back")
    {
        uno::Any aAny = popFromStack();
        if (aAny.hasValue())
        {
            uno::Reference<uno::XInterface> xInterface(aAny, uno::UNO_QUERY);
            inspectObject(xInterface);
        }
    }
    else if (rSelectionId == "refresh")
    {
        auto rPageId = mpObjectInspectorNotebook->get_current_page_ident();
        NotebookEnterPage(rPageId);
    }
}

IMPL_LINK(ObjectInspectorTreeHandler, NotebookEnterPage, const OString&, rPageId, void)
{
    uno::Any aAny = maInspectionStack.back();
    if (aAny.hasValue())
    {
        uno::Reference<uno::XInterface> xInterface(aAny, uno::UNO_QUERY);
        if (rPageId == "object_inspector_interfaces_tab")
        {
            mpInterfacesTreeView->freeze();
            clearAll(mpInterfacesTreeView);
            appendInterfaces(xInterface);
            mpInterfacesTreeView->thaw();
        }
        else if (rPageId == "object_inspector_services_tab")
        {
            mpServicesTreeView->freeze();
            clearAll(mpServicesTreeView);
            appendServices(xInterface);
            mpServicesTreeView->thaw();
        }
        else if (rPageId == "object_inspector_properties_tab")
        {
            mpPropertiesTreeView->freeze();
            clearAll(mpPropertiesTreeView);
            appendProperties(xInterface);
            mpPropertiesTreeView->thaw();
        }
        else if (rPageId == "object_inspector_methods_tab")
        {
            mpMethodsTreeView->freeze();
            clearAll(mpMethodsTreeView);
            appendMethods(xInterface);
            mpMethodsTreeView->thaw();
        }
    }
}

IMPL_LINK(ObjectInspectorTreeHandler, NotebookLeavePage, const OString&, rPageId, bool)
{
    if (rPageId == "object_inspector_interfaces_tab")
    {
        mpInterfacesTreeView->freeze();
        clearAll(mpInterfacesTreeView);
        mpInterfacesTreeView->thaw();
    }
    else if (rPageId == "object_inspector_services_tab")
    {
        mpServicesTreeView->freeze();
        clearAll(mpServicesTreeView);
        mpServicesTreeView->thaw();
    }
    else if (rPageId == "object_inspector_properties_tab")
    {
        mpPropertiesTreeView->freeze();
        clearAll(mpPropertiesTreeView);
        mpPropertiesTreeView->thaw();
    }
    else if (rPageId == "object_inspector_methods_tab")
    {
        mpMethodsTreeView->freeze();
        clearAll(mpMethodsTreeView);
        mpMethodsTreeView->thaw();
    }
    return true;
}

void ObjectInspectorTreeHandler::clearObjectInspectorChildren(
    std::unique_ptr<weld::TreeView>& pTreeView, weld::TreeIter const& rParent)
{
    bool bChild = false;
    do
    {
        bChild = pTreeView->iter_has_child(rParent);
        if (bChild)
        {
            std::unique_ptr<weld::TreeIter> pChild = pTreeView->make_iterator(&rParent);
            bChild = pTreeView->iter_children(*pChild);
            if (bChild)
            {
                clearObjectInspectorChildren(pTreeView, *pChild);
                OUString sID = pTreeView->get_id(*pChild);
                auto* pEntry = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64());
                delete pEntry;
                pTreeView->remove(*pChild);
            }
        }
    } while (bChild);
}

void ObjectInspectorTreeHandler::clearAll(std::unique_ptr<weld::TreeView>& pTreeView)
{
    // destroy all ObjectInspectorNodes from the tree
    pTreeView->all_foreach([&pTreeView](weld::TreeIter& rEntry) {
        OUString sID = pTreeView->get_id(rEntry);
        auto* pEntry = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64());
        delete pEntry;
        return false;
    });
    pTreeView->clear();
}

void ObjectInspectorTreeHandler::appendInterfaces(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;
    uno::Reference<lang::XTypeProvider> xTypeProvider(xInterface, uno::UNO_QUERY);
    if (xTypeProvider.is())
    {
        const auto xSequenceTypes = xTypeProvider->getTypes();
        for (auto const& xType : xSequenceTypes)
        {
            OUString aName = xType.getTypeName();
            lclAppendNode(mpInterfacesTreeView, new SimpleStringNode(aName));
        }
    }
}

void ObjectInspectorTreeHandler::appendServices(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    const uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
    for (auto const& aServiceName : aServiceNames)
    {
        lclAppendNode(mpServicesTreeView, new SimpleStringNode(aServiceName));
    }
}

void ObjectInspectorTreeHandler::appendProperties(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;
    GenericPropertiesNode aNode("", uno::Any(xInterface), comphelper::getProcessComponentContext());
    aNode.fillChildren(mpPropertiesTreeView, nullptr);
}

void ObjectInspectorTreeHandler::appendMethods(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    uno::Reference<beans::XIntrospection> xIntrospection
        = beans::theIntrospection::get(comphelper::getProcessComponentContext());
    auto xIntrospectionAccess = xIntrospection->inspect(uno::Any(xInterface));

    const auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
    for (auto const& xMethod : xMethods)
    {
        lclAppendNode(mpMethodsTreeView, new MethodNode(xMethod));
    }
}

void ObjectInspectorTreeHandler::updateBackButtonState()
{
    mpObjectInspectorToolbar->set_item_sensitive("back", maInspectionStack.size() > 1);
}

void ObjectInspectorTreeHandler::clearStack()
{
    maInspectionStack.clear();
    updateBackButtonState();
}

void ObjectInspectorTreeHandler::addToStack(css::uno::Any const& rAny)
{
    maInspectionStack.push_back(rAny);
    updateBackButtonState();
}

css::uno::Any ObjectInspectorTreeHandler::popFromStack()
{
    maInspectionStack.pop_back();
    uno::Any aAny = maInspectionStack.back();
    updateBackButtonState();
    return aAny;
}

void ObjectInspectorTreeHandler::inspectObject(uno::Reference<uno::XInterface> const& xInterface)
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

    auto rPageId = mpObjectInspectorNotebook->get_current_page_ident();
    NotebookEnterPage(rPageId);
}

void ObjectInspectorTreeHandler::introspect(uno::Reference<uno::XInterface> const& xInterface)
{
    clearStack();
    addToStack(uno::Any(xInterface));
    inspectObject(xInterface);
}

void ObjectInspectorTreeHandler::dispose()
{
    clearAll(mpInterfacesTreeView);
    clearAll(mpServicesTreeView);
    clearAll(mpPropertiesTreeView);
    clearAll(mpMethodsTreeView);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
