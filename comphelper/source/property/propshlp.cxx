

#include <comphelper/propshlp2.hxx>
#include <comphelper/sequence.hxx>

namespace
{
struct PropertyNameLess
{
    bool operator()(const css::beans::Property& first, const css::beans::Property& second)
    {
        return first.Name.compareTo(second.Name) < 0;
    }
};
}

namespace comphelper
{
OPropertyArrayHelper2::OPropertyArrayHelper2(std::vector<css::beans::Property> aProps)
    : mbRightOrdered(false)
{
    std::sort(aProps.begin(), aProps.end(), PropertyNameLess());
    maInfos = comphelper::containerToSequence(aProps);
    for (auto const& rProp : aProps)
        maInfosMap.emplace(rProp.Name, rProp);
    sal_Int32 nElements = maInfos.getLength();
    for (sal_Int32 i = 0; i < nElements; i++)
        if (maInfos[i].Handle != i)
            return;
    // The handle is the index
    mbRightOrdered = true;
}

OPropertyArrayHelper2::OPropertyArrayHelper2(const css::uno::Sequence<css::beans::Property>& aProps)
    : maInfos(aProps)
    , mbRightOrdered(false)
{
    for (auto const& rProp : aProps)
        maInfosMap.emplace(rProp.Name, rProp);
    sal_Int32 nElements = maInfos.getLength();
    for (sal_Int32 i = 0; i < nElements; i++)
        if (maInfos[i].Handle != i)
            return;
    // The handle is the index
    mbRightOrdered = true;
}

sal_Bool OPropertyArrayHelper2::fillPropertyMembersByHandle(OUString* pPropName,
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

css::uno::Sequence<css::beans::Property> OPropertyArrayHelper2::getProperties() { return maInfos; }

css::beans::Property OPropertyArrayHelper2::getPropertyByName(const OUString& aPropertyName)
{
    auto it = maInfosMap.find(aPropertyName);
    if (it == maInfosMap.end())
        throw css::beans::UnknownPropertyException(aPropertyName);
    return it->second;
}

sal_Bool OPropertyArrayHelper2::hasPropertyByName(const OUString& aPropertyName)
{
    auto it = maInfosMap.find(aPropertyName);
    return it != maInfosMap.end();
}

sal_Int32 OPropertyArrayHelper2::getHandleByName(const OUString& rPropName)
{
    auto it = maInfosMap.find(rPropName);
    if (it != maInfosMap.end())
        return it->second.Handle;
    return -1;
}

sal_Int32 OPropertyArrayHelper2::fillHandles(sal_Int32* pHandles,
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
