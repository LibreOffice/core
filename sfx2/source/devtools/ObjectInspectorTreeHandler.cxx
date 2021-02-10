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

#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/Invocation.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include <comphelper/processfactory.hxx>

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

OUString AnyToString(const uno::Any& aValue)
{
    uno::Type aValType = aValue.getValueType();
    uno::TypeClass eType = aValType.getTypeClass();

    OUString aRetStr;
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

class ObjectInspectorNode : public ObjectInspectorNodeInterface
{
public:
    css::uno::Reference<css::uno::XInterface> mxObject;

    ObjectInspectorNode() = default;

    ObjectInspectorNode(css::uno::Reference<css::uno::XInterface> const& xObject)
        : mxObject(xObject)
    {
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

class ObjectInspectorNamedNode : public ObjectInspectorNode
{
protected:
    OUString msName;
    uno::Any maAny;
    uno::Reference<uno::XComponentContext> mxContext;

public:
    ObjectInspectorNamedNode(OUString const& rName,
                             css::uno::Reference<css::uno::XInterface> const& xObject,
                             uno::Any aAny = uno::Any(),
                             uno::Reference<uno::XComponentContext> const& xContext
                             = uno::Reference<uno::XComponentContext>())
        : ObjectInspectorNode(xObject)
        , msName(rName)
        , maAny(aAny)
        , mxContext(xContext)
    {
    }

    bool shouldShowExpander() override
    {
        if (maAny.hasValue())
        {
            auto xInterface = uno::Reference<uno::XInterface>(maAny, uno::UNO_QUERY);
            return xInterface.is();
        }
        return false;
    }

    OUString getObjectName() override { return msName; }

    void fillChildren(std::unique_ptr<weld::TreeView>& /*rTree*/,
                      weld::TreeIter const& /*rParent*/) override
    {
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        if (maAny.hasValue())
        {
            OUString aValue = AnyToString(maAny);
            OUString aType = getAnyType(maAny, mxContext);

            return {
                { 1, aValue },
                { 2, aType },
            };
        }

        return ObjectInspectorNodeInterface::getColumnValues();
    }
};

class ServicesNode : public ObjectInspectorNamedNode
{
public:
    ServicesNode(css::uno::Reference<css::uno::XInterface> const& xObject)
        : ObjectInspectorNamedNode("Services", xObject)
    {
    }

    bool shouldShowExpander() override { return true; }

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
    GenericPropertiesNode(OUString const& rName, uno::Reference<uno::XInterface> const& xObject,
                          uno::Any aAny, uno::Reference<uno::XComponentContext> const& xContext)
        : ObjectInspectorNamedNode(rName, xObject, aAny, xContext)
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
        : GenericPropertiesNode("Properties", xObject, uno::Any(), xContext)
    {
    }

    bool shouldShowExpander() override { return true; }
};

class InterfacesNode : public ObjectInspectorNamedNode
{
public:
    InterfacesNode(css::uno::Reference<css::uno::XInterface> const& xObject)
        : ObjectInspectorNamedNode("Interfaces", xObject)
    {
    }

    bool shouldShowExpander() override { return true; }

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
    MethodsNode(css::uno::Reference<css::uno::XInterface> const& xObject,
                uno::Reference<uno::XComponentContext> const& xContext)
        : ObjectInspectorNamedNode("Methods", xObject, uno::Any(xObject), xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

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

class SequenceNode : public ObjectInspectorNodeInterface
{
private:
    OUString maString;
    uno::Any maAny;
    uno::Reference<uno::XComponentContext> mxContext;

public:
    SequenceNode(OUString const& rString, uno::Any const& rAny,
                 uno::Reference<uno::XComponentContext> const& xContext)
        : maString(rString)
        , maAny(rAny)
        , mxContext(xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    OUString getObjectName() override { return maString; }

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
            uno::Any aCurrentAny = xIdlArray->get(maAny, i);
            uno::Reference<uno::XInterface> xCurrent;
            if (aCurrentAny.hasValue())
            {
                auto xInterface = uno::Reference<uno::XInterface>(aCurrentAny, uno::UNO_QUERY);
                if (xInterface.is())
                    xCurrent = xInterface;
            }

            lclAppendNodeToParent(
                pTree, rParent,
                new GenericPropertiesNode(OUString::number(i), xCurrent, aCurrentAny, mxContext));
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
    if (!mxObject.is())
        return;

    uno::Reference<beans::XIntrospection> xIntrospection = beans::theIntrospection::get(mxContext);
    auto xIntrospectionAccess = xIntrospection->inspect(uno::makeAny(mxObject));
    auto xInvocationFactory = css::script::Invocation::create(mxContext);
    uno::Sequence<uno::Any> aParameters = { uno::Any(mxObject) };
    auto xInvocationInterface = xInvocationFactory->createInstanceWithArguments(aParameters);
    uno::Reference<script::XInvocation> xInvocation(xInvocationInterface, uno::UNO_QUERY);

    const auto xProperties = xIntrospectionAccess->getProperties(
        beans::PropertyConcept::ALL - beans::PropertyConcept::DANGEROUS);

    for (auto const& xProperty : xProperties)
    {
        uno::Any aAny;
        uno::Reference<uno::XInterface> xCurrent = mxObject;

        try
        {
            if (xInvocation->hasProperty(xProperty.Name))
            {
                aAny = xInvocation->getValue(xProperty.Name);
            }
        }
        catch (...)
        {
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

        uno::TypeClass eTypeClass = aAny.getValueType().getTypeClass();

        if (bComplex)
        {
            lclAppendNodeToParent(
                pTree, rParent,
                new GenericPropertiesNode(xProperty.Name, xCurrent, aAny, mxContext));
        }
        else if (eTypeClass == uno::TypeClass_SEQUENCE)
        {
            lclAppendNodeToParent(pTree, rParent,
                                  new SequenceNode(xProperty.Name, aAny, mxContext));
        }
        else
        {
            lclAppendNodeToParent(
                pTree, rParent,
                new ObjectInspectorNamedNode(xProperty.Name, xCurrent, aAny, mxContext));
        }
    }
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

    lclAppendNode(mpObjectInspectorTree, new ServicesNode(xInterface));
    lclAppendNode(mpObjectInspectorTree, new InterfacesNode(xInterface));
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
