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
#include <sfx2/sfxresid.hxx>
#include "DevToolsStrings.hrc"

#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XEnumTypeDescription.hpp>

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/script/Invocation.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/script/MemberType.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>

using namespace css;

namespace
{
constexpr OUStringLiteral constTypeDescriptionManagerSingletonName
    = u"/singletons/com.sun.star.reflection.theTypeDescriptionManager";

OUString enumValueToEnumName(uno::Any const& aValue,
                             uno::Reference<uno::XComponentContext> const& xContext)
{
    sal_Int32 nIntValue = 0;
    if (!cppu::enum2int(nIntValue, aValue))
        return OUString();

    uno::Reference<container::XHierarchicalNameAccess> xManager;
    xManager.set(xContext->getValueByName(constTypeDescriptionManagerSingletonName),
                 uno::UNO_QUERY);

    uno::Reference<reflection::XEnumTypeDescription> xTypeDescription;
    xTypeDescription.set(xManager->getByHierarchicalName(aValue.getValueType().getTypeName()),
                         uno::UNO_QUERY);

    uno::Sequence<sal_Int32> aValues = xTypeDescription->getEnumValues();
    sal_Int32 nValuesIndex = std::find(aValues.begin(), aValues.end(), nIntValue) - aValues.begin();
    uno::Sequence<OUString> aNames = xTypeDescription->getEnumNames();
    return aNames[nValuesIndex];
}

OUString getInterfaceImplementationClass(uno::Reference<uno::XInterface> const& xInterface)
{
    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    if (xServiceInfo.is())
        return xServiceInfo->getImplementationName();
    return OUString();
}

/** converts basic any value to a string */
OUString convertBasicValueToString(const uno::Any& aValue,
                                   const uno::Reference<uno::XComponentContext>& xContext)
{
    OUString aRetStr;

    // return early if we don't have any value
    if (!aValue.hasValue())
        return SfxResId(STR_ANY_VALUE_NULL);

    uno::Type aValType = aValue.getValueType();
    uno::TypeClass eType = aValType.getTypeClass();

    switch (eType)
    {
        case uno::TypeClass_BOOLEAN:
        {
            bool bBool = aValue.get<bool>();
            aRetStr = bBool ? SfxResId(STR_ANY_VALUE_TRUE) : SfxResId(STR_ANY_VALUE_FALSE);
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
            aRetStr = u"\"" + aValue.get<OUString>() + u"\"";
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
            aRetStr = enumValueToEnumName(aValue, xContext);
            break;
        }

        default:
            break;
    }
    return aRetStr;
}

// returns a name of the object, if available
OUString getInterfaceName(uno::Reference<uno::XInterface> const& xInterface,
                          const uno::Reference<uno::XComponentContext>& xContext)
{
    uno::Reference<container::XNamed> xNamed(xInterface, uno::UNO_QUERY);
    if (xNamed.is())
        return xNamed->getName();

    auto xInvocationFactory = css::script::Invocation::create(xContext);
    uno::Sequence<uno::Any> aParameters = { uno::Any(xInterface) };
    auto xInvocationInterface = xInvocationFactory->createInstanceWithArguments(aParameters);
    if (xInvocationInterface.is())
    {
        uno::Reference<script::XInvocation2> xInvocation(xInvocationInterface, uno::UNO_QUERY);
        if (xInvocation.is() && xInvocation->hasProperty("Name"))
        {
            uno::Any aAny = xInvocation->getValue("Name");
            if (aAny.hasValue() && aAny.getValueTypeClass() == uno::TypeClass_STRING)
                return aAny.get<OUString>();
        }
    }
    return OUString();
}

OUString convertAnyToString(const uno::Any& aValue,
                            const uno::Reference<uno::XComponentContext>& xContext)
{
    // return early if we don't have any value
    if (!aValue.hasValue())
        return SfxResId(STR_ANY_VALUE_NULL);

    OUString aRetStr;

    uno::TypeClass eType = aValue.getValueTypeClass();

    switch (eType)
    {
        case uno::TypeClass_INTERFACE:
        {
            uno::Reference<uno::XInterface> xInterface(aValue, uno::UNO_QUERY);
            if (!xInterface.is())
                aRetStr = SfxResId(STR_ANY_VALUE_NULL);
            else
            {
                OUString aString = getInterfaceName(xInterface, xContext);
                if (!aString.isEmpty())
                    aRetStr = "{" + aString + "} ";

                OUString aImplementationClass = getInterfaceImplementationClass(xInterface);
                if (aImplementationClass.isEmpty())
                    aImplementationClass = SfxResId(STR_CLASS_UNKNOWN);
                aRetStr
                    += SfxResId(STR_PROPERTY_VALUE_OBJECT).replaceFirst("%1", aImplementationClass);
            }
            break;
        }
        case uno::TypeClass_STRUCT:
        {
            aRetStr = SfxResId(STR_PROPERTY_VALUE_STRUCT);
            break;
        }
        default:
        {
            aRetStr = convertBasicValueToString(aValue, xContext);
            break;
        }
    }
    return aRetStr;
}

OUString convertAnyToShortenedString(const uno::Any& aValue,
                                     const uno::Reference<uno::XComponentContext>& xContext)
{
    // return early if we don't have any value
    if (!aValue.hasValue())
        return SfxResId(STR_ANY_VALUE_NULL);

    OUString aRetStr;

    uno::TypeClass eType = aValue.getValueTypeClass();

    switch (eType)
    {
        case uno::TypeClass_INTERFACE:
        {
            uno::Reference<uno::XInterface> xInterface(aValue, uno::UNO_QUERY);
            if (!xInterface.is())
                aRetStr = SfxResId(STR_ANY_VALUE_NULL);
            else
            {
                OUString aString = getInterfaceName(xInterface, xContext);
                if (!aString.isEmpty())
                    aRetStr = "{" + aString + "} ";

                OUString aImplementationClass = getInterfaceImplementationClass(xInterface);
                if (aImplementationClass.isEmpty())
                    aImplementationClass = SfxResId(STR_CLASS_UNKNOWN);
                aRetStr
                    += SfxResId(STR_PROPERTY_VALUE_OBJECT).replaceFirst("%1", aImplementationClass);

                if (aRetStr.getLength() > 43)
                    aRetStr = OUString::Concat(aRetStr.subView(0, 40)) + u"...";
            }
            break;
        }
        case uno::TypeClass_STRING:
        {
            OUString aStringValue = u"\"" + aValue.get<OUString>() + u"\"";
            if (aStringValue.getLength() > 44)
                aStringValue = OUString::Concat(aStringValue.subView(0, 40)) + u"\"...";
            aRetStr = aStringValue;
            break;
        }
        default:
        {
            aRetStr = convertAnyToString(aValue, xContext);
            break;
        }
    }
    return aRetStr;
}

/** converts an any's type to a string (in a short form) */
OUString getAnyType(const uno::Any& aValue)
{
    OUString aTypeName = aValue.getValueType().getTypeName();
    return aTypeName.replaceAll("com.sun.star", "css");
}

/** converts a Type to a XIdlClass */
uno::Reference<reflection::XIdlClass>
convertTypeToIdlClass(const uno::Type& rType,
                      const uno::Reference<uno::XComponentContext>& xContext)
{
    auto xReflection = reflection::theCoreReflection::get(xContext);
    return xReflection->forName(rType.getTypeName());
}

// Object inspector nodes

/** Object inspector node's main interface
 *
 * The interface for the "attached" object to a tree view nodes that
 * are added to the tree views of the object inspector part. The node
 * can return the main value of the node (object name) and if present
 * also the values for additional columns. It signals if a tree needs
 * an expander and fills the children of the tree is any exists.
 *
 */
class ObjectInspectorNodeInterface
{
public:
    ObjectInspectorNodeInterface() = default;

    virtual ~ObjectInspectorNodeInterface() {}

    // main value (object name) of the tree view node
    virtual OUString getObjectName() = 0;

    // should show the expander for the tree view node
    virtual bool shouldShowExpander() { return false; }

    // fill the children for the current tree view node
    virtual void fillChildren(std::unique_ptr<weld::TreeView>& rTree, const weld::TreeIter* pParent)
        = 0;

    // fill any additional column values for the current tree view node
    virtual std::vector<std::pair<sal_Int32, OUString>> getColumnValues()
    {
        return std::vector<std::pair<sal_Int32, OUString>>();
    }
};

// appends the node to the root of the tree view
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

// appends the node to the parent
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

/** Node that represent just a simple string with no children or columns */
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

/** Node represents a method of an object */
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
                return SfxResId(STR_METHOD_TYPE_OBJECT);
            case uno::TypeClass_STRUCT:
                return SfxResId(STR_METHOD_TYPE_STRUCT);
            case uno::TypeClass_ENUM:
                return SfxResId(STR_METHOD_TYPE_ENUM);
            case uno::TypeClass_SEQUENCE:
                return SfxResId(STR_METHOD_TYPE_SEQUENCE);
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
                    aInString += SfxResId(STR_PARMETER_MODE_IN) + " ";
                    break;
                case reflection::ParamMode_OUT:
                    aInString += SfxResId(STR_PARMETER_MODE_OUT) + " ";
                    break;
                case reflection::ParamMode_INOUT:
                    aInString += SfxResId(STR_PARMETER_MODE_IN_AND_OUT) + " ";
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

/** Node represents a class (XIdlClass) of an object.
 *
 * Children are superclasses of the current class. XInterface superclass
 * is ignored.
 *
 */
class ClassNode : public ObjectInspectorNodeInterface
{
private:
    uno::Reference<reflection::XIdlClass> mxClass;

    static bool isXInterface(uno::Reference<reflection::XIdlClass> const& xClass)
    {
        return xClass->getName() == "com.sun.star.uno.XInterface";
    }

public:
    ClassNode(uno::Reference<reflection::XIdlClass> const& xClass)
        : mxClass(xClass)
    {
    }

    bool shouldShowExpander() override
    {
        auto const& xSuperClasses = mxClass->getSuperclasses();
        return xSuperClasses.getLength() > 2
               || (xSuperClasses.getLength() == 1 && !isXInterface(xSuperClasses[0]));
    }

    OUString getObjectName() override { return mxClass->getName(); }

    // Fill superclasses
    void fillChildren(std::unique_ptr<weld::TreeView>& rTree,
                      const weld::TreeIter* pParent) override
    {
        auto const& xSuperClasses = mxClass->getSuperclasses();
        for (auto const& xSuper : xSuperClasses)
        {
            if (!isXInterface(xSuper))
                lclAppendNodeToParent(rTree, pParent, new ClassNode(xSuper));
        }
    }
};

/** Node represents a basic value, that can be any object, sequence, struct */
class BasicValueNode : public SimpleStringNode
{
protected:
    uno::Any maAny;
    OUString mrInfo;
    uno::Reference<uno::XComponentContext> mxContext;

    ObjectInspectorNodeInterface* createNodeObjectForAny(OUString const& rName, uno::Any& rAny,
                                                         OUString const& mrInfo);

public:
    BasicValueNode(OUString const& rName, uno::Any const& rAny, OUString const& rInfo,
                   uno::Reference<uno::XComponentContext> const& xContext)
        : SimpleStringNode(rName)
        , maAny(rAny)
        , mrInfo(rInfo)
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
                {
                    uno::Reference<uno::XInterface> xInterface(maAny, uno::UNO_QUERY);
                    return xInterface.is();
                }
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
        OUString aValue = convertAnyToShortenedString(maAny, mxContext);
        OUString aType = getAnyType(maAny);

        return { { 1, aValue }, { 2, aType }, { 3, mrInfo } };
    }
};

/** Node represents a property */
class GenericPropertiesNode : public BasicValueNode
{
public:
    GenericPropertiesNode(OUString const& rName, uno::Any const& rAny, OUString const& rInfo,
                          uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode(rName, rAny, rInfo, xContext)
    {
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      const weld::TreeIter* pParent) override;
};

/** Node represents a struct */
class StructNode : public BasicValueNode
{
public:
    StructNode(OUString const& rName, uno::Any const& rAny, OUString const& rInfo,
               uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode(rName, rAny, rInfo, xContext)
    {
    }

    bool shouldShowExpander() override { return true; }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      const weld::TreeIter* pParent) override;
};

/** Node represents a sequence */
class SequenceNode : public BasicValueNode
{
    uno::Reference<reflection::XIdlArray> mxIdlArray;

public:
    SequenceNode(OUString const& rName, uno::Any const& rAny, OUString const& rInfo,
                 uno::Reference<uno::XComponentContext> const& xContext)
        : BasicValueNode(rName, rAny, rInfo, xContext)
    {
        auto xClass = convertTypeToIdlClass(maAny.getValueType(), mxContext);
        mxIdlArray = xClass->getArray();
    }

    bool shouldShowExpander() override
    {
        // Show expander only if the sequence has elements
        int nLength = mxIdlArray->getLen(maAny);
        return nLength > 0;
    }

    void fillChildren(std::unique_ptr<weld::TreeView>& pTree,
                      const weld::TreeIter* pParent) override
    {
        int nLength = mxIdlArray->getLen(maAny);

        for (int i = 0; i < nLength; i++)
        {
            uno::Any aArrayValue = mxIdlArray->get(maAny, i);
            uno::Reference<uno::XInterface> xCurrent;

            auto* pObjectInspectorNode
                = createNodeObjectForAny(OUString::number(i), aArrayValue, "");
            if (pObjectInspectorNode)
                lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
    }

    std::vector<std::pair<sal_Int32, OUString>> getColumnValues() override
    {
        int nLength = mxIdlArray->getLen(maAny);

        OUString aType = getAnyType(maAny).replaceAll(u"[]", u"");
        aType += u"[" + OUString::number(nLength) + u"]";

        OUString aValue
            = SfxResId(STR_PROPERTY_VALUE_SEQUENCE).replaceFirst("%1", OUString::number(nLength));

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

    try
    {
        const auto xNameAccess = uno::Reference<container::XNameAccess>(maAny, uno::UNO_QUERY);
        if (xNameAccess.is())
        {
            const uno::Sequence<OUString> aNames = xNameAccess->getElementNames();
            for (OUString const& rName : aNames)
            {
                uno::Any aAny = xNameAccess->getByName(rName);
                auto* pObjectInspectorNode = createNodeObjectForAny(
                    u"@" + rName, aAny, SfxResId(STR_PROPERTY_TYPE_IS_NAMED_CONTAINER));
                lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
            }
        }
    }
    catch (...)
    {
    }

    try
    {
        const auto xIndexAccess = uno::Reference<container::XIndexAccess>(maAny, uno::UNO_QUERY);
        if (xIndexAccess.is())
        {
            for (sal_Int32 nIndex = 0; nIndex < xIndexAccess->getCount(); ++nIndex)
            {
                uno::Any aAny = xIndexAccess->getByIndex(nIndex);
                auto* pObjectInspectorNode
                    = createNodeObjectForAny(u"@" + OUString::number(nIndex), aAny,
                                             SfxResId(STR_PROPERTY_TYPE_IS_INDEX_CONTAINER));
                lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
            }
        }
    }
    catch (...)
    {
    }

    try
    {
        const auto xEnumAccess
            = uno::Reference<container::XEnumerationAccess>(maAny, uno::UNO_QUERY);
        if (xEnumAccess.is())
        {
            uno::Reference<container::XEnumeration> xEnumeration = xEnumAccess->createEnumeration();
            if (xEnumeration.is())
            {
                for (sal_Int32 nIndex = 0; xEnumeration->hasMoreElements(); nIndex++)
                {
                    uno::Any aAny = xEnumeration->nextElement();
                    auto* pObjectInspectorNode
                        = createNodeObjectForAny(u"@" + OUString::number(nIndex), aAny,
                                                 SfxResId(STR_PROPERTY_TYPE_IS_ENUMERATION));
                    lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
                }
            }
        }
    }
    catch (...)
    {
    }

    auto xInvocationFactory = css::script::Invocation::create(mxContext);
    uno::Sequence<uno::Any> aParameters = { maAny };
    auto xInvocationInterface = xInvocationFactory->createInstanceWithArguments(aParameters);
    if (!xInvocationInterface.is())
        return;

    uno::Reference<script::XInvocation2> xInvocation(xInvocationInterface, uno::UNO_QUERY);
    if (!xInvocation.is())
        return;

    auto const& xInvocationAccess = xInvocation->getIntrospection();
    if (!xInvocationAccess.is())
        return;

    uno::Sequence<script::InvocationInfo> aInvocationInfoSequence;
    try
    {
        aInvocationInfoSequence = xInvocation->getInfo();
    }
    catch (...)
    {
    }

    for (auto const& aInvocationInfo : std::as_const(aInvocationInfoSequence))
    {
        if (aInvocationInfo.eMemberType == script::MemberType_PROPERTY)
        {
            uno::Any aCurrentAny;
            auto const& aPropertyName = aInvocationInfo.aName;

            bool bIsAttribute = false;
            bool bIsGetSetMethod = false;
            bool bMethodGet = false;
            bool bMethodSet = false;
            bool bMethodIs = false;
            try
            {
                aCurrentAny = xInvocation->getValue(aPropertyName);
                bIsAttribute = xInvocationAccess->hasProperty(aPropertyName,
                                                              beans::PropertyConcept::ATTRIBUTES);
                bIsGetSetMethod = xInvocationAccess->hasProperty(aPropertyName,
                                                                 beans::PropertyConcept::METHODS);
                if (bIsGetSetMethod)
                {
                    bMethodGet = xInvocationAccess->hasMethod(u"get" + aPropertyName,
                                                              beans::MethodConcept::PROPERTY);
                    bMethodSet = xInvocationAccess->hasMethod(u"set" + aPropertyName,
                                                              beans::MethodConcept::PROPERTY);
                    bMethodIs = xInvocationAccess->hasMethod(u"is" + aPropertyName,
                                                             beans::MethodConcept::PROPERTY);
                }
            }
            catch (...)
            {
            }

            std::vector<OUString> aInfoCollection;
            if (bIsAttribute)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_IS_ATTRIBUTE));
            if (bIsGetSetMethod)
            {
                bool bHasGet = false;
                OUString aString;
                if (bMethodGet || bMethodIs)
                {
                    aString += SfxResId(STR_PROPERTY_ATTRIBUTE_GET);
                    bHasGet = true;
                }
                if (bMethodSet)
                {
                    if (bHasGet)
                        aString += u"+";
                    aString += SfxResId(STR_PROPERTY_ATTRIBUTE_SET);
                }
                aInfoCollection.push_back(aString);
                if (bMethodSet && !bHasGet)
                    aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_WRITEONLY));
            }
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::MAYBEVOID)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_MAYBEVOID));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::READONLY)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_READONLY));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::REMOVABLE)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_REMOVABLE));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::BOUND)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_BOUND));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::CONSTRAINED)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_CONSTRAINED));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::TRANSIENT)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_TRANSIENT));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::MAYBEAMBIGUOUS)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_MAYBEAMBIGUOUS));
            if (aInvocationInfo.PropertyAttribute & beans::PropertyAttribute::MAYBEDEFAULT)
                aInfoCollection.push_back(SfxResId(STR_PROPERTY_ATTRIBUTE_MAYBEDEFAULT));

            bool bSet = false;
            OUString aInfoString;
            for (auto const& rString : aInfoCollection)
            {
                if (bSet)
                    aInfoString += ", ";
                else
                    bSet = true;

                aInfoString += rString;
            }

            auto* pObjectInspectorNode
                = createNodeObjectForAny(aPropertyName, aCurrentAny, aInfoString);
            if (pObjectInspectorNode)
                lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
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

        auto* pObjectInspectorNode = createNodeObjectForAny(aFieldName, aFieldValue, "");
        if (pObjectInspectorNode)
        {
            lclAppendNodeToParent(pTree, pParent, pObjectInspectorNode);
        }
    }
}

ObjectInspectorNodeInterface*
BasicValueNode::createNodeObjectForAny(OUString const& rName, uno::Any& rAny, OUString const& rInfo)
{
    switch (rAny.getValueType().getTypeClass())
    {
        case uno::TypeClass_INTERFACE:
            return new GenericPropertiesNode(rName, rAny, rInfo, mxContext);

        case uno::TypeClass_SEQUENCE:
            return new SequenceNode(rName, rAny, rInfo, mxContext);

        case uno::TypeClass_STRUCT:
            return new StructNode(rName, rAny, rInfo, mxContext);

        default:
            break;
    }

    return new BasicValueNode(rName, rAny, rInfo, mxContext);
}

} // end anonymous namespace

// Object inspector tree view helper functions
namespace
{
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
    std::unique_ptr<ObjectInspectorWidgets>& pObjectInspectorWidgets)
    : mpObjectInspectorWidgets(pObjectInspectorWidgets)
    , mxContext(comphelper::getProcessComponentContext())
    , mbPanedResetSize(true)
{
    mpObjectInspectorWidgets->mpInterfacesTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerInterfaces));
    mpObjectInspectorWidgets->mpServicesTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerServices));
    mpObjectInspectorWidgets->mpPropertiesTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerProperties));
    mpObjectInspectorWidgets->mpMethodsTreeView->connect_expanding(
        LINK(this, ObjectInspectorTreeHandler, ExpandingHandlerMethods));

    mpObjectInspectorWidgets->mpPropertiesTreeView->connect_popup_menu(
        LINK(this, ObjectInspectorTreeHandler, PopupMenuHandler));

    mpObjectInspectorWidgets->mpInterfacesTreeView->connect_changed(
        LINK(this, ObjectInspectorTreeHandler, SelectionChanged));
    mpObjectInspectorWidgets->mpServicesTreeView->connect_changed(
        LINK(this, ObjectInspectorTreeHandler, SelectionChanged));
    mpObjectInspectorWidgets->mpPropertiesTreeView->connect_changed(
        LINK(this, ObjectInspectorTreeHandler, SelectionChanged));
    mpObjectInspectorWidgets->mpMethodsTreeView->connect_changed(
        LINK(this, ObjectInspectorTreeHandler, SelectionChanged));

    mpObjectInspectorWidgets->mpInterfacesTreeView->make_sorted();
    mpObjectInspectorWidgets->mpServicesTreeView->make_sorted();
    mpObjectInspectorWidgets->mpPropertiesTreeView->make_sorted();
    mpObjectInspectorWidgets->mpMethodsTreeView->make_sorted();

    mpObjectInspectorWidgets->mpToolbar->connect_clicked(
        LINK(this, ObjectInspectorTreeHandler, ToolbarButtonClicked));
    mpObjectInspectorWidgets->mpToolbar->set_item_sensitive("inspect", false);
    mpObjectInspectorWidgets->mpToolbar->set_item_sensitive("back", false);

    mpObjectInspectorWidgets->mpTextView->hide();

    mpObjectInspectorWidgets->mpNotebook->connect_leave_page(
        LINK(this, ObjectInspectorTreeHandler, NotebookLeavePage));
    mpObjectInspectorWidgets->mpNotebook->connect_enter_page(
        LINK(this, ObjectInspectorTreeHandler, NotebookEnterPage));

    mpObjectInspectorWidgets->mpPaned->connect_size_allocate(
        LINK(this, ObjectInspectorTreeHandler, PanedSizeChange));
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
    handleExpanding(mpObjectInspectorWidgets->mpInterfacesTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerServices, weld::TreeIter const&, rParent,
          bool)
{
    handleExpanding(mpObjectInspectorWidgets->mpServicesTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerProperties, weld::TreeIter const&, rParent,
          bool)
{
    handleExpanding(mpObjectInspectorWidgets->mpPropertiesTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, ExpandingHandlerMethods, weld::TreeIter const&, rParent, bool)
{
    handleExpanding(mpObjectInspectorWidgets->mpMethodsTreeView, rParent);
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, SelectionChanged, weld::TreeView&, rTreeView, void)
{
    bool bHaveNodeWithObject = false;
    mpObjectInspectorWidgets->mpTextView->set_text("");
    if (mpObjectInspectorWidgets->mpPropertiesTreeView.get() == &rTreeView)
    {
        auto* pNode = getSelectedNode(rTreeView);
        if (auto* pBasicValueNode = dynamic_cast<BasicValueNode*>(pNode))
        {
            uno::Any aAny = pBasicValueNode->getAny();
            uno::Reference<uno::XInterface> xInterface(aAny, uno::UNO_QUERY);
            bHaveNodeWithObject = xInterface.is();
            mpObjectInspectorWidgets->mpTextView->set_text(convertAnyToString(aAny, mxContext));
        }
    }

    mpObjectInspectorWidgets->mpToolbar->set_item_sensitive("inspect", bHaveNodeWithObject);
}

IMPL_LINK(ObjectInspectorTreeHandler, PopupMenuHandler, const CommandEvent&, rCommandEvent, bool)
{
    if (rCommandEvent.GetCommand() != CommandEventId::ContextMenu)
        return false;

    auto xInterface = getSelectedXInterface(*mpObjectInspectorWidgets->mpPropertiesTreeView);
    if (xInterface.is())
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(
            mpObjectInspectorWidgets->mpPropertiesTreeView.get(), "sfx/ui/devtoolsmenu.ui"));
        std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu("inspect_menu"));

        OString sCommand(
            xMenu->popup_at_rect(mpObjectInspectorWidgets->mpPropertiesTreeView.get(),
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
        auto xInterface = getSelectedXInterface(*mpObjectInspectorWidgets->mpPropertiesTreeView);
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
        auto rPageId = mpObjectInspectorWidgets->mpNotebook->get_current_page_ident();
        NotebookEnterPage(rPageId);
    }
}

IMPL_LINK(ObjectInspectorTreeHandler, NotebookEnterPage, const OString&, rPageId, void)
{
    mpObjectInspectorWidgets->mpTextView->hide();

    uno::Any aAny = maInspectionStack.back();
    if (aAny.hasValue())
    {
        uno::Reference<uno::XInterface> xInterface(aAny, uno::UNO_QUERY);
        if (rPageId == "object_inspector_interfaces_tab")
        {
            mpObjectInspectorWidgets->mpInterfacesTreeView->freeze();
            clearAll(mpObjectInspectorWidgets->mpInterfacesTreeView);
            appendInterfaces(xInterface);
            mpObjectInspectorWidgets->mpInterfacesTreeView->thaw();
        }
        else if (rPageId == "object_inspector_services_tab")
        {
            mpObjectInspectorWidgets->mpServicesTreeView->freeze();
            clearAll(mpObjectInspectorWidgets->mpServicesTreeView);
            appendServices(xInterface);
            mpObjectInspectorWidgets->mpServicesTreeView->thaw();
        }
        else if (rPageId == "object_inspector_properties_tab")
        {
            mbPanedResetSize = true;
            mpObjectInspectorWidgets->mpPropertiesTreeView->freeze();
            clearAll(mpObjectInspectorWidgets->mpPropertiesTreeView);
            appendProperties(xInterface);
            mpObjectInspectorWidgets->mpPropertiesTreeView->thaw();
            mpObjectInspectorWidgets->mpTextView->show();
        }
        else if (rPageId == "object_inspector_methods_tab")
        {
            mpObjectInspectorWidgets->mpMethodsTreeView->freeze();
            clearAll(mpObjectInspectorWidgets->mpMethodsTreeView);
            appendMethods(xInterface);
            mpObjectInspectorWidgets->mpMethodsTreeView->thaw();
        }
    }
}

IMPL_LINK(ObjectInspectorTreeHandler, NotebookLeavePage, const OString&, rPageId, bool)
{
    if (rPageId == "object_inspector_interfaces_tab")
    {
        mpObjectInspectorWidgets->mpInterfacesTreeView->freeze();
        clearAll(mpObjectInspectorWidgets->mpInterfacesTreeView);
        mpObjectInspectorWidgets->mpInterfacesTreeView->thaw();
    }
    else if (rPageId == "object_inspector_services_tab")
    {
        mpObjectInspectorWidgets->mpServicesTreeView->freeze();
        clearAll(mpObjectInspectorWidgets->mpServicesTreeView);
        mpObjectInspectorWidgets->mpServicesTreeView->thaw();
    }
    else if (rPageId == "object_inspector_properties_tab")
    {
        mpObjectInspectorWidgets->mpPropertiesTreeView->freeze();
        clearAll(mpObjectInspectorWidgets->mpPropertiesTreeView);
        mpObjectInspectorWidgets->mpPropertiesTreeView->thaw();
    }
    else if (rPageId == "object_inspector_methods_tab")
    {
        mpObjectInspectorWidgets->mpMethodsTreeView->freeze();
        clearAll(mpObjectInspectorWidgets->mpMethodsTreeView);
        mpObjectInspectorWidgets->mpMethodsTreeView->thaw();
    }
    return true;
}

IMPL_LINK(ObjectInspectorTreeHandler, PanedSizeChange, const Size&, rSize, void)
{
    if (mbPanedResetSize)
    {
        // Set position at 90% of the height
        tools::Long nHeight = rSize.Height();
        mpObjectInspectorWidgets->mpPaned->set_position(nHeight * 0.9);
        mbPanedResetSize = false;
    }
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

/** Deletes all the node objects in a tree view */
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

/** Append interfaces to the "interfaces" tree view */
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
            auto xClass = convertTypeToIdlClass(xType, mxContext);
            lclAppendNode(mpObjectInspectorWidgets->mpInterfacesTreeView, new ClassNode(xClass));
        }
    }
}

/** Append services to the "services" tree view */
void ObjectInspectorTreeHandler::appendServices(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    auto xServiceInfo = uno::Reference<lang::XServiceInfo>(xInterface, uno::UNO_QUERY);
    const uno::Sequence<OUString> aServiceNames(xServiceInfo->getSupportedServiceNames());
    for (auto const& aServiceName : aServiceNames)
    {
        lclAppendNode(mpObjectInspectorWidgets->mpServicesTreeView,
                      new SimpleStringNode(aServiceName));
    }
}

/** Append properties to the "properties" tree view */
void ObjectInspectorTreeHandler::appendProperties(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;
    GenericPropertiesNode aNode("", uno::Any(xInterface), "", mxContext);
    aNode.fillChildren(mpObjectInspectorWidgets->mpPropertiesTreeView, nullptr);
}

/** Append methods to the "methods" tree view */
void ObjectInspectorTreeHandler::appendMethods(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    uno::Reference<beans::XIntrospection> xIntrospection = beans::theIntrospection::get(mxContext);
    auto xIntrospectionAccess = xIntrospection->inspect(uno::Any(xInterface));

    const auto xMethods = xIntrospectionAccess->getMethods(beans::MethodConcept::ALL);
    for (auto const& xMethod : xMethods)
    {
        lclAppendNode(mpObjectInspectorWidgets->mpMethodsTreeView, new MethodNode(xMethod));
    }
}

// Update the back button state depending if there are objects in the stack
void ObjectInspectorTreeHandler::updateBackButtonState()
{
    mpObjectInspectorWidgets->mpToolbar->set_item_sensitive("back", maInspectionStack.size() > 1);
}

// Clears all the objects from the stack
void ObjectInspectorTreeHandler::clearStack()
{
    maInspectionStack.clear();
    updateBackButtonState();
}

// Adds an object to the stack
void ObjectInspectorTreeHandler::addToStack(css::uno::Any const& rAny)
{
    maInspectionStack.push_back(rAny);
    updateBackButtonState();
}

// Removes an object from the back of the stack and return it
css::uno::Any ObjectInspectorTreeHandler::popFromStack()
{
    maInspectionStack.pop_back();
    uno::Any aAny = maInspectionStack.back();
    updateBackButtonState();
    return aAny;
}

// Inspect the input object in the object inspector
void ObjectInspectorTreeHandler::inspectObject(uno::Reference<uno::XInterface> const& xInterface)
{
    if (!xInterface.is())
        return;

    // Set implementation name
    OUString aImplementationName = getInterfaceImplementationClass(xInterface);
    mpObjectInspectorWidgets->mpClassNameLabel->set_label(aImplementationName);

    // Fire entering the current opened page manually
    auto rPageId = mpObjectInspectorWidgets->mpNotebook->get_current_page_ident();
    NotebookEnterPage(rPageId);
}

// Inspect the input object in the object inspector.
// Make the input object the root of the stack (clear all other
// objects from the stack).
void ObjectInspectorTreeHandler::introspect(uno::Reference<uno::XInterface> const& xInterface)
{
    clearStack();
    addToStack(uno::Any(xInterface));
    inspectObject(xInterface);
}

void ObjectInspectorTreeHandler::dispose()
{
    // We need to clear all the nodes
    clearAll(mpObjectInspectorWidgets->mpInterfacesTreeView);
    clearAll(mpObjectInspectorWidgets->mpServicesTreeView);
    clearAll(mpObjectInspectorWidgets->mpPropertiesTreeView);
    clearAll(mpObjectInspectorWidgets->mpMethodsTreeView);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
