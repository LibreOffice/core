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

    virtual void fillChildren(std::unique_ptr<weld::TreeView>& rTree, weld::TreeIter const& rParent)
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
                               weld::TreeIter const& rParent, ObjectInspectorNodeInterface* pEntry)
{
    OUString sName = pEntry->getObjectName();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    std::unique_ptr<weld::TreeIter> pCurrent = pTree->make_iterator();
    pTree->insert(&rParent, -1, &sName, &sId, nullptr, nullptr, pEntry->shouldShowExpander(),
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
                      weld::TreeIter const& /*rParent*/) override
    {
    }

    OUString getObjectName() override { return msName; }
};

class BasicValueNode : public SimpleStringNode
{
protected:
    uno::Any maAny;
    uno::Reference<uno::XComponentContext> mxContext;

    uno::Reference<uno::XInterface> getObjectFromAny()
    {
        uno::Reference<uno::XInterface> xInterface;
        if (maAny.hasValue() && maAny.getValueType().getTypeClass() == uno::TypeClass_INTERFACE)
        {
            xInterface = uno::Reference<uno::XInterface>(maAny, uno::UNO_QUERY);
        }
        return xInterface;
    }

    ObjectInspectorNodeInterface* createNodeObjectForAny(OUString const& rName, uno::Any& rAny);

public:
    BasicValueNode(OUString const& rName, uno::Any const& rAny,
                   uno::Reference<uno::XComponentContext> const& xContext)
        : SimpleStringNode(rName)
        , maAny(rAny)
        , mxContext(xContext)
    {
    }

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

class ServicesNode : public BasicValueNode
{
public:
    ServicesNode(css::uno::Reference<css::uno::XInterface> const& xObject,
                 uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode("Services", uno::Any(xObject), xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        auto xObject = getObjectFromAny();
        if (xObject.is())
        {
            auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xObject, uno::UNO_QUERY);
            const uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
            for (auto const& aServiceName : aServiceNames)
            {
                lclAppendNodeToParent(pTree, rParent, new SimpleStringNode(aServiceName));
            }
        }
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        return std::vector<std::pair<sal_Int32, OUString>>();
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
                      weld::TreeIter const& rParent) override;
};

class PropertiesNode : public GenericPropertiesNode
{
public:
    PropertiesNode(uno::Reference<uno::XInterface> const& xObject,
                   uno::Reference<uno::XComponentContext> const& xContext)
        : GenericPropertiesNode("Properties", uno::Any(xObject), xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        return ObjectInspectorNodeInterface::getColumnValues();
    }
};

class InterfacesNode : public BasicValueNode
{
public:
    InterfacesNode(css::uno::Reference<css::uno::XInterface> const& xObject,
                   uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode("Interfaces", uno::Any(xObject), xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        auto xObject = getObjectFromAny();
        if (xObject.is())
        {
            uno::Reference<lang::XTypeProvider> xTypeProvider(xObject, uno::UNO_QUERY);
            if (xTypeProvider.is())
            {
                const auto xSequenceTypes = xTypeProvider->getTypes();
                for (auto const& xType : xSequenceTypes)
                {
                    OUString aName = xType.getTypeName();
                    lclAppendNodeToParent(pTree, rParent, new SimpleStringNode(aName));
                }
            }
        }
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        return std::vector<std::pair<sal_Int32, OUString>>();
    }
};

class MethodsNode : public BasicValueNode
{
public:
    MethodsNode(css::uno::Reference<css::uno::XInterface> const& xObject,
                uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode("Methods", uno::Any(xObject), xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      weld::TreeIter const& rParent) override
    {
        uno::Reference<beans::XIntrospection> xIntrospection
            = beans::theIntrospection::get(mxContext);
        auto xIntrospectionAccess = xIntrospection->inspect(maAny);

        const auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
        for (auto const& xMethod : xMethods)
        {
            OUString aMethodName = xMethod->getName();

            lclAppendNodeToParent(pTree, rParent, new SimpleStringNode(aMethodName));
        }
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        return std::vector<std::pair<sal_Int32, OUString>>();
    }
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
                      weld::TreeIter const& rParent) override;
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
                      weld::TreeIter const& rParent) override
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
                lclAppendNodeToParent(pTree, rParent, pObjectInspectorNode);
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
                                         weld::TreeIter const& rParent)
{
    if (!maAny.hasValue())
        return;

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
            lclAppendNodeToParent(pTree, rParent, pObjectInspectorNode);
    }
}

void StructNode::fillChildren(std::unique_ptr<weld::TreeView>& pTree, weld::TreeIter const& rParent)
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
            lclAppendNodeToParent(pTree, rParent, pObjectInspectorNode);
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

} // end anonymous namespace

ObjectInspectorTreeHandler::ObjectInspectorTreeHandler(
    std::unique_ptr<weld::TreeView>& pObjectInspectorTree,
    std::unique_ptr<weld::Label>& pClassNameLabel)
    : mpObjectInspectorTree(pObjectInspectorTree)
    , mpClassNameLabel(pClassNameLabel)
{
    mpObjectInspectorTree->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandler));
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandler, weld::TreeIter const&, rParent, bool)
{
    OUString sID = mpObjectInspectorTree->get_id(rParent);
    if (sID.isEmpty())
        return true;

    clearObjectInspectorChildren(rParent);
    auto* pNode = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64());
    pNode->fillChildren(mpObjectInspectorTree, rParent);

    return true;
}

void ObjectInspectorTreeHandler::clearObjectInspectorChildren(weld::TreeIter const& rParent)
{
    bool bChild = false;
    do
    {
        bChild = mpObjectInspectorTree->iter_has_child(rParent);
        if (bChild)
        {
            std::unique_ptr<weld::TreeIter> pChild = mpObjectInspectorTree->make_iterator(&rParent);
            bChild = mpObjectInspectorTree->iter_children(*pChild);
            if (bChild)
            {
                clearObjectInspectorChildren(*pChild);
                OUString sID = mpObjectInspectorTree->get_id(*pChild);
                auto* pEntry = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64());
                delete pEntry;
                mpObjectInspectorTree->remove(*pChild);
            }
        }
    } while (bChild);
}

void ObjectInspectorTreeHandler::introspect(uno::Reference<uno::XInterface> const& xInterface)
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
    mpObjectInspectorTree->freeze();
    mpObjectInspectorTree->clear();

    lclAppendNode(mpObjectInspectorTree, new ServicesNode(xInterface, xContext));
    lclAppendNode(mpObjectInspectorTree, new InterfacesNode(xInterface, xContext));
    lclAppendNode(mpObjectInspectorTree, new PropertiesNode(xInterface, xContext));
    lclAppendNode(mpObjectInspectorTree, new MethodsNode(xInterface, xContext));

    mpObjectInspectorTree->thaw();
}

void ObjectInspectorTreeHandler::dispose()
{
    // destroy all ObjectInspectorNodes from the tree
    mpObjectInspectorTree->all_foreach([this](weld::TreeIter& rEntry) {
        OUString sID = mpObjectInspectorTree->get_id(rEntry);
        auto* pEntry = reinterpret_cast<ObjectInspectorNodeInterface*>(sID.toInt64());
        delete pEntry;
        return false;
    });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
