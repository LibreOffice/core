

#include <comphelper/propshlp.hxx>
#include <comphelper/sequence.hxx>

namespace comphelper
{
OPropertyArrayHelper::OPropertyArrayHelper(const std::vector<css::beans::Property>& aProps)
    : maInfos(comphelper::containerToSequence(aProps))
    , mbRightOrdered(false)
{
    sal_Int32 i, nElements = maInfos.size();

    for (i = 1; i < nElements; i++)
    {
        if (maInfos[i - 1].Name > maInfos[i].Name)
            assert(false && "Property array is not sorted");
    }
    for (auto const& rProp : aProps)
        maInfosMap.emplace(rProp.Name, rProp);
    for (i = 0; i < nElements; i++)
        if (maInfos[i].Handle != i)
            return;
    // The handle is the index
    mbRightOrdered = true;
}

OPropertyArrayHelper::OPropertyArrayHelper(const css::uno::Sequence<css::beans::Property>& aProps)
    : maInfos(aProps)
    , mbRightOrdered(false)
{
    sal_Int32 i, nElements = maInfos.getLength();

    for (i = 1; i < nElements; i++)
    {
        if (maInfos[i - 1].Name > maInfos[i].Name)
            assert(false && "Property array is not sorted");
    }
    for (auto const& rProp : aProps)
        maInfosMap.emplace(rProp.Name, rProp);
    for (i = 0; i < nElements; i++)
        if (maInfos[i].Handle != i)
            return;
    // The handle is the index
    mbRightOrdered = true;
}

sal_Bool OPropertyArrayHelper::fillPropertyMembersByHandle(OUString* pPropName,
                                                           sal_Int16* pAttributes,
                                                           sal_Int32 nHandle)
{
    sal_Int32 nElements = maInfos.size();

    if (mbRightOrdered)
    {
        if (nHandle < 0 || nHandle >= nElements)
            return false;
        if (pPropName)
            *pPropName = maInfos[nHandle].Name;
        if (pAttributes)
            *pAttributes = maInfos[nHandle].Attributes;
        return true;
    }
    // normally the array is sorted
    for (sal_Int32 i = 0; i < nElements; i++)
    {
        if (maInfos[i].Handle == nHandle)
        {
            if (pPropName)
                *pPropName = maInfos[i].Name;
            if (pAttributes)
                *pAttributes = maInfos[i].Attributes;
            return true;
        }
    }
    return false;
}

css::uno::Sequence<css::beans::Property> OPropertyArrayHelper::getProperties() { return maInfos; }

css::beans::Property OPropertyArrayHelper::getPropertyByName(const OUString& aPropertyName)
{
    auto it = maInfosMap.find(aPropertyName);
    if (it == maInfosMap.end())
        throw css::beans::UnknownPropertyException(aPropertyName);
    return it->second;
}

sal_Bool OPropertyArrayHelper::hasPropertyByName(const OUString& aPropertyName)
{
    auto it = maInfosMap.find(aPropertyName);
    return it != maInfosMap.end();
}

sal_Int32 OPropertyArrayHelper::getHandleByName(const OUString& rPropName)
{
    auto it = maInfosMap.find(rPropName);
    if (it != maInfosMap.end())
        return it->second.Handle;
    return -1;
}

sal_Int32 OPropertyArrayHelper::fillHandles(sal_Int32* pHandles,
                                            const css::uno::Sequence<OUString>& rPropNames)
{
    sal_Int32 nHitCount = 0;
    const OUString* pReqProps = rPropNames.getConstArray();
    sal_Int32 nReqLen = rPropNames.getLength();

    for (sal_Int32 i = 0; i < nReqLen; i++)
    {
        auto it = maInfosMap.find(pReqProps[i]);
        if (it == maInfosMap.end())
            pHandles[i] = -1;
        else
        {
            pHandles[i] = it->second.Handle;
            nHitCount++;
        }
    }
    return nHitCount;
}

} // namespace comphelper
